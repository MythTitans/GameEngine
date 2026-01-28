#pragma once

#include <glm/glm.hpp>

template < typename T >
class Array;

struct OrientedBox;

struct AxisAlignedBox
{
	static AxisAlignedBox	FromOrientedBox( const OrientedBox& oOrientedBox );
	
	AxisAlignedBox();

	glm::vec3				GetCenter() const;
	glm::vec3				GetExtends() const;

	bool					IsValid() const;

	glm::vec3 m_vMin;
	glm::vec3 m_vMax;
};

struct OrientedBox
{
	static OrientedBox FromAxisAlignedBox( const AxisAlignedBox& oAxisAlignedBox, const glm::mat4x3& mMatrix );

	glm::vec3 m_vCenter;
	glm::mat3 m_mExtends;
};

struct Plane
{
	static Plane FromPlaneEquation( const glm::vec4& vPlane );

	glm::vec3	m_vNormal;
	float		m_fDistance;
};

struct Frustum
{
	static Frustum	FromViewProjection( const glm::mat4& mViewProjection );

	bool			IsVisible( const AxisAlignedBox& oAxisAlignedBox ) const;

	enum FrustumPlane
	{
		LEFT_PLANE,
		RIGHT_PLANE,
		BOTTOM_PLANE,
		TOP_PLANE,
		NEAR_PLANE,
		FAR_PLANE,
		_COUNT
	};

	Plane m_aPlanes[ FrustumPlane::_COUNT ];
};

void FitAxisAlignedBox( AxisAlignedBox& oAABB, const Array< glm::vec3 >& aPoints );
