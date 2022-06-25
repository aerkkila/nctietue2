#ifndef __NCTIETUE__
#define __NCTIETUE__

#include <netcdf.h>

typedef struct nct_att nct_att;
typedef struct nct_var nct_var;
typedef struct nct_vset nct_vset;

struct nct_att {
    char* name;
    void* value;
    nc_type xtype;
    int len;
    unsigned freeable;
};

struct nct_var {
    nct_vset* super;
    char*     name;
    char      freeable_name;
    int       ndims;
    int       dimcapacity;
    int*      dimids;
    int       natts;
    int       attcapacity;
    nct_att*  atts;
    size_t    len;
    nc_type   xtype;
    char      nonfreeable_data;
    void*     data;
};

struct nct_vset {
    int nvars;
    int varcapacity;
    nct_var** vars;
    int ndims;
    int dimcapacity;
    nct_var** dims; // points to same name variable if available
    int natts;
    int attcapacity;
    nct_att* atts;
    int ncid;
};

extern const char* nct_error_color;
extern const char* nct_varset_color;
extern const char* nct_varname_color;
extern const char* nct_dimname_color;
extern const char* nct_type_color;
extern const char* nct_default_color;
extern       int   nct_ncret;
#define NCERROR(arg) printf("%sNetcdf-error: %s%s\n", nct_error_color, nct_default_color, nc_strerror(arg))
#define NCFUNK(fun, ...)			\
    do {					\
	if((nct_ncret = fun(__VA_ARGS__))) {	\
	    NCERROR(nct_ncret);			\
	    asm("int $3");			\
	}					\
    } while(0)
#define NCTDIM(vset,name) (*(vset).dims[nct_get_dimid(&(vset),name)])
#define NCTVAR(vset,name) (*(vset).vars[nct_get_varid(&(vset),name)])
#define NCTVARDIM(var,dimnum) (*(var).super->dims[(var).dimids[dimnum]])

/* With this macro one can define functions for all nct_var types without repeating things.
   First define ONE_TYPE in a wanted way, then add ALL_TYPES then undef ONE_TYPE
   Functions are be further added into an array of function pointers with the same syntax
   which allows use of nc_type (int) as an index to access the right function. */
#define ALL_TYPES_EXCEPT_STRING				\
    ONE_TYPE(NC_BYTE, hhi, char)			\
	ONE_TYPE(NC_UBYTE, hhu, unsigned char)		\
	ONE_TYPE(NC_CHAR, c, char)			\
	ONE_TYPE(NC_SHORT, hi, short)		        \
	ONE_TYPE(NC_USHORT, hu, unsigned short)		\
	ONE_TYPE(NC_INT, i, int)			\
	ONE_TYPE(NC_UINT, u, unsigned)			\
	ONE_TYPE(NC_UINT64, llu, long long unsigned)	\
	ONE_TYPE(NC_INT64, lli, long long int)		\
	ONE_TYPE(NC_FLOAT, .4f, float)			\
	ONE_TYPE(NC_DOUBLE, .4lf, double)
#define ALL_TYPES				\
    ALL_TYPES_EXCEPT_STRING			\
    ONE_TYPE(NC_STRING, s, char*)

/*The same thing is done here with operations than with types*/
#if 0
#define ALL_EQ_OPERATIONS			\
    ONE_OPERATION(pluseq, +=)         		\
	ONE_OPERATION(minuseq, -=)		\
	ONE_OPERATION(muleq, *=)		\
	ONE_OPERATION(diveq, /=)      		\
	ONE_OPERATION(modeq, %=)		\
	ONE_OPERATION(bitoreq, |=)     		\
	ONE_OPERATION(bitandeq, &=)		\
	ONE_OPERATION(bitxoreq, ^=)		\
	ONE_OPERATION(bitlshifteq, <<=)		\
	ONE_OPERATION(bitrshifteq, >>=)

#define ALL_CMP_OPERATIONS			\
    ONE_OPERATION(lt, <)			\
	ONE_OPERATION(gt, >)			\
	ONE_OPERATION(le, <=)			\
	ONE_OPERATION(ge, >=)			\
	ONE_OPERATION(eq, ==)			\
	ONE_OPERATION(ne, !=)

