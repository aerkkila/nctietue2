include config.mk
cfiles=nctietue.c nct_png.c nct_sdl2.c
hfiles=nctietue.h nct_operations_and_types.h

libnctietue.so: ${cfiles} ${hfiles}
	gcc -Wall -shared -fpic -g3 -gdwarf-2 -o $@ ${cfiles} -Og

libnctietue0.so: ${cfiles} ${hfiles}
	gcc -Wall -shared -fpic -o $@ ${cfiles} -O3

nctietue.pc: nctietue.pc.1 config.mk
	cat config.mk nctietue.pc.1 > nctietue.pc

install: libnctietue.so nctietue.pc ${hfiles}
	cp ${hfiles} ${includedir}
	cp libnctietue.so ${libdir}
	cp nctietue.pc /usr/lib/pkgconfig/

uninstall:
	rm ${libdir}/libnctietue.so
	rm $(addprefix ${includedir}/, ${hfiles})
	rm /usr/lib/pkgconfig/nctietue.pc

.PHONY: uninstall



testi.out: testi.c
	gcc -g -Wall -o $@ testi.c -lnctietue -lSDL2 -lpng -lnetcdf

gtktesti.out: nct_gtk.c
	gcc -Wall -o $@ nct_gtk.c -DGTKTESTI `pkg-config --cflags --libs glib-2.0 gtk4 nctietue`

gtktesti: gtktesti.out
	./gtktesti.out
