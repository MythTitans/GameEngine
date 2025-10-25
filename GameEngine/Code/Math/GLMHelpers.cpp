#include "GLMHelpers.h"

#include <glm/glm.hpp>

namespace glm
{
	glm::mat4x3 operator*( const glm::mat4x3& mA, const glm::mat4x3& mB )
	{
		const glm::mat3 mRA( mA[ 0 ], mA[ 1 ], mA[ 2 ] );
		const glm::mat3 mR = mRA * glm::mat3( mB[ 0 ], mB[ 1 ], mB[ 2 ] );

		return glm::mat4x3( mR[ 0 ], mR[ 1 ], mR[ 2 ], mRA * mB[ 3 ] + mA[ 3 ] );
	}

	glm::mat4x3 inverse( const glm::mat4x3& mM )
	{
		const glm::mat3 mRInverse( glm::inverse( glm::mat3( mM[ 0 ], mM[ 1 ], mM[ 2 ] ) ) );

		return glm::mat4x3( mRInverse[ 0 ], mRInverse[ 1 ], mRInverse[ 2 ], -mRInverse * mM[ 3 ] );
	}
}

glm::mat4 ToMat4( const glm::mat3& mM )
{
	return glm::mat4( glm::vec4( mM[ 0 ], 0.f ), glm::vec4( mM[ 1 ], 0.f ), glm::vec4( mM[ 2 ], 0.f ), glm::vec4( 0.f, 0.f, 0.f, 1.f ) );
}

glm::mat4 ToMat4( const glm::mat4x3& mM )
{
	return glm::mat4( glm::vec4( mM[ 0 ], 0.f ), glm::vec4( mM[ 1 ], 0.f ), glm::vec4( mM[ 2 ], 0.f ), glm::vec4( mM[ 3 ], 1.f ) );
}

glm::vec3 TransformPoint( const glm::mat4x3& mA, const glm::vec3& vB )
{
	return glm::mat3( mA[ 0 ], mA[ 1 ], mA[ 2 ] ) * vB + mA[ 3 ];
}

glm::vec3 TransformDirection( const glm::mat4x3& mA, const glm::vec3& vB )
{
	return glm::mat3( mA[ 0 ], mA[ 1 ], mA[ 2 ] ) * vB;
}
