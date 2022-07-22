#!/bin/sh
file=nct_functions_prototype.c
outheader=nct_functions.h

[ -f $outheader ] && rm ${outheader}

nctype=NC_BYTE
printf "#define ctype char\n#define form hhi\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_UBYTE
printf "#define ctype unsigned char\n#define form hhu\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_CHAR
printf "#define ctype char\n#define form c\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_SHORT
printf "#define ctype short\n#define form hi\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_USHORT
printf "#define ctype unsigned short\n#define form hu\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_INT
printf "#define ctype int\n#define form i\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_UINT
printf "#define ctype unsigned\n#define form u\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_INT64
printf "#define ctype long long int\n#define form lli\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_UINT64
printf "#define ctype long long unsigned\n#define form llu\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_FLOAT
printf "#define ctype float\n#define form f\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_DOUBLE
printf "#define ctype double\n#define form lf\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
printf "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
printf "#include \"nct_functions_${nctype}.c\"" >> ${outheader}
