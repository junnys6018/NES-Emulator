#!/bin/bash
pushd "$(dirname "$0")/.."
mkdir -p build build/Debug build/Release
cmake -S . -B build/Debug -D CMAKE_BUILD_TYPE=Debug
cmake -S . -B build/Release -D CMAKE_BUILD_TYPE=Release
popd