@echo off

REM Visual Studio build

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

mkdir build
pushd build
cmake ..
msbuild C7Engine.sln /property:Configuration=Release
popd


