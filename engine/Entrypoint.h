//Wrapper header for quick glfw/glew context init
#pragma once
#include <iostream>
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Window.h"

template<class... Args>
static Window InitContext(Args&&... args)
{
	glfwInit();
	u32 major_version = 4, minor_version = 3;
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, major_version);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, minor_version);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	Window w(std::forward<Args>(args)...);
	//Allows glew initialization
	w.AttachWndToCurrentContext();

	log_message("{}\nOpenGL version: {}.{}\n", glfwGetVersionString(),
		major_version, minor_version);

	assert(glewInit() == GLEW_OK, "could not initialize glew");

	return w;
}

static void TerminateContext(Window& w)
{
	w.Destroy();
	glfwTerminate();
}