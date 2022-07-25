ctype nct_varmax_##nctype(nct_var* var) {
    return nct_varmax_anyd_##nctype(var).a.form;
}

nct_anyd nct_varmax_anyd_##nctype(nct_var* var) {
    if(!(var->len=nct_get_varlen(var)))
	return (nct_anyd){ {0}, -1 };
#if __nctype__==NC_FLOAT || __nctype__==NC_DOUBLE
    size_t numi=0;
    ctype num = -INFINITY;
    for(int i=0; i<var->len; i++)
#else
    size_t numi=0;
    ctype num = ((ctype*)var->data)[0];
    for(size_t i=1; i<var->len; i++)
#endif
	if(num < ((ctype*)var->data)[i])
	    num = ((ctype*)var->data)[numi=i];
    return (nct_anyd){ {.form=num}, numi };
}

ctype nct_varmin_##nctype(nct_var* var) {
    return nct_varmin_anyd_##nctype(var).a.form;
}

nct_anyd nct_varmin_anyd_##nctype(nct_var* var) {
    if(!(var->len=nct_get_varlen(var)))
	return (nct_anyd){ {0}, -1 };
    /* using the first value would not work with nan-values */
#if __nctype__==NC_FLOAT || __nctype__==NC_DOUBLE
    size_t numi=0;
    ctype num = INFINITY;
    for(int i=0; i<var->len; i++)
#else
    size_t numi=0;
    ctype num = ((ctype*)var->data)[0];
    for(size_t i=1; i<var->len; i++)
#endif
	if(num > ((ctype*)var->data)[i])
	    num = ((ctype*)var->data)[numi=i];
    return (nct_anyd){ {.form=num}, numi };
}

void* nct_varminmax_##nctype(nct_var* var, void* resultv) {
    ctype maxval, minval, *result=resultv;
    /* using the first value would not work with nan-values */
#if __nctype__==NC_FLOAT || __nctype__==NC_DOUBLE
    maxval = -INFINITY;
    minval = INFINITY;
    for(int i=0; i<var->len; i++)
#else
    maxval = minval = ((ctype*)var->data)[0];
    for(int i=1; i<var->len; i++)
#endif
	if(maxval < ((ctype*)var->data)[i])
	    maxval = ((ctype*)var->data)[i];
	else if(minval > ((ctype*)var->data)[i])
	    minval = ((ctype*)var->data)[i];
    result[0] = minval;
    result[1] = maxval;
    return result;
}

ctype* nct_range_##nctype(ctype i0, ctype i1, ctype gap) {
    size_t len = (i1-i0)/gap;
    while(i0+len*gap < i1) len++;
    ctype* dest = malloc(len*sizeof(ctype));
    ctype num = i0-gap;
    for(size_t i=0; i<len; i++)
	dest[i] = num+=gap;
    return dest;
}

nct_var* nct_varmean_first_##nctype(nct_var* var) {
    size_t zerolen = NCTVARDIM(*var,0).len;
    size_t new_len = nct_get_varlen(var) / zerolen;
    for(size_t i=0; i<new_len; i++) {
	for(size_t j=1; j<zerolen; j++)
	    ((ctype*)var->data)[i] += ((ctype*)var->data)[i+new_len*j];
	((ctype*)var->data)[i] /= zerolen;
    }
    return nct_var_dropdim_first(var);
}

nct_var* nct_varmeannan_first_##nctype(nct_var* var) {
#if __nctype__==NC_FLOAT || __nctype__==NC_DOUBLE
    size_t zerolen = NCTVARDIM(*var,0).len;
    size_t new_len = nct_get_varlen(var) / zerolen;
    for(size_t i=0; i<new_len; i++) {
	int count = 0;
	ctype new_value = 0;
	for(size_t j=0; j<zerolen; j++) {
	    ctype test = ((ctype*)var->data)[i+new_len*j];
	    if(test==test) {
		count++;
		new_value += test;
	    }
	}
	((ctype*)var->data)[i] = new_value/count;
    }
    return nct_var_dropdim_first(var);
#else
    return nct_varmean_first_##nctype(var);
#endif
}
