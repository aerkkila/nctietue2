#include <nctietue2.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

char *filename, *varname;

int filename_from_dir(const char* restrict dirname) {
    DIR *d;
    struct dirent *dir;
    if(!(d = opendir(dirname))) {
	perror(dirname);
	return 1;
    }
    errno = 0;
    while(1) {
	if(!(dir = readdir(d))) {
	    if(!errno)
		break;
	    perror("readdir");
	}
	int len = strlen(dir->d_name);
	if(len > 2 && !strcmp(dir->d_name+len-3, ".nc")) {
	    filename = strdup(dir->d_name);
	    break;
	}
    }
    closedir(d);
    return 0;
}

int args(int argc, char** argv) {
    if(argc < 2)
	return 1;
    for(int a=1; a<argc; a++)
	if(!strcmp(argv[a], "-d")) {
	    if(a+1>=argc) return 1;
	    filename_from_dir(argv[++a]);
	} else if (!strcmp(argv[a], "-v")) {
	    if(a+1>=argc) return 1;
	    varname = argv[++a];
	} else
	    filename = strdup(argv[a]);
    return 0;
}

int main(int argc, char** argv) {
    nct_var* var;
    args(argc, argv);
    if(!filename)
	filename_from_dir(".");
    if(!filename)
	return 1;
    puts(filename);

    nct_vset* vset = nct_read_ncfile_info(filename);
    if(varname)
	var = &NCTVAR(*vset, varname);
    else
	var = nct_next_truevar(vset->vars[0]);
    if(!var) {
	NCFUNK(nc_close, vset->ncid);
	nct_free_vset(vset);
	return 1;
    }
    puts(var->name);
    nct_load_var(var, nct_get_id_thisvar(var));
    NCFUNK(nc_close, vset->ncid);
    nct_plot_var(var);
    nct_free_vset(vset);
    free(filename);
    filename = NULL;
}
