
static void _nct_read_dim(nct_vset* vset, int dimid) {
/* _nct_read_var_info must be called first for all variables*/
    char name[256];
    size_t len;
    int varid;
    nct_var** v = vset->dims + dimid;
    NCFUNK(nc_inq_dim, vset->ncid, dimid, name, &len);
    if((varid=nct_get_varid(vset, name)) >= 0) {
	*v = vset->vars[varid];
	return;
    }
    *v = calloc(1, sizeof(nct_var));
    **v = (nct_var) {
	.super         = vset,
	.name          = strdup(name),
	.freeable_name = 1,
	.len           = len,
    };
    return;
}

static nct_vset* _nct_read_var_info(nct_vset *vset, int varid) {
    int ndims, dimids[128], natts;
    nc_type xtype;
    size_t len;
    char name[512];
    NCFUNK(nc_inq_var, vset->ncid, varid, name, &xtype, &ndims, dimids, &natts);
    nct_var* dest = vset->vars[varid];
    dest->super            = vset;
    dest->name             = strdup(name);
    dest->freeable_name    = 1;
    dest->ndims            = ndims;
    dest->dimcapacity      = ndims+1;
    dest->dimids           = malloc(dest->dimcapacity*sizeof(int));
    dest->natts            = natts;
    dest->attcapacity      = natts;
    dest->atts             = malloc(dest->attcapacity*sizeof(nct_att));
    dest->len              = -1;
    dest->xtype            = xtype;
    dest->nonfreeable_data = 0;
    dest->data             = NULL;
    memcpy(dest->dimids, dimids, ndims*sizeof(int));
    for(int i=0; i<natts; i++) {
	nct_att* att= dest->atts+i;
	NCFUNK(nc_inq_attname, vset->ncid, varid, i, name);
	NCFUNK(nc_inq_att, vset->ncid, varid, name, &xtype, &len);
	att->name     = strdup(name);
	att->value    = malloc(len*nctypelen(xtype) + xtype==NC_STRING);
	att->xtype    = xtype;
	att->len      = len;
	att->freeable = 3;
	NCFUNK(nc_get_att, vset->ncid, varid, name, att->value);
	if(att->xtype == NC_STRING) {
	    if(((char*)att->value)[len-1] != '\0')
		att->len++;
	    ((char*)att->value)[len-1] = '\0';
	}
    }
    return vset;
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
    size_t len_after = nctypelen(var->xtype); //interval to step given coordinate
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
    var->len = blocklen / nctypelen(var->xtype) * n_blocks;
    return var;
}

#define ONE_TYPE(nctype, ...) [nctype]=nct_print_##nctype,
void (*_printfunctions[])(void*, int, int) = { ALL_TYPES };
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_varmean0_##nctype,
nct_var* (*_varmean0[])(nct_var*) = { ALL_TYPES_EXCEPT_STRING };
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_varminmax_##nctype,
void* (*_varminmax[])(nct_var*, void*) = { ALL_TYPES_EXCEPT_STRING };
#undef ONE_TYPE

#define ONE_TYPE(nctype, a, b) [nctype]=nct_varnanmean0_##nctype,
nct_var* (*_varnanmean0[])(nct_var*) = { ALL_TYPES_EXCEPT_STRING };
#undef ONE_TYPE
