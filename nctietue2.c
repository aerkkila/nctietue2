#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <stdint.h>
#include "nctietue2.h"
#include "internals.h"
#include "nct_png.c"
#include "nct_sdl2.c"

const char* nct_error_color   = "\033[1;31m";
const char* nct_varset_color  = "\033[1;35m";
const char* nct_varname_color = "\033[92m";
const char* nct_dimname_color = "\033[44;92m";
const char* nct_type_color    = "\033[93m";
const char* nct_default_color = "\033[0m";
int         nct_ncret;

void nct_add_varatt_text(nct_var* var, char* name, char* value, unsigned freeable) {
    void* vp;
    if(var->attcapacity < var->natts+1) {
	if(!(vp=realloc(var->atts, (var->attcapacity=var->natts+3)*sizeof(nct_att))))
	    goto failed;
	var->atts = vp;
    }
    var->atts[var->natts++] = (nct_att){ .name     = name,
					   .value    = value,
					   .xtype    = NC_STRING,
					   .len      = strlen(value)+1,
					   .freeable = freeable };
    return;
failed:
    var->attcapacity = var->natts;
    fprintf(stderr, "realloc failed in nct_add_varatt_text.\n");
}

nct_var* nct_add_dim(nct_vset* vset, void* src, size_t len, nc_type xtype, char* name) {
    void* vp;
    int id;
    if(vset->dimcapacity < vset->ndims+1) {
	if(!(vp = realloc(vset->dims, (vset->dimcapacity=vset->ndims+3)*sizeof(void*))))
	    goto failed;
	vset->dims = vp;
    }
    id = vset->ndims++;
    if(src)
	(vset->dims[id] = nct_add_var(vset, src, xtype, name, 1, &id))->len = len;
    else {
	*(vset->dims[id] = calloc(1,sizeof(nct_var))) = (nct_var) {
	    .super = vset,
	    .name  = name,
	    .len   = len,
	};
    }
    return vset->dims[id];
failed:
    fprintf(stderr, "realloc failed in nct_add_dim\n");
    vset->dimcapacity = vset->ndims;
    return NULL;
}

nct_var* nct_add_var(nct_vset* vset, void* src, nc_type xtype, char* name,
		     int ndims, int* dimids) {
    if(vset->varcapacity < vset->nvars+1)
	if(!(vset->vars=realloc(vset->vars, (vset->varcapacity=vset->nvars+3)*sizeof(void*))))
	    goto failed;
    nct_var* var = vset->vars[vset->nvars] = calloc(1,sizeof(nct_var));
    *var = (nct_var) {
	.super = vset,
	.name = name,
	.ndims = ndims,
	.dimcapacity = ndims+1,
	.dimids = malloc((ndims+1)*(sizeof(int))),
	.xtype = xtype,
	.data = src,
    };
    if(!var->dimids)
	goto failed;
    memcpy(var->dimids, dimids, ndims*sizeof(int));
    vset->nvars++;
    return var;
failed:
    fprintf(stderr, "(re/m)alloc failed in nct_add_var\n");
    return NULL;
}

/* dimids can contain negative values indicating dimension to be created
 * dimnames can contain non-existent names that will be created if dimids==NULL */
nct_var* nct_add_var_(nct_vset* vset, void* src, nc_type xtype, char* name,
			  int ndims, int* dimids, size_t* dimlens, char** dimnames) {
    if(dimids)
	return nct_add_var_dimids(vset, src, xtype, name, ndims, dimids, dimlens, dimnames);
    int ids[ndims];
    for(int i=0; i<ndims; i++)
	ids[i] = nct_get_dimid(vset, dimnames[i]); // new dimension is -1
    return nct_add_var_dimids(vset, src, xtype, name, ndims, ids, dimlens, dimnames);
}

/* This is used like nct_add_var but dimids must not be NULL.
   If all dimids are given (non-negative), then this is like nct_add_var. */
