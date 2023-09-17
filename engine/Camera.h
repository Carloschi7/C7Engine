#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Window.h"
#include <functional>

class Camera;

//Old style
//typedef void (*KeyFun)(const Window&, Camera*);
//typedef void (*MouseFun)(const Window&, Camera*, double, double, double, double);

enum class CameraType
{
	ORTHOGRAPHIC = 0, PERSPECTIVE, UNDEFINED
};

class Camera
{
public:
	using KeyFun = std::function<void(const Window&, Camera*, double)>;
	using MouseFun = std::function<void(const Window&, Camera*, double, double, double, double)>;
public:
	Camera();
	~Camera();
	Camera(const glm::vec3& pos, const glm::vec3& dir);

	/*FUNCTIONS FOR 3D PERSPECTIVE ENVIRONMENT*/

	void SetVectors(const glm::vec3& pos, const glm::vec3& dir);
	void StrafeX(float fSpeed);
	void StrafeY(float fSpeed);
	void MoveTowardsFront(float fSpeed);
	void RotateX(float fAngle);
	void RotateY(float fAngle);
	void SetMouseScrollSpeed(float fDpi) { m_Dpi = fDpi; }

	/*FUNCTIONS FOR 2D ORTHOGRAPHIC ENVIRONMENT*/

	void MoveX(float fSpeed);
	void MoveY(float fSpeed);

	void ProcessInput(const Window& window, double deltaTime);
	void ProcessInput(const Window& window, double keyDeltaTime, double mouseDeltaTime);
	void SetKeyboardFunction(const KeyFun& kf);
	void SetMouseFunction(const MouseFun& mf);
	//Semi-deprecated
	const glm::vec3& GetPosition() const;
	const glm::vec3& GetFront() const;
	glm::mat4 GetViewMatrix() const;
	const glm::mat4& GetProjMatrix() const;

	//Projection matrix setup
	void SetPerspectiveValues(float fAngle, float fAspect, float fNear, float fFar);
	void SetOrthographicValues(float fLeft, float fRight, float fBottom, float fTop);

	void Zoom(float fMultiplyRatio);

	//Global tools
	inline void ResetPosition() { position = { 0.0f, 0.0f, 0.0f }; }
	Camera& operator+=(const glm::vec3& vector);
	Camera& operator-=(const glm::vec3& vector);
	Camera& operator+=(const glm::vec2& vector);
	Camera& operator-=(const glm::vec2& vector);

private:
	void UpdateFrontCamera();
	void ClampAngleY();

public:
	glm::vec3 position;

private:
	//Variables for 3D camera, except position, which is used for both 3D and 2D
	float fAngleX, fAngleY, fZoom;
	double m_MouseX, m_MouseY, m_Dpi;
	glm::vec3 m_Front;
	glm::mat4 m_ProjMat;


	KeyFun keyfun;
	MouseFun mousefun;

	union
	{
		struct
		{
			float fAngle,fAspectRatio,fNear,fFar;
		};
		struct
		{
			float fLeft, fRight, fBottom, fTop;
		};
	} m_ProjParams;
	CameraType m_CameraType;
};