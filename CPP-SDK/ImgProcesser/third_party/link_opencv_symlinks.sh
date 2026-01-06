#!/bin/bash
set -e

# OpenCV version configuration
VERSION="4.12.0"
SHORTVER="412"

# Target lib directory (relative path)
LIBDIR="./opencv/mac/lib"

cd "$LIBDIR"

# Find all OpenCV major-version dylib files
for full in libopencv_*.$VERSION.dylib; do
    [ -f "$full" ] || continue  # skip if no match

    base="${full%.$VERSION.dylib}"  # Strip the version suffix to get libopencv_xxx

    echo "🔗 Linking: $base.dylib and $base.$SHORTVER.dylib -> $full"
    ln -sf "$full" "$base.dylib"
    ln -sf "$full" "$base.$SHORTVER.dylib"
done

echo "✅ All symlinks created in $LIBDIR."
