#!/bin/bash -e

SCRIPT_ROOT=`dirname "${0}"`
cd "${SCRIPT_ROOT}"
SCRIPT_ROOT="${PWD}"
OJPH_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'OpenJPH-*' | head -1`

rm -rf "${SCRIPT_ROOT}/openjph"
mkdir -p "${SCRIPT_ROOT}/openjph"

for i in $(find "${OJPH_ROOT}/src/core/openjph/" -name '*.h' | cut -c $((${#SCRIPT_ROOT} + 2))-)
do
	echo "#include \"${i}\"" > "${SCRIPT_ROOT}/openjph/$(basename "${i}")"
done

