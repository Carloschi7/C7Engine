#include "Camera.h"

Camera::Camera()
	: Camera(glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 0.0f, -1.0f))
{
}

Camera::~Camera()
{

}

Camera::Camera(const glm::vec3& pos, const glm::vec3& dir)
	: fAngleX(0.0f), fAngleY(0.0f), fZoom(1.0f),
	m_MouseX(0.0f), m_MouseY(0.0f), m_Dpi(0.0025f),
	m_CameraType(CameraType::UNDEFINED)
{
	m_ProjParams.fAngle = 0.0f;
	m_ProjParams.fAspectRatio = 0.0f;
	m_ProjParams.fNear = 0.0f;
	m_ProjParams.fFar = 0.0f;

	SetVectors(pos, dir);
}

void Camera::SetVectors(const glm::vec3& pos, const glm::vec3& dir)
{
	m_Pos = pos;
	m_Front = dir;
}

void Camera::StrafeX(float fSpeed)
{
	m_Pos += glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f))) * fSpeed;
}

void Camera::StrafeY(float fSpeed)
{
	m_Pos += glm::vec3(0.0f, 1.0f, 0.0f) * fSpeed;
}

void Camera::MoveTowardsFront(float fSpeed)
{
	m_Pos += m_Front * fSpeed;
}

void Camera::RotateX(float fAngle)
{
	fAngleX += fAngle;
	UpdateFrontCamera();
}

void Camera::RotateY(float fAngle)
{
	fAngleY += fAngle;
	ClampAngleY();
	UpdateFrontCamera();
}

void Camera::MoveX(float fSpeed)
{
	m_Pos += glm::vec3(1.0f, 0.0f, 0.0f) * fSpeed * fZoom;
}

void Camera::MoveY(float fSpeed)
{
	m_Pos += glm::vec3(0.0f, 1.0f, 0.0f) * fSpeed * fZoom;
}

void Camera::ProcessInput(const Window& window, double deltaTime)
{
	if (keyfun)
		keyfun(window, this, deltaTime);

	if (mousefun)
	{
		if (m_MouseX == 0.0f && m_MouseY == 0.0f)
		{
			window.GetCursorCoord(m_MouseX, m_MouseY);
		}

		mousefun(window, this, m_MouseX, m_MouseY, m_Dpi, deltaTime);
		window.GetCursorCoord(m_MouseX, m_MouseY);
	}
}

void Camera::ProcessInput(const Window& window, double keyDeltaTime, double mouseDeltaTime)
{
	if (keyfun)
		keyfun(window, this, keyDeltaTime);

	if (mousefun)
	{
		if (m_MouseX == 0.0f && m_MouseY == 0.0f)
		{
			window.GetCursorCoord(m_MouseX, m_MouseY);
		}

		mousefun(window, this, m_MouseX, m_MouseY, m_Dpi, mouseDeltaTime);
		window.GetCursorCoord(m_MouseX, m_MouseY);
	}
}

void Camera::SetKeyboardFunction(const KeyFun& kf)
{
	keyfun = kf;
}

void Camera::SetMouseFunction(const MouseFun& mf)
{
	mousefun = mf;
}

glm::mat4 Camera::GetViewMatrix() const
{
	if (m_CameraType == CameraType::ORTHOGRAPHIC)
	{
		return glm::translate(glm::mat4(1.0f), m_Pos);
	}

	if (m_CameraType == CameraType::PERSPECTIVE)
	{
		return glm::lookAt(m_Pos, m_Pos + m_Front, glm::vec3(0.0f, 1.0f, 0.0f));
	}
	
}

const glm::vec3& Camera::GetPosition() const
{
	return m_Pos;
}

const glm::vec3& Camera::GetFront() const
{
	return m_Front;
}

void Camera::SetPerspectiveValues(float fAngle, float fAspect, float fNear, float fFar)
{
	assert(m_CameraType == CameraType::UNDEFINED);
	m_CameraType = CameraType::PERSPECTIVE;
	m_ProjParams.fAngle = fAngle;
	m_ProjParams.fAspectRatio = fAspect;
	m_ProjParams.fNear = fNear;
	m_ProjParams.fFar = fFar;
}

void Camera::SetOrthographicValues(float fLeft, float fRight, float fBottom, float fTop)
{
	assert(m_CameraType == CameraType::UNDEFINED);
	m_CameraType = CameraType::ORTHOGRAPHIC;
	m_ProjParams.fLeft = fLeft;
	m_ProjParams.fRight = fRight;
	m_ProjParams.fBottom = fBottom;
	m_ProjParams.fTop = fTop;
}

glm::mat4 Camera::GetProjMatrix() const
{
	if (m_CameraType == CameraType::ORTHOGRAPHIC)
	{
		glm::mat4 proj = glm::ortho(m_ProjParams.fLeft, m_ProjParams.fRight, m_ProjParams.fBottom, m_ProjParams.fTop);
		return proj;
	}

	if (m_CameraType == CameraType::PERSPECTIVE)
	{
		glm::mat4 proj = glm::perspective(m_ProjParams.fAngle, m_ProjParams.fAspectRatio, m_ProjParams.fNear, m_ProjParams.fFar);
		return proj;
	}
	
}

void Camera::Zoom(float fMultiplyRatio)
{
	if (m_CameraType == CameraType::ORTHOGRAPHIC)
	{
		m_ProjParams.fLeft *= fMultiplyRatio;
		m_ProjParams.fRight *= fMultiplyRatio;
		m_ProjParams.fBottom *= fMultiplyRatio;
		m_ProjParams.fTop *= fMultiplyRatio;
		fZoom *= fMultiplyRatio;
	}

	if (m_CameraType == CameraType::PERSPECTIVE)
	{
		m_ProjParams.fAngle *= fMultiplyRatio;
		fZoom *= fMultiplyRatio;
		if (m_ProjParams.fAngle > glm::pi<float>()) m_ProjParams.fAngle = glm::pi<float>();
		if (m_ProjParams.fAngle < 0.0f) m_ProjParams.fAngle = 0.0f;
	}
}

Camera& Camera::operator+=(const glm::vec3& vector)
{
	m_Pos += vector;
	return *this;
}

Camera& Camera::operator-=(const glm::vec3& vector)
{
	m_Pos -= vector;
	return *this;
}

Camera& Camera::operator+=(const glm::vec2& vector)
{
	m_Pos += glm::vec3(vector, 0.0f);
	return *this;
}

Camera& Camera::operator-=(const glm::vec2& vector)
{
	m_Pos -= glm::vec3(vector, 0.0f);
	return *this;	
}

void Camera::UpdateFrontCamera()
{
	m_Front.x = sinf(fAngleX) * cosf(fAngleY);
	m_Front.y = sinf(fAngleY);
	m_Front.z = -cosf(fAngleX) * cosf(fAngleY);
}

void Camera::ClampAngleY()
{
	if (fAngleY > glm::radians(89.0f))
		fAngleY = glm::radians(89.0f);
	if (fAngleY < glm::radians(-89.0f))
		fAngleY = glm::radians(-89.0f);
}


