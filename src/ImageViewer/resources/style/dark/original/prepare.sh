#!/bin/bash -e

source_path="${PWD}"
destination_path=$(readlink -e "${source_path}/..")

inkscape_cmd=inkscape
optipng_cmd=optipng
convert_cmd=convert

cd "${source_path}"

for input in `ls *.svg | egrep '^checkbox_|^radiobutton_'`
do
    sizes="28"
    input_name="${input%.*}"
    input_path="${source_path}/${input}"
    for size in ${sizes}
    do
        output_path="${destination_path}/${input_name}@2x.png"
        "${inkscape_cmd}" -z -C -e "${output_path}" -w ${size} -h ${size} "${input_path}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path}"
    done
done

for input in `ls *.svg | egrep '^arrow_up_5_'`
do
    sizes="10"
    input_name="${input%.*}"
    input_path="${source_path}/${input}"
    for size in ${sizes}
    do
        output_path_up="${destination_path}/${input_name}@2x.png"
        output_path_down="${destination_path}/${input_name/_up_/_down_}@2x.png"
        "${inkscape_cmd}" -z -C -e "${output_path_up}" -w ${size} -h ${size} "${input_path}"
        "${convert_cmd}" "${output_path_up}" -rotate 180 +repage "${output_path_down}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path_up}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path_down}"
    done
done

for input in `ls *.svg | egrep '^arrow_up_6_'`
do
    sizes="12"
    input_name="${input%.*}"
    input_path="${source_path}/${input}"
    for size in ${sizes}
    do
        output_path_up="${destination_path}/${input_name}@2x.png"
        output_path_down="${destination_path}/${input_name/_up_/_down_}@2x.png"
        output_path_left="${destination_path}/${input_name/_up_/_left_}@2x.png"
        output_path_right="${destination_path}/${input_name/_up_/_right_}@2x.png"
        "${inkscape_cmd}" -z -C -e "${output_path_up}" -w ${size} -h ${size} "${input_path}"
        "${convert_cmd}" "${output_path_up}" -rotate 90 +repage "${output_path_right}"
        "${convert_cmd}" "${output_path_up}" -rotate 180 +repage "${output_path_down}"
        "${convert_cmd}" "${output_path_up}" -rotate 270 +repage "${output_path_left}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path_up}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path_down}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path_right}"
        "${optipng_cmd}" -o7 -zm1-9 -strip all "${output_path_left}"
    done
done

