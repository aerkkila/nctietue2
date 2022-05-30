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
  void nct_print_##nctype(void* arr, int i, int end) {	\
    for(; i<end; i++)					\
      printf("%"#form", ", ((ctype*)arr)[i]);		\
  }
ALL_TYPES
#undef ONE_TYPE

#define ONE_TYPE(nctype, ...) [nctype]=nct_print_##nctype, //array of printfunctions
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

void nct_print_var_data(nct_var* var) {
  void (*printfun)(void*,int,int) = printfunctions[var->xtype];
  if(var->len <= 17) {
    printfun(var->data, 0, var->len);
    return;
  }
  printfun(var->data, 0, 8);
  printf(" ..., ");
  printfun(var->data, var->len-8, var->len);
}

void nct_print_var(nct_var* var, const char* indent) {
  printf("%s%s%s %s%s(%zu)%s%s:\n%s  %i dims: ( ",
	 indent, type_color, type_names[var->xtype],
	 varname_color, var->name, var->len, default_color, var->iscoordinate? " (coordinate)": "",
	 indent, var->ndims);
  for(int i=0; i<var->ndims; i++)
    printf("%s(%zu), ", var->dimnames[i], var->dimlens[i]);
  printf(")\n");
  printf("%s  [", indent);
  nct_print_var_data(var);
  puts("]");
}

void nct_print_vset(nct_vset* vs) {
  printf("%s%i vars, %i dims%s\n", varset_color, vs->nvars, vs->ndims, default_color);
  for(int i=0; i<vs->nvars; i++)
    nct_print_var(vs->vars+i, "  ");
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

#define OPERATION(nctype, a, ctype, opername, oper)			\
  nct_var* nct_var_##opername##_##nctype(nct_var* var, void* vvalue)	\
  {									\
    ctype value = *((ctype*)vvalue);					\
    for(size_t i=0; i<var->len; i++)					\
      ((ctype*)var->data)[i] oper value;				\
    return var;								\
  }
#include "operations_and_types.h"
#undef OPERATION

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
#define ONE_OPERATION(opername, a) nct_var* (*nct_var_##opername##_functions[NC_STRING])(nct_var* v0, void* value);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define ONE_OPERATION(opername, a) nct_var* (*vararr_##opername##_functions[NC_STRING])(nct_var* v0, void* arr);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define ONE_OPERATION(opername, oper)					\
  nct_var* nct_var_##opername(nct_var* v0, void* value) {		\
    return nct_var_##opername##_functions[v0->xtype](v0, value);	\
  }
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define ONE_OPERATION(opername, oper)				\
  nct_var* vararr_##opername(nct_var* v0, void* arr) {		\
    return vararr_##opername##_functions[v0->xtype](v0, arr);	\
  }
ALL_EQ_OPERATIONS
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
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define OPERATION(nctype, a, b, opername, c)				\
  vararr_##opername##_functions[nctype] = vararr_##opername##_##nctype;	\
  nct_var_##opername##_functions[nctype] = nct_var_##opername##_##nctype;
void nct_init() {
#include "operations_and_types.h"
}
#undef OPERATION

#define ONE_TYPE(nctype, b, ctype)					\
  void* nct_minmax_##nctype(nct_var* var, void* resultv)		\
  {									\
    ctype maxval, minval, *result=resultv;				\
    maxval = minval = ((ctype*)var->data)[0];				\
    for(int i=1; i<var->len; i++)					\
      if(maxval < ((ctype*)var->data)[i])				\
	maxval = ((ctype*)var->data)[i];				\
      else if(minval > ((ctype*)var->data)[i])				\
	minval = ((ctype*)var->data)[i];				\
    result[0] = minval;							\
    result[1] = maxval;							\
    return result;							\
  }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_minmax_##nctype,
void* (*minmax[])(nct_var*, void*) =
  {
    ALL_TYPES_EXCEPT_STRING
  };
#undef ONE_TYPE

void* nct_minmax(nct_var* var, void* resultv) {
  return minmax[var->xtype](var, resultv);
}

