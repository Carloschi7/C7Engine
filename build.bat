@echo off

REM Visual Studio build

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

mkdir build
pushd build
REM assimp usage from Model can be disabled
if "%1"== "NO_ASSIMP" (
	echo ASSIMP library not used, won't be required assimp dll
	cmake -DASSIMP_USED=FALSE ..
) else (
	echo ASSIMP used, assimp dll will be required by the application	
	cmake -DASSIMP_USED=TRUE ..
)
msbuild C7Engine.sln /property:Configuration=Release
popd


