include config.mk
cfiles=nctietue2.c nct_png.c nct_sdl2.c
hfiles=nct2.h nct_operations_and_types.h

libnctietue2.so: ${cfiles} ${hfiles}
	gcc -Wall -shared -fpic -g3 -gdwarf-2 -o $@ ${cfiles} -Og

nctietue2.pc: nctietue2.pc.1 config.mk
	cat config.mk nctietue2.pc.1 > nctietue2.pc

install: libnctietue2.so nctietue2.pc ${hfiles}
	cp ${hfiles} ${includedir}
	cp libnctietue2.so ${libdir}
	cp nctietue2.pc /usr/lib/pkgconfig/

uninstall:
	rm ${libdir}/libnctietue2.so
	rm $(addprefix ${includedir}/, ${hfiles})
	rm /usr/lib/pkgconfig/nctietue2.pc

.PHONY: uninstall



testi.out: testi.c
	gcc -g -Wall -o $@ testi.c `pkg-config --libs nctietue2`

gtktesti.out: nct_gtk.c
	gcc -Wall -o $@ nct_gtk.c -DGTKTESTI `pkg-config --cflags --libs glib-2.0 gtk4 nctietue2`

gtktesti: gtktesti.out
	./gtktesti.out
