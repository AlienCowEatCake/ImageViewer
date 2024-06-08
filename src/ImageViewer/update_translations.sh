#!/bin/bash -e

LUPDATE_CMD='lupdate'
TEMP_PRO_FILE='Translation.pro'

HEADERS=`find . \( -iname '*.h' -o -iname '*.hpp' -o -iname '*.hxx' \) | sed 's|^\./|    | ; s|$| \\\|'`
SOURCES=`find . \( -iname '*.c' -o -iname '*.cpp' -o -iname '*.cxx' -o -iname '*.m' -o -iname '*.mm' \) | sed 's|^\./|    | ; s|$| \\\|'`

cat << EOF > "${TEMP_PRO_FILE}"
TRANSLATIONS += \\
    resources/translations/imageviewer_en.ts \\
    resources/translations/imageviewer_ru.ts \\
    resources/translations/imageviewer_zh.ts \\

HEADERS += \\
${HEADERS}

SOURCES += \\
${SOURCES}

EOF

"${LUPDATE_CMD}" $@ "${TEMP_PRO_FILE=}"

rm "${TEMP_PRO_FILE=}"

