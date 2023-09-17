#pragma once
#include "GLFW/glfw3.h"
#include "utils/types.h"

#ifndef C7_STATE_BUFFER_INTERVALS
#	define C7_STATE_BUFFER_INTERVALS
#	define PRINTABLE_INTERVAL (GLFW_KEY_WORLD_2 - GLFW_KEY_SPACE + 1)
#	define FUNCTION_INTERVAL (GLFW_KEY_MENU - GLFW_KEY_ESCAPE + 1)
#	define MOUSE_INTERVAL (GLFW_MOUSE_BUTTON_8 - GLFW_MOUSE_BUTTON_1 + 1)
#endif

class Window;

enum class KeyType { None, Printable, Function, Mouse };

//Finds whether or not a key is pressed in the current frame
class InputEvent
{
public:
	InputEvent(i32 key, i32 state) : m_Key(key), m_State(state) {}
	~InputEvent() {}

	inline i32 Key() const { return m_Key; }
	inline i32 State() const { return m_State; }

private:
	i32 m_Key, m_State;
};

//Implements detection of a key being pressed, held down or released
class InputState
{
public:
	InputState() {}
	void Update(const Window* window);

	bool IsKeyPressed(u16 key) const;
	bool IsKeyHeld(u16 key) const;
	bool IsKeyReleased(u16 key) const;

private:
	KeyType _GetKeyType(u16 key) const;
private:
	struct InternalInputState
	{
		bool printable_keys[PRINTABLE_INTERVAL]{0};
		bool function_keys[FUNCTION_INTERVAL]{0};
		bool mouse_keys[MOUSE_INTERVAL]{0};
	};

	InternalInputState m_CurrentState, m_OldState;
};

class Window
{
public:
	Window(u32 width, u32 height, const char* title, bool bFullscreen);
	~Window();

	Window(const Window&) = delete;
	Window(Window&& wnd) noexcept;

	void Update() const;
	void UpdateKeys();

	void Destroy();
	static void ClearScreen(u32 flags);
	void AttachWndToCurrentContext() const;
	void DetachWndFromContext() const;
	//Directly access the GLFW API
	bool IsKeyboardEvent(const InputEvent& ie) const;
	bool IsMouseEvent(const InputEvent& ie) const;

	bool IsKeyPressed(u16 key) const;
	bool IsKeyHeld(u16 key) const;
	bool IsKeyReleased(u16 key) const;

	bool IsMouseWheelUp() const;
	bool IsMouseWheelDown() const;
	void GetCursorCoord(double& x, double& y) const;
	void SetVsync(bool true_or_false) const;
	bool ShouldClose() const;
	void EnableCursor();
	void DisableCursor();

	inline u32 Width() const { return m_Width; }
	inline u32 Height() const { return m_Height; }

private: //Callbacks
	static void ScrollCallback(GLFWwindow* window, double offsetx, double offsety)
	{
		//Just retrieving the offsety(mouse wheel scroll) parameter
		s_MouseWheelY = offsety;
	}


private:
	GLFWwindow* m_Window;
	GLFWmonitor* m_Monitor;
	//Handles key states more accurately
	InputState m_InputState;
	u32 m_Width, m_Height;
	bool m_Fullscreen;

	//Callback variables
	static double s_MouseWheelY;
};