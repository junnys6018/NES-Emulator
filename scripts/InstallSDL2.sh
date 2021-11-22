#!/bin/bash
mkdir -p "$(dirname "$0")/temp"
pushd "$(dirname "$0")/temp"

wget https://www.libsdl.org/release/SDL2-2.0.12.zip
unzip SDL2-2.0.12.zip

pushd SDL2-2.0.12
./configure
make all
make install

popd
popd
rm -rf "$(dirname "$0")/temp"