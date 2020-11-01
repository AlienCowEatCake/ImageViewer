#!/bin/bash -e
BUNDLE_ID="ru.codefreak.fami.imageviewer"
APP_PATH=$(osascript -e "POSIX path of (path to application id \"${BUNDLE_ID}\")" | sed 's|:/$||')
for ext in $(cat "${APP_PATH}/Contents/Info.plist" | egrep '\t\t\t\t<string>[a-z0-9]{3,5}</string>' | sed 's|.*<string>\(.*\)</string>|\1|') ; do
	duti -s ${BUNDLE_ID} ${ext} viewer
done
