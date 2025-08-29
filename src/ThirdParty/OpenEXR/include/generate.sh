#!/bin/bash -e

SCRIPT_ROOT=`dirname "${0}"`
cd "${SCRIPT_ROOT}"
SCRIPT_ROOT="${PWD}"
OPENEXR_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'openexr-*' | head -1`
IMATH_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'Imath-*' | head -1`
ILMTHREADQT_ROOT="${SCRIPT_ROOT}/../IlmThreadQt"
CONFIG_ROOT="${SCRIPT_ROOT}/../config"

rm -rf "${SCRIPT_ROOT}/Imath" "${SCRIPT_ROOT}/OpenEXR"
mkdir -p "${SCRIPT_ROOT}/Imath" "${SCRIPT_ROOT}/OpenEXR"

function generate
{
    for i in `find "${1}/${2}" -name '*.h' | cut -c $((${#1} + 2))-`
    do
        echo "#include \"${3}/${i}\"" > "${SCRIPT_ROOT}/${4}/${i##*/}"
    done
}

for i in "src/lib/Iex" "src/lib/IlmThread" "src/lib/OpenEXR" "src/lib/OpenEXRCore" "src/lib/OpenEXRUtil"
do
    INCLUDE_BASE=`echo "${OPENEXR_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
    generate "${OPENEXR_ROOT}" "${i}" "../${INCLUDE_BASE}" "OpenEXR"
done

for i in "src/Imath"
do
    INCLUDE_BASE=`echo "${IMATH_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
    generate "${IMATH_ROOT}" "${i}" "../${INCLUDE_BASE}" "Imath"
done

for i in "IexConfig.h" "IlmThreadConfig.h" "OpenEXRConfig.h"
do
    INCLUDE_BASE=`echo "${CONFIG_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
    echo "#include \"../${INCLUDE_BASE}/${i}\"" > "${SCRIPT_ROOT}/OpenEXR/${i##*/}"
done

for i in "ImathConfig.h"
do
    INCLUDE_BASE=`echo "${CONFIG_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
    echo "#include \"../${INCLUDE_BASE}/${i}\"" > "${SCRIPT_ROOT}/Imath/${i##*/}"
done
