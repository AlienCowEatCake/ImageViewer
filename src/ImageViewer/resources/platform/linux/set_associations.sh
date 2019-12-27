#!/bin/bash -e

DESKTOP_NAME="ImageViewer.desktop"

function FindDesktopFile()
{
	local FILENAME="${1}"
	local PATHS=(
		"${HOME}/.local/share/applications/${FILENAME}"
		"/usr/local/share/applications/${FILENAME}"
		"/usr/share/applications/${FILENAME}"
		"$(dirname "${0}")/${FILENAME}"
		)
	for i in "${PATHS[@]}"; do
		if [ -f "${i}" ]; then
			echo "${i}"
			return
		fi
	done
}

DESKTOP_FILE_PATH="$(FindDesktopFile "${DESKTOP_NAME}")"
if [ -z "${DESKTOP_FILE_PATH}" ]; then
	echo "Can't find desktop file with name: ${DESKTOP_NAME}" >&2
	exit 1
fi

echo "Path to desktop file: ${DESKTOP_FILE_PATH}"
xdg-mime default "${DESKTOP_NAME}" `grep 'MimeType=' "${DESKTOP_FILE_PATH}" | sed -e 's/.*=//' -e 's/;/ /g'`
exit ${?}
