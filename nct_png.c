#include <png.h>
#include <errno.h>
#include <stdlib.h>
#include "nctietue2.h"

nct_vset* nct_open_png_gd(nct_vset* dest, char* name) {
    FILE* filein = fopen(name, "r");
    if(!filein) {
	perror("nct_open_png_gd");
	return NULL;
    }
    png_structp png_p = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if(!png_p) {
	fclose(filein);
	return NULL;
    }
    png_infop info_p = png_create_info_struct(png_p);
    if(!info_p) {
	fclose(filein);
	png_destroy_read_struct(&png_p, (png_infopp)NULL, (png_infopp)NULL);
	return NULL;
    }

    png_init_io(png_p, filein);
    png_read_info(png_p, info_p);

    unsigned width, height;
    int bit_depth, color_type, interlace_type;
    png_get_IHDR(png_p, info_p, &width, &height, &bit_depth, &color_type, &interlace_type, NULL, NULL);
    if(interlace_type != PNG_INTERLACE_NONE) {
	fprintf(stderr, "png_interlace_type must be PNG_INTERLACE_NONE\n");
	return dest;
    }
    if(color_type == PNG_COLOR_TYPE_RGB || color_type == PNG_COLOR_TYPE_RGB_ALPHA)
	png_set_rgb_to_gray_fixed(png_p, 1, -1, -1);
    png_read_update_info(png_p, info_p);

    int pixelsize = 1 + (bit_depth==16);
    int rowlen = pixelsize*width;
    png_bytep data = malloc(pixelsize*width*height);
    for(int i=0; i<height; i++)
	png_read_row(png_p, data+i*rowlen, NULL);
  
    fclose(filein);
    png_destroy_read_struct(&png_p, &info_p, NULL);

    int dimids[] = { -1, -1 };
    size_t dimlens[] = { height, width };
    char* dimnames[] = { "fig_y", "fig_x" };
    nct_add_var_dimids(dest, data, NC_UBYTE, "data", 2, dimids, dimlens, dimnames);
    return dest;
}
