#include "nctietue.c"

int main(int argc, char** argv) {
  int *latdata = malloc(5*sizeof(int));
  for(int i=0; i<5; i++)
    latdata[i] = i*i;
  nct_dim* lat = to_nct_coord(latdata, 5, NC_INT, strdup("lat"));
  print_nct_var(lat->coord, "");
  free_nct_var(lat->coord);
  free_nct_dim(lat);
  free(lat->coord);
  free(lat);

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
