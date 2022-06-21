
#define ONE_TYPE(nctype, form, ctype)				\
    void nct_print_##nctype(void* arr, int i, int end) {	\
	for(; i<end; i++)					\
	    printf("%"#form", ", ((ctype*)arr)[i]);		\
    }
ALL_TYPES
#undef ONE_TYPE

#define ONE_TYPE(nctype, ...) [nctype]=nct_print_##nctype,
void (*printfunctions[])(void*, int, int) = { ALL_TYPES };
#undef ONE_TYPE

#define ONE_TYPE(nctype, form, ctype) [nctype]=#ctype,
char* type_names[] = { ALL_TYPES };
#undef ONE_TYPE

#define OPERATION(nctype, a, ctype, opername, oper)			\
    nct_var* nct_var_##opername##_##nctype(nct_var* var, void* vvalue)	\
    {									\
	ctype value = *((ctype*)vvalue);				\
	for(size_t i=0; i<var->len; i++)				\
	    ((ctype*)var->data)[i] oper value;				\
	return var;							\
    }
#include "nct_operations_and_types.h"
#undef OPERATION

#define OPERATION(nctype, form, ctype, opername, oper)			\
    nct_var* vararr_##opername##_##nctype(nct_var* v0, void* arr) {	\
	for(size_t i=0; i<v0->len; i++)					\
	    ((ctype*)v0->data)[i] oper ((ctype*)arr)[i];		\
	return v0;							\
    }
#include "nct_operations_and_types.h"
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

#define ONE_OPERATION(opername, oper)					\
    nct_var* vararr_##opername(nct_var* v0, void* arr) {		\
	return vararr_##opername##_functions[v0->xtype](v0, arr);	\
    }
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define ONE_OPERATION(opername, oper)			\
    nct_var* vararrs_##opername(nct_var* var, ...) {	\
	va_list ptr;					\
	int va_len = 4;					\
	int i=0;					\
	while(1) {					\
	    va_start(ptr, var);				\
	    for(int _i=0; _i<i; _i++)			\
		va_arg(ptr, void*);			\
	    for(; i<va_len; i++) {			\
		void* v1 = va_arg(ptr, void*);		\
		if(!v1)					\
		    goto FINISHED;			\
		vararr_##opername(var, v1);		\
	    }						\
	    va_end(ptr);				\
	    va_len *= 2;				\
	}						\
    FINISHED:						\
	va_end(ptr);					\
	return var;					\
    }
ALL_EQ_OPERATIONS
#undef ONE_OPERATION

#define OPERATION(nctype, a, b, opername, c)				\
    vararr_##opername##_functions[nctype] = vararr_##opername##_##nctype; \
    nct_var_##opername##_functions[nctype] = nct_var_##opername##_##nctype;
void nct_init() {
#include "nct_operations_and_types.h"
}
#undef OPERATION

#define ONE_TYPE(nctype, b, ctype)				\
    void* nct_minmax_##nctype(nct_var* var, void* resultv)	\
    {								\
	ctype maxval, minval, *result=resultv;			\
	maxval = minval = ((ctype*)var->data)[0];		\
	for(int i=1; i<var->len; i++)				\
	    if(maxval < ((ctype*)var->data)[i])			\
		maxval = ((ctype*)var->data)[i];		\
	    else if(minval > ((ctype*)var->data)[i])		\
		minval = ((ctype*)var->data)[i];		\
	result[0] = minval;					\
	result[1] = maxval;					\
	return result;						\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_minmax_##nctype,
void* (*minmax[])(nct_var*, void*) = { ALL_TYPES_EXCEPT_STRING };
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, ctype)					\
    nct_var* nct_varmean0_##nctype(nct_var* var)			\
    {									\
	size_t new_len = var->len / NCTVARDIM(*var,0).len;		\
	for(size_t i=0; i<new_len; i++) {				\
	    for(size_t j=1; j<NCTVARDIM(*var,0).len; j++)		\
		((ctype*)var->data)[i] += ((ctype*)var->data)[i+new_len*j]; \
	    ((ctype*)var->data)[i] /= NCTVARDIM(*var,0).len;		\
	}								\
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

#define ONE_TYPE(nctype, a, ctype)				\
    nct_var* nct_varnanmean0_##nctype(nct_var* var)		\
    {								\
	size_t new_len = var->len / NCTVARDIM(*var,0).len;	\
	for(size_t i=0; i<new_len; i++) {			\
	    int count = 0;					\
	    ctype new_value = 0;				\
	    for(size_t j=0; j<NCTVARDIM(*var,0).len; j++) {	\
		ctype test = ((ctype*)var->data)[i+new_len*j];	\
		if(test==test) {				\
		    count++;					\
		    new_value += test;				\
		}						\
	    }							\
	    ((ctype*)var->data)[i] = new_value/count;		\
	}							\
	return nct_var_dropdim0(var);				\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_varnanmean0_##nctype,
nct_var* (*varnanmean0[])(nct_var*) =
{
    ALL_TYPES_EXCEPT_STRING
};
#undef ONE_TYPE

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
	len_after *= NCTVARDIM(*var,i).len;
    size_t length_around = len_after * NCTVARDIM(*var,id).len;
    size_t n_blocks = 1;
    for(int i=0; i<id; i++)
	n_blocks *= NCTVARDIM(*var,i).len;
    int blocklen = (ind1-ind0) * len_after;
    void* destp = var->data;
    void* srcp = var->data + len_after*ind0;
    for(int i=0; i<n_blocks; i++) {
	memmove(destp, srcp, blocklen);
	destp += blocklen;
	srcp += length_around;
    }
    var->data = realloc(var->data, blocklen*n_blocks);
    //NCTVARDIM(*var,id).len = ind1-ind0; This is done at vset_isel when all vars are changed.
    var->len = blocklen / var->size1 * n_blocks;
    return var;
}

#if 0
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
#endif

#define ONE_TYPE(nctype,a,ctype) nct_vset* nct_vset_from_data_##nctype(nct_vset* p, ...) \
    {									\
	nct_vset* vset;							\
	vset = p? p: calloc(1, sizeof(nct_vset));			\
	va_list ptr;							\
	va_start(ptr, p);						\
	int count = 0;							\
	while(va_arg(ptr, void*)) count++;				\
	count /= 2;							\
	vset->vars = malloc(count*sizeof(nct_var));			\
	va_start(ptr, p);						\
	for(int i=0; i<count; i++) {					\
	    ctype* data = va_arg(ptr, ctype*);				\
	    nct_simply_add_var(vset, data, nctype, 0, NULL, va_arg(ptr, char*)); \
	}								\
	va_end(ptr);							\
	return vset;							\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

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
	size_t len = (i1-i0)/gap;				\
	while(i0+len*gap < i1) len++;				\
	ctype* dest = malloc(len*sizeof(ctype));		\
	ctype num = i0-gap;					\
	for(size_t i=0; i<len; i++)				\
	    dest[i] = num+=gap;					\
	return dest;						\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE
