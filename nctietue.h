#ifndef __NC_TIETUE__
#define __NC_TIETUE__
typedef struct {
  char* name;
  char iscoordinate;
  int ndims;
  char** dimnames; //pointers to names in dimension-structs
  size_t* dimlens;
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
  dimension* dims;
  variable* vars;
  int nvars;
  int ndims;
  char** dimnames; //pointers to names in dimension-structs
  char** varnames; //pointers to names in variable-structs
  size_t* dimlens;
} variable_set;

variable_set* read_ncfile(const char* restrict filename, variable_set* dest);
variable* read_ncvariable(int ncid, int varid, dimension* dims, variable* dest);
dimension* read_ncdim(int ncid, int dimid, dimension* dest);
void free_dimension(dimension*);
void free_variable(variable*);
void free_variable_set(variable_set*);
void print_variable_set(variable_set* vs);
void print_variable(variable* var, const char* indent);
void print_variable_data(variable*);
#endif
