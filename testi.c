#include "nctietue.c"

int main(int argc, char** argv) {
  int *xdata = calloc(5,sizeof(int));
  nct_dim* x = to_nct_coord(NULL, xdata, 5, NC_INT, strdup("x"));
  free_nct_coord(x);
  free(x);

  return 0;
  nct_init();
  nct_vset* vs = read_ncfile("../koodit/köppen.nc", NULL);
  print_nct_vset(vs);
  putchar('\n');
  nct_vset* uusi = nct_vsetcpy(NULL, vs);
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
