#!/bin/bash -e

sizes="28"
source_path="${PWD}"
destination_path=$(readlink -e "${source_path}/..")

inkscape_cmd=inkscape
optipng_cmd=optipng

cd "${source_path}"
for input in `ls *.svg`
do
	input_name="${input%.*}"
	input_path="${source_path}/${input}"
	for size in ${sizes}
	do
		output_path="${destination_path}/${input_name}@2x.png"
		"${inkscape_cmd}" -z -C -e "${output_path}" -w ${size} -h ${size} "${input_path}"
		"${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path}"
	done
done
