#!/bin/bash -e

function command_exists() {
    type "${1}" &>/dev/null
}

function platform_is_windows() {
    local PLATFORM_IS_WINDOWS=$(uname -s | grep -E -i 'cygwin|mingw|msys')
    [ ! -z "${PLATFORM_IS_WINDOWS}" ]
}

function platform_is_macos() {
    local PLATFORM_IS_MACOS=$(uname -s | grep -E -i 'darwin')
    [ ! -z "${PLATFORM_IS_MACOS}" ]
}

source_path="${PWD}"
destination_path=$(pushd "${source_path}/.." > /dev/null && pwd && popd > /dev/null)

if platform_is_macos ; then
    export PATH="${PATH}:/Applications/Inkscape.app/Contents/MacOS"
elif platform_is_windows ; then
    export PATH="${PATH}:$(cygpath -u "${PROGRAMFILES}\\Inkscape\\bin")"
fi
inkscape_cmd="$(which inkscape 2>/dev/null)"

python3_cmd="python3"
if platform_is_windows && command_exists "python" ; then
    python3_cmd="python"
fi
venv_path="${source_path}/venv"
trap 'rm -rf "${venv_path}"' EXIT
rm -rf "${venv_path}"
"${python3_cmd}" -m venv "${venv_path}"
if [ -d "${venv_path}/Scripts" ] ; then
    source "${venv_path}/Scripts/activate"
else
    source "${venv_path}/bin/activate"
fi
"${python3_cmd}" -m pip install "scour" --upgrade
scour_cmd="scour"

cd "${source_path}"
for input in `ls *.svg`
do
    input_name="${input%.*}"
    input_path="${source_path}/${input}"
    intermediate_path="${destination_path}/${input_name}_intermediate.svg"
    output_path="${destination_path}/${input}"
    "${inkscape_cmd}" --export-plain-svg --vacuum-defs --export-text-to-path --export-filename="${output_path}" "${input_path}"
    sed 's|\([";]\)opacity:0\.97[^";]*[;]*|\1|g ; s|shape-rendering="crispEdges"||g' "${output_path}" > "${intermediate_path}"
    "${scour_cmd}" -i "${intermediate_path}" -o "${output_path}" \
            --set-precision=5 --create-groups --remove-descriptive-elements --enable-comment-stripping --enable-viewboxing \
            --indent=none --no-line-breaks --strip-xml-space --enable-id-stripping --shorten-ids
    rm -f "${intermediate_path}"
done
