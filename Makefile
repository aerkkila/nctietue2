include config.mk

testi.out: testi.c
	gcc -g -Wall -o $@ testi.c -lnctietue -lSDL2 -lpng -lnetcdf

libnctietue.so: nct*.[ch] operations_and_types.h
	gcc -Wall -shared -fpic -g3 -gdwarf-2 -o $@ nct*.c

install: libnctietue.so
	mkdir -p ${prefix}/include/nctietue
	cp nct*.h operations_and_types.h ${prefix}/include/nctietue/
	cp libnctietue.so ${prefix}/lib/x86_64-linux-gnu/

uninstall:
	rm -rf ${prefix}/lib/x86_64-linux-gnu/libnctietue.so ${prefix}/include/nctietue
