include config.mk

testi.out: testi.c
	gcc -g -Wall -o $@ testi.c -lnctietue -lSDL2 -lpng -lnetcdf

libnctietue.so: nct*.[ch] operations_and_types.h
	gcc -Wall -g3 -gdwarf-2 -o $@ nct*.c -shared -fpic -lSDL2 -lpng -lnetcdf

install: libnctietue.so
	mkdir -p ${prefix}/include/nctietue
	cp nct*.h operations_and_types.h ${prefix}/include/nctietue/
	cp libnctietue.so ${prefix}/lib/

uninstall:
	rm -rf ${prefix}/lib/libnctietue.so ${prefix}/include/nctietue
