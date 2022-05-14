testi.out: testi.c nctietue.c nctietue.h nct_png.c
	gcc -g -o testi.out testi.c -lnetcdf -lpng
