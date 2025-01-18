#include "MathUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/projection.hpp"

glm::vec3 Project( const Segment& oSegment, const glm::vec3& vDirection )
{
	const glm::vec3 vSegmentDirection = oSegment.m_vTo - oSegment.m_vFrom;
	return oSegment.m_vFrom + glm::proj( vSegmentDirection, vDirection );
}

bool Intersect( const Ray& oRay, const Plane& oPlane, glm::vec3& vIntersection )
{
	float fDistance;
	if( glm::intersectRayPlane( oRay.m_vOrigin, oRay.m_vDirection, oPlane.m_vOrigin, oPlane.m_vNormal, fDistance ) )
	{
		vIntersection = oRay.m_vOrigin + oRay.m_vDirection * fDistance;
		return true;
	}

	return false;
}

Ray::Ray()
	: m_vOrigin( 0.f, 0.f, 0.f )
	, m_vDirection( 0.f, 0.f, 0.f )
{
}

Ray::Ray( const glm::vec3& vOrigin, const glm::vec3& vDirection )
	: m_vOrigin( vOrigin )
	, m_vDirection( glm::normalize( vDirection ) )
{
}

Segment::Segment()
	: m_vFrom( 0.f, 0.f, 0.f )
	, m_vTo( 0.f, 0.f, 0.f )
{
}

Segment::Segment( const glm::vec3& vFrom, const glm::vec3& vTo )
	: m_vFrom( vFrom )
	, m_vTo( vTo )
{
}

Plane::Plane()
	: m_vOrigin( 0.f, 0.f, 0.f )
	, m_vNormal( 0.f, 0.f, 0.f )
{
}

Plane::Plane( const glm::vec3& vOrigin, const glm::vec3& vNormal )
	: m_vOrigin( vOrigin )
	, m_vNormal( glm::normalize( vNormal ) )
{
}
