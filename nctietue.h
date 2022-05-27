#ifndef __NC_TIETUE__
#define __NC_TIETUE__

#include <netcdf.h>

typedef struct {
  char* name;
  char freeable_name;
  char iscoordinate;
  int ndims;
  char** dimnames; //pointers to names in nct_dim-structs
  size_t* dimlens; //
  int* dimids;     //these three arrays come from one malloc for dimnames + dimlens + dimids
  size_t len;
  int size1;
  nc_type xtype;
  void* data;
} nct_var;

typedef struct {
  char* name;
  char freeable_name;
  size_t len;
  nct_var* coordv;
} nct_dim;

typedef struct {
  int ndims;
  nct_dim* dims;
  int nvars;
  nct_var* vars;
} nct_vset;

#include "nct_png.h"
#include "nct_sdl2.h"

/*With this macro one can define functions for all nct_var types without repeating things.
  First define ONE_TYPE in a wanted way, then add ALL_TYPES then undef ONE_TYPE
  Functions can be further added into an array of function pointers with the same syntax
  which allows use of nc_type (int) as an index to access the right function.*/
#define ALL_TYPES_EXCEPT_STRING			\
  ONE_TYPE(NC_BYTE, hhi, char)			\
  ONE_TYPE(NC_UBYTE, hhu, unsigned char)     	\
  ONE_TYPE(NC_CHAR, c, char)			\
  ONE_TYPE(NC_SHORT, hi, short)		        \
  ONE_TYPE(NC_USHORT, hu, unsigned short)	\
  ONE_TYPE(NC_INT, i, int)			\
  ONE_TYPE(NC_UINT, u, unsigned)	        \
  ONE_TYPE(NC_UINT64, llu, long long unsigned)	\
  ONE_TYPE(NC_INT64, lli, long long int)	\
  ONE_TYPE(NC_FLOAT, .4f, float)		\
  ONE_TYPE(NC_DOUBLE, .4lf, double)
#define ALL_TYPES				\
  ALL_TYPES_EXCEPT_STRING			\
  ONE_TYPE(NC_STRING, s, char*)

/*The same thing is done here with operations than with types*/
#define ALL_EQ_OPERATIONS			\
  ONE_OPERATION(pluseq, +=)         		\
  ONE_OPERATION(minuseq, -=)       		\
  ONE_OPERATION(muleq, *=)        		\
  ONE_OPERATION(diveq, /=)      		\
  ONE_OPERATION(modeq, %=)        		\
  ONE_OPERATION(bitoreq, |=)     		\
  ONE_OPERATION(bitandeq, &=)      		\
  ONE_OPERATION(bitxoreq, ^=)      		\
  ONE_OPERATION(bitlshifteq, <<=)		\
  ONE_OPERATION(bitrshifteq, >>=)

#if 0
#define ALL_CMP_OPERATIONS	\
  ONE_OPERATION(lt, <)		\
  ONE_OPERATION(gt, >)		\
  ONE_OPERATION(le, <=)		\
  ONE_OPERATION(ge, >=)		\
  ONE_OPERATION(eq, ==)		\
  ONE_OPERATION(ne, !=)
#endif

/*All allowed combinations of types (except string) and operations are in "operations_and_types.h"*/

#define ONE_TYPE(nctype, ...) void nct_print_##nctype(void* arr, int i, int end);
ALL_TYPES
#undef ONE_TYPE
void nct_print_var_data(nct_var*);
void nct_print_var(nct_var* var, const char* indent);
void nct_print_vset(nct_vset* vs);
//nct_var* var_from_vset(nct_vset* vs, char* name);

#define OPERATION(nctype, a, ctype, opername, b) nct_var* nct_var_##opername##_##nctype(nct_var*, void*);
#include "operations_and_types.h"
#undef OPERATION
#define ONE_OPERATION(opername, a) nct_var* nct_var_##opername(nct_var*, void*);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define OPERATION(nctype, a, ctype, opername, b) nct_var* vararr_##opername##_##nctype(nct_var*, void*);
#include "operations_and_types.h"
#undef OPERATION
#define ONE_OPERATION(opername, a) nct_var* vararr_##opername(nct_var*, void*);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION
#define ONE_OPERATION(opername, a) nct_var* vararrs_##opername(nct_var*, ...);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

void nct_init();

#define ONE_TYPE(nctype, a, ctype) void* nct_minmax_##nctype(nct_var*, void*);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE
void* nct_minmax(nct_var*, void*);
#define ONE_TYPE(nctype, a, ctype) nct_var* nct_varmean0_##nctype(nct_var*);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE
nct_var* nct_varmean0(nct_var*);

nct_vset* nct_vset_isel(nct_vset* vset, int dimid, size_t ind0, size_t ind1);
int nct_get_dimid(nct_vset* vset, char* name);
int nct_get_varid(nct_vset* vset, char* name);

nct_dim* nct_read_dim_gd(nct_dim* dest, int ncid, int dimid);
nct_dim* nct_read_dim(int ncid, int dimid);
nct_dim* nct_dimcpy_gd(nct_dim* dest, const nct_dim* src);
nct_dim* nct_dimcpy(const nct_dim* src);
void nct_free_dim(nct_dim*);

nct_var* nct_read_var_gd(nct_var* dest, int ncid, int varid, nct_dim* dims);
nct_var* nct_read_var(int ncid, int varid, nct_dim* dims);
nct_var* nct_varcpy_gd(nct_var* dest, const nct_var* src);
nct_var* nct_varcpy(const nct_var* src);
nct_var* nct_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_to_var(void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_copy_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_copy_to_var(void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_var_dropdim0(nct_var*);
void nct_free_var(nct_var*);

nct_vset* nct_read_ncfile_gd(nct_vset* dest, const char* restrict filename);
nct_vset* nct_read_ncfile(const char* restrict filename);
void nct_write_ncfile(const nct_vset* src, const char* name);
nct_vset* nct_vsetcpy_gd(nct_vset* dest, const nct_vset* src);
nct_vset* nct_vsetcpy(const nct_vset* src);
#define ONE_TYPE(nctype,b,c) nct_vset* nct_vset_from_data_##nctype(nct_vset* vset, ...);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE
nct_vset* nct_add_dim(nct_vset*, size_t, char*);
nct_vset* nct_add_coord(nct_vset*, void*, size_t, nc_type, char*);
nct_vset* nct_simply_add_var(nct_vset*, void*, nc_type, int, int*, char*);
nct_vset* nct_add_var_with_dimids(nct_vset*, void*, nc_type, int, int*, char**, size_t*, char*);
nct_vset* nct_add_var(nct_vset*, void*, nc_type, int, int*, char**, size_t*, char*);
nct_vset* nct_assign_var(nct_vset*, nct_var*);
void nct_free_vset(nct_vset*);
void nct_link_vars_to_dimnames(nct_vset* vs);
void nct_link_dims_to_coords(nct_vset* dest);
#define ONE_TYPE(nctype,b,ctype) ctype* nct_range_##nctype(ctype p0, ctype p1, ctype gap);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE
nct_vset* nct_assign_shape(nct_vset*, ...);

extern const char* error_color;
extern const char* default_color;
#endif
