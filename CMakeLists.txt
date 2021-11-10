cmake_minimum_required(VERSION 3.12)

project(C7Engine)
set(CMAKE_GENERATOR_PLATFORM x86)
add_library(${PROJECT_NAME} STATIC engine/Origine.cpp
	engine/Camera.h 
	engine/Camera.cpp
	engine/Entity.h 
	engine/Entity.cpp
	engine/FrameBuffer.h 
	engine/FrameBuffer.cpp
	engine/MainIncl.h
	engine/Model.h 
	engine/Model.cpp
	engine/Shader.h 
	engine/Shader.cpp
	engine/Texture.h 
	engine/Texture.cpp
	engine/VertexManager.h 
	engine/VertexManager.cpp
	engine/Window.h 
	engine/Window.cpp
	application/Application.h)


#32 bit only for this version
target_link_libraries(${PROJECT_NAME}
	opengl32.lib
	${CMAKE_SOURCE_DIR}/Dependencies/ASSIMP/bin/assimp.lib
	${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/Win32/glew32s.lib
	${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2019/glfw3.lib)

target_include_directories(${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/engine 
	${PROJECT_SOURCE_DIR}/Dependencies/ASSIMP/include
	${PROJECT_SOURCE_DIR}/Dependencies/GLM
	${PROJECT_SOURCE_DIR}/Dependencies/stbi
	${PROJECT_SOURCE_DIR}/Dependencies/GLFW
	${PROJECT_SOURCE_DIR}/Dependencies/GLEW)

target_compile_definitions(${PROJECT_NAME} PUBLIC GLEW_STATIC)

target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_17)

