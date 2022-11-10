//Wrapper header for quick glfw/glew context init
#pragma once
#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "../application/Application.h"
#include "Window.h"

template<class... Args>
static Window InitContext(Args&&... args)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//Printing OpenGL version
	std::cout << glfwGetVersionString() << std::endl;

	Window w(std::forward<Args>(args)...);
	//Allows glew initialization
	w.AttachWndToCurrentContext();

	//GLEW
	if (glewInit() != GLEW_OK)
	{
		throw std::runtime_error("Could not initialize glew");
	}

	return std::move(w);
}

static void TerminateContext()
{
	glfwTerminate();
}