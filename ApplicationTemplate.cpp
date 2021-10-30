#include "../application/Application.h"

class UserVars
{
public:
	friend class Application;
private:
	UserVars(){}
	~UserVars(){}
};

Application::Application(const Window& window)
{
	m_Window = (Window*)::operator new(sizeof(Window));
	memcpy(m_Window, &window, sizeof(Window));
}

Application::~Application()
{
	m_VertexManagers.clear();
	m_SceneObjs.clear();
	m_Textures.clear();
	m_CubeMaps.clear();
	m_Shaders.clear();
	m_Models.clear();
	m_CustomFrameBuffers.clear();
	::operator delete (m_Window);
}


void Application::OnUserCreate()
{
	//User code
}

void Application::OnUserRun()
{
	//User code

	while (!m_Window->ShouldClose())
	{
		m_Window->Update();
	}

}
