testi.out: testi.c nctietue.c nctietue.h nct_png.c nct_sdl2.c
	gcc -g -Wall -o testi.out testi.c -lnetcdf -lpng -lSDL2
