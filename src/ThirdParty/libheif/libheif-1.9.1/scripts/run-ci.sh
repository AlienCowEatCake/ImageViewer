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

ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")" >/dev/null 2>&1 && pwd)"

BUILD_ROOT=$ROOT/..
CURRENT_OS=$TRAVIS_OS_NAME
if [ -z "$CURRENT_OS" ]; then
    if [ "$(uname)" != "Darwin" ]; then
        CURRENT_OS=linux
    else
        CURRENT_OS=osx
    fi
fi

# Don't run regular tests on Coverity scan builds.
if [ ! -z "${COVERITY_SCAN_BRANCH}" ]; then
    echo "Skipping tests on Coverity scan build ..."
    exit 0
fi

if [ ! -z "$CHECK_LICENSES" ]; then
    echo "Checking licenses ..."
    ./scripts/check-licenses.sh
fi

if [ ! -z "$CPPLINT" ]; then
    echo "Running cpplint ..."
    find -name "*.c" -o -name "*.cc" -o -name "*.h" | sort | xargs ./scripts/cpplint.py --extensions=c,cc,h
    ./scripts/check-emscripten-enums.sh
    ./scripts/check-go-enums.sh

    echo "Running gofmt ..."
    ./scripts/check-gofmt.sh
    exit 0
fi

BIN_SUFFIX=
BIN_WRAPPER=
if [ "$MINGW" == "32" ]; then
    # Make sure the correct compiler will be used.
    unset CC
    unset CXX
    BIN_SUFFIX=.exe
    BIN_WRAPPER=wine
    export WINEPATH="/usr/lib/gcc/i686-w64-mingw32/7.3-posix/;/usr/i686-w64-mingw32/lib"
elif [ "$MINGW" == "64" ]; then
    # Make sure the correct compiler will be used.
    unset CC
    unset CXX
    BIN_SUFFIX=.exe
    BIN_WRAPPER=wine64
    export WINEPATH="/usr/lib/gcc/x86_64-w64-mingw32/7.3-posix/;/usr/x86_64-w64-mingw32/lib"
elif [ ! -z "$FUZZER" ]; then
    export CC="$BUILD_ROOT/clang/bin/clang"
    export CXX="$BUILD_ROOT/clang/bin/clang++"
fi

if [ ! -z "$CMAKE" ]; then
    echo "Preparing cmake build files ..."
    CMAKE_OPTIONS=
    if [ "$CURRENT_OS" = "osx" ] ; then
        # Make sure the homebrew installed libraries are used when building instead
        # of the libraries provided by Apple.
        CMAKE_OPTIONS="$CMAKE_OPTIONS -DCMAKE_FIND_FRAMEWORK=LAST"
    fi
    cmake . $CMAKE_OPTIONS
fi

