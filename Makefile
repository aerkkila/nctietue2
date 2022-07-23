include config.mk
cfiles=nctietue2.c internals.h nct_png.c nct_sdl2.c nct_functions.h colormaps.h
hfiles=nctietue2.h

libnctietue2.so: ${cfiles} nctietue2.h
	gcc -Wall -shared -fpic -g3 -gdwarf-2 -o $@ nctietue2.c -Og

nctietue2.pc: nctietue2.pc.1 config.mk
	cat config.mk nctietue2.pc.1 > nctietue2.pc

nct_functions.h nct_draw.h: nct_functions_make.pl nct_functions_prototype.c nct_draw_prototype.c
	./nct_functions_make.pl

install: libnctietue2.so nctietue2.pc ${hfiles}
	cp ${hfiles} ${includedir}
	cp libnctietue2.so ${libdir}
	cp nctietue2.pc /usr/lib/pkgconfig/

uninstall:
	rm ${libdir}/libnctietue2.so
	rm $(addprefix ${includedir}/, ${hfiles})
	rm /usr/lib/pkgconfig/nctietue2.pc

.PHONY: uninstall

colormaps.h: colormaps_h_make.sh colormaps/
	./colormaps_h_make.sh

testi.out: testi.c ${hfiles}
	gcc -g3 -gdwarf-2 -Wall -o $@ testi.c `pkg-config --libs nctietue2`

gtktesti.out: nct_gtk.c
	gcc -g -Wall -o $@ nct_gtk.c -DGTKTESTI `pkg-config --cflags --libs glib-2.0 gtk4 nctietue2`

gtktesti: gtktesti.out
	./gtktesti.out
