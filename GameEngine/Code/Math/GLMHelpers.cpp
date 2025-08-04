#include "GLMHelpers.h"

#include <glm/glm.hpp>

namespace glm
{
	glm::mat4x3 operator*( const glm::mat4x3& mA, const glm::mat4x3& mB )
	{
		const glm::mat3 mRA( mA[ 0 ], mA[ 1 ], mA[ 2 ] );
		const glm::vec3 vTA( mA[ 3 ] );

		const glm::mat3 mRB( mB[ 0 ], mB[ 1 ], mB[ 2 ] );
		const glm::vec3 vTB( mB[ 3 ] );

		const glm::mat3 mR = mRA * mRB;
		const glm::vec3 vT = mRA * vTB + vTA;

		return glm::mat4x3( mR[ 0 ], mR[ 1 ], mR[ 2 ], vT );
	}

	glm::mat4x3 inverse( const glm::mat4x3& mM )
	{
		const glm::mat3 mR( mM[ 0 ], mM[ 1 ], mM[ 2 ] );
		const glm::vec3 vT( mM[ 3 ] );

		const glm::mat3 mRInverse( glm::inverse( mR ) );
		const glm::vec3 vTInverse( -mRInverse * vT );

		return glm::mat4x3( mRInverse[ 0 ], mRInverse[ 1 ], mRInverse[ 2 ], vTInverse );
	}
}

glm::mat4 ToMat4( const glm::mat4x3& mM )
{
	return glm::mat4( glm::vec4( mM[ 0 ], 0.f ), glm::vec4( mM[ 1 ], 0.f ), glm::vec4( mM[ 2 ], 0.f ), glm::vec4( mM[ 3 ], 1.f ) );
}
