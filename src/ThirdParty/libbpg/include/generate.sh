#!/bin/bash -e

SCRIPT_ROOT=`dirname "${0}"`
cd "${SCRIPT_ROOT}"
SCRIPT_ROOT="${PWD}"
LIBBPG_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'libbpg-*' | head -1`
INCLUDE_BASE=`echo "${LIBBPG_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`

for i in "bpgenc.h" "libbpg.h"
do
    echo "#include \"${INCLUDE_BASE}/${i}\"" > "${SCRIPT_ROOT}/${i##*/}"
done
