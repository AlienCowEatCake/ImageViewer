#!/bin/bash -e

SCRIPT_ROOT=`dirname "${0}"`
cd "${SCRIPT_ROOT}"
SCRIPT_ROOT="${PWD}"
OPENEXR_ROOT=`find "${SCRIPT_ROOT}/.." -mindepth 1 -maxdepth 1 -name 'openexr-*' | head -1`
ILMTHREADQT_ROOT="${SCRIPT_ROOT}/../IlmThreadQt"
CONFIG_ROOT="${SCRIPT_ROOT}/../config"

function generate
{
	for i in `find "${1}/${2}" -name '*.h' | cut -c $((${#1} + 2))-`
	do
		echo "#include \"${3}/${i}\"" > "${SCRIPT_ROOT}/OpenEXR/${i##*/}"
	done
}

for i in "IlmBase/Half" "IlmBase/Iex" "IlmBase/IexMath" "IlmBase/Imath" "OpenEXR/IlmImf" "OpenEXR/IlmImfUtil"
do
	INCLUDE_BASE=`echo "${OPENEXR_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
	generate "${OPENEXR_ROOT}" "${i}" "../${INCLUDE_BASE}"
done

for i in "IlmThread"
do
	INCLUDE_BASE=`echo "${ILMTHREADQT_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
	generate "${ILMTHREADQT_ROOT}" "${i}" "../${INCLUDE_BASE}"
done

for i in "OpenEXRConfig.h" "IlmBaseConfig.h"
do
	INCLUDE_BASE=`echo "${CONFIG_ROOT}" | cut -c $((${#SCRIPT_ROOT} + 2))-`
	echo "#include \"../${INCLUDE_BASE}/${i}\"" > "${SCRIPT_ROOT}/OpenEXR/${i##*/}"
done

for i in "ImfZlibWorkaround.h"
do
	find "${SCRIPT_ROOT}/OpenEXR" -name "${i}" -delete
done
