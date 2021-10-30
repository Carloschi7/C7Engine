# C7Engine
Mini library used to process basic 2D/3D rendering (uses OpenGL)

HOW TO SETUP AN EXTERNAL PROJECT TO USE THIS ENGINE

1) New Project Settings Configurations

To get started, create a blank project using visual studio, then add the engine's project
by going to Solution -> Add -> Existing project

In the solution folder, copy the Dependencies directory which can be found in the original
Dummy solution

To be safe, in the new project's properties -> General, Select the c++ standard as stdc++:17

Go to new project's properties -> C/C++ -> General -> Include additional directories. Paste the
same paths found in the Dummy project. Tweak the $(ProjectDir)Engine environment variable to the
relative or absolute path of that directory from the new project's location 

Compile the Dummy project. It will produce a lib file (Dummy.lib) which will be stored
in the $(SolutionDir)/Debug or $(SolutionDir)/Release

Open the new project's linker options. Copy all the additional include directories and
all the lib files from dummy's linker. Then:
-> Add to the lib files Dummy.lib
-> Add to the additional directories $(SolutionDir)/Debug or $(SolutionDir)/Release depending
	on your configuration

Go to new project->Add->Reference and tic the dummy project. This will make sure every time
you compile your application also the Dummy.lib file will be recompiled if there were implemented
modifications to the engine

2) Source Code Implementation

To use this engine you need to create an Application.cpp file in a newly
created project and to implement the methods required.

A possible Application.cpp implementation can be:

///////////////////////////

#include "../application/Application.h"

class UserVars\n
{
public:
	friend class Application;
private:
	UserVars(){}
	~UserVars(){}
};

Application::Application(const Window& window)
{
	m_Window = (Window*)::operator new(sizeof(Window));
	memcpy(m_Window, &window, sizeof(Window));
}

Application::~Application()
{
	m_VertexManagers.clear();
	m_SceneObjs.clear();
	m_Textures.clear();
	m_CubeMaps.clear();
	m_Shaders.clear();
	m_Models.clear();
	m_CustomFrameBuffers.clear();
	::operator delete (m_Window);
}


void Application::OnUserCreate()
{
	//User code
}

void Application::OnUserRun()
{
	//User code

	while (!m_Window->ShouldClose())
	{
		m_Window->Update();
	}

}

///////////////////////////

NOTES: the class UserVars needs to be used to introduce user defined variables
for the current application

