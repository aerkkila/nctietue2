#!/bin/sh
rm colormaps.h

for file in `ls colormaps/colormaps_*.h`; do
    echo "#include \"${file}\"" >> colormaps.h
done

echo \
"\n#define COLORMAP(map) cmap_##map, #map,"\
"\nstatic char* colormaps[] = {" >> colormaps.h

for map in `ls colormaps/colormaps_*.h |grep -Po '(?<=colormaps_)\w*'`; do
    echo "    COLORMAP(${map})" >> colormaps.h
done

echo \
"};\n"\
"#undef COLORMAP\n"\
"\n#define COLORVALUE(imap,value) (colormaps[(imap)*2] + (value)*3)" >> colormaps.h
