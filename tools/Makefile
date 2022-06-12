include ../config.mk
tiedostot=nct_info nct_add_similars

all: ${tiedostot}

nct_info: nct_info.c
	gcc -g -Og -o $@ nct_info.c `pkg-config --libs nctietue`

nct_add_similars: nct_add_similars.c
	gcc -g -Og -o $@ nct_add_similars.c `pkg-config --libs nctietue`

install: ${tiedostot}
	cp ${tiedostot} ${bindir}/

uninstall:
	rm $(addprefix ${bindir}/, ${tiedostot})

.PHONY: uninstall