nct_var* nct_add_var_dimids(nct_vset* vset, void* src, nc_type xtype, char* name,
				  int ndims, int* dimids, size_t* dimlens, char** dimnames) {
    int new_dims = 0;
    for(int i=0; i<ndims; i++)
	if(dimids[i]<0) new_dims++;
    if(new_dims) {
	if(vset->dimcapacity < vset->ndims+new_dims)
	    vset->dims = realloc(vset->dims, (vset->dimcapacity=vset->ndims+new_dims+1)*sizeof(nct_var*));
	for(int i=0; i<ndims; i++)
	    if(dimids[i]<0) {
		nct_add_dim(vset, NULL, dimlens[i], NC_NAT, dimnames[i]);
		dimids[i] = vset->ndims-1;
	    }
    }
    return nct_add_var(vset, src, xtype, name, ndims, dimids);
}

nct_vset* nct_assign_shape(nct_vset* vset, ...) {
    va_list ptr;
    int count = 0;
    va_start(ptr, vset);
    while(va_arg(ptr, char*)) count++;
    int dimids[count];

    va_start(ptr, vset);
    for(int i=0; i<count; i++)
	dimids[i] = nct_get_dimid(vset, va_arg(ptr, char*));
    va_end(ptr);

    void* vp;
    for(int i=0; i<vset->nvars; i++) {
	nct_var* var = vset->vars[i];
	if(var->dimcapacity < count) {
	    if(!(vp = realloc(var->dimids, (var->dimcapacity=count+1)*sizeof(int)))) {
		var->dimcapacity = var->ndims;
		goto failed;
	    }
	    var->dimids = vp;
	}
	memcpy(var->dimids, dimids, count*sizeof(int));
	var->ndims = count;
    }

    return vset;
failed:
    fprintf(stderr, "Realloc failed in nct_assign_shape\n");
    return vset;
}

nct_var* nct_assign_var(nct_var* var) {
    if(var->super->varcapacity < var->super->nvars+1)
	var->super->vars = realloc(var->super->vars, (var->super->varcapacity=var->super->nvars+3)*sizeof(void*));
    var->super->vars[var->super->nvars++] = var;
    return var;
}

char* nct_find_unique_varname(nct_vset* vset, const char* initname) {
    char newname[strlen(initname)+6];
    strcpy(newname, initname);
    char* ptr = newname + strlen(newname);
    for(int num=0; num<99999; num++) {
	sprintf(ptr, "%i", num);
	for(int i=0; i<vset->nvars; i++)
	    if(!strcmp(newname, vset->vars[i]->name))
		goto NEXT;
	return strdup(newname);
    NEXT:;
    }
    return NULL;
}

void nct_free_att(nct_att* att) {
    if(att->xtype == NC_STRING)
	for(int i=0; i<att->len; i++)
	    free(((char**)att->value)[i]);
    if(att->freeable & 1)
	free(att->value);
    if(att->freeable & 2)
	free(att->name);
    att->freeable = 0;
}

void nct_free_var(nct_var* var) {
    free(var->dimids);
    for(int i=0; i<var->natts; i++)
	nct_free_att(var->atts+i);
    free(var->atts);
    if(var->xtype == NC_STRING)
	for(int i=0; i<var->len; i++)
	    free(((char**)var->data)[i]);
    if(!var->nonfreeable_data)
	free(var->data);
    if(var->freeable_name)
	free(var->name);
}

void nct_free_vset(nct_vset* vs) {
    if(!vs)
	return;
    for(int i=0; i<vs->natts; i++)
	nct_free_att(vs->atts+1);
    free(vs->atts);
    for(int i=0; i<vs->nvars; i++)
	if(nct_get_id_thisdim(vs->vars[i]) < 0) {
	    nct_free_var(vs->vars[i]);
	    memset(vs->vars[i], 0, sizeof(nct_var));
	    free(vs->vars[i]);
	}
    for(int i=0; i<vs->ndims; i++) {
	nct_free_var(vs->dims[i]);
	memset(vs->dims[i], 0, sizeof(nct_var));
	free(vs->dims[i]);
    }
    memset(vs->vars, 0, sizeof(void*)*vs->nvars);
    free(vs->vars);
    memset(vs->dims, 0, sizeof(void*)*vs->ndims);
    free(vs->dims);
}

int nct_get_dimid(nct_vset* vset, const char* name) {
    for(int i=0; i<vset->ndims; i++)
	if(!strcmp(vset->dims[i]->name, name))
	    return i;
    return -1;
}

int nct_get_id_thisdim(nct_var* v) {
    for(int i=0; i<v->super->ndims; i++)
	if(v->super->dims[i] == v)
	    return i;
    return -1;
}