nct_var* nct_var_dropdim0(nct_var* var) {
  size_t new_len = var->len / var->dimlens[0];
  var->data = realloc(var->data, new_len*var->size1);
  var->len = new_len;
  void* ptr = var->dimnames;
  memmove(ptr, ptr+sizeof(char*), var->ndims*(sizeof(char*)+sizeof(size_t)+sizeof(int))-sizeof(char*));
  ptr += (var->ndims-1)*sizeof(char*);
  memmove(ptr, ptr+sizeof(size_t), var->ndims*(sizeof(size_t)+sizeof(int))-sizeof(size_t));
  ptr += (var->ndims-1)*sizeof(size_t);
  memmove(ptr, ptr+sizeof(int), var->ndims*sizeof(int));
  var->ndims--;
  var->dimnames = realloc(var->dimnames, var->ndims*(sizeof(char*)+sizeof(size_t)+sizeof(int)));
  var->dimlens = (size_t*)(var->dimnames+var->ndims);
  var->dimids = (int*)(var->dimlens+var->ndims);
  return var;
}

#define ONE_TYPE(nctype, a, ctype)					\
  nct_var* nct_varmean0_##nctype(nct_var* var)				\
  {									\
    size_t new_len = var->len / var->dimlens[0];			\
    for(size_t i=0; i<new_len; i++) {					\
      for(size_t j=1; j<var->dimlens[0]; j++)				\
	((ctype*)var->data)[i] += ((ctype*)var->data)[i+new_len*j];	\
      ((ctype*)var->data)[i] /= var->dimlens[0];			\
    }									\
    return nct_var_dropdim0(var);					\
  }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_varmean0_##nctype,
nct_var* (*varmean0[])(nct_var*) =
  {
    ALL_TYPES_EXCEPT_STRING
  };
#undef ONE_TYPE

nct_var* nct_varmean0(nct_var* var) {
  return varmean0[var->xtype](var);
}

#define ONE_TYPE(nctype, a, ctype)					\
  nct_var* nct_varnanmean0_##nctype(nct_var* var)			\
  {									\
    size_t new_len = var->len / var->dimlens[0];			\
    for(size_t i=0; i<new_len; i++) {					\
      int count = 1;							\
      for(size_t j=1; j<var->dimlens[0]; j++) {				\
	ctype test = ((ctype*)var->data)[i+new_len*j];			\
	if(test==test) {						\
	  count++;							\
	  ((ctype*)var->data)[i] += test;				\
	}								\
      }									\
      ((ctype*)var->data)[i] /= count;					\
    }									\
    return nct_var_dropdim0(var);					\
  }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_varnanmean0_##nctype,
nct_var* (*varnanmean0[])(nct_var*) =
  {
    ALL_TYPES_EXCEPT_STRING
  };
#undef ONE_TYPE

nct_var* nct_varnanmean0(nct_var* var) {
  return varnanmean0[var->xtype](var);
}

static nct_var* _nct_var_isel(nct_var* var, int dimid, size_t ind0, size_t ind1) {
  int id;
  for(int i=0; i<var->ndims; i++)
    if(var->dimids[i] == dimid) {
      id = i;
      goto FOUND;
    }
  return var;
 FOUND:;
  size_t len_after = var->size1; //interval to step given coordinate
  for(int i=id+1; i<var->ndims; i++)
    len_after *= var->dimlens[i];
  size_t length_around = len_after * var->dimlens[id];
  size_t n_blocks = 1;
  for(int i=0; i<id; i++)
    n_blocks *= var->dimlens[i];
  int blocklen = (ind1-ind0) * len_after;
  void* destp = var->data;
  void* srcp = var->data + len_after*ind0;
  for(int i=0; i<n_blocks; i++) {
    memmove(destp, srcp, blocklen);
    destp += blocklen;
    srcp += length_around;
  }
  var->data = realloc(var->data, blocklen*n_blocks);
  var->dimlens[id] = ind1-ind0;
  var->len = blocklen / var->size1 * n_blocks;
  return var;
}

nct_vset* nct_vset_isel(nct_vset* vset, int dimid, size_t ind0, size_t ind1) {
  for(int i=0; i<vset->nvars; i++)
    vset->vars[i] = *(_nct_var_isel(vset->vars+i, dimid, ind0, ind1));
  vset->dims[dimid].len = ind1-ind0;
  return vset;
}

int nct_get_dimid(nct_vset* vset, char* name) {
  for(int i=0; i<vset->ndims; i++)
    if(!strcmp(vset->dims[i].name, name))
      return i;
  return -1;
}

int nct_get_varid(nct_vset* vset, char* name) {
  for(int i=0; i<vset->nvars; i++)
    if(!strcmp(vset->vars[i].name, name))
      return i;
  return -1;
}

/*–––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––
 * Reading and writing data
 *––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––––*/

nct_dim* nct_read_dim_gd(nct_dim* dest, int ncid, int dimid) {
  char name[256];
  nc_inq_dim(ncid, dimid, name, &dest->len);
  dest->name = strdup(name);
  dest->freeable_name = 1;
  return dest;
}
nct_dim* nct_read_dim(int ncid, int dimid) {
  return nct_read_dim_gd(calloc(1, sizeof(nct_dim)), ncid, dimid);
}

nct_var* nct_read_var_gd(nct_var* dest, int ncid, int varid, nct_dim* dims) {
  int ndims, dimids[128];
  nc_type xtype;
  char name[256];
  NCFUNK(nc_inq_var, ncid, varid, name, &xtype, &ndims, dimids, NULL);
  dest->name = strdup(name);
  dest->freeable_name = 1;
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
nct_var* nct_read_var(int ncid, int varid, nct_dim* dims) {
  return nct_read_var_gd(calloc(1,sizeof(nct_var)), ncid, varid, dims);
}

nct_vset* nct_read_ncfile_gd(nct_vset* dest, const char* restrict filename) {
  int ncid;
  NCFUNK(nc_open, filename, NC_NOWRITE, &ncid);
  NCFUNK(nc_inq_ndims, ncid, &(dest->ndims));
  NCFUNK(nc_inq_nvars, ncid, &(dest->nvars));
  dest->dims = calloc(dest->ndims, sizeof(nct_dim));
  dest->vars = calloc(dest->nvars, sizeof(nct_var));
  for(int i=0; i<dest->ndims; i++) //read dims
    nct_read_dim_gd(dest->dims+i, ncid, i);
  for(int i=0; i<dest->nvars; i++) //read nct_vars
    nct_read_var_gd(dest->vars+i, ncid, i, dest->dims);
  NCFUNK(nc_close, ncid);
  nct_link_dims_to_coords(dest);
  return dest;
}
nct_vset* nct_read_ncfile(const char* restrict filename) {
  return nct_read_ncfile_gd(calloc(1,sizeof(nct_vset)), filename);
}

void nct_link_dims_to_coords(nct_vset* dest) {
  for(int i=0; i<dest->ndims; i++)
    for(int j=0; j<dest->nvars; j++)
      if(!strcmp(dest->dims[i].name, dest->vars[j].name)) {
	dest->dims[i].coordv = dest->vars+j;
	dest->vars[j].iscoordinate = 1;
	break;
      }
}

void nct_write_ncfile(const nct_vset* src, const char* name) {
  int ncid, id;
  NCFUNK(nc_create, name, NC_NETCDF4|NC_CLOBBER, &ncid);
  for(int i=0; i<src->ndims; i++)
    NCFUNK(nc_def_dim, ncid, src->dims[i].name, src->dims[i].len, &id);
  for(int i=0; i<src->nvars; i++) {
    nct_var* v = src->vars+i;
    NCFUNK(nc_def_var, ncid, v->name, v->xtype, v->ndims, v->dimids, &id);
    NCFUNK(nc_put_var, ncid, id, v->data);
  }
  NCFUNK(nc_close, ncid);
}

void nct_link_vars_to_dimnames(nct_vset* vs) {
  for(int v=0; v<vs->nvars; v++) {
    nct_var* var = vs->vars+v;
    for(int d=0; d<var->ndims; d++)
      var->dimnames[d] = vs->dims[var->dimids[d]].name;
  }
}

/*does not copy the nct_var that src may point to*/
nct_dim* nct_dimcpy_gd(nct_dim* dest, const nct_dim* src) {
  dest->name = strdup(src->name);
  dest->freeable_name = 1;
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
  dest->freeable_name = 1;
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
  for(int i=0; i<dest->ndims; i++)
    nct_dimcpy_gd(dest->dims+i, src->dims+i);
  /*vars*/
  dest->nvars = src->nvars;
  dest->vars = malloc(dest->nvars*sizeof(nct_var));
  for(int i=0; i<dest->nvars; i++)
    nct_varcpy_gd(dest->vars+i, src->vars+i);
  
  nct_link_dims_to_coords(dest);
  nct_link_vars_to_dimnames(dest);
  return dest;
}
nct_vset* nct_vsetcpy(const nct_vset* src) {
  nct_vset* dest = malloc(sizeof(nct_vset));
  return nct_vsetcpy_gd(dest, src);
}

#define ONE_TYPE(nctype,a,ctype) nct_vset* nct_vset_from_data_##nctype(nct_vset* p, ...) \
  {									\
    nct_vset* vset;							\
    vset = p? p: calloc(1, sizeof(nct_vset));				\
    va_list ptr;							\
    va_start(ptr, p);							\
    int count = 0;							\
    while(va_arg(ptr, void*)) count++;					\
    count /= 2;								\
    vset->vars = malloc(count*sizeof(nct_var));				\
    va_start(ptr, p);							\
    for(int i=0; i<count; i++) {					\
      vset->nvars++;							\
      ctype* data = va_arg(ptr, ctype*);				\
      nct_simply_add_var(vset, data, nctype, 0, NULL, va_arg(ptr, char*)); \
      }									\
       va_end(ptr);							\
    return vset;							\
  }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

void nct_free_dim(nct_dim* dim) {
  if (dim->freeable_name)
    free(dim->name);
  memset(dim, 0, sizeof(nct_dim));
}

void nct_free_var(nct_var* var) {
  free(var->dimnames);
  if(var->xtype == NC_STRING)
    for(int i=0; i<var->len; i++)
      free(((char**)var->data)[i]);
  free(var->data);
  if(var->freeable_name)
    free(var->name);
  memset(var, 0, sizeof(nct_var));
}

void nct_free_coord(nct_dim* coord) {
  *(intptr_t*)(&coord->coordv->name) *= coord->coordv->name!=coord->name; //branchlessly: if(equal) other=NULL
  nct_free_dim(coord);
  nct_free_var(coord->coordv);
  free(coord->coordv);
}

void nct_free_vset(nct_vset* vs) {
  for(int i=0; i<vs->ndims; i++)
    nct_free_dim(vs->dims+i);
  for(int i=0; i<vs->nvars; i++)
    nct_free_var(vs->vars+i);
  free(vs->dims);
  free(vs->vars);
  memset(vs, 0, sizeof(nct_vset));
}

nct_var* nct_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name) {
  dest->name = name;
  dest->len = len;
  dest->size1 = nctypelen(xtype);
  dest->xtype = xtype;
  dest->data = arr;
  return dest;
}
nct_var* nct_to_var(void* arr, size_t len, nc_type xtype, char* name) {
  nct_var* dest = calloc(1, sizeof(nct_var));
  return nct_to_var_gd(dest, arr, len, xtype, name);
}