if [ -z "$EMSCRIPTEN_VERSION" ] && [ -z "$CHECK_LICENSES" ] && [ -z "$TARBALL" ]; then
    echo "Building libheif ..."
    make -j $(nproc)
    if [ "$CURRENT_OS" = "linux" ] && [ -z "$CMAKE" ] && [ -z "$MINGW" ] && [ -z "$FUZZER" ]; then
        echo "Running tests ..."
        make test
    fi
    echo "Dumping information of sample file ..."
    ${BIN_WRAPPER} ./examples/heif-info${BIN_SUFFIX} --dump-boxes examples/example.heic
    if [ ! -z "$WITH_GRAPHICS" ] && [ ! -z "$WITH_LIBDE265" ]; then
        echo "Converting sample file to JPEG ..."
        ${BIN_WRAPPER} ./examples/heif-convert${BIN_SUFFIX} examples/example.heic example.jpg
        echo "Checking first generated file ..."
        [ -s "example-1.jpg" ] || exit 1
        echo "Checking second generated file ..."
        [ -s "example-2.jpg" ] || exit 1
        echo "Converting sample file to PNG ..."
        ${BIN_WRAPPER} ./examples/heif-convert${BIN_SUFFIX} examples/example.heic example.png
        echo "Checking first generated file ..."
        [ -s "example-1.png" ] || exit 1
        echo "Checking second generated file ..."
        [ -s "example-2.png" ] || exit 1
        if [ ! -z "$WITH_X265" ]; then
            echo "Converting single JPEG file to heif ..."
            ${BIN_WRAPPER} ./examples/heif-enc${BIN_SUFFIX} -o output-single.heic -v -v -v --thumb 320x240 example-1.jpg
            echo "Checking generated file ..."
            [ -s "output-single.heic" ] || exit 1
            echo "Converting back generated heif to JPEG ..."
            ${BIN_WRAPPER} ./examples/heif-convert${BIN_SUFFIX} output-single.heic output-single.jpg
            echo "Checking generated file ..."
            [ -s "output-single.jpg" ] || exit 1
            echo "Converting multiple JPEG files to heif ..."
            ${BIN_WRAPPER} ./examples/heif-enc${BIN_SUFFIX} -o output-multi.heic -v -v -v --thumb 320x240 example-1.jpg example-2.jpg
            echo "Checking generated file ..."
            [ -s "output-multi.heic" ] || exit 1
            ${BIN_WRAPPER} ./examples/heif-convert${BIN_SUFFIX} output-multi.heic output-multi.jpg
            echo "Checking first generated file ..."
            [ -s "output-multi-1.jpg" ] || exit 1
            echo "Checking second generated file ..."
            [ -s "output-multi-2.jpg" ] || exit 1
        fi
    fi
    if [ ! -z "$GO" ]; then
        echo "Installing library ..."
        make -j $(nproc) install

        echo "Running golang example ..."
        export GOPATH="$BUILD_ROOT/gopath"
        export PKG_CONFIG_PATH="$BUILD_ROOT/dist/lib/pkgconfig"
        export LD_LIBRARY_PATH="$BUILD_ROOT/dist/lib"
        mkdir -p "$GOPATH/src/github.com/strukturag"
        ln -s "$BUILD_ROOT" "$GOPATH/src/github.com/strukturag/libheif"
        go run examples/heif-test.go examples/example.heic
        echo "Checking first generated file ..."
        [ -s "examples/example_lowlevel.png" ] || exit 1
        echo "Checking second generated file ..."
        [ -s "examples/example_highlevel.png" ] || exit 1
        echo "Checking race tester ..."
        go run tests/test-race.go examples/example.heic
    fi
fi

if [ ! -z "$EMSCRIPTEN_VERSION" ]; then
    echo "Building with emscripten $EMSCRIPTEN_VERSION ..."
    source ./emscripten/emsdk/emsdk_env.sh && ./build-emscripten.sh
    source ./emscripten/emsdk/emsdk_env.sh && node scripts/test-javascript.js
fi

if [ ! -z "$TARBALL" ]; then
    VERSION=$(grep AC_INIT configure.ac | sed -r 's/^[^0-9]*([0-9]+\.[0-9]+\.[0-9]+).*/\1/g')
    echo "Creating tarball for version $VERSION ..."
    make dist

    echo "Building from tarball ..."
    tar xf libheif-$VERSION.tar*
    pushd libheif-$VERSION
    ./configure
    make -j $(nproc)
    popd
fi

if [ ! -z "$FUZZER" ] && [ "$CURRENT_OS" = "linux" ]; then
    export ASAN_SYMBOLIZER="$BUILD_ROOT/clang/bin/llvm-symbolizer"
    ./libheif/file-fuzzer ./fuzzing/corpus/*

    echo "Running color conversion fuzzer ..."
    ./libheif/color-conversion-fuzzer -max_total_time=120
    echo "Running encoder fuzzer ..."
    ./libheif/encoder-fuzzer -max_total_time=120
    echo "Running file fuzzer ..."
    ./libheif/file-fuzzer -dict=./fuzzing/dictionary.txt -max_total_time=120
fi
