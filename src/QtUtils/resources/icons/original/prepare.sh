#!/bin/bash -e

sizes="16 32"
source_path="${PWD}"
destination_path=$(readlink -e "${source_path}/..")

inkscape_cmd=inkscape
gimp_cmd=gimp
optipng_cmd=optipng

cd "${source_path}"
for input in `ls *.svg`
do
	input_name="${input%.*}"
	input_path="${source_path}/${input}"
	for size in ${sizes}
	do
		output_path="${destination_path}/${input_name}_${size}.png"
		"${inkscape_cmd}" -z -C -e "${output_path}" -w ${size} -h ${size} "${input_path}"
		"${gimp_cmd}" -c -d -i -b "(let* ((image (car (file-png-load 0 \"${output_path}\" \"${output_path}\"))) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-grayscale image) (file-png-save 1 image drawable \"${output_path}\" \"${output_path}\" 0 9 0 0 0 0 0) )" -b "(gimp-quit 0)"
		"${optipng_cmd}" -o7 -strip all "${output_path}"
	done
done
