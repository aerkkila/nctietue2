testi.out: testi.c nctietue.c nctietue.h
	gcc -g -o testi.out testi.c -lnetcdf
