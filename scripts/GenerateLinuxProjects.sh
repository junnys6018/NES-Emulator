pushd "$(dirname "$0")/.."
vendor/premake5/premake5_linux --os=linux gmake2
popd