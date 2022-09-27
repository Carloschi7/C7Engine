# C7Engine
Mini library used to process basic 2D/3D rendering (uses OpenGL)

HOW TO COMPILE THIS ENGINE (retrieve the C7Engine.lib)

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



7)You will find your C7Engine.lib file in build/Debug




8)If you want the latest version of the C7Engine.lib, download the most recent upload of this repo and
	repeat exacly the same process



Recommended steps with terminal (requires cmake to function as a terminal command, 
the executable needs to be linked to the PATH section of Windows' environmental variables):



1) (In a chosen dir) git clone https://github.com/Carloschi7/C7Engine.git



2) mkdir build



3) cd build



4) cmake -A Win32 ..



(Depending on which method you prefer according to your configuration)




5) make (needs to be installed)



(For Windows + Visual Studio)

5) (YourProjectName).sln (which opens up the project in Visual Studio, then you can follow the previous
points from 5) to the end)

Enjoy!

