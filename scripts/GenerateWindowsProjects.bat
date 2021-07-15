pushd %~dp0..
call vendor\premake5\premake5_windows.exe --os=windows vs2019
popd