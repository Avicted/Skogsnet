#!/bin/bash

echo "Compiling raylib_frontend"
echo ""

cmake -S ./raylib_frontend/ -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build ./build --config Debug --target all --

make -C ../build

# Run
echo "Running raylib_frontend"
echo ""
./build/Frontend