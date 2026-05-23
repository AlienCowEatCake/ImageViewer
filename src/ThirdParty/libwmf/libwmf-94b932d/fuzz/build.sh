#!/bin/bash
# Build the libwmf fuzzer harness.
#
# Local usage:
#   cd <libwmf checkout>
#   ./fuzz/build.sh
#   ./fuzz/wmf_fuzzer -max_total_time=60 fuzz/corpus
#
# This builds an isolated copy of libwmflite in ./fuzz/build with sanitizer +
# coverage instrumentation, then links the harness against the resulting
# static archive. Will not disturb any existing in-tree build.
#
# In oss-fuzz this script is invoked with $CC, $CFLAGS, $LIB_FUZZING_ENGINE
# already set by the build environment. Outside of oss-fuzz we default to
# clang with -fsanitize=fuzzer-no-link,address,undefined.

set -eu

cd "$(dirname "$0")/.."
SRCDIR="$(pwd)"
BUILDDIR="$SRCDIR/fuzz/build"

: "${CC:=clang}"
: "${CFLAGS:=-O1 -g -fsanitize=fuzzer-no-link,address,undefined -fno-omit-frame-pointer}"
: "${LIB_FUZZING_ENGINE:=-fsanitize=fuzzer,address,undefined}"

# Stage a clean copy of the source so we don't have to distclean the user's
# in-tree build. In oss-fuzz the src tree is fresh and this is a no-op rsync.
rm -rf "$BUILDDIR"
mkdir -p "$BUILDDIR"
(cd "$SRCDIR" && git ls-files | tar -cf - -T -) | tar -xf - -C "$BUILDDIR"

cd "$BUILDDIR"
autoreconf -fi >/dev/null
./configure \
	--disable-shared --enable-static \
	--without-expat --with-libxml2 \
	CC="$CC" CFLAGS="$CFLAGS"

make -j"$(nproc)" -C src/extra/gd libgd.la
make -j"$(nproc)" -C src/ipa libipa.la
make -j"$(nproc)" -C src libwmf.la libwmflite.la

cd "$SRCDIR"

FT_CFLAGS=$(pkg-config --cflags freetype2)
DEP_LIBS=$(pkg-config --libs freetype2 libpng libxml-2.0 zlib)

$CC $CFLAGS \
	$FT_CFLAGS \
	-I"$SRCDIR/include" -I"$BUILDDIR" \
	-c fuzz/wmf_fuzzer.c -o fuzz/wmf_fuzzer.o

$CC $CFLAGS $LIB_FUZZING_ENGINE \
	fuzz/wmf_fuzzer.o \
	"$BUILDDIR/src/.libs/libwmf.a" \
	"$BUILDDIR/src/.libs/libwmflite.a" \
	$DEP_LIBS -ljpeg -lX11 \
	-lm \
	-o fuzz/wmf_fuzzer

if [ ! -d fuzz/corpus ] || [ -z "$(ls -A fuzz/corpus 2>/dev/null)" ]; then
	mkdir -p fuzz/corpus
	curl -fsSL --no-progress-meter \
		https://dev-www.libreoffice.org/corpus/wmffuzzer_seed_corpus.zip \
		-o fuzz/corpus.zip
	unzip -q -j -o fuzz/corpus.zip -d fuzz/corpus
	rm fuzz/corpus.zip
fi

echo "Built fuzz/wmf_fuzzer"
