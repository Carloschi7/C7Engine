#pragma once
#include "GL/glew.h"
#include "GLFW/glfw3.h"
#include "utils/types.h"
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
#include <functional>
#include <format>


#define FlushErrors() while(glGetError() != GL_NO_ERROR)
#define ER(x) FlushErrors();\
x;\
while(GLenum e = glGetError())\
{\
	std::cout << "Error at line " << __LINE__ << ": " << e << std::endl;\
}

#define log_message(msg, ...) std::cout << std::format(msg, __VA_ARGS__);
#define assert(x, msg) if(!(x)) {\
        log_message("[ASSERTION FAILED]: in file: {}, on line: {}, msg: {}", __FILE__, __LINE__, msg);\
        *(int*)0 = 0;\
    }

#ifndef defer

struct DeferObject
{
	template<class... _Args, std::enable_if_t<std::is_constructible_v<std::function<void()>, _Args...>, int>  = 0>
	DeferObject(_Args&&... args) :_func(std::forward<_Args>(args)...) {}
	DeferObject(std::function<void()> func) : _func(func) {}
	~DeferObject() {
		_func();
	}

	std::function<void()> _func;
};

#define STR(x) #x
#define CONCAT(a, b, c) a##b##c
#define CONCAT_WITH(name, line) CONCAT(name, line)
#define defer DeferObject CONCAT_WITH(__internal_defer_obj_, __LINE__) = [&]()

#endif
