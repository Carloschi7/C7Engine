@echo off

REM Visual Studio build

call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" x64

mkdir build
pushd build
REM assimp usage from Model can be disabled

if "%1" == "NO_ASSIMP" (
	echo assimp wont be used	
	set assimp_used=FALSE
) else (
	set assimp_used=TRUE
)

if "%2" == "STANDALONE" (
	echo standalone build
	cmake -DASSIMP_USED=%assimp_used% -DSTANDALONE=1 ..
) else (
	cmake -DASSIMP_USED=%assimp_used% ..
)

msbuild C7Engine.sln /property:Configuration=Debug
msbuild C7Engine.sln /property:Configuration=Release
popd


