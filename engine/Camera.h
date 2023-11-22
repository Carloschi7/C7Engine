#pragma once

#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Window.h"
#include <functional>

class Camera;

//Old style
//typedef void (*KeyFun)(const Window&, Camera*);
//typedef void (*MouseFun)(const Window&, Camera*, f64, f64, f64, f64);

enum class CameraType
{
	ORTHOGRAPHIC = 0, PERSPECTIVE, UNDEFINED
};

class Camera
{
public:
	using KeyFun = std::function<void(const Window&, Camera*, f64)>;
	using MouseFun = std::function<void(const Window&, Camera*, f64, f64, f64, f64)>;
public:
	Camera();
	~Camera();
	Camera(const glm::vec3& pos, const glm::vec3& dir);

	/*FUNCTIONS FOR 3D PERSPECTIVE ENVIRONMENT*/

	void SetVectors(const glm::vec3& pos, const glm::vec3& dir);
	void StrafeX(f32 fSpeed);
	void StrafeY(f32 fSpeed);
	void MoveTowardsFront(f32 fSpeed);
	void RotateX(f32 fAngle);
	void RotateY(f32 fAngle);
	void SetMouseScrollSpeed(f32 fDpi) { m_Dpi = fDpi; }

	/*FUNCTIONS FOR 2D ORTHOGRAPHIC ENVIRONMENT*/

	void MoveX(f32 fSpeed);
	void MoveY(f32 fSpeed);

	void ProcessInput(const Window& window, f64 deltaTime);
	void ProcessInput(const Window& window, f64 keyDeltaTime, f64 mouseDeltaTime);
	void UpdateMousePosition(const Window& window);
	void SetKeyboardFunction(const KeyFun& kf);
	void SetMouseFunction(const MouseFun& mf);
	//Semi-deprecated
	const glm::vec3& GetPosition() const;
	const glm::vec3& GetFront() const;
	//Computes the orthogonal-up vector from the m_Front, can be useful to computed
	//relative rotated vectors to the current player view
	const glm::vec3 ComputeRelativeUp();
	glm::mat4 GetViewMatrix() const;
	const glm::mat4& GetProjMatrix() const;

	//Projection matrix setup
	void SetPerspectiveValues(f32 fAngle, f32 fAspect, f32 fNear, f32 fFar);
	void SetOrthographicValues(f32 fLeft, f32 fRight, f32 fBottom, f32 fTop);

	void Zoom(f32 fMultiplyRatio);

	//Global tools
	inline void ResetPosition() { position = { 0.0f, 0.0f, 0.0f }; }
	Camera& operator+=(const glm::vec3& vector);
	Camera& operator-=(const glm::vec3& vector);
	Camera& operator+=(const glm::vec2& vector);
	Camera& operator-=(const glm::vec2& vector);

private:
	void UpdateFrontCamera();
	void UpdateRelativeUp();
	void ClampAngleY();

public:
	glm::vec3 position;

private:
	//Variables for 3D camera, except position, which is used for both 3D and 2D
	f32 fAngleX, fAngleY, fZoom;
	f64 m_MouseX, m_MouseY, m_Dpi;
	glm::vec3 m_Front;
	glm::mat4 m_ProjMat;


	KeyFun keyfun;
	MouseFun mousefun;

	union
	{
		struct
		{
			f32 fAngle,fAspectRatio,fNear,fFar;
		};
		struct
		{
			f32 fLeft, fRight, fBottom, fTop;
		};
	} m_ProjParams;
	CameraType m_CameraType;
};