include config.mk

libnctietue.so: nct*.[ch] operations_and_types.h
	gcc -Wall -shared -fpic -g3 -gdwarf-2 -o $@ nct*.c

libnctietue0.so: nct*.[ch] operations_and_types.h
	gcc -Wall -shared -fpic -o $@ nct*.c -O3

install: libnctietue.so
	mkdir -p ${prefix}/include/nctietue
	cp nct*.h operations_and_types.h ${prefix}/include/nctietue/
	cp libnctietue.so ${prefix}/lib/

uninstall:
	rm -rf ${prefix}/lib/libnctietue.so ${prefix}/include/nctietue

testi.out: testi.c
	gcc -g -Wall -o $@ testi.c -lnctietue -lSDL2 -lpng -lnetcdf
