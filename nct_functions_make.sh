#!/bin/sh
file=nct_functions_prototype.c
outheader=nct_functions.h

[ -f $outheader ] && rm ${outheader}

nctype=NC_BYTE
echo "#define ctype char\n#define form hhi\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_UBYTE
echo "#define ctype unsigned char\n#define form hhu\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_CHAR
echo "#define ctype char\n#define form c\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_SHORT
echo "#define ctype short\n#define form hi\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_USHORT
echo "#define ctype unsigned short\n#define form hu\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_INT
echo "#define ctype int\n#define form i\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_UINT
echo "#define ctype unsigned\n#define form u\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_INT64
echo "#define ctype long long int\n#define form lli\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_UINT64
echo "#define ctype long long unsigned\n#define form llu\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_FLOAT
echo "#define ctype float\n#define form f\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}

nctype=NC_DOUBLE
echo "#define ctype double\n#define form lf\n#define nctype ${nctype}\n" > nct_functions_${nctype}.c
sed "s/##nctype/${nctype}/" ${file} >> nct_functions_${nctype}.c
echo "\n#undef ctype\n#undef form\n#undef nctype" >> nct_functions_${nctype}.c
echo "#include \"nct_functions_${nctype}.c\"" >> ${outheader}
