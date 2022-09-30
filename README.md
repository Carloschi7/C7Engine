# C7Engine
Mini library used to process basic 2D/3D rendering (uses OpenGL)

## Building on Windows + Visual Studio 

For now the engine on Windows is only available in 32bits due to some problems during development,
this will be fixed soon

#### Building with the cmake gui

Requirements:
1) The latest version of Visual Studio (preferably)
2) CMake GUI (version 3.12 or higher)
3) GIT BASH

Steps with GUI: ("C:/Engine" will be just a placeholder dir, feel free to use a different one)



1)Clone this repo with the command "git clone https://github.com/Carloschi7/C7Engine.git" in a chosen dir,
	for example "C:/Engine"



2)Open the CMake GUI: in the source code section insert "C:/Engine/C7Engine" and in the binaries folder choose
	"C:/Engine/C7Engine/build" (strongly recommended)



3)Click Configure and set the project platform to Win32 when the popup window appears (this step is fundamental, 
	because a x64 Visual Studio project won't link against 32 bit libs), then click Generate



4)Open the build folder and open the sln file with Visual Studio



5)Set the C7Engine project as the starting project(right click on the project icon to find the setting)



6)Right click on the C7Engine project and click Build



7)You will find your C7Engine.lib file in build/$(Configuration), where $(Configuration) is
your project building configuration (Debug, Release)




#### Building with the terminal

Requirements:
1) The latest version of Visual Studio (preferably)
2) CMake linked to the console via Windows' environmental variables
3) GIT BASH



1) (In a chosen dir) git clone https://github.com/Carloschi7/C7Engine.git



2) cd C7Engine



3) mkdir build



4) cd build



5) cmake -A Win32 ..



6) (YourProjectName).sln (which opens up the project in Visual Studio, then you can follow the previous
points from 5) to the end)



## Building on Linux Ubuntu

To build the library on Ubuntu, execute the following commands


1) `$ sudo apt-get install libglfw3 libglfw3-dev libglew-dev libassimp-dev`



2) `$ git clone --recursive https://github.com/Carloschi7/C7Engine.git`



3) `cd C7Engine && mkdir build && cd build`



4) `cmake .. && make`



And there you have it, you should file the libC7Engine.so in the build folder

Enjoy!

