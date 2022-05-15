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
  putchar('\n');
  nct_print_vset(uusi);
  putchar('\n');
  nct_print_vset(vs);
  nct_free_vset(vs);
  nct_free_vset(uusi);
  free(uusi);
  free(vs);
  return 0;
}
