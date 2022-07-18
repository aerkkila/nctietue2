include ../config.mk
tiedostot=nct_plot

all: ${tiedostot}

nct_info: nct_info.c
	gcc -g -Og -o $@ nct_info.c `pkg-config --libs nctietue2`

nct_add_similars: nct_add_similars.c
	gcc -g -Og -o $@ nct_add_similars.c `pkg-config --libs nctietue2`

nct_plot: nct_plot.c
	gcc -g -Og -o $@ nct_plot.c `pkg-config --libs nctietue2`

install: ${tiedostot}
	cp ${tiedostot} ${bindir}/

uninstall:
	rm $(addprefix ${bindir}/, ${tiedostot})

.PHONY: uninstall
