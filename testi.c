#include "nctietue.c"

int main(int argc, char** argv) {
  nct_init();
  nct_vset* vs = read_ncfile("../koodit/köppen.nc", NULL);
  print_nct_vset(vs);
  putchar('\n');
  nct_vset* uusi = nct_vsetcpy(NULL, vs);
  vs->vars[3].name[2] = '9';
  varvar_pluseq(var_from_vset(vs, "lat"), var_from_vset(vs, "lon"));
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
