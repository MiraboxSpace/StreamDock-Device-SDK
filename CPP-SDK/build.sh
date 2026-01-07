#!/bin/bash
# Remove the old executable (if it exists)
if [ -f ./bin/main ]; then
    echo "Removing existing ./bin/main..."
    rm ./bin/main
fi
# Step 1: Create the build directory
mkdir -p build
# Step 2: Run make clean (if build files exist)
if [ -f build/Makefile ]; then
    echo "Cleaning previous build..."
    make -C build clean
fi
# Step 3: Run cmake to generate build files in build/
cmake -B build
# Step 4: Build the project
cmake --build build
