#ifndef __NC_TIETUE__
#define __NC_TIETUE__
typedef struct {
  char* name;
  char iscoordinate;
  int ndims;
  char** dimnames; //pointers to names in dimension-structs
  size_t* dimlens; //
  int* dimids;     //these three arrays come from one malloc for dimnames + dimlens + dimids
  size_t len;
  int size1;
  nc_type xtype;
  void* data;
} variable;

typedef struct {
  char* name;
  size_t len;
  variable* coord;
} dimension;

typedef struct {
  int ndims;
  dimension* dims;
  char** dimnames; //pointers to names in dimension-structs
  size_t* dimlens;
  int nvars;
  variable* vars;
  char** varnames; //pointers to names in variable-structs
} variable_set;

/*With this macro one can define functions for all variable types without repeating things.
  First define ONE_TYPE in a wanted way, then add ALL_TYPES then undef ONE_TYPE
  Functions can be further added into an array of function pointers with the same syntax
  which allows use of nc_type (int) variable as an index to access the right function.
  Reading the code will make this clearer.*/
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
#define ALL_OPERATIONS				\
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

/*All allowed combinations of types (except string) and operations are in "operations_and_types.h"*/

#define ONE_TYPE(nctype, ...) void print_##nctype(void* arr, int i, int end);
ALL_TYPES
#undef ONE_TYPE
void print_variable_data(variable*);
void print_variable(variable* var, const char* indent);
void print_variable_set(variable_set* vs);
variable* var_from_vset(variable_set* vs, char* name);
#define OPERATION(nctype, a, b, opername, c) variable* varvar_##opername##_##nctype(variable*, variable*);
#include "operations_and_types.h"
#undef OPERATION
#define ONE_OPERATION(opername, a) variable* varvar_##opername(variable*, variable*);
ALL_OPERATIONS
#undef ONE_OPERATION
#define ONE_OPERATION(opername, a) variable* varvars_##opername(variable*, ...);
ALL_OPERATIONS
#undef ONE_OPERATION
void nctietue_init();
dimension* read_ncdim(int ncid, int dimid, dimension* dest);
variable* read_ncvariable(int ncid, int varid, dimension* dims, variable* dest);
variable_set* read_ncfile(const char* restrict filename, variable_set* dest);
void link_variables_to_dimnames(variable_set* vs);
void link_dims_to_coords(variable_set* dest);
dimension* dimcpy(dimension* dest, const dimension* src);
variable* varcpy(variable* dest, const variable* src);
variable_set* vsetcpy(variable_set* dest, const variable_set* src);
void free_dimension(dimension*);
void free_variable(variable*);
void free_variable_set(variable_set*);
#endif
