#!/bin/bash -e

if [ ! -z "$(uname -s | grep -E -i 'darwin')" ] ; then
    export PATH="${PATH}:/Applications/Inkscape.app/Contents/MacOS"
    export PATH="${PATH}:/Applications/GIMP-2.10.app/Contents/MacOS"
fi

sizes="16 32"
source_path="${PWD}"
destination_path=$(pushd "${source_path}/.." > /dev/null && pwd && popd > /dev/null)

inkscape_cmd=$(which inkscape 2>/dev/null)
gimp_cmd=$(which gimp 2>/dev/null)
optipng_cmd=$(which optipng 2>/dev/null)

cd "${source_path}"
for input in `ls *.svg`
do
    input_name="${input%.*}"
    input_path="${source_path}/${input}"
    for size in ${sizes}
    do
        output_path="${destination_path}/${input_name}_${size}.png"
        "${inkscape_cmd}" -z -C -e "${output_path}" -w ${size} -h ${size} "${input_path}" || "${inkscape_cmd}" -C -o "${output_path}" -w ${size} -h ${size} "${input_path}"
        "${gimp_cmd}" -c -d -i -b "(let* ((image (car (file-png-load 0 \"${output_path}\" \"${output_path}\"))) (drawable (car (gimp-image-get-active-layer image)))) (gimp-image-convert-grayscale image) (file-png-save 1 image drawable \"${output_path}\" \"${output_path}\" 0 9 0 0 0 0 0) )" -b "(gimp-quit 0)"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path}"
    done
done
