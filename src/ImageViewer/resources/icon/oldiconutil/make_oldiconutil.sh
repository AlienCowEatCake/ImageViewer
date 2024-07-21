#!/bin/bash -e

APP_CERT="Developer ID Application: Petr Zhigalov (48535TNTA7)"
NOTARIZE_USERNAME="peter.zhigalov@gmail.com"
NOTARIZE_PASSWORD="@keychain:Notarize: ${NOTARIZE_USERNAME}"
NOTARIZE_ASC_PROVIDER="${APP_CERT: -11:10}"

cd "$(dirname $0)"
SOURCE_PATH="${PWD}/../../../../.."

clang -arch x86_64 -arch arm64 -Os -DNDEBUG -mmacosx-version-min=10.4 \
    -framework Cocoa oldiconutil-e2e1986/oldiconutil/main.m \
    -o oldiconutil
/usr/bin/codesign \
    --sign "${APP_CERT}" \
    --timestamp \
    --options runtime \
    --verbose \
    --strict \
    oldiconutil
/usr/bin/python3 "${SOURCE_PATH}/buildscripts/helpers/MacNotarizer.py" \
    --application oldiconutil \
    --primary-bundle-id oldiconutil \
    --username "${NOTARIZE_USERNAME}" \
    --password "${NOTARIZE_PASSWORD}" \
    --asc-provider "${NOTARIZE_ASC_PROVIDER}"
