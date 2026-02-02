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

	bool bVisible = true;

	for( uint uPlane = LEFT_PLANE; uPlane < FrustumPlane::_COUNT; ++uPlane )
	{
		glm::vec3 vPositive;
		vPositive.x = m_aPlanes[ uPlane ].m_vNormal.x > 0.f ? oAxisAlignedBox.m_vMax.x : oAxisAlignedBox.m_vMin.x;
		vPositive.y = m_aPlanes[ uPlane ].m_vNormal.y > 0.f ? oAxisAlignedBox.m_vMax.y : oAxisAlignedBox.m_vMin.y;
		vPositive.z = m_aPlanes[ uPlane ].m_vNormal.z > 0.f ? oAxisAlignedBox.m_vMax.z : oAxisAlignedBox.m_vMin.z;

		bVisible &= glm::dot( vPositive, m_aPlanes[ uPlane ].m_vNormal ) + m_aPlanes[ uPlane ].m_fDistance >= 0.f;
	}

	return bVisible;
}

void Frustum::AreVisible( bool& bVisibleA, bool& bVisibleB, bool& bVisibleC, bool& bVisibleD, const AxisAlignedBox& oAxisAlignedBoxA, const AxisAlignedBox& oAxisAlignedBoxB, const AxisAlignedBox& oAxisAlignedBoxC, const AxisAlignedBox& oAxisAlignedBoxD ) const
{
	bVisibleA = true;
	bVisibleB = true;
	bVisibleC = true;
	bVisibleD = true;

	for( uint uPlane = LEFT_PLANE; uPlane < FrustumPlane::_COUNT; ++uPlane )
	{
#if 1
		__m128 _vMinX = _mm_set_ps( oAxisAlignedBoxA.m_vMin.x, oAxisAlignedBoxB.m_vMin.x, oAxisAlignedBoxC.m_vMin.x, oAxisAlignedBoxD.m_vMin.x );
		__m128 _vMinY = _mm_set_ps( oAxisAlignedBoxA.m_vMin.y, oAxisAlignedBoxB.m_vMin.y, oAxisAlignedBoxC.m_vMin.y, oAxisAlignedBoxD.m_vMin.y );
		__m128 _vMinZ = _mm_set_ps( oAxisAlignedBoxA.m_vMin.z, oAxisAlignedBoxB.m_vMin.z, oAxisAlignedBoxC.m_vMin.z, oAxisAlignedBoxD.m_vMin.z );

		__m128 _vMaxX = _mm_set_ps( oAxisAlignedBoxA.m_vMax.x, oAxisAlignedBoxB.m_vMax.x, oAxisAlignedBoxC.m_vMax.x, oAxisAlignedBoxD.m_vMax.x );
		__m128 _vMaxY = _mm_set_ps( oAxisAlignedBoxA.m_vMax.y, oAxisAlignedBoxB.m_vMax.y, oAxisAlignedBoxC.m_vMax.y, oAxisAlignedBoxD.m_vMax.y );
		__m128 _vMaxZ = _mm_set_ps( oAxisAlignedBoxA.m_vMax.z, oAxisAlignedBoxB.m_vMax.z, oAxisAlignedBoxC.m_vMax.z, oAxisAlignedBoxD.m_vMax.z );

		__m128 _vValidX = _mm_cmple_ps( _vMinX, _vMaxX );
		__m128 _vValidY = _mm_cmple_ps( _vMinY, _vMaxY );
		__m128 _vValidZ = _mm_cmple_ps( _vMinZ, _vMaxZ );

		const int iValidityMask = _mm_movemask_ps( _mm_and_ps( _mm_and_ps( _vValidX, _vValidY ), _vValidZ ) );

		__m128 _vNormalX = _mm_set1_ps( m_aPlanes[ uPlane ].m_vNormal.x );
		__m128 _vNormalY = _mm_set1_ps( m_aPlanes[ uPlane ].m_vNormal.y );
		__m128 _vNormalZ = _mm_set1_ps( m_aPlanes[ uPlane ].m_vNormal.z );
		__m128 _vDistance = _mm_set1_ps( -m_aPlanes[ uPlane ].m_fDistance );

		__m128 _vPositiveX = _mm_blendv_ps( _vMaxX, _vMinX, _vNormalX );
		__m128 _vPositiveY = _mm_blendv_ps( _vMaxY, _vMinY, _vNormalY );
		__m128 _vPositiveZ = _mm_blendv_ps( _vMaxZ, _vMinZ, _vNormalZ );

		__m128 _vMulX = _mm_mul_ps( _vNormalX, _vPositiveX );
		__m128 _vMulY = _mm_mul_ps( _vNormalY, _vPositiveY );
		__m128 _vMulZ = _mm_mul_ps( _vNormalZ, _vPositiveZ );

		__m128 _vDot = _mm_add_ps( _mm_add_ps( _vMulX, _vMulY ), _vMulZ );

		const int iVisibilityMask =_mm_movemask_ps( _mm_cmpge_ps( _vDot, _vDistance ) );

		bVisibleA &= ( iValidityMask & 8 ) != 0 && ( iVisibilityMask & 8 ) != 0;
		bVisibleB &= ( iValidityMask & 4 ) != 0 && ( iVisibilityMask & 4 ) != 0;
		bVisibleC &= ( iValidityMask & 2 ) != 0 && ( iVisibilityMask & 2 ) != 0;
		bVisibleD &= ( iValidityMask & 1 ) != 0 && ( iVisibilityMask & 1 ) != 0;
#else
		bVisibleA = oAxisAlignedBoxA.IsValid();
		bVisibleB = oAxisAlignedBoxB.IsValid();
		bVisibleC = oAxisAlignedBoxC.IsValid();
		bVisibleD = oAxisAlignedBoxD.IsValid();

		glm::vec3 vPositiveA, vPositiveB, vPositiveC, vPositiveD;

		vPositiveA.x = m_aPlanes[ uPlane ].m_vNormal.x > 0.f ? oAxisAlignedBoxA.m_vMax.x : oAxisAlignedBoxA.m_vMin.x;
		vPositiveA.y = m_aPlanes[ uPlane ].m_vNormal.y > 0.f ? oAxisAlignedBoxA.m_vMax.y : oAxisAlignedBoxA.m_vMin.y;
		vPositiveA.z = m_aPlanes[ uPlane ].m_vNormal.z > 0.f ? oAxisAlignedBoxA.m_vMax.z : oAxisAlignedBoxA.m_vMin.z;

		vPositiveB.x = m_aPlanes[ uPlane ].m_vNormal.x > 0.f ? oAxisAlignedBoxB.m_vMax.x : oAxisAlignedBoxB.m_vMin.x;
		vPositiveB.y = m_aPlanes[ uPlane ].m_vNormal.y > 0.f ? oAxisAlignedBoxB.m_vMax.y : oAxisAlignedBoxB.m_vMin.y;
		vPositiveB.z = m_aPlanes[ uPlane ].m_vNormal.z > 0.f ? oAxisAlignedBoxB.m_vMax.z : oAxisAlignedBoxB.m_vMin.z;

		vPositiveC.x = m_aPlanes[ uPlane ].m_vNormal.x > 0.f ? oAxisAlignedBoxC.m_vMax.x : oAxisAlignedBoxC.m_vMin.x;
		vPositiveC.y = m_aPlanes[ uPlane ].m_vNormal.y > 0.f ? oAxisAlignedBoxC.m_vMax.y : oAxisAlignedBoxC.m_vMin.y;
		vPositiveC.z = m_aPlanes[ uPlane ].m_vNormal.z > 0.f ? oAxisAlignedBoxC.m_vMax.z : oAxisAlignedBoxC.m_vMin.z;

		vPositiveD.x = m_aPlanes[ uPlane ].m_vNormal.x > 0.f ? oAxisAlignedBoxD.m_vMax.x : oAxisAlignedBoxD.m_vMin.x;
		vPositiveD.y = m_aPlanes[ uPlane ].m_vNormal.y > 0.f ? oAxisAlignedBoxD.m_vMax.y : oAxisAlignedBoxD.m_vMin.y;
		vPositiveD.z = m_aPlanes[ uPlane ].m_vNormal.z > 0.f ? oAxisAlignedBoxD.m_vMax.z : oAxisAlignedBoxD.m_vMin.z;

		bVisibleA &= glm::dot( vPositiveA, m_aPlanes[ uPlane ].m_vNormal ) + m_aPlanes[ uPlane ].m_fDistance >= 0.f;
		bVisibleB &= glm::dot( vPositiveB, m_aPlanes[ uPlane ].m_vNormal ) + m_aPlanes[ uPlane ].m_fDistance >= 0.f;
		bVisibleC &= glm::dot( vPositiveC, m_aPlanes[ uPlane ].m_vNormal ) + m_aPlanes[ uPlane ].m_fDistance >= 0.f;
		bVisibleD &= glm::dot( vPositiveD, m_aPlanes[ uPlane ].m_vNormal ) + m_aPlanes[ uPlane ].m_fDistance >= 0.f;
#endif
	}
}
