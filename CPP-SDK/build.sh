#!/bin/bash
# 删除旧的可执行文件（如果存在）
if [ -f ./bin/main ]; then
    echo "Removing existing ./bin/main..."
    rm ./bin/main
fi
# Step 1: 创建 build 文件夹
mkdir -p build
# Step 2: 执行 make clean（如果已有构建文件）
if [ -f build/Makefile ]; then
    echo "Cleaning previous build..."
    make -C build clean
fi
# Step 3: 运行 cmake，生成构建文件到 build/
cmake -B build
# Step 4: 编译项目
cmake --build build
