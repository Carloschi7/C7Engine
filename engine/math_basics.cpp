#include "math_basics.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/quaternion.hpp>

#ifndef NO_ASSIMP
#	include <assimp/quaternion.h>
#	include <assimp/matrix4x4.h>
#endif

namespace gfx
{
	glm::mat3 rotation_matrix_axis_x(f32 theta)
	{
		return glm::mat3(
			{ 1,               0,                 0},
			{ 0,               glm::cos(theta),   -glm::sin(theta)},
			{ 0, 			   glm::sin(theta),   glm::cos(theta)});
	}

	glm::mat3 rotation_matrix_axis_y(f32 theta)
	{
		return glm::mat3(
			{ glm::cos(theta), 0,                  glm::sin(theta)},
			{ 0,               1,                  0},
			{-glm::sin(theta), 0,                  glm::cos(theta)});
	}

	glm::mat3 rotation_matrix_axis_z(f32 theta)
	{
		return glm::mat3(
			{glm::cos(theta),  -glm::sin(theta),   0},
			{glm::sin(theta),	glm::cos(theta),   0},
			{0,                 0,				   1});
	}

	static glm::vec3 RotateVector(const glm::vec3& vec, const glm::vec3& rotation_axis, f32 theta)
	{
		return vec * glm::cos(theta) + glm::cross(vec, rotation_axis) * glm::sin(theta) + rotation_axis * glm::dot(rotation_axis, vec) * (1 - glm::cos(theta));
	}

	//apparently aiMatrix4x4 stores the data in a trensposed way compared to glm::mat4
	glm::mat4 glm_mat_cast(const aiMatrix4x4t<f32>& matrix)
	{
		return glm::transpose(glm::make_mat4((f32*)&matrix));
	}

	glm::quat glm_quat_cast(const aiQuaterniont<f32>& q)
	{
		glm::quat ret;
		ret.x = q.x;
		ret.y = q.y;
		ret.z = q.z;
		ret.w = q.w;
		return ret;
	}

	bool matrix_epsilon_check(const glm::mat4& m1, const glm::mat4& m2, f32 epsilon)
	{
		for(u32 i = 0; i < 4; i++)
			for(u32 j = 0; j < 4; j++)
				if(glm::abs(m1[i][j] - m1[i][j]) > epsilon)
					return false;

		return true;
	}

}
