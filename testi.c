#include "nctietue.c"

int main(int argc, char** argv) {
  int xdata[5] = {0};
  nct_dim x = {0};
  to_nct_coord_gd(&x, xdata, 5, NC_INT, "x");
  print_nct_var(x.coordv, "");
  /*poistetaan viitteet pinomuistiin ennen vapautusfunktion kutsumista*/
  x.name = NULL;
  x.coordv->name = NULL;
  x.coordv->data = NULL;
  free_nct_coord(&x);

  return 0;
  nct_init();
  nct_vset* vs = read_ncfile("../koodit/köppen.nc");
  print_nct_vset(vs);
  putchar('\n');
  nct_vset* uusi = nct_vsetcpy(vs);
  vs->vars[3].name[2] = '9';
  vararr_pluseq(var_from_vset(vs, "lat"), var_from_vset(vs, "lon")->data);
  putchar('\n');
  print_nct_vset(uusi);
  putchar('\n');
  print_nct_vset(vs);
  free_nct_vset(vs);
  free_nct_vset(uusi);
  free(uusi);
  free(vs);
  return 0;
}
