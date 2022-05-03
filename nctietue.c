#include <netcdf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "nctietue.h"

const char* error_color   = "\033[1;31m";
const char* varset_color  = "\033[1;93m";
const char* varname_color = "\033[92m";
const char* type_color    = "\033[35m";
const char* default_color = "\033[0m";

int ncret;
#define NCERROR(arg) printf("%sNetcdf-error: %s%s\n", error_color, default_color, nc_strerror(arg))
#define NCFUNK(fun, ...)		\
  do {					\
    if((ncret = fun(__VA_ARGS__))) {	\
      NCERROR(ncret);			\
      asm("int $3");			\
    }					\
  } while(0)

/*printfunctions for all nct_var types*/
#define ONE_TYPE(nctype, form, ctype)			\
  void print_##nctype(void* arr, int i, int end) {	\
    for(; i<end; i++)					\
      printf("%"#form", ", ((ctype*)arr)[i]);		\
  }
ALL_TYPES
#undef ONE_TYPE

#define ONE_TYPE(nctype, ...) [nctype]=print_##nctype, //array of printfunctions
void (*printfunctions[])(void*, int, int) =
  {
   ALL_TYPES
  };
#undef ONE_TYPE

#define ONE_TYPE(nctype, form, ctype) [nctype]=#ctype, //array of type names
char* type_names[] =
  {
   ALL_TYPES
  };
#undef ONE_TYPE

void print_nct_var_data(nct_var* var) {
  void (*printfun)(void*,int,int) = printfunctions[var->xtype];
  if(var->len <= 17) {
    printfun(var->data, 0, var->len);
    return;
  }
  printfun(var->data, 0, 8);
  printf(" ..., ");
  printfun(var->data, var->len-8, var->len);
}

void print_nct_var(nct_var* var, const char* indent) {
  printf("%s%s%s %s%s(%zu)%s%s:\n%s  %i dims: ( ",
	 indent, type_color, type_names[var->xtype],
	 varname_color, var->name, var->len, default_color, var->iscoordinate? " (coordinate)": "",
	 indent, var->ndims);
  for(int i=0; i<var->ndims; i++)
    printf("%s(%zu), ", var->dimnames[i], var->dimlens[i]);
  printf(")\n");
  printf("%s  [", indent);
  print_nct_var_data(var);
  puts("]");
}

void print_nct_vset(nct_vset* vs) {
  printf("%s%i nct_vars, %i dims%s\n", varset_color, vs->nvars, vs->ndims, default_color);
  for(int i=0; i<vs->nvars; i++)
    print_nct_var(vs->vars+i, "  ");
}

nct_var* var_from_vset(nct_vset* vs, char* name) {
  for(int i=0; i<vs->nvars; i++)
    if(!strcmp(vs->vars[i].name, name))
      return vs->vars+i;
  return NULL;
}

/*–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
 * define all oper-eq-functions for all types
 * using operations_and_types.h which defines all their combinations
 *––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––*/

#define OPERATION(nctype, form, ctype, opername, oper)			\
  nct_var* vararr_##opername##_##nctype(nct_var* v0, void* arr) {	\
    for(size_t i=0; i<v0->len; i++)					\
      ((ctype*)v0->data)[i] oper ((ctype*)arr)[i];			\
    return v0;								\
  }
#include "operations_and_types.h"
#undef OPERATION

/*Define arrays of the functions. NC_STRING has greatest index.
  A separate init-function is needed to put function pointers into the arrays.*/
#define ONE_OPERATION(opername, a) nct_var* (*vararr_##opername##_functions[NC_STRING])(nct_var* v0, void* arr);
ALL_OPERATIONS
#undef ONE_OPERATION

#define ONE_OPERATION(opername, oper)				\
  nct_var* vararr_##opername(nct_var* v0, void* arr) {		\
    return vararr_##opername##_functions[v0->xtype](v0, arr);	\
  }
ALL_OPERATIONS
#undef ONE_OPERATION

#define ONE_OPERATION(opername, oper)			\
  nct_var* vararrs_##opername(nct_var* var, ...) {	\
  va_list ptr;						\
  int va_len = 4;					\
  int i=0;						\
  while(1) {						\
    va_start(ptr, var);					\
    for(int _i=0; _i<i; _i++)				\
      va_arg(ptr, void*);				\
    for(; i<va_len; i++) {				\
      void* v1 = va_arg(ptr, void*);			\
      if(!v1)						\
	goto FINISHED;					\
      vararr_##opername(var, v1);			\
    }							\
    va_end(ptr);					\
    va_len *= 2;					\
  }							\
