#include <netcdf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include "nctietue.h"

const char* error_color   = "\033[1;31m";
const char* varset_color  = "\033[1;93m";
const char* varname_color = "\033[92m";
const char* type_color    = "\033[35m";
const char* default_color = "\033[0m";

int ncret;
#define NCERROR(arg) printf("%sNetcdf-error:%s %s\n", error_color, nc_strerror(arg), default_color)
#define NCFUNK(fun, ...)		\
  do {					\
    if((ncret = fun(__VA_ARGS__))) {	\
      NCERROR(ncret);			\
      asm("int $3");			\
    }					\
  } while(0)

/*printfunctions for all variable types*/
#define ONE_TYPE(nctype, form, ctype)			\
  void print_##nctype(void* arr, int i, int end) {	\
    for(; i<end; i++)					\
      printf("%"#form", ", ((ctype*)arr)[i]);		\
  }
APPLY_FOR_ALL_TYPES
#undef ONE_TYPE

#define ONE_TYPE(nctype, ...) [nctype]=print_##nctype, //array of printfunctions
void (*printfunctions[])(void*, int, int) =
  {
   APPLY_FOR_ALL_TYPES
  };
#undef ONE_TYPE

#define ONE_TYPE(nctype, form, ctype) [nctype]=#ctype, //array of type names
char* type_names[] =
  {
   APPLY_FOR_ALL_TYPES
  };
#undef ONE_TYPE

void print_variable_data(variable* var) {
  void (*printfun)(void*,int,int) = printfunctions[var->xtype];
  if(var->len <= 17) {
    printfun(var->data, 0, var->len);
    return;
  }
  printfun(var->data, 0, 8);
  printf(" ..., ");
  printfun(var->data, var->len-8, var->len);
}

void print_variable(variable* var, const char* indent) {
  printf("%s%s%s %s%s(%zu)%s%s:\n%s  %i dims: ( ",
	 indent, type_color, type_names[var->xtype],
	 varname_color, var->name, var->len, default_color, var->iscoordinate? " (coordinate)": "",
	 indent, var->ndims);
  for(int i=0; i<var->ndims; i++)
    printf("%s(%zu), ", var->dimnames[i], var->dimlens[i]);
  printf(")\n");
  printf("%s  [", indent);
  print_variable_data(var);
  puts("]");
}

void print_variable_set(variable_set* vs) {
  printf("%s%i variables, %i dimensions%s\n", varset_color, vs->nvars, vs->ndims, default_color);
  for(int i=0; i<vs->nvars; i++)
    print_variable(vs->vars+i, "  ");
}

variable* var_from_vset(variable_set* vs, char* name) {
  for(int i=0; i<vs->nvars; i++)
    if(!strcmp(vs->vars[i].name, name))
      return vs->vars+i;
  return NULL;
}

/*–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
 * define pluseq-functions for all types
 *––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––*/

#define ONE_TYPE(nctype, form, ctype)					\
  variable* var_pluseq_var_##nctype(variable* v0, variable* v1)	{	\
    for(size_t i=0; i<v0->len; i++)					\
      ((ctype*)v0->data)[i] += ((ctype*)v1->data)[i];			\
    return v0;								\
  }
ALL_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, form, ctype) [nctype]=var_pluseq_var_##nctype, //array of pluseq functions
variable* (*var_pluseq_var_functions[])(variable* v0, variable* v1) =
  {
   ALL_EXCEPT_STRING
  };
#undef onetype

variable* var_pluseq_var(variable* v0, variable* v1) {
  return var_pluseq_var_functions[v0->xtype](v0, v1);
}

variable* var_pluseq_vars(variable* var, ...) {
  va_list ptr;
  int va_len = 4;
  int i=0;
  while(1) {
    va_start(ptr, var);
    for(int _i=0; _i<i; _i++)
      va_arg(ptr, void*);
    for(; i<va_len; i++) {
      variable* v1 = va_arg(ptr, void*);
      if(!v1)
	goto FINISHED;
      var_pluseq_var(var, v1);
    }
    va_end(ptr);
    va_len *= 2;
  }
 FINISHED:
  va_end(ptr);
  return var;
}

/*–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
 * Reading and writing data
 *––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––*/

dimension* read_ncdim(int ncid, int dimid, dimension* dest) {
  char name[256];
  if(!dest)
    dest = calloc(1, sizeof(dimension));
  size_t* len_p = &dest->len;
  nc_inq_dim(ncid, dimid, name, len_p);
  dest->name = strdup(name);
  return dest;
}

variable* read_ncvariable(int ncid, int varid, dimension* dims, variable* dest) {
  int ndims, dimids[128];
  nc_type xtype;
  char name[256];
  if(!dest)
    dest = calloc(1, sizeof(variable));
  NCFUNK(nc_inq_var, ncid, varid, name, &xtype, &ndims, dimids, NULL);
  dest->name = strdup(name);
  dest->ndims = ndims;
  dest->dimnames = malloc(ndims * (sizeof(char*) + sizeof(size_t) + sizeof(int)));
  dest->dimlens = (size_t*)(dest->dimnames + ndims);
  dest->dimids = (int*)(dest->dimlens + ndims);
  dest->len = 1;
  for(int i=0; i<ndims; i++) {
    dest->dimnames[i] = dims[dimids[i]].name;
    dest->dimlens[i] = dims[dimids[i]].len;
    dest->dimids[i] = dimids[i];
    dest->len *= dims[dimids[i]].len;
  }
  dest->xtype = xtype;
  dest->size1 = nctypelen(xtype);
  dest->data = malloc(dest->len*dest->size1);
  NCFUNK(nc_get_var, ncid, varid, dest->data);
  return dest;
}

