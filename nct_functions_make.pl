#!/usr/bin/perl
@infiles   = ('nct_functions_prototype.c', 'nct_draw_prototype.c');
@func_outh = ('nct_functions.h', 'nct_draw.h');
@nctypes   = ('NC_BYTE', 'NC_UBYTE', 'NC_CHAR', 'NC_SHORT', 'NC_USHORT', 'NC_INT', 'NC_UINT',
	         'NC_INT64', 'NC_UINT64', 'NC_FLOAT', 'NC_DOUBLE');
@formats   = ('hhi', 'hhu', 'c', 'hi', 'hu', 'i', 'u', 'lli', 'llu', 'f', 'lf');
@ctypes    = ('char', 'unsigned char', 'char', 'short', 'unsigned short', 'int', 'unsigned',
	      'long long int', 'long long unsigned', 'float', 'double');

$len0 = @infiles;
$len1 = @nctypes;

for ($i=0; $i<$len0; $i++) {
    open H, ">@func_outh[$i]";
    open IN, "<@infiles[$i]";
    for($j=0; $j<$len1; $j++) {
	$a = $infiles[$i];
	$a =~ s/prototype/$nctypes[$j]/;

	open C, ">$a";
	print C "#define ctype @ctypes[$j]\n#define form @formats[$j]\n#define __nctype__ @nctypes[$j]\n\n";
	seek IN, 0, 0;
	while(<IN>) {
	    $_ =~ s/##nctype/@nctypes[$j]/g;
	    $_ =~ s/#form/\"@formats[$j]\"/g;
	    print C $_;
	}
	print C "\n#undef ctype\n#undef form\n#undef __nctype__\n";
	close C;

	print H "#include \"$a\"\n";
    }
    close H;
}