nct_var* nct_copy_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name) {
  dest->name = name;
  dest->len = len;
  dest->size1 = nctypelen(xtype);
  dest->xtype = xtype;
  dest->data = malloc(len*dest->size1);
  memcpy(dest->data, arr, len*dest->size1);
  return dest;
}
nct_var* nct_copy_to_var(void* arr, size_t len, nc_type xtype, char* name) {
  nct_var* dest = calloc(1, sizeof(nct_var));
  return nct_copy_to_var_gd(dest, arr, len, xtype, name);
}

#if 0
nct_dim* nct_to_coord_gd(nct_dim* dest, void* arr, size_t len, nc_type xtype, char* name) {
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
nct_dim* nct_to_coord(void* arr, size_t len, nc_type xtype, char* name) {
  nct_dim* dest = calloc(1, sizeof(nct_dim));
  nct_to_coord_gd(dest, arr, len, xtype, name);
}
#endif

#define ONE_TYPE(nctype,form,ctype)				\
  ctype* nct_range_##nctype(ctype i0, ctype i1, ctype gap) {	\
    size_t len = (i1-i0)/gap;					\
    ctype* dest = malloc(len*sizeof(ctype));			\
    ctype num = i0-gap;						\
    for(size_t i=0; i<len; i++)					\
      dest[i] = num+=gap;					\
    return dest;						\
  }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

nct_vset* nct_add_dim(nct_vset* vset, size_t len, char* name) {
  int ind = vset->ndims-1;
  vset->dims[ind].name = name;
  vset->dims[ind].freeable_name = 0;
  vset->dims[ind].len = len;
  return vset;
}

