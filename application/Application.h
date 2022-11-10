#pragma once
#include "MainIncl.h"
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
	static void KeyboardForCameraFun(const Window& window, Camera* camera)
	{
		float fScalar = 0.6f;

		if (window.IsKeyboardEvent({GLFW_KEY_W, GLFW_PRESS}))
			camera->MoveTowardsFront(fScalar);
		if (window.IsKeyboardEvent({ GLFW_KEY_S, GLFW_PRESS }))
			camera->MoveTowardsFront(-fScalar);
		if (window.IsKeyboardEvent({ GLFW_KEY_A, GLFW_PRESS }))
			camera->StrafeX(-fScalar);
		if (window.IsKeyboardEvent({ GLFW_KEY_D, GLFW_PRESS }))
			camera->StrafeX(fScalar);
		if (window.IsKeyboardEvent({ GLFW_KEY_E, GLFW_PRESS }))
			camera->StrafeY(fScalar);
		if (window.IsKeyboardEvent({ GLFW_KEY_C, GLFW_PRESS }))
			camera->StrafeY(-fScalar);
	}

	static void MouseForCameraFun(const Window& window, Camera* camera, double x, double y, double dpi, double time)
	{
		double localx, localy;
		window.GetCursorCoord(localx, localy);

		camera->RotateX((localx - x) * dpi * time);
		camera->RotateY((y - localy) * dpi * time);
	}

private:
	Window m_Window;
	std::vector<VertexManager> m_VertexManagers;
	std::vector<Entity> m_SceneObjs;
	std::vector<Texture> m_Textures;
	std::vector<CubeMap> m_CubeMaps;
	std::vector<Shader> m_Shaders;
	std::vector<Model> m_Models;
	std::vector<FrameBuffer> m_CustomFrameBuffers;
	//Different working threads, can be used for logic, rendering, ...
	std::vector<std::thread> m_AppThreads;
	Camera m_Camera;
};