int nct_get_id_thisvar(nct_var* v) {
    for(int i=0; i<v->super->nvars; i++)
	if(v->super->vars[i] == v)
	    return i;
    return -1;
}

char* nct_get_varatt_text(nct_var* var, const char* name) {
    for(int i=0; i<var->natts; i++)
	if(!strcmp(var->atts[i].name, name))
	    return var->atts[i].value;
    return NULL;
}

int nct_get_varid(nct_vset* vset, const char* name) {
    for(int i=0; i<vset->nvars; i++)
	if(!strcmp(vset->vars[i]->name, name))
	    return i;
    return -1;
}

size_t nct_get_varlen(nct_var* var) {
    if(var->len > 0)
	return var->len;
    size_t len = 1;
    for(int i=0; i<var->ndims; i++)
	len *= var->super->dims[var->dimids[i]]->len;
    return len;
}

void nct_init() {
    printf("nct_init puuttuu\n");
}

nct_var* nct_load_var(nct_var* var, int ncvarid) {
    var->len = nct_get_varlen(var);
    if(!var->data)
	if(!(var->data = malloc(var->len*nctypelen(var->xtype))))
	    goto failed;
    if(ncvarid < 0)
	ncvarid = nct_get_id_thisvar(var);
    NCFUNK(nc_get_var, var->super->ncid, ncvarid, var->data);
    return var;
failed:
    fprintf(stderr, "malloc failed in nct_load_var\n");
    return var;
}

#define ONE_TYPE(nctype, form, ctype) [nctype]=#ctype,
const char* nct_typenames[] = { ALL_TYPES };
#undef ONE_TYPE

nct_any nct_varmax(nct_var* var) {
    return _varmax_anyd[var->xtype](var).a;
}

