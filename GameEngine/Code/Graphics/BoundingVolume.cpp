#include "BoundingVolume.h"

#include "Core/Array.h"
#include "Math/GLMHelpers.h"

AxisAlignedBox AxisAlignedBox::FromOrientedBox( const OrientedBox& oOrientedBox )
{
	const glm::vec3 vWorldExtends = glm::abs( oOrientedBox.m_mExtends[ 0 ] ) + glm::abs( oOrientedBox.m_mExtends[ 1 ] ) + glm::abs( oOrientedBox.m_mExtends[ 2 ] );

	AxisAlignedBox oAxisAlignedBox;
	oAxisAlignedBox.m_vMin = oOrientedBox.m_vCenter - vWorldExtends;
	oAxisAlignedBox.m_vMax = oOrientedBox.m_vCenter + vWorldExtends;

	return oAxisAlignedBox;
}

AxisAlignedBox::AxisAlignedBox()
	: m_vMin( glm::vec3( FLT_MAX ) )
	, m_vMax( glm::vec3( -FLT_MAX ) )
{
}

glm::vec3 AxisAlignedBox::GetCenter() const
{
	return 0.5f * ( m_vMin + m_vMax );
}

glm::vec3 AxisAlignedBox::GetExtends() const
{
	return 0.5f * ( m_vMax - m_vMin );
}

bool AxisAlignedBox::IsValid() const
{
	return m_vMin.x <= m_vMax.x && m_vMin.y <= m_vMax.y && m_vMin.z <= m_vMax.z;
}

void FitAxisAlignedBox( AxisAlignedBox& oAABB, const Array< glm::vec3 >& aVertices )
{
	if( aVertices.Empty() )
		return;

	for( const glm::vec3& vVertex : aVertices )
	{
		oAABB.m_vMin = glm::min( oAABB.m_vMin, vVertex );
		oAABB.m_vMax = glm::max( oAABB.m_vMax, vVertex );
	}
}

OrientedBox OrientedBox::FromAxisAlignedBox( const AxisAlignedBox& oAxisAlignedBox, const glm::mat4x3& mMatrix )
{
	const glm::vec3 vCenter = oAxisAlignedBox.GetCenter();
	const glm::vec3 vExtends = oAxisAlignedBox.GetExtends();
	const glm::mat3 mScaledRotation( mMatrix );

	OrientedBox oOrientedBox;
	oOrientedBox.m_vCenter = TransformPoint( mMatrix, vCenter );
	oOrientedBox.m_mExtends[ 0 ] = mScaledRotation[ 0 ] * vExtends.x;
	oOrientedBox.m_mExtends[ 1 ] = mScaledRotation[ 1 ] * vExtends.y;
	oOrientedBox.m_mExtends[ 2 ] = mScaledRotation[ 2 ] * vExtends.z;

	return oOrientedBox;
}

Plane Plane::FromPlaneEquation( const glm::vec4& vPlane )
{
	Plane oPlane;
	oPlane.m_vNormal = vPlane;
	oPlane.m_fDistance = vPlane.w;

	const float fInverseNormalLength = 1.f / glm::length( oPlane.m_vNormal );
	oPlane.m_vNormal *= fInverseNormalLength;
	oPlane.m_fDistance *= fInverseNormalLength;

	return oPlane;
}

Frustum Frustum::FromViewProjection( const glm::mat4& mViewProjection )
{
	const glm::mat4 mViewProjectionTranspose = glm::transpose( mViewProjection );

	Frustum oFrustum;
	oFrustum.m_aPlanes[ FrustumPlane::LEFT_PLANE ] = Plane::FromPlaneEquation( mViewProjectionTranspose[ 3 ] + mViewProjectionTranspose[ 0 ] );
	oFrustum.m_aPlanes[ FrustumPlane::RIGHT_PLANE ] = Plane::FromPlaneEquation( mViewProjectionTranspose[ 3 ] - mViewProjectionTranspose[ 0 ] );
	oFrustum.m_aPlanes[ FrustumPlane::BOTTOM_PLANE ] = Plane::FromPlaneEquation( mViewProjectionTranspose[ 3 ] + mViewProjectionTranspose[ 1 ] );
	oFrustum.m_aPlanes[ FrustumPlane::TOP_PLANE ] = Plane::FromPlaneEquation( mViewProjectionTranspose[ 3 ] - mViewProjectionTranspose[ 1 ] );
	oFrustum.m_aPlanes[ FrustumPlane::NEAR_PLANE ] = Plane::FromPlaneEquation( mViewProjectionTranspose[ 3 ] + mViewProjectionTranspose[ 2 ] );
	oFrustum.m_aPlanes[ FrustumPlane::FAR_PLANE ] = Plane::FromPlaneEquation( mViewProjectionTranspose[ 3 ] - mViewProjectionTranspose[ 2 ] );

	return oFrustum;
}

bool Frustum::IsVisible( const AxisAlignedBox& oAxisAlignedBox ) const
{
	if( oAxisAlignedBox.IsValid() == false )
		return false;

	for( uint uPlane = LEFT_PLANE; uPlane < FrustumPlane::_COUNT; ++uPlane )
	{
		glm::vec3 vPositive;
		vPositive.x = m_aPlanes[ uPlane ].m_vNormal.x > 0.f ? oAxisAlignedBox.m_vMax.x : oAxisAlignedBox.m_vMin.x;
		vPositive.y = m_aPlanes[ uPlane ].m_vNormal.y > 0.f ? oAxisAlignedBox.m_vMax.y : oAxisAlignedBox.m_vMin.y;
		vPositive.z = m_aPlanes[ uPlane ].m_vNormal.z > 0.f ? oAxisAlignedBox.m_vMax.z : oAxisAlignedBox.m_vMin.z;

		if( glm::dot( vPositive, m_aPlanes[ uPlane ].m_vNormal ) + m_aPlanes[ uPlane ].m_fDistance < 0.f )
			return false;
	}

	return true;
}
