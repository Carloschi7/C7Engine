#pragma once
#include <glm/glm.hpp>
#include "utils/types.h"

#ifndef NO_ASSIMP
	template<typename TReal> class aiMatrix4x4t;
	template<typename TReal> class aiQuaterniont;
#endif

namespace gfx
{
	glm::mat3 rotation_matrix_axis_x(f32 theta);
	glm::mat3 rotation_matrix_axis_y(f32 theta);
	glm::mat3 rotation_matrix_axis_z(f32 theta);

	glm::vec3 rotate_vector(const glm::vec3& vec, const glm::vec3& rotation_axis, f32 theta);

	//INFO @C7, if this flag is specified, assimp and also its headers might not be defined here

#ifndef NO_ASSIMP
	glm::mat4 glm_mat_cast(const aiMatrix4x4t<f32>& matrix);
	glm::quat glm_quat_cast(const aiQuaterniont<f32>& q);
#endif
	bool      matrix_epsilon_check(const glm::mat4& m1, const glm::mat4& m2, f32 epsilon);
}

