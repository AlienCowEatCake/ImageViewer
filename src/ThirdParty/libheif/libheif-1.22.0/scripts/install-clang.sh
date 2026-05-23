#!/bin/bash
set -e
#
# HEIF codec.
# Copyright (c) 2018 struktur AG, Joachim Bauch <bauch@struktur.de>
#
# This file is part of libheif.
#
# libheif is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# libheif is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with libheif.  If not, see <http://www.gnu.org/licenses/>.
#

# Use script from https://chromium.googlesource.com/chromium/src/tools/clang/
# to download prebuilt version of clang. This commit defines which version of
# the script should be used (and thus defines the version of clang).
COMMIT_HASH=13d4d9000d7320838a4f4068751e23e909809ac0

DEST=$1

if [ -z "${DEST}" ]; then
    echo "USAGE: $0 <destination>"
    exit 1
fi

url="https://github.com/chromium/chromium/raw/${COMMIT_HASH}/tools/clang/scripts/update.py"

tmpdir=$(mktemp -d)
echo "Using ${tmpdir} as temporary folder"

script_folder=${tmpdir}/tools/clang/scripts
mkdir -p "${script_folder}"
echo "Downloading from ${url} ..."
curl --fail --location -o "${script_folder}/update.py" ${url}

echo "Running ${script_folder}/update.py ..."
python3 "${script_folder}/update.py" --output-dir "${tmpdir}"

echo "Copying to ${DEST} ..."
mkdir -p "$DEST"
cp -rf "${tmpdir}/"* "${DEST}"

echo "Cleaning up ..."
rm -rf "${tmpdir}"
