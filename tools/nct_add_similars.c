#include <nctietue/nctietue.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

void one_argument() {
  char* lang = getenv("LANG");
  lang[2] = '\0';
#define MESSAGE(msg) fprintf(stderr, msg)
  if(!strcmp(lang,"fi"))
    MESSAGE("Ei lisättäviä tiedostoja\n");
  else
    MESSAGE("No files to add\n");
#undef MESSAGE
  exit(EXIT_FAILURE);
}

int main(int argc, char** argv) {
  if(argc<2)
    no_argument();
  if(argc<3)
    one_argument();
  nct_vset vset0, vset;
  nct_read_ncfile_gd(&vset0, argv[1]);
  for(int i=2; i<argc; i++) {
    nct_read_ncfile_gd(&vset, argv[i]);
    nct_move_similar_vset(&vset0, &vset);
    nct_free_vset(&vset);
  }
  nct_write_ncfile(&vset0, argv[1]);
  nct_free_vset(&vset0);
  return 0;
}