#define OPERATION(nctype, a, ctype, opername, b) nct_var* nct_var_##opername##_##nctype(nct_var*, void*);
#include "nct_operations_and_types.h"
#undef OPERATION
#define ONE_OPERATION(opername, a) nct_var* nct_var_##opername(nct_var*, void*);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define OPERATION(nctype, a, ctype, opername, b) nct_var* nct_vararr_##opername##_##nctype(nct_var*, void*);
#include "nct_operations_and_types.h"
#undef OPERATION
#define ONE_OPERATION(opername, a) nct_var* nct_vararr_##opername(nct_var*, void*);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION
#define ONE_OPERATION(opername, a) nct_var* nct_vararrs_##opername(nct_var*, ...);
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#endif

/*All allowed combinations of types and operations are in "nct_operations_and_types.h"*/

#define ONE_TYPE(nctype, ...) void nct_print_##nctype(void* arr, int i, int end);
ALL_TYPES
#undef ONE_TYPE

#define ONE_TYPE(nctype,b,ctype) ctype* nct_range_##nctype(ctype p0, ctype p1, ctype gap);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, ctype) void* nct_varminmax_##nctype(nct_var*, void*);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, ctype) nct_var* nct_varmean0_##nctype(nct_var*);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, ctype) nct_var* nct_varnanmean0_##nctype(nct_var*);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype,b,c) nct_vset* nct_vset_from_data_##nctype(nct_vset* vset, ...);
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

extern const char* nct_typenames[];

nct_vset* nct_open_png_gd(nct_vset* dest, char* name); // nct_png
void      nct_plot_var(nct_vset*, int);                // nct_sdl2

void      nct_add_varatt_text(nct_var* var, char* name, char* value, unsigned freeable);
nct_var*  nct_add_dim(nct_vset*, void*, size_t, nc_type, char*);
nct_var*  nct_add_var(nct_vset*, void*, nc_type, char*, int, int*);
nct_var*  nct_add_var_(nct_vset*, void*, nc_type, char*, int, int*, size_t*, char**);
nct_var*  nct_add_var_dimids(nct_vset*, void*, nc_type, char*, int, int*, size_t*, char**);
nct_vset* nct_assign_shape(nct_vset*, ...);
char*     nct_find_unique_varname(nct_vset* vset, char* initname);
void      nct_free_att(nct_att*);
void      nct_free_var(nct_var*);
void      nct_free_vset(nct_vset*);
int       nct_get_dimid(nct_vset*, char*);
int       nct_get_id_thisdim(nct_var*);
int       nct_get_id_thisvar(nct_var*);
char*     nct_get_varatt_text(nct_var*, char*);
int       nct_get_varid(nct_vset*, char*);
size_t    nct_get_varlen(nct_var*);
void      nct_init();
nct_var*  nct_load_var(nct_var* var, int ncvarid);
void*     nct_varminmax(nct_var*, void*);
nct_vset* nct_move_var_tosimilar(nct_vset* dest, nct_var* srcvar);
nct_vset* nct_move_vset_tosimilar(nct_vset* dest, nct_vset* src);
void      nct_print_var(nct_var* var, const char* indent);
void      nct_print_var_data(nct_var*);
void      nct_print_vset(nct_vset* vs);
void      nct_print_vset_title(nct_vset* vs);
nct_vset* nct_read_ncfile(const char* restrict filename);
nct_vset* nct_read_ncfile_gd(nct_vset* dest, const char* restrict filename);
nct_vset* nct_read_ncfile_info(const char* restrict);
nct_vset* nct_read_ncfile_info_gd(nct_vset*, const char* restrict);
nct_vset* nct_read_var(nct_vset* vset, int varid);
nct_vset* nct_read_var_info(nct_vset *vset, int varid);
nct_var*  nct_to_var(void* arr, size_t len, nc_type xtype, char* name);
nct_var*  nct_to_var_gd(nct_var* dest, void* arr, size_t len, nc_type xtype, char* name);
nct_var*  nct_varcpy_similar_gd(nct_var* dest, nct_var* src);
nct_var*  nct_var_dropdim0(nct_var*);
nct_var*  nct_varmean0(nct_var*);
nct_var*  nct_varnanmean0(nct_var*);
nct_vset* nct_vset_isel(nct_vset* vset, int dimid, size_t ind0, size_t ind1);
nct_vset* nct_vsetcpy(const nct_vset* src);
nct_vset* nct_vsetcpy_gd(nct_vset* dest, const nct_vset* src);
void      nct_write_ncfile(const nct_vset* src, const char* name);

#endif
