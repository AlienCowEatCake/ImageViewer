#!/bin/bash -e

APP_CERT="Developer ID Application: Petr Zhigalov (48535TNTA7)"
NOTARIZE_USERNAME="peter.zhigalov@gmail.com"
NOTARIZE_PASSWORD="@keychain:Notarize: ${NOTARIZE_USERNAME}"
NOTARIZE_ASC_PROVIDER="${APP_CERT: -11:10}"

cd "$(dirname $0)"
SOURCE_PATH="${PWD}/../../../../.."

curl -L 'https://github.com/moretension/duti/archive/refs/tags/duti-1.5.4.tar.gz' | tar -xvp
cd duti-duti-1.5.4
curl -L 'https://github.com/moretension/duti/commit/825b5e6a92770611b000ebdd6e3d3ef8f47f1c47.patch?full_index=1' | patch -p1
curl -L 'https://github.com/moretension/duti/commit/4a1f54faf29af4f125134aef3a47cfe05c7755ff.patch?full_index=1' | patch -p1
curl -L 'https://github.com/moretension/duti/commit/ec195e261f8a48a1a18e262a2b1f0ef26a0bc1ee.patch?full_index=1' | patch -p1
autoreconf -i
CFLAGS='-Os' LDFLAGS='-Os' ./configure --with-macosx-deployment-target=10.5
make
/usr/bin/codesign \
    --sign "${APP_CERT}" \
    --timestamp \
    --options runtime \
    --verbose \
    --strict \
    duti
/usr/bin/python3 "${SOURCE_PATH}/buildscripts/helpers/MacNotarizer.py" \
    --application duti \
    --primary-bundle-id duti \
    --username "${NOTARIZE_USERNAME}" \
    --password "${NOTARIZE_PASSWORD}" \
    --asc-provider "${NOTARIZE_ASC_PROVIDER}"
bzip2 -9v duti
mv duti.bz2 ..
cd ..
rm -rf duti-duti-1.5.4
