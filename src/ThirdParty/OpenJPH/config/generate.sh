#!/bin/bash -e

SCRIPT_ROOT=`dirname "${0}"`
cd "${SCRIPT_ROOT}"
SCRIPT_ROOT="${PWD}"
OJPH_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'OpenJPH-*' | head -1`

echo "#include \"$(echo "${OJPH_ROOT}/src/core/others/ojph_mem.c" | cut -c $((${#SCRIPT_ROOT} + 2))-)\"" > "${SCRIPT_ROOT}/ojph_mem_c.c"
echo "#include \"$(echo "${OJPH_ROOT}/src/core/others/ojph_mem.cpp" | cut -c $((${#SCRIPT_ROOT} + 2))-)\"" > "${SCRIPT_ROOT}/ojph_mem_cpp.cpp"

