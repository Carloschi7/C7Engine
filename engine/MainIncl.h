#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "Camera.h"
#include "Entity.h"
#include "FrameBuffer.h"
#ifndef NO_ASSIMP
	#include "Model.h"
#endif
#include "Shader.h"
#include "Texture.h"
#include "VertexManager.h"
#include "Window.h"

#define FlushErrors() while(glGetError() != GL_NO_ERROR)
#define ER(x) FlushErrors();\
x;\
while(GLenum e = glGetError())\
{\
	std::cout << "Error at line " << __LINE__ << ": " << e << std::endl;\
}
