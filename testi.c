#include "nctietue.c"

int main(int argc, char** argv) {
  variable_set* vs = read_ncfile("../koodit/köppen.nc", NULL);
  print_variable_set(vs);
  free_variable_set(vs);
  return 0;
}
