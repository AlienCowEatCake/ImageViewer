#!/bin/sh
# Run each converter over every example WMF to check for crashes.

srcdir="${srcdir:-.}"
top_srcdir="${top_srcdir:-..}"
top_builddir="${top_builddir:-..}"

FONTDIR="$top_srcdir/fonts"
EXAMPLES="$top_srcdir/examples"
CONVERTDIR="$top_builddir/src/convert"

converters=""
for tool in wmf2svg wmf2eps wmf2fig wmf2gd; do
    if [ -x "$CONVERTDIR/$tool" ]; then
        converters="$converters $tool"
    fi
done

if [ -z "$converters" ]; then
    echo "SKIP: no converters built"
    exit 77
fi

failures=0
count=0
for tool in $converters; do
    for f in "$EXAMPLES"/*.wmf; do
        [ -f "$f" ] || continue
        count=$((count + 1))
        if "$CONVERTDIR/$tool" --wmf-fontdir="$FONTDIR" "$f" -o /dev/null 2>/dev/null; then
            echo "PASS: $tool $f"
        else
            echo "FAIL: $tool $f"
            failures=$((failures + 1))
        fi
    done
done

if [ "$count" -eq 0 ]; then
    echo "SKIP: no example WMF files found"
    exit 77
fi

echo "$count tests, $failures failures"
exit $failures
