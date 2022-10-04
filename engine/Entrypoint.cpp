#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../application/Application.h"
#include "Window.h"

int main()
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Printing OpenGL version
	std::cout << glfwGetVersionString() << std::endl;

	std::unique_ptr<Window> w = WindowMaker::MakeWindow();
	//Allows glew initialization
	w->SetWndInCurrentContext();

	//GLEW
	if (glewInit() != GLEW_OK)
	{
		std::cout << "Could not initialize glew\n";
		return -1;
	}

	Application* app = new Application(*w);
	app->OnUserCreate();
	app->OnUserRun();

	delete app;
	
	glfwTerminate();
	return 0;
}
