#include <netcdf.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
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
  dest->dims = malloc(dest->ndims * sizeof(dimension));
  dest->vars = malloc(dest->nvars * sizeof(variable));
  dest->dimnames = malloc(dest->ndims * sizeof(char*));
  dest->varnames = malloc(dest->nvars * sizeof(char*));
  dest->dimlens = malloc(dest->ndims * sizeof(size_t));
  for(int i=0; i<dest->ndims; i++)
    read_ncdim(ncid, i, dest->dims+i);
  for(int i=0; i<dest->nvars; i++)
    read_ncvariable(ncid, i, dest->dims, dest->vars+i);
  NCFUNK(nc_close, ncid);
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
  free(vs);
}

/*These macros are used to set array of functions pointers of form print_NC_BYTE(void* arr, int n)
  nc_type value can be then be used to get the right function pointer from the array.*/
#define _TYPES					\
  _SET(NC_BYTE, hhi, char)			\
    _SET(NC_UBYTE, hhu, unsigned char)		\
    _SET(NC_CHAR, c, char)			\
    _SET(NC_SHORT, hi, short)			\
    _SET(NC_USHORT, hu, unsigned short)		\
    _SET(NC_INT, i, int)			\
    _SET(NC_UINT, u, unsigned)			\
    _SET(NC_UINT64, llu, long long unsigned)	\
    _SET(NC_INT64, lli, long long int)		\
    _SET(NC_FLOAT, .4f, float)			\
    _SET(NC_DOUBLE, .4lf, double)		\
    _SET(NC_STRING, s, char*)
#define _SET(nctype, form, ctype)		\
  void print_##nctype(void* arr, int i) {	\
    printf("%"#form", ", ((ctype*)arr)[i]);	\
  }
  _TYPES
#undef _SET
#define _SET(nctype, ...) [nctype]=print_##nctype,
  void (*printfunctions[])(void*, int) =
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

void print_variable_data(variable* var) {
  void (*printfun)(void*,int) = printfunctions[var->xtype];
  if(var->len <= 16) {
    for(int i=0; i<var->len; i++)
      printfun(var->data, i);
    return;
  }
  for(int i=0; i<8; i++)
    printfun(var->data, i);
  printf(" ..., ");
  for(int i=var->len-8; i<var->len; i++)
    printfun(var->data, i);
}

void print_variable(variable* var, const char* indent) {
  printf("%s%s%s %s%s(%zu):%s\n"
	 "%s  %i dims: ( ",
	 indent, type_color, type_names[var->xtype], varname_color, var->name, var->len, default_color,
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
