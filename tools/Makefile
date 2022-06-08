tiedtot=nct_info nct_add_similars
linkit=-lnctietue -lnetcdf -lpng -lSDL2

all: ${tiedtot}

nct_info: nct_info.c
	gcc -g -Og -o $@ nct_info.c ${linkit}

nct_add_similars: nct_add_similars.c
	gcc -g -Og -o $@ nct_add_similars.c ${linkit}

install: ${tiedtot}
	cp ${tiedtot} /usr/bin/
