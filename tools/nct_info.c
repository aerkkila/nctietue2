#include "../nctietue.c"
#include <stdlib.h>

void no_argument() {
  char* lang = getenv("LANG");
  lang[2] = '\0';
#define MESSAGE(msg) fprintf(stderr, msg)
  if(!strcmp(lang,"fi"))
    MESSAGE("Ei luettavia tiedostoja\n");
  else
    MESSAGE("No files to read\n");
#undef MESSAGE
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  if(argc < 2)
    no_argument();
  nct_vset vset;
  int halflen = 8;
  char dataarr[8*2*halflen];
  for(int i=1; i<argc; i++) {
    puts(argv[i]);
    nct_read_ncfile_info_gd(&vset, argv[i]);
    nct_print_vset_title(&vset);
    for(int varid=0; varid<vset.nvars; varid++) {
      nct_var* var = vset.vars+varid;
      var->data = dataarr;
      if(nct_getlen(&vset, varid) <= 2*halflen)
	nct_load_var(&vset, varid);
      else {
	nct_nload_var(&vset, varid, 0, halflen);
	nct_nload_var(&vset, varid, nct_getlen(&vset, varid)-halflen, halflen);
      }
      nct_print_var(&vset, varid, "  ");
      var->data = NULL;
    }
    NCFUNK(nc_close, vset.ncid);
    nct_free_vset(&vset);
    memset(&vset, 0, sizeof(nct_vset));
  }
  return 0;
}
