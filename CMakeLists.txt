cmake_minimum_required(VERSION 3.12)

project(C7Engine)
set(CMAKE_GENERATOR_PLATFORM x86)

set(C7ENGINE_SOURCE_FILES engine/Entrypoint.h
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
	engine/TextRenderer.h
	engine/TextRenderer.cpp
	engine/Window.h 
	engine/Window.cpp
	engine/utils/types.h
	application/Application.h)


string(SUBSTRING "${CMAKE_GENERATOR}" 0 13 VISUAL_STUDIO_DEF)
if("${VISUAL_STUDIO_DEF}" STREQUAL "Visual Studio")

	add_library(${PROJECT_NAME} STATIC ${C7ENGINE_SOURCE_FILES})
	
	#32 bit only for this version
	target_link_libraries(${PROJECT_NAME}
		opengl32.lib
		${CMAKE_SOURCE_DIR}/Dependencies/ASSIMP/lib/assimp-vc143-mtd.lib
		${CMAKE_SOURCE_DIR}/Dependencies/GLEW/lib/Release/x64/glew32sd.lib
		${CMAKE_SOURCE_DIR}/Dependencies/GLFW/lib-vc2019/glfw3.lib)

else() #for now only linux ubuntu
	set(UBUNTU_LIB_PATH /usr/lib/x86_64-linux-gnu)
	add_library(${PROJECT_NAME} SHARED ${C7ENGINE_SOURCE_FILES})
	target_link_libraries(${PROJECT_NAME} glfw GLEW ${UBUNTU_LIB_PATH}/libGL.so
		${UBUNTU_LIB_PATH}/libassimp.so)
endif()

target_include_directories(${PROJECT_NAME} PUBLIC
	${PROJECT_SOURCE_DIR}/engine 
	${PROJECT_SOURCE_DIR}/Dependencies/ASSIMP/include
	${PROJECT_SOURCE_DIR}/Dependencies/GLM
	${PROJECT_SOURCE_DIR}/Dependencies/stbi
	${PROJECT_SOURCE_DIR}/Dependencies/GLFW
	${PROJECT_SOURCE_DIR}/Dependencies/GLEW)

file(COPY assets/textures DESTINATION assets)
file(COPY assets/shaders DESTINATION assets)

target_compile_definitions(${PROJECT_NAME} PRIVATE C7MACRO_ENGINE_PATH="${CMAKE_CURRENT_LIST_DIR}")

#This is a preprocessor instruction required by glew to build as a static library
#check the header glew.h for further informations
target_compile_definitions(${PROJECT_NAME} PUBLIC GLEW_STATIC)

#Setting the default c++ version to c++17
target_compile_features(${PROJECT_NAME} PRIVATE cxx_std_20)