variable_set* read_ncfile(const char* restrict filename, variable_set* dest) {
  int ncid, id;
  if(!dest)
    dest = calloc(1, sizeof(variable_set));
  NCFUNK(nc_open, filename, NC_NOWRITE, &ncid);
  NCFUNK(nc_inq_ndims, ncid, &(dest->ndims));
  NCFUNK(nc_inq_nvars, ncid, &(dest->nvars));
  dest->dims = calloc(dest->ndims, sizeof(dimension));
  dest->vars = calloc(dest->nvars, sizeof(variable));
  dest->dimnames = calloc(dest->ndims, sizeof(char*));
  dest->varnames = calloc(dest->nvars, sizeof(char*));
  dest->dimlens = calloc(dest->ndims, sizeof(size_t));
  for(int i=0; i<dest->ndims; i++) //read dimensionss
    read_ncdim(ncid, i, dest->dims+i);
  for(int i=0; i<dest->nvars; i++) //read variables
    read_ncvariable(ncid, i, dest->dims, dest->vars+i);
  NCFUNK(nc_close, ncid);
  link_dims_to_coords(dest);
  /*link names and write lengths*/
  for(int i=0; i<dest->ndims; i++) {
    dest->dimnames[i] = dest->dims[i].name;
    dest->dimlens[i] = dest->dims[i].len;
  }
  for(int i=0; i<dest->nvars; i++)
    dest->varnames[i] = dest->vars[i].name;
  return dest;
}

void link_dims_to_coords(variable_set* dest) {
  for(int i=0; i<dest->ndims; i++)
    for(int j=0; j<dest->nvars; j++)
      if(!strcmp(dest->dims[i].name, dest->vars[j].name)) {
	dest->dims[i].coord = dest->vars+j;
	dest->vars[j].iscoordinate = 1;
	break;
      }
}

void link_variables_to_dimnames(variable_set* vs) {
  for(int v=0; v<vs->nvars; v++) {
    variable* var = vs->vars+v;
    for(int d=0; d<var->ndims; d++)
      var->dimnames[d] = vs->dims[var->dimids[d]].name;
  }
}

/*does not copy the variable that src may point to*/
dimension* dimcpy(dimension* dest, const dimension* src) {
  if(!dest)
    dest = malloc(sizeof(dimension));
  *dest = *src;
  dest->name = strdup(src->name);
  return dest;
}

/*does not change which dimension names are pointed to*/
variable* varcpy(variable* dest, const variable* src) {
  if(!dest)
    dest = malloc(sizeof(variable));
  *dest = *src;
  dest->name = strdup(src->name);
  size_t len = src->ndims * (sizeof(char*) + sizeof(size_t) + sizeof(int));
  dest->dimnames = malloc(len);
  dest->dimlens = (size_t*)(dest->dimnames + dest->ndims);
  dest->dimids = (int*)(dest->dimlens + dest->ndims);
  memcpy(dest->dimnames, src->dimnames, len);
  dest->data = malloc(src->len*src->size1);
  if(dest->xtype == NC_STRING)
    for(size_t i=0; i<dest->len; i++)
      ((char**)dest->data)[i] = strdup(((char**)src->data)[i]);
  else
    memcpy(dest->data, src->data, src->len*src->size1);
  return dest;
}

variable_set* vsetcpy(variable_set* dest, const variable_set* src) {
  if(!dest)
    dest = malloc(sizeof(variable_set));
  /*dims*/
  dest->ndims = src->ndims;
  dest->dims = malloc(dest->ndims*sizeof(dimension));
  dest->dimnames = malloc(dest->ndims*sizeof(char*));
  for(int i=0; i<dest->ndims; i++) {
    dimcpy(dest->dims+i, src->dims+i);
    dest->dimnames[i] = dest->dims[i].name;
  }
  dest->dimlens = malloc(dest->ndims*sizeof(size_t));
  memcpy(dest->dimlens, src->dimlens, dest->ndims*sizeof(size_t));
  /*vars*/
  dest->nvars = src->nvars;
  dest->vars = malloc(dest->nvars*sizeof(variable));
  dest->varnames = malloc(dest->nvars*sizeof(char*));
  for(int i=0; i<dest->nvars; i++) {
    varcpy(dest->vars+i, src->vars+i);
    dest->varnames[i] = dest->vars[i].name;
  }
  link_dims_to_coords(dest);
  link_variables_to_dimnames(dest);
  return dest;
}

void free_dimension(dimension* dim) {
  free(dim->name);
  dim->name = NULL;
}

void free_variable(variable* var) {
  free(var->dimnames);
  if(var->xtype == NC_STRING)
    for(int i=0; i<var->len; i++)
      free(((char**)var->data)[i]);
  free(var->data); var->data = NULL;
  free(var->name); var->name = NULL;
}

void free_variable_set(variable_set* vs) {
  for(int i=0; i<vs->ndims; i++)
    free_dimension(vs->dims+i);
  for(int i=0; i<vs->nvars; i++)
    free_variable(vs->vars+i);
  free(vs->dims);
  free(vs->vars);
  free(vs->dimnames);
  free(vs->varnames);
  free(vs->dimlens);
}
