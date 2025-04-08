@echo off
setlocal
if not exist build (
    mkdir build
)
cmake -B build -G "Visual Studio 17 2022" -A x64
cmake --build build --config Release
endlocal
