pushd %~dp0..
mkdir build
cmake -S . -B build
popd