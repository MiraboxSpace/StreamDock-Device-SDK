#!/bin/bash
set -e

# OpenCV 版本配置
VERSION="4.12.0"
SHORTVER="412"

# 目标 lib 目录（相对路径）
LIBDIR="./opencv/mac/lib"

cd "$LIBDIR"

# 查找所有 OpenCV 主版本 dylib 文件
for full in libopencv_*.$VERSION.dylib; do
    [ -f "$full" ] || continue  # skip if no match

    base="${full%.$VERSION.dylib}"  # 去掉版本后缀，得到 libopencv_xxx

    echo "🔗 Linking: $base.dylib and $base.$SHORTVER.dylib -> $full"
    ln -sf "$full" "$base.dylib"
    ln -sf "$full" "$base.$SHORTVER.dylib"
done

echo "✅ All symlinks created in $LIBDIR."
