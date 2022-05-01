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
  First define ONE_TYPE in a wanted way, then add APPLY_FOR_ALL_TYPES then undef ONE_TYPE
  Functions can be further added into an array of function pointers with the same syntax
  which allows use of nc_type (int) variable as an index to access the right function.
  Reading the code will make this clearer.*/
#define ALL_EXCEPT_STRING			\
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
#define APPLY_FOR_ALL_TYPES			\
  ALL_EXCEPT_STRING				\
  ONE_TYPE(NC_STRING, s, char*)

#define ONE_TYPE(nctype, ...) void print_##nctype(void* arr, int i, int end);
APPLY_FOR_ALL_TYPES
#undef ONE_TYPE
void print_variable_data(variable*);
void print_variable(variable* var, const char* indent);
void print_variable_set(variable_set* vs);
variable* var_from_vset(variable_set* vs, char* name);
variable* var_pluseq_vars(variable*, ...);
variable* var_pluseq_var(variable*, variable*);
#define ONE_TYPE(nctype, ...) variable* var_pluseq_var_##nctype(variable*, variable*);
ALL_EXCEPT_STRING
#undef ONE_TYPE
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
