#!/bin/bash

for input in `ls *.svg`
do
	output_black_svg=`echo "../${input}" | sed 's/\.svg$/_black.svg/'`
	output_black_png=`echo ${output_black_svg} | sed 's/\.svg$/.png/'`
	output_white_svg=`echo ${output_black_svg} | sed 's/_black\.svg$/_white.svg/'`
	output_white_png=`echo ${output_white_svg} | sed 's/\.svg$/.png/'`
	# Convert to Plain SVG
	inkscape -z -T -C --vacuum-defs -l ${output_black_svg} ${input}
	# Replace Black to White
	cat ${output_black_svg} | sed 's/#000000/#ffffff/g' > ${output_white_svg}
	# Convert SVG to PNG
	inkscape -z -C -e ${output_black_png} ${output_black_svg}
	inkscape -z -C -e ${output_white_png} ${output_white_svg}
	# Compress PNG
	optipng -o7 -strip all ${output_black_png}
	optipng -o7 -strip all ${output_white_png}
	# Fix rendering issues
	sed -i 's/crispEdges/auto/' ${output_black_svg}
	sed -i 's/crispEdges/auto/' ${output_white_svg}
done
