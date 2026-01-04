#include "Animation.h"

#include "Math/GLMHelpers.h"

NodeAnimation::NodeAnimation()
	: m_uMatrixIndex( 0 )
{
}

Animation::Animation()
	: m_fDuration( 0.f )
{
}

Skeleton::Skeleton()
	: m_uMatrixIndex( 0 )
{
}

void Skeleton::Update( const glm::mat4x3& mParentMatrix, ArrayView< glm::mat4x3 > aAnimationMatrices ) const
{
	aAnimationMatrices[ m_uMatrixIndex ] = mParentMatrix * aAnimationMatrices[ m_uMatrixIndex ];

	for( const Skeleton& oSkeleton : m_aChildren )
		oSkeleton.Update( aAnimationMatrices[ m_uMatrixIndex ], aAnimationMatrices );
}
