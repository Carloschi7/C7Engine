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
	position = pos;
	m_Front = dir;
}

void Camera::StrafeX(f32 fSpeed)
{
	position += glm::normalize(glm::cross(m_Front, glm::vec3(0.0f, 1.0f, 0.0f))) * fSpeed;
}

void Camera::StrafeY(f32 fSpeed)
{
	position += glm::vec3(0.0f, 1.0f, 0.0f) * fSpeed;
}

void Camera::MoveTowardsFront(f32 fSpeed)
{
	position += m_Front * fSpeed;
}

void Camera::RotateX(f32 fAngle)
{
	fAngleX += fAngle;
	UpdateFrontCamera();
}

void Camera::RotateY(f32 fAngle)
{
	fAngleY += fAngle;
	ClampAngleY();
	UpdateFrontCamera();
}

void Camera::MoveX(f32 fSpeed)
{
	position += glm::vec3(1.0f, 0.0f, 0.0f) * fSpeed * fZoom;
}

void Camera::MoveY(f32 fSpeed)
{
	position += glm::vec3(0.0f, 1.0f, 0.0f) * fSpeed * fZoom;
}

void Camera::ProcessInput(const Window& window, f64 deltaTime)
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

void Camera::ProcessInput(const Window& window, f64 keyDeltaTime, f64 mouseDeltaTime)
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

void Camera::UpdateMousePosition(const Window& window)
{
	window.GetCursorCoord(m_MouseX, m_MouseY);
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
	switch (m_CameraType) 
	{
	case CameraType::ORTHOGRAPHIC:
		return glm::translate(glm::mat4(1.0f), position);
	case CameraType::PERSPECTIVE:
		return glm::lookAt(position, position + m_Front, glm::vec3(0.0f, 1.0f, 0.0f));
	}
}

const glm::mat4& Camera::GetProjMatrix() const
{
	return m_ProjMat;
}

const glm::vec3& Camera::GetPosition() const
{
	return position;
}

const glm::vec3& Camera::GetFront() const
{
	return m_Front;
}

const glm::vec3 Camera::ComputeRelativeUp()
{
	glm::vec3 result;
	result.x = -glm::sin(fAngleY) * glm::sin(fAngleX);
	result.y = glm::cos(fAngleY);
	result.z = glm::cos(fAngleX) * glm::sin(fAngleY);
	return result;
}

void Camera::SetPerspectiveValues(f32 fAngle, f32 fAspect, f32 fNear, f32 fFar)
{
	assert(m_CameraType == CameraType::UNDEFINED);
	m_CameraType = CameraType::PERSPECTIVE;
	m_ProjParams.fAngle = fAngle;
	m_ProjParams.fAspectRatio = fAspect;
	m_ProjParams.fNear = fNear;
	m_ProjParams.fFar = fFar;

	//Set matrix as perspective
	m_ProjMat = glm::perspective(fAngle, fAspect, fNear, fFar);
}

void Camera::SetOrthographicValues(f32 fLeft, f32 fRight, f32 fBottom, f32 fTop)
{
	assert(m_CameraType == CameraType::UNDEFINED);
	m_CameraType = CameraType::ORTHOGRAPHIC;
	m_ProjParams.fLeft = fLeft;
	m_ProjParams.fRight = fRight;
	m_ProjParams.fBottom = fBottom;
	m_ProjParams.fTop = fTop;

	//Set matrix as ortho
	m_ProjMat = glm::ortho(fLeft, fRight, fBottom, fTop);
}



void Camera::Zoom(f32 fMultiplyRatio)
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
		if (m_ProjParams.fAngle > glm::pi<f32>()) m_ProjParams.fAngle = glm::pi<f32>();
		if (m_ProjParams.fAngle < 0.0f) m_ProjParams.fAngle = 0.0f;
	}
}

Camera& Camera::operator+=(const glm::vec3& vector)
{
	position += vector;
	return *this;
}

Camera& Camera::operator-=(const glm::vec3& vector)
{
	position -= vector;
	return *this;
}

Camera& Camera::operator+=(const glm::vec2& vector)
{
	position += glm::vec3(vector, 0.0f);
	return *this;
}

Camera& Camera::operator-=(const glm::vec2& vector)
{
	position -= glm::vec3(vector, 0.0f);
	return *this;	
}

void Camera::UpdateFrontCamera()
{
	m_Front.x = glm::sin(fAngleX) * glm::cos(fAngleY);
	m_Front.y = glm::sin(fAngleY);
	m_Front.z = -glm::cos(fAngleX) * glm::cos(fAngleY);
}

void Camera::UpdateRelativeUp()
{
	m_RelativeUp.x = -glm::sin(fAngleY) * glm::sin(fAngleX);
	m_RelativeUp.y = glm::cos(fAngleY);
	m_RelativeUp.z = glm::cos(fAngleX) * glm::sin(fAngleY);
}

void Camera::ClampAngleY()
{
	if (fAngleY > glm::radians(89.0f))
		fAngleY = glm::radians(89.0f);
	if (fAngleY < glm::radians(-89.0f))
		fAngleY = glm::radians(-89.0f);
}


