#pragma once

#include <glm/fwd.hpp>

namespace glm
{
	glm::mat4x3 operator*( const glm::mat4x3& mA, const glm::mat4x3& mB );
	glm::mat4x3 inverse( const glm::mat4x3& mM );
}

glm::mat4 ToMat4( const glm::mat3& mM );
glm::mat4 ToMat4( const glm::mat4x3& mM );
