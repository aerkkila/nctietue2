#include "nctietue.c"

int main(int argc, char** argv) {
  variable_set* vs = read_ncfile("../koodit/köppen.nc", NULL);
  print_variable_set(vs);
  putchar('\n');
  variable_set* uusi = vsetcpy(NULL, vs);
  vs->vars[3].name[2] = '9';
  var_pluseq_var(var_from_vset(vs, "lat"), var_from_vset(vs, "lon"));
  putchar('\n');
  print_variable_set(uusi);
  putchar('\n');
  print_variable_set(vs);
  free_variable_set(vs);
  free_variable_set(uusi);
  free(uusi);
  free(vs);
  return 0;
}
