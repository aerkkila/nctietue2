#include "nctietue.c"
#include "nct_png.c"

int main(int argc, char** argv) {
  int xdata[5] = {0};
  nct_vset vset = {0};
  nct_add_coord(&vset, xdata, 5, NC_INT, "x");
  nct_print_vset(&vset);
  vset.vars[0].data = NULL;
  nct_free_vset(&vset);

  nct_open_png_gd(&vset, "./kuva.png");
  nct_print_vset(&vset);
  nct_write_ncfile(&vset, "./kuva.nc");
  nct_free_vset(&vset);

  //nct_init();
  nct_vset* vs = nct_read_ncfile("./kuva.nc");
  nct_vset* uusi = nct_vsetcpy(vs);
  vs->vars[2].name[2] = '9';

  nct_add_coord(uusi, nct_range_NC_UBYTE(0,10,1), 10, NC_UBYTE, "z");
  nct_vset_isel(uusi, nct_vset_get_dimid(uusi, "y"), 500, 1200);
  nct_vset_isel(uusi, nct_vset_get_dimid(uusi, "x"), 100, 400);
  nct_print_vset(uusi);
  nct_write_ncfile(uusi, "./kuva_rajattu.nc");

  nct_free_vset(vs);
  nct_free_vset(uusi);
  free(uusi);
  free(vs);
  return 0;
}
