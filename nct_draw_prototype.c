#define A echo_highlight
#define B nct_default_color
static void draw2d_echo_##nctype(ctype minmax[]) {
    if(!has_echoed++)
	for(int i=0; i<5; i++)
	    putchar('\n');
    printf("\033[5F%s%s%s: min %s%" #form "%s, max %s%" #form "%s\033[K\n"
	   "x: %s%s(%zu)%s, y: %s%s(%zu)%s\033[K\033[s\n"
	   "minshift %s%.4f%s, maxshift %s%.4f%s\033[K\n"
	   "space = %s%.4f%s\033[K\n"
	   "colormap = %s%s%s\033[K\n",
	   A,var->name,B, A,minmax[0],B, A,minmax[1],B,
	   A,NCTVARDIM(*var, xid).name,NCTVARDIM(*var, xid).len,B,
	   A,NCTVARDIM(*var, yid).name,NCTVARDIM(*var, yid).len,B,
	   A,minshift,B, A,maxshift,B,
	   A,space,B, A,colormaps[cmapnum*2+1],B);
    if(zid>=0)
	printf("\033[u z: %s%s(%i/%zu)%s\n\033[2B\n",
	       A,NCTVARDIM(*var, zid).name,znum+1,NCTVARDIM(*var, zid).len,B);
}
#undef A
#undef B

static void draw2d_##nctype(nct_var* var) {
    int xlen = NCTVARDIM(*var, xid).len;
    float di=0, dj=0;
    ctype minmax[2], range;
    nct_varminmax_##nctype(var, minmax);
    range = minmax[1]-minmax[0];
    minmax[0] += range*minshift;
    minmax[1] += range*maxshift;
    draw2d_echo_##nctype(minmax);

    size_t offset = znum*stepsize_z;
    SDL_SetRenderDrawColor(rend, color_bg[0], color_bg[1], color_bg[2], 255);
    SDL_RenderClear(rend);
    if(invert_y) {
	for(int j=draw_h-1; j>=0; j--, dj+=space) {
	    for(int i=0; i<draw_w; i++, di+=space) {
		ctype val = ((ctype*)var->data)[offset + (size_t)dj*xlen + (size_t)di];
#if __nctype__==NC_DOUBLE || __nctype__==NC_FLOAT
		if(val != val)
		    continue;
#endif
		int value = ( val <  minmax[0]? 0   :
			      val >= minmax[1]? 255 :
			      (val - minmax[0]) * 255 / (minmax[1]-minmax[0]) );
		if(invert_c) value = 255-value;
		char* c = COLORVALUE(cmapnum,value);
		SDL_SetRenderDrawColor(rend, c[0], c[1], c[2], 255);
		SDL_RenderDrawPoint(rend, i, j);
	    }
	    di = 0;
	}
    } else {
	for(int j=0; j<draw_h; j++, dj+=space) {
	    for(int i=0; i<draw_w; i++, di+=space) {
		ctype val = ((ctype*)var->data)[offset + (size_t)dj*xlen + (size_t)di];
#if __nctype__==NC_DOUBLE || __nctype__==NC_FLOAT
		if(val != val)
		    continue;
#endif
		int value = ( val <  minmax[0]? 0   :
			      val >= minmax[1]? 255 :
			      (val - minmax[0]) * 255 / (minmax[1]-minmax[0]) );
		if(invert_c) value = 255-value;
		char* c = COLORVALUE(cmapnum,value);
		SDL_SetRenderDrawColor(rend, c[0], c[1], c[2], 255);
		SDL_RenderDrawPoint(rend, i, j);
	    }
	    di = 0;
	}
    }
    draw_colormap();
}
