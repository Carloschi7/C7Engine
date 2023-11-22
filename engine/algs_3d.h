#pragma once
#include <glm/glm.hpp>
#include "utils/types.h"

namespace Algs3D{

	static glm::mat3 MatRotationX(f32 theta){
		return glm::mat3(
			{1,			0,				0},
			{0,			glm::cos(theta), 		-glm::sin(theta)},
			{0, 			glm::sin(theta),		glm::cos(theta)});
	}

	static glm::mat3 MatRotationY(f32 theta){
		return glm::mat3(
			{glm::cos(theta),	0,				glm::sin(theta)},
			{0,			1, 				0},
			{-glm::sin(theta),	0,				glm::cos(theta)});
	}

	static glm::mat3 MatRotationZ(f32 theta){
		return glm::mat3(
			{glm::cos(theta),	-glm::sin(theta),		0},
			{glm::sin(theta),	glm::cos(theta), 		0},
			{0, 			0,				1});
	}


	static glm::vec3 RotateVector(const glm::vec3& vec, const glm::vec3& rotation_axis, f32 theta){
		return vec * glm::cos(theta) + glm::cross(vec, rotation_axis) * glm::sin(theta) + rotation_axis * glm::dot(rotation_axis, vec) * (1 - glm::cos(theta));
	} 
}
