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

/*With this macro one can define functions for all variable types without repeating things.
  First define _SET in a wanted way, then add _TYPES then undef _SET
  Functions can be further added into an array of function pointers with the same syntax
  which allows use of nc_type (aka int) variable as an index to access the right function.
  Reading the code will make this clearer.*/
#define _TYPES					\
  _SET(NC_BYTE, hhi, char)			\
  _SET(NC_UBYTE, hhu, unsigned char)		\
  _SET(NC_CHAR, c, char)			\
  _SET(NC_SHORT, hi, short)			\
  _SET(NC_USHORT, hu, unsigned short)		\
  _SET(NC_INT, i, int)	         		\
  _SET(NC_UINT, u, unsigned)			\
  _SET(NC_UINT64, llu, long long unsigned)	\
  _SET(NC_INT64, lli, long long int)		\
  _SET(NC_FLOAT, .4f, float)			\
  _SET(NC_DOUBLE, .4lf, double)	        	\
  _SET(NC_STRING, s, char*)

/*print functions for all variable types*/
#define _SET(nctype, form, ctype)			\
  void print_##nctype(void* arr, int i, int end) {	\
    for(; i<end; i++)					\
      printf("%"#form", ", ((ctype*)arr)[i]);		\
  }
_TYPES
#undef _SET

/*set all print-function-pointers into an array using nc_type as index*/
#define _SET(nctype, ...) [nctype]=print_##nctype,
void (*printfunctions[])(void*, int, int) =
  {
   _TYPES
  };
#undef _SET

#define _SET(nctype, form, ctype) [nctype]=#ctype,
char* type_names[] =
  {
   _TYPES
  };
#undef _SET
#undef _TYPES

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

void print_variable_data(variable* var) {
  void (*printfun)(void*,int) = printfunctions[var->xtype];
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
  dest->dimnames = malloc(ndims*sizeof(char*));
  dest->dimlens = malloc(ndims*sizeof(size_t));
  dest->len = 1;
  for(int i=0; i<ndims; i++) {
    dest->dimnames[i] = dims[dimids[i]].name;
    dest->dimlens[i] = dims[dimids[i]].len;
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
  for(int i=0; i<dest->ndims; i++) //link dimensions to corresponding variables
    for(int j=0; j<dest->nvars; j++)
      if(!strcmp(dest->dims[i].name, dest->vars[j].name)) {
	dest->dims[i].coord = dest->vars+j;
	dest->vars[j].iscoordinate = 1;
	break;
      }
  /*link names and length*/
  for(int i=0; i<dest->ndims; i++) {
    dest->dimnames[i] = dest->dims[i].name;
    dest->dimlens[i] = dest->dims[i].len;
  }
  for(int i=0; i<dest->nvars; i++)
    dest->varnames[i] = dest->vars[i].name;
  return dest;
}

void free_dimension(dimension* dim) {
  free(dim->name);
  dim->name = NULL;
}

void free_variable(variable* var) {
  free(var->dimnames);
  free(var->dimlens);
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
