#include <nctietue2.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void t_create_and_write() {
    int dimids[4], data0[202];
    char *data1;
    nct_vset vset = {0};
    nct_var* apuvar;
    srand(1234);

    nct_add_dim(&vset, nct_range_NC_FLOAT(-10, 10.11, 0.1), 202, NC_FLOAT, "xdim");

    for(int i=0; i<202; i++)
	data0[i] = rand() % 32;
    dimids[0] = 0;
    nct_add_var(&vset, data0, NC_INT, "data0", 1, dimids)->nonfreeable_data = 1;

    nct_add_dim(&vset, nct_range_NC_FLOAT(24, 64.5, 0.5), 80, NC_FLOAT, "ydim");

    data1 = malloc(202*80);
    for(int i=0; i<202*80; i++)
	data1[i] = i/202 + 5*(i%202);
    dimids[1] = 1;
    nct_add_var(&vset, data1, NC_BYTE, "data1", 2, dimids);

    nct_print_vset(&vset);

    nct_add_varatt_text(&NCTVAR(vset,"data1"), "numero nolla", strdup("Добро пожоловать"), 1);
    nct_add_varatt_text(&NCTVAR(vset,"data1"), "numero yksi", "Tämä on tekstiä", 0);

    dimids[0] = 1; dimids[1] = 0;
    (apuvar=nct_add_var(&vset, data1, NC_UBYTE, nct_find_unique_varname(&vset, "data"), 2, dimids))
	-> freeable_name = 1;
    apuvar->nonfreeable_data = 1;

    nct_print_vset(&vset);

    nct_write_ncfile(&vset, "testi.nc");
    nct_free_vset(&vset);
    printf("\033[7;1mt_create_and_write valmis\033[0m\n");
}

void t_read_and_copy() {
    nct_vset a={0}, b={0};
    nct_read_ncfile_gd(&a, "testi.nc");
    printf(" -----------a---------- \n"
	   " ––––––––––aaa––––––––– \n");
    nct_print_vset(&a);
    printf(" -----------b---------- \n"
	   " ––––––––––bbb––––––––– \n");
    nct_vsetcpy_gd(&b, &a);
    nct_print_vset(&b);
    nct_write_ncfile(&b, "testi1.nc");
    nct_free_vset(&a);
    nct_free_vset(&b);
    printf("\033[7;1mt_read_and_copy valmis\033[0m\n");
}

void t_operations() {
    nct_vset a    = {0};
    float af[8]   = {14, 1, -31.1, 20, 0, 72.1,  -60.8, 32.99};
    double ad[8]  = {14, 1, -31.1, 20, 0, 0.0/0, -60.8, 32.99};
    int ai[8]     = {14, 1, -31,   20, 0, 72,    -61,   33};
    char* nimet[] = {"Xäöþ", "Yöξκ"};
    size_t pit[]  = {4, 2};
    int dimids[]  = {0, 1};
    nct_add_var_(&a, af, NC_FLOAT,  "liukuluku", 2, NULL,   pit,  nimet)->nonfreeable_data = 1;
    nct_add_var_(&a, ai, NC_INT,    "kokonluku", 2, NULL,   NULL, nimet)->nonfreeable_data = 1;
    nct_add_var_(&a, ad, NC_DOUBLE, "epäluku",   2, dimids, NULL, NULL)->nonfreeable_data = 1;
    nct_print_vset(&a);
    nct_vardup(&NCTVAR(a, "liukuluku"), "dropliuku");
    nct_vardup(&NCTVAR(a, "kokonluku"), NULL);
    nct_var_dropdim0(&NCTVAR(a, "dropliuku"));
    //nct_varmean0(&NCTVAR(a, "liukuluku2"));
    //nct_varmean0(&NCTVAR(a, "kokonluku"));
    //nct_vardup(&NCTVAR(a, "epäluku"))->name = "epäluku2";
    //nct_varmean0(&NCTVAR(a, "epäluku"));
    //nct_varnanmean0(&NCTVAR(a, "epäluku2"));
    printf("––––––––––––––––––––");
    nct_print_vset(&a);
    nct_free_vset(&a);
    
    printf("\033[7;1mt_operations valmis\033[0m\n");
}

int main(int argc, char** argv) {
    //t_create_and_write();
    //t_read_and_copy();
    t_operations();
    return 0;
}
