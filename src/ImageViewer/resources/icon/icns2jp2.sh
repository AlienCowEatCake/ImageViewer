#!/bin/bash -e

oldiconutil="$(cd "$(dirname "${0}")" && pwd)/oldiconutil/oldiconutil"
icns_png="icon.icns"
icns_jp2="icon_jp2.icns"

cp -a "${icns_png}" "${icns_jp2}"
"${oldiconutil}" --inplace "${icns_jp2}"