#define ONE_TYPE(nctype, idtype, ctype)				\
    ctype nct_varmax_##nctype(nct_var* var)			\
    {								\
	return nct_varmax_anyd_##nctype(var).a.idtype;		\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

nct_anyd nct_varmax_anyd(nct_var* var) {
    return _varmax_anyd[var->xtype](var);
}

#define ONE_TYPE(nctype, idtype, ctype)				\
    nct_anyd nct_varmax_anyd_##nctype(nct_var* var)		\
    {								\
	if(!(var->len=nct_get_varlen(var)))			\
	    return (nct_anyd){ {0}, -1 };			\
	size_t numi=0;						\
	ctype num = ((ctype*)var->data)[0];			\
	for(size_t i=1; i<var->len; i++)			\
	    if(num < ((ctype*)var->data)[i])			\
		num = ((ctype*)var->data)[numi=i];		\
	return (nct_anyd){ {.idtype=num}, numi };		\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

nct_any nct_varmin(nct_var* var) {
    return _varmin_anyd[var->xtype](var).a;
}

#define ONE_TYPE(nctype, idtype, ctype)				\
    ctype nct_varmin_##nctype(nct_var* var)			\
    {								\
	return nct_varmin_anyd_##nctype(var).a.idtype;		\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

nct_anyd nct_varmin_anyd(nct_var* var) {
    return _varmin_anyd[var->xtype](var);
}

#define ONE_TYPE(nctype, idtype, ctype)				\
    nct_anyd nct_varmin_anyd_##nctype(nct_var* var)		\
    {								\
	if(!(var->len=nct_get_varlen(var)))			\
	    return (nct_anyd){ {0}, -1 };			\
	size_t numi=0;						\
	ctype num = ((ctype*)var->data)[0];			\
	for(size_t i=1; i<var->len; i++)			\
	    if(num > ((ctype*)var->data)[i])			\
		num = ((ctype*)var->data)[numi=i];		\
	return (nct_anyd){ {.idtype=num}, numi };		\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

void* nct_varminmax(nct_var* var, void* resultv) {
    return _varminmax[var->xtype](var, resultv);
}

#define ONE_TYPE(nctype, b, ctype)				\
    void* nct_varminmax_##nctype(nct_var* var, void* resultv)	\
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

nct_vset* nct_move_var_tosimilar(nct_vset* dest, nct_var* srcvar) {
    puts("this does not move attributes yet");
    char* new_varname = srcvar->name;
    int freeable_name_dest = srcvar->freeable_name;
    for(int i=0; i<dest->nvars; i++)
	if(!strcmp(new_varname, dest->vars[i]->name)) {
	    if(!(new_varname = nct_find_unique_varname(dest, new_varname)))
		asm("int $3");
	    freeable_name_dest = 1;
	    goto done;
	}
    srcvar->freeable_name = 0;
done:
    nct_add_var(dest, srcvar->data, srcvar->xtype, new_varname, srcvar->ndims, srcvar->dimids);
    dest->vars[dest->nvars-1]->freeable_name = freeable_name_dest;
    srcvar->nonfreeable_data = 1;
    return dest;
}

nct_vset* nct_move_vset_tosimilar(nct_vset* dest, nct_vset* src) {
    void* vp = 0;
    if(dest->varcapacity < dest->nvars+src->nvars) {
	if(!(vp=realloc(dest->vars, (dest->varcapacity=dest->nvars+src->nvars+2)*sizeof(nct_var))))
	    goto failed;
	dest->vars = vp;
    }
    for(nct_var** v=src->vars; v<src->vars+src->nvars; v++)
	nct_move_var_tosimilar(dest, *v);
    return dest;
failed:
    dest->varcapacity = dest->nvars;
    fprintf(stderr, "realloc failed in nct_move_vset_tosimilar\n");
    return dest;
}

nct_var* nct_next_truevar(nct_var* var) {
    int len = var->super->nvars;
    for(int i=0; i<len; i++)
	if(nct_get_id_thisdim(var->super->vars[i]) < 0)
	    return var->super->vars[i];
    return NULL;
}

int nct_next_truevar_i(nct_vset* vset, int i0) {
    for(int i=i0; i<vset->nvars; i++)
	if(nct_get_id_thisdim(vset->vars[i]) < 0)
	    return i;
    return -1;
}

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

#define ONE_TYPE(nctype, form, ctype)				\
    void nct_print_##nctype(void* arr, int i, int end) {	\
	for(; i<end; i++)					\
	    printf("%"#form", ", ((ctype*)arr)[i]);		\
    }
ALL_TYPES
#undef ONE_TYPE

void nct_print_dim(nct_var* var, const char* indent) {
    printf("%s%s%s %s%s(%zu)%s:\n",
	   indent, nct_type_color, nct_typenames[var->xtype],
	   nct_dimname_color, var->name, var->len, nct_default_color);
    if(var->data) {
	printf("%s  [", indent);
	nct_print_var_data(var);
	puts("]");
    }
}

void nct_print_var(nct_var* var, const char* indent) {
    printf("%s%s%s %s%s(%zu)%s:\n%s  %i dimensions: ( ",
	   indent, nct_type_color, nct_typenames[var->xtype],
	   nct_varname_color, var->name, nct_get_varlen(var), nct_default_color,
	   indent, var->ndims);
    for(int i=0; i<var->ndims; i++) {
	nct_var* dim = var->super->dims[var->dimids[i]];
	printf("%s(%zu), ", dim->name, dim->len);
    }
    printf(")\n");
    printf("%s  [", indent);
    nct_print_var_data(var);
    puts("]");
}

void nct_print_var_data(nct_var* var) {
    void (*printfun)(void*,int,int) = _printfunctions[var->xtype];
    size_t len = nct_get_varlen(var);
    if(len <= 17) {
	printfun(var->data, 0, len);
	return;
    }
    printfun(var->data, 0, 8);
    printf(" ..., ");
    printfun(var->data, len-8, len);
}

void nct_print_vset(nct_vset* vs) {
    printf("%s%i variables, %i dimensions%s\n", nct_varset_color, vs->nvars, vs->ndims, nct_default_color);
    for(int i=0; i<vs->ndims; i++)
	nct_print_dim(vs->dims[i], "  ");
    for(int i=0; i<vs->nvars; i++) {
	for(int j=0; j<vs->ndims; j++)
	    if(vs->dims[j] == vs->vars[i])
		goto do_not_print;
	nct_print_var(vs->vars[i], "  ");
    do_not_print:;
    }
}

nct_vset* nct_read_ncfile(const char* restrict filename) {
    return nct_read_ncfile_gd(calloc(1,sizeof(nct_vset)), filename);
}

nct_vset* nct_read_ncfile_gd(nct_vset* dest, const char* restrict filename) {
    nct_read_ncfile_info_gd(dest, filename);
    for(int i=0; i<dest->nvars; i++)
	nct_load_var(dest->vars[i], i);
    NCFUNK(nc_close, dest->ncid);
    dest->ncid = -1;
    return dest;
}

nct_vset* nct_read_ncfile_gd0(nct_vset* dest, const char* restrict filename) {
    memset(dest, 0, sizeof(nct_vset));
    return nct_read_ncfile_gd(dest, filename);
}

nct_vset* nct_read_ncfile_info(const char* restrict filename) {
    return nct_read_ncfile_info_gd(calloc(1,sizeof(nct_vset)), filename);
}

nct_vset* nct_read_ncfile_info_gd(nct_vset* dest, const char* restrict filename) {
    int ncid;
    NCFUNK(nc_open, filename, NC_NOWRITE, &ncid);
    NCFUNK(nc_inq_ndims, ncid, &(dest->ndims));
    NCFUNK(nc_inq_nvars, ncid, &(dest->nvars));
    dest->ncid = ncid;
    dest->dims = calloc((dest->dimcapacity=dest->ndims+1), sizeof(void*));
    dest->vars = calloc((dest->varcapacity=dest->nvars+3), sizeof(void*));
    for(int i=0; i<dest->nvars; i++)
	_nct_read_var_info(dest, i);
    for(int i=0; i<dest->ndims; i++)
	_nct_read_dim(dest, i);
    return dest;
}

nct_vset* nct_read_ncfile_info_gd0(nct_vset* dest, const char* restrict filename) {
    memset(dest, 0, sizeof(nct_vset));
    return nct_read_ncfile_info_gd(dest, filename);
}

nct_var* nct_var_dropdim_first(nct_var* var) {
    if(!var->ndims)
	return var;
    size_t new_len = nct_get_varlen(var) / NCTVARDIM(*var,0).len;
    if(!(var->nonfreeable_data))
	var->data = realloc(var->data, new_len*nctypelen(var->xtype));
    var->len = new_len;
    int* ptr = var->dimids;
    memmove(ptr, ptr+1, --var->ndims*sizeof(int));
    return var;
}

nct_var* nct_varcpy(nct_var* src) {
    return nct_varcpy_gd(calloc(1,sizeof(nct_var)), src);
}

nct_var* nct_varcpy_gd(nct_var* dest, nct_var* src) {
    dest->super             = src->super;
    if(!dest->name) {
	dest->name          = nct_find_unique_varname(src->super, src->name);
	dest->freeable_name = 1;
    }
    dest->ndims             = src->ndims;
    dest->dimcapacity       = src->ndims+1;
    dest->dimids            = malloc(dest->dimcapacity*sizeof(int));
    dest->natts             = src->natts;
    dest->attcapacity       = src->natts;
    dest->atts              = malloc(dest->attcapacity*sizeof(nct_att));
    dest->len               = nct_get_varlen(src);
    dest->xtype             = src->xtype;
    dest->nonfreeable_data  = 0;
    dest->data              = malloc(dest->len*nctypelen(dest->xtype));
    memcpy(dest->dimids, src->dimids, src->ndims*sizeof(int));
    if(dest->xtype == NC_STRING)
	for(size_t i=0; i<dest->len; i++)
	    ((char**)dest->data)[i] = strdup(((char**)src->data)[i]);
    else
	memcpy(dest->data, src->data, dest->len*nctypelen(src->xtype));
    for(int i=0; i<dest->natts; i++) {
	nct_att* attdest = dest->atts + i;
	nct_att* attsrc = src->atts + i;
	attdest->name     = strdup(attsrc->name);
	attdest->value    = malloc(attsrc->len*nctypelen(attsrc->xtype));
	attdest->xtype    = attsrc->xtype;
	attdest->len      = attsrc->len;
	attdest->freeable = 3;
	memcpy(attdest->value, attsrc->value, attsrc->len*nctypelen(attsrc->xtype));
    }
    return dest;
}

nct_var* nct_vardup(nct_var* src, char* name) {
    nct_var* v = calloc(1, sizeof(nct_var));
    v->name = name;
    return nct_assign_var(nct_varcpy_gd(v, src));
}

nct_var* nct_varmean_first(nct_var* var) {
    return _varmean_first[var->xtype](var);
}

#define ONE_TYPE(nctype, a, ctype)					\
    nct_var* nct_varmean_first_##nctype(nct_var* var)			\
    {									\
	size_t zerolen = NCTVARDIM(*var,0).len;				\
	size_t new_len = nct_get_varlen(var) / zerolen;			\
	for(size_t i=0; i<new_len; i++) {				\
	    for(size_t j=1; j<zerolen; j++)				\
		((ctype*)var->data)[i] += ((ctype*)var->data)[i+new_len*j]; \
	    ((ctype*)var->data)[i] /= zerolen;				\
	}								\
	return nct_var_dropdim_first(var);				\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

nct_var* nct_varmeannan_first(nct_var* var) {
    return _varmeannan_first[var->xtype](var);
}

#define ONE_TYPE(nctype, a, ctype)				\
    nct_var* nct_varmeannan_first_##nctype(nct_var* var)	\
    {								\
	size_t zerolen = NCTVARDIM(*var,0).len;			\
	size_t new_len = nct_get_varlen(var) / zerolen;		\
	for(size_t i=0; i<new_len; i++) {			\
	    int count = 0;					\
	    ctype new_value = 0;				\
	    for(size_t j=0; j<zerolen; j++) {			\
		ctype test = ((ctype*)var->data)[i+new_len*j];	\
		if(test==test) {				\
		    count++;					\
		    new_value += test;				\
		}						\
	    }							\
	    ((ctype*)var->data)[i] = new_value/count;		\
	}							\
	return nct_var_dropdim_first(var);			\
    }
ALL_TYPES_EXCEPT_STRING
#undef ONE_TYPE

nct_vset* nct_vset_isel(nct_vset* vset, int dimid, size_t ind0, size_t ind1) {
    if(ind0 > vset->dims[dimid]->len)
	ind1 = ind0 = vset->dims[dimid]->len;
    else if(ind1 > vset->dims[dimid]->len)
	ind1 = vset->dims[dimid]->len;
    for(int i=0; i<vset->nvars; i++)
	*vset->vars[i] = *(_nct_var_isel(vset->vars[i], dimid, ind0, ind1));
    vset->dims[dimid]->len = ind1-ind0;
    return vset;
}

nct_vset* nct_vsetcpy(const nct_vset* src) {
    nct_vset* dest = calloc(1, sizeof(nct_vset));
    return nct_vsetcpy_gd(dest, src);
}

nct_vset* nct_vsetcpy_gd(nct_vset* dest, const nct_vset* src) {
    /*vars*/
    dest->nvars = src->nvars;
    dest->vars = malloc((dest->varcapacity=src->nvars+1)*sizeof(void*));
    for(int i=0; i<dest->nvars; i++)
	(dest->vars[i] = nct_varcpy(src->vars[i])) -> super = dest;
    /*dims*/
    dest->dims = malloc((dest->dimcapacity=src->ndims+1)*sizeof(void*));
    dest->ndims = 0;
    for(int i=0; i<src->ndims; i++) {
	int id = nct_get_id_thisvar(src->dims[i]);
	if(id < 0) {
	    src->vars[id]->len = nct_get_varlen(src->vars[id]);
	    nct_add_dim(dest, src->vars[id]->data, src->vars[id]->len, src->vars[id]->xtype,
			strdup(src->vars[id]->name))->freeable_name = 1;
	} else
	    dest->dims[i] = dest->vars[id];
    }
    dest->ndims = src->ndims;
    return dest;
}

void nct_write_ncfile(const nct_vset* src, const char* name) {
    int ncid, id;
    NCFUNK(nc_create, name, NC_NETCDF4|NC_CLOBBER, &ncid);
    for(int i=0; i<src->ndims; i++)
	NCFUNK(nc_def_dim, ncid, src->dims[i]->name, src->dims[i]->len, &id);
    for(int i=0; i<src->nvars; i++) {
	nct_var* v = src->vars[i];
	NCFUNK(nc_def_var, ncid, v->name, v->xtype, v->ndims, v->dimids, &id);
	NCFUNK(nc_put_var, ncid, id, v->data);
	for(int a=0; a<v->natts; a++)
	    NCFUNK(nc_put_att_text, ncid, i, v->atts[a].name, strlen(v->atts[a].value)+1, v->atts[a].value);
    }
    NCFUNK(nc_close, ncid);
}
