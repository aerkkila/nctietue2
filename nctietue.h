#ifndef __NC_TIETUE__
#define __NC_TIETUE__

#include <netcdf.h>

typedef struct nct_att nct_att;
typedef struct nct_var nct_var;
typedef struct nct_dim nct_dim;
typedef struct nct_vset nct_vset;

struct nct_att {
  char* name;
  char* value;
  unsigned freeable;
  nc_type xtype;
};

struct nct_var {
  nct_vset* super;
  char* name;
  char freeable_name;
  char iscoordinate;
  int ndims;
  char** dimnames; //pointers to names in nct_dim-structs
  size_t* dimlens; //
  int* dimids;     //these three arrays come from one malloc for dimnames + dimlens + dimids
  int nattrs;
  nct_att* attrs;
  size_t len;
  int size1;
  nc_type xtype;
  void* data;
};

struct nct_dim {
  char* name;
  char freeable_name;
  size_t len;
  nct_var* coordv;
};

struct nct_vset {
  int ndims;
  nct_dim* dims;
  int nvars;
  nct_var* vars;
  int ncid;
};

extern const char* nct_error_color;
extern const char* nct_varset_color;
extern const char* nct_varname_color;
extern const char* nct_type_color;
extern const char* nct_default_color;

extern int ncret;
#define NCERROR(arg) printf("%sNetcdf-error: %s%s\n", nct_error_color, nct_default_color, nc_strerror(arg))
#define NCFUNK(fun, ...)		\
  do {					\
    if((ncret = fun(__VA_ARGS__))) {	\
      NCERROR(ncret);			\
      asm("int $3");			\
    }					\
  } while(0)
#define NCTVAR(vset,name) ((vset).vars[nct_get_varid(&(vset),name)])
#define NCTDIM(vset,name) ((vset).dims[nct_get_dimid(&(vset),name)])
#define NCTVARDIM(var,dimnum) ((var).super->dims[(var).dimids[dimnum]])

nct_vset* nct_open_png_gd(nct_vset* dest, char* name); //nct_png
void nct_plot_var(nct_vset*, int); //nct_sdl2

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

/*All allowed combinations of types (except string) and operations are in "nct_operations_and_types.h"*/

#define ONE_TYPE(nctype, ...) void nct_print_##nctype(void* arr, int i, int end);
ALL_TYPES
#undef ONE_TYPE
void nct_print_var_data(nct_var*);
void nct_print_var(nct_vset* vset, int varid, const char* indent);
void nct_print_vset_title(nct_vset* vs);
void nct_print_vset(nct_vset* vs);

size_t nct_getlen(nct_vset* vset, int varid);

#define OPERATION(nctype, a, ctype, opername, b) nct_var* nct_var_##opername##_##nctype(nct_var*, void*);
#include "nct_operations_and_types.h"
#undef OPERATION
#define ONE_OPERATION(opername, a) nct_var* nct_var_##opername(nct_var*, void*);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define OPERATION(nctype, a, ctype, opername, b) nct_var* vararr_##opername##_##nctype(nct_var*, void*);
#include "nct_operations_and_types.h"
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
#define ONE_TYPE(nctype, a, ctype) nct_var* nct_varnanmean0_##nctype(nct_var*);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE
nct_var* nct_varnanmean0(nct_var*);

nct_vset* nct_vset_isel(nct_vset* vset, int dimid, size_t ind0, size_t ind1);
int nct_get_dimid(nct_vset* vset, char* name);
int nct_get_varid(nct_vset* vset, char* name);
char* nct_get_att_text(nct_vset* vset, int varid, char* name);
int nct_get_noncoord_varid(nct_vset* vset);

nct_vset* nct_read_dim(nct_vset*, int dimid);
nct_dim* nct_dimcpy_gd(nct_dim* dest, const nct_dim* src);
nct_dim* nct_dimcpy(const nct_dim* src);
void nct_free_dim(nct_dim*);

nct_vset* nct_load_var(nct_vset* vset, int varid);
nct_vset* nct_nload_var(nct_vset* vset, int varid, size_t start, size_t len);
nct_vset* nct_read_var_info(nct_vset *vset, int varid);
nct_vset* nct_read_var(nct_vset* vset, int varid);
nct_var* nct_varcpy_gd(nct_var* dest, const nct_var* src);
nct_var* nct_varcpy(const nct_var* src);
nct_var* nct_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_to_var(void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_copy_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_copy_to_var(void* arr, size_t len, nc_type xtype, char* name);
nct_var* nct_var_dropdim0(nct_var*);
void nct_free_var(nct_var*);

nct_vset* nct_read_ncfile_info(const char* restrict);
nct_vset* nct_read_ncfile_info_gd(nct_vset*, const char* restrict);
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
nct_vset* nct_add_att_text(nct_vset* vset, int varid, char* name, char* value, unsigned freeable);
nct_vset* nct_simply_add_var(nct_vset*, void*, nc_type, int, int*, char*);
char* nct_find_unique_varname(nct_vset* vset, char* initname);
nct_vset* nct_move_similar_var(nct_vset* vset0, nct_vset* vset, int varid);
nct_vset* nct_move_similar_vset(nct_vset* vset0, nct_vset* vset);
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
