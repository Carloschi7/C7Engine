#pragma once
#include "GLFW/glfw3.h"

class InputEvent
{
public:
	InputEvent(int32_t key, int32_t state) : m_Key(key), m_State(state) {}
	~InputEvent() {}

	inline int32_t Key() const { return m_Key; }
	inline int32_t State() const { return m_State; }

private:
	int32_t m_Key, m_State;
};

class Window
{
public:
	Window(uint32_t width, uint32_t height, const char* title, bool bFullscreen);
	~Window();

	Window(const Window&) = delete;
	Window(Window&&) = delete;

	void Update();
	void Destroy();
	void SetWndInCurrentContext();
	bool IsKeyboardEvent(const InputEvent& ie) const;
	bool IsMouseEvent(const InputEvent& ie) const;
	bool IsMouseWheelUp() const;
	bool IsMouseWheelDown() const;
	void GetCursorCoord(double& x, double& y) const;
	void SetVsync(bool true_or_false) const;
	bool ShouldClose() const;

	inline uint32_t Width() const { return m_Width; }
	inline uint32_t Height() const { return m_Height; }

private: //Callbacks
	static void ScrollCallback(GLFWwindow* window, double offsetx, double offsety)
	{
		//Just retrieving the offsety(mouse wheel scroll) parameter
		s_MouseWheelY = offsety;
	}


private:
	GLFWwindow* m_Window;
	GLFWmonitor* m_Monitor;
	uint32_t m_Width, m_Height;
	bool m_Fullscreen;

	//Callback variables
	static double s_MouseWheelY;
};