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
#include <format>


#define FlushErrors() while(glGetError() != GL_NO_ERROR)
#define ER(x) FlushErrors();\
x;\
while(GLenum e = glGetError())\
{\
	std::cout << "Error at line " << __LINE__ << ": " << e << std::endl;\
}

#define log_message(msg, ...) std::cout << std::format(msg, __VA_ARGS__)

#ifdef assert
#   undef assert
#endif

#define assert(x, msg) if(!(x)) {\
        log_message("[ASSERTION FAILED]: in file: {}, on line: {}, msg: {}", __FILE__, __LINE__, msg);\
        *(int*)0 = 0;\
    }

#ifndef defer

//If defer is already defined, the implementation should be very similar

template<class T>
struct Callable
{
    Callable(T&& _func) : func(_func) {}
    ~Callable() {func();}

    const T func;
};

//This exist as a mean to provide the clean defer call defer{}; without
//having to put up with multiple closing bracklets
struct MakeCallable
{
    template<class T>
    Callable<T> operator<<(T&& lambda)
    {
        return Callable<T>(std::forward<T>(lambda));
    }
};

MakeCallable __make_callable_util;

#define STRINGIFY(x) #x
#define CONCAT(a, b) a##b
#define CONCAT_WITH(a, b) CONCAT(a, b)
#define defer auto CONCAT_WITH(callable_, __LINE__) = __make_callable_util << [&]()


#endif