FINISHED:						\
 va_end(ptr);						\
 return var;						\
 }
ALL_OPERATIONS
#undef ONE_OPERATION

#define OPERATION(nctype, a, b, opername, c) vararr_##opername##_functions[nctype] = vararr_##opername##_##nctype;
void nct_init() {
#include "operations_and_types.h"
}
#undef OPERATION

/*–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
 * Reading and writing data
 *––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––*/

nct_dim* read_nct_dim_gd(nct_dim* dest, int ncid, int dimid) {
  char name[256];
  size_t* len_p = &dest->len;
  nc_inq_dim(ncid, dimid, name, len_p);
  dest->name = strdup(name);
  return dest;
}
nct_dim* read_nct_dim(int ncid, int dimid) {
  return read_nct_dim_gd(calloc(1, sizeof(nct_dim)), ncid, dimid);
}

nct_var* read_nct_var_gd(nct_var* dest, int ncid, int varid, nct_dim* dims) {
  int ndims, dimids[128];
  nc_type xtype;
  char name[256];
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
nct_var* read_nct_var(int ncid, int varid, nct_dim* dims) {
  return read_nct_var_gd(calloc(1,sizeof(nct_var)), ncid, varid, dims);
}

nct_vset* read_ncfile_gd(nct_vset* dest, const char* restrict filename) {
  int ncid, id;
  NCFUNK(nc_open, filename, NC_NOWRITE, &ncid);
  NCFUNK(nc_inq_ndims, ncid, &(dest->ndims));
  NCFUNK(nc_inq_nvars, ncid, &(dest->nvars));
  dest->dims = calloc(dest->ndims, sizeof(nct_dim));
  dest->vars = calloc(dest->nvars, sizeof(nct_var));
  dest->dimnames = calloc(dest->ndims, sizeof(char*));
  dest->varnames = calloc(dest->nvars, sizeof(char*));
  dest->dimlens = calloc(dest->ndims, sizeof(size_t));
  for(int i=0; i<dest->ndims; i++) //read dims
    read_nct_dim_gd(dest->dims+i, ncid, i);
  for(int i=0; i<dest->nvars; i++) //read nct_vars
    read_nct_var_gd(dest->vars+i, ncid, i, dest->dims);
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
nct_vset* read_ncfile(const char* restrict filename) {
  return read_ncfile_gd(calloc(1,sizeof(nct_vset)), filename);
}

void link_dims_to_coords(nct_vset* dest) {
  for(int i=0; i<dest->ndims; i++)
    for(int j=0; j<dest->nvars; j++)
      if(!strcmp(dest->dims[i].name, dest->vars[j].name)) {
	dest->dims[i].coordv = dest->vars+j;
	dest->vars[j].iscoordinate = 1;
	break;
      }
}

void link_nct_vars_to_dimnames(nct_vset* vs) {
  for(int v=0; v<vs->nvars; v++) {
    nct_var* var = vs->vars+v;
    for(int d=0; d<var->ndims; d++)
      var->dimnames[d] = vs->dims[var->dimids[d]].name;
  }
}

/*does not copy the nct_var that src may point to*/
nct_dim* nct_dimcpy_gd(nct_dim* dest, const nct_dim* src) {
  dest->name = strdup(src->name);
  dest->len = src->len;
  //don't change dest->coordv
  return dest;
}
nct_dim* nct_dimcpy(const nct_dim* src) {
  nct_dim* dest = calloc(1, sizeof(nct_dim));
  return nct_dimcpy_gd(dest, src);
}

/*copies the dim and the var that src points to*/
nct_dim* nct_coordcpy_gd(nct_dim* dest, const nct_dim* src) {
  nct_dimcpy_gd(dest, src);
  dest->coordv = nct_varcpy(src->coordv);
  return dest;
}
nct_dim* nct_coordcpy(const nct_dim* src) {
  nct_dim* dest = malloc(sizeof(nct_dim));
  return nct_coordcpy_gd(dest, src);
}

/*does not change which nct_dim names are pointed to*/
nct_var* nct_varcpy_gd(nct_var* dest, const nct_var* src) {
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
nct_var* nct_varcpy(const nct_var* src) {
  nct_var* dest = malloc(sizeof(nct_var));
  return nct_varcpy_gd(dest, src);
}

nct_vset* nct_vsetcpy_gd(nct_vset* dest, const nct_vset* src) {
  /*dims*/
  dest->ndims = src->ndims;
  dest->dims = malloc(dest->ndims*sizeof(nct_dim));
  dest->dimnames = malloc(dest->ndims*sizeof(char*));
  for(int i=0; i<dest->ndims; i++) {
    nct_dimcpy_gd(dest->dims+i, src->dims+i);
    dest->dimnames[i] = dest->dims[i].name;
  }
  dest->dimlens = malloc(dest->ndims*sizeof(size_t));
  memcpy(dest->dimlens, src->dimlens, dest->ndims*sizeof(size_t));
  /*vars*/
  dest->nvars = src->nvars;
  dest->vars = malloc(dest->nvars*sizeof(nct_var));
  dest->varnames = malloc(dest->nvars*sizeof(char*));
  for(int i=0; i<dest->nvars; i++) {
    nct_varcpy_gd(dest->vars+i, src->vars+i);
    dest->varnames[i] = dest->vars[i].name;
  }
  link_dims_to_coords(dest);
  link_nct_vars_to_dimnames(dest);
  return dest;
}
nct_vset* nct_vsetcpy(const nct_vset* src) {
  nct_vset* dest = malloc(sizeof(nct_vset));
  return nct_vsetcpy_gd(dest, src);
}

void free_nct_dim(nct_dim* dim) {
  free(dim->name);
  dim->name = NULL;
}

void free_nct_var(nct_var* var) {
  free(var->dimnames);
  if(var->xtype == NC_STRING)
    for(int i=0; i<var->len; i++)
      free(((char**)var->data)[i]);
  free(var->data); var->data = NULL;
  free(var->name); var->name = NULL;
}

void free_nct_coord(nct_dim* coord) {
  *(intptr_t*)(&coord->coordv->name) *= coord->coordv->name!=coord->name; //branchlessly: if(equal) other=NULL
  free_nct_dim(coord);
  free_nct_var(coord->coordv);
  free(coord->coordv);
}

void free_nct_vset(nct_vset* vs) {
  for(int i=0; i<vs->ndims; i++)
    free_nct_dim(vs->dims+i);
  for(int i=0; i<vs->nvars; i++)
    free_nct_var(vs->vars+i);
  free(vs->dims);
  free(vs->vars);
  free(vs->dimnames);
  free(vs->varnames);
  free(vs->dimlens);
}

nct_var* to_nct_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name) {
  dest->name = name;
  dest->len = len;
  dest->size1 = nctypelen(xtype);
  dest->xtype = xtype;
  dest->data = arr;
  return dest;
}
nct_var* to_nct_var(void* arr, size_t len, nc_type xtype, char* name) {
    nct_var* dest = calloc(1, sizeof(nct_var));
    return to_nct_var_gd(dest, arr, len, xtype, name);
}

nct_var* copy_to_nct_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name) {
  dest->name = name;
  dest->len = len;
  dest->size1 = nctypelen(xtype);
  dest->xtype = xtype;
  dest->data = malloc(len*dest->size1);
  memcpy(dest->data, arr, len*dest->size1);
  return dest;
}
nct_var* copy_to_nct_var(void* arr, size_t len, nc_type xtype, char* name) {
  nct_var* dest = calloc(1, sizeof(nct_var));
  return copy_to_nct_var_gd(dest, arr, len, xtype, name);
}

nct_dim* to_nct_coord_gd(nct_dim* dest, void* arr, size_t len, nc_type xtype, char* name) {
  dest->coordv = calloc(1, sizeof(nct_var));
  dest->name = name;
  dest->len = len;
  nct_var* destv = dest->coordv;
  destv->name = name;
  destv->iscoordinate = 1;
  destv->ndims = 1;
  destv->dimnames = malloc(sizeof(char*)+sizeof(size_t)+sizeof(int));
  *destv->dimnames = dest->name;
  destv->dimlens = (size_t*)(destv->dimnames+1);
  *destv->dimlens = dest->len;
  destv->dimids = (int*)(destv->dimlens+1);
  *destv->dimids = 0;
  destv->len = len;
  destv->size1 = nctypelen(xtype);
  destv->xtype = xtype;
  destv->data = arr;
  return dest;
}
nct_dim* to_nct_coord(void* arr, size_t len, nc_type xtype, char* name) {
  nct_dim* dest = calloc(1, sizeof(nct_dim));
  to_nct_coord_gd(dest, arr, len, xtype, name);
}
