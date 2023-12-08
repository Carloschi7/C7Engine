#pragma once
#include "MainIncl.h"
#include "utils/types.h"
#include <memory>
#include <thread>

class Application
{
public:
	Application();
	~Application();

	void OnUserCreate();
	void OnUserRun();
private:
	//3D applications utilities
	static void KeyboardForCameraFun(const Window& window, Camera* camera, f64 time)
	{
		f32 fScalar = 0.6f;

		if (window.IsKeyboardEvent({GLFW_KEY_W, GLFW_PRESS}))
			camera->MoveTowardsFront(fScalar * time);
		if (window.IsKeyboardEvent({ GLFW_KEY_S, GLFW_PRESS }))
			camera->MoveTowardsFront(-fScalar * time);
		if (window.IsKeyboardEvent({ GLFW_KEY_A, GLFW_PRESS }))
			camera->StrafeX(-fScalar * time);
		if (window.IsKeyboardEvent({ GLFW_KEY_D, GLFW_PRESS }))
			camera->StrafeX(fScalar * time);
		if (window.IsKeyboardEvent({ GLFW_KEY_E, GLFW_PRESS }))
			camera->StrafeY(fScalar * time);
		if (window.IsKeyboardEvent({ GLFW_KEY_C, GLFW_PRESS }))
			camera->StrafeY(-fScalar * time);
	}

	static void MouseForCameraFun(const Window& window, Camera* camera, f64 x, f64 y, f64 dpi, f64 time)
	{
		f64 localx, localy;
		window.GetCursorCoord(localx, localy);

		camera->RotateX((localx - x) * dpi * time);
		camera->RotateY((y - localy) * dpi * time);
	}

private:
	//The window needs to be defined (the InitContext function in Entrypoint.h is recommended)
	//If your app is multithreaded, the main thread needs to be the one which handles opengl
	//stuff
	Window m_Window;
	std::vector<VertexManager> m_VertexManagers;
	std::vector<Entity> m_SceneObjs;
	std::vector<Texture> m_Textures;
	std::vector<CubeMap> m_CubeMaps;
	std::vector<Shader> m_Shaders;
#ifndef NO_ASSIMP
	std::vector<Model> m_Models;
#endif
	std::vector<FrameBuffer> m_CustomFrameBuffers;
	//Different working threads, can be used for logic, rendering, ...
	std::vector<std::thread> m_AppThreads;
	Camera m_Camera;
};