nct_vset* nct_add_coord(nct_vset* vset, void* src, size_t len, nc_type xtype, char* name) {
  vset->ndims++;
  vset->dims = realloc(vset->dims, vset->ndims*sizeof(nct_dim));
  nct_add_dim(vset, len, name);

  vset->nvars++;
  vset->vars = realloc(vset->vars, vset->nvars*sizeof(nct_var));
  int dimid = vset->ndims-1;
  nct_simply_add_var(vset, src, xtype, 1, &dimid, name)->vars[vset->nvars-1].iscoordinate = 1;

  vset->dims[vset->ndims-1].coordv = vset->vars + vset->nvars-1;
  return vset;
}

nct_vset* nct_simply_add_var(nct_vset* vset, void* src, nc_type xtype,
			     int ndims, int* dimids, char* name) {
  nct_var* var = vset->vars + vset->nvars-1;
  *var = (nct_var) {
    .name = name,
    .freeable_name = 0,
    .iscoordinate = 0,
    .ndims = ndims,
    .dimnames = malloc(ndims*(sizeof(char*)+sizeof(size_t)+sizeof(int))),
    .len = 1,
    .size1 = nctypelen(xtype),
    .xtype = xtype,
    .data = src,
  };
  var->dimlens = (size_t*)(var->dimnames + ndims);
  var->dimids = (int*)(var->dimlens + ndims);
  for(int i=0; i<ndims; i++) {
    nct_dim* dim = vset->dims + dimids[i];
    var->len         *= dim->len;
    var->dimlens[i]  = dim->len;
    var->dimnames[i] = dim->name;
    var->dimids[i]   = dimids[i];
  }
  return vset;
}

nct_vset* nct_add_var_with_dimids(nct_vset* vset, void* src, nc_type xtype,
				  int ndims, int* dimids, char** dimnames, size_t* dimlens, char* name) {
  int new_dims = 0;
  for(int i=0; i<ndims; i++)
    if(dimids[i]<0) new_dims++; //negative id is new dimension
  if(new_dims) {
    vset->dims = realloc(vset->dims, (vset->ndims+new_dims)*sizeof(nct_dim));
    for(int i=0; i<ndims; i++)
      if(dimids[i]<0) {
	dimids[i] = vset->ndims++;
	nct_add_dim(vset, dimlens[i], dimnames[i]);
      }
  }
  vset->vars = realloc(vset->vars, ++vset->nvars*sizeof(nct_var));
  return nct_simply_add_var(vset, src, xtype, ndims, dimids, name);
}

nct_vset* nct_add_var(nct_vset* vset, void* src, nc_type xtype,
		      int ndims, int* dimids, char** dimnames, size_t* dimlens, char* name) {
  if(!dimids) {
    int ids[ndims];
    for(int i=0; i<ndims; i++)
      for(int j=0; j<vset->ndims; j++) {
	if(!strcmp(vset->dims[j].name, dimnames[i])) {
	  ids[i] = j;
	  break;
	}
	ids[i] = -1; //new dim
      }
    return nct_add_var_with_dimids(vset, src, xtype, ndims, ids, dimnames, dimlens, name);
  }
  return nct_add_var_with_dimids(vset, src, xtype, ndims, dimids, dimnames, dimlens, name);
}

nct_vset* nct_assign_shape(nct_vset* vset, ...) {
  va_list ptr;
  int count = 0;
  va_start(ptr, vset);
  while(va_arg(ptr, char*)) count++;
  int dimids[count];
  va_start(ptr, vset);
  size_t varlen = 1;
  for(int i=0; i<count; i++) {
    dimids[i] = nct_get_dimid(vset, va_arg(ptr, char*));
    varlen *= vset->dims[dimids[i]].len;
  }
  va_end(ptr);
  for(int i=0; i<vset->nvars; i++) {
    nct_var* var = vset->vars+i;
    var->dimnames = realloc(var->dimnames, count*(sizeof(char*)+sizeof(size_t)+sizeof(int)));
    void* running = var->dimnames;
    for(int j=0; j<count; j++) {
      *(char**)running = vset->dims[dimids[j]].name;
      running += sizeof(char*);
    }
    var->dimlens = running;
    for(int j=0; j<count; j++) {
      *(size_t*)running = vset->dims[dimids[j]].len;
      running += sizeof(size_t);
    }
    var->dimids = running;
    memcpy(var->dimids, dimids, count*sizeof(int));
    var->ndims = count;
    var->len = varlen;
  }
  return vset;
}
