#!/bin/bash -e

SCRIPT_ROOT=`dirname "${0}"`
cd "${SCRIPT_ROOT}"
SCRIPT_ROOT="${PWD}"
FLIF_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'FLIF-*' | head -1`

mkdir -p FLIF

for i in "flif.h" "flif_common.h" "flif_enc.h" "flif_dec.h"
do
    INCLUDE_BASE=`echo "${FLIF_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
    echo "#include \"../${INCLUDE_BASE}/src/library/${i}\"" > "${SCRIPT_ROOT}/FLIF/${i}"
done

