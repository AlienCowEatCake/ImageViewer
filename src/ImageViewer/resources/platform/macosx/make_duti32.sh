#!/bin/bash -e

APP_CERT="Developer ID Application: Petr Zhigalov (48535TNTA7)"

cd "$(dirname $0)"

curl --insecure -L 'https://github.com/moretension/duti/archive/refs/tags/duti-1.5.4.tar.gz' | tar -xvp
cd duti-duti-1.5.4
autoreconf -i
CC=gcc-4.0 CFLAGS='-Os -arch i386 -arch ppc' LDFLAGS='-Os -arch i386 -arch ppc' ./configure --build=i386-apple-darwin8.0.0 --with-macosx-sdk=/Developer/SDKs/MacOSX10.4u.sdk --with-macosx-deployment-target=10.4
make
/usr/bin/codesign \
    --sign "${APP_CERT}" \
    --verbose \
    duti
bzip2 -9v duti
mv duti.bz2 ..
cd ..
rm -rf duti-duti-1.5.4
