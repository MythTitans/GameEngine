#include "MathUtils.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtx/intersect.hpp"
#include "glm/gtx/projection.hpp"

glm::vec3 Project( const SegmentUtil& oSegment, const glm::vec3& vDirection )
{
	const glm::vec3 vSegmentDirection = oSegment.m_vTo - oSegment.m_vFrom;
	return oSegment.m_vFrom + glm::proj( vSegmentDirection, vDirection );
}

bool Intersect( const RayUtil& oRay, const PlaneUtil& oPlane, glm::vec3& vIntersection )
{
	float fDistance;
	if( glm::intersectRayPlane( oRay.m_vOrigin, oRay.m_vDirection, oPlane.m_vOrigin, oPlane.m_vNormal, fDistance ) )
	{
		vIntersection = oRay.m_vOrigin + oRay.m_vDirection * fDistance;
		return true;
	}

	return false;
}

RayUtil::RayUtil()
	: m_vOrigin( 0.f, 0.f, 0.f )
	, m_vDirection( 0.f, 0.f, 0.f )
{
}

RayUtil::RayUtil( const glm::vec3& vOrigin, const glm::vec3& vDirection )
	: m_vOrigin( vOrigin )
	, m_vDirection( glm::normalize( vDirection ) )
{
}

SegmentUtil::SegmentUtil()
	: m_vFrom( 0.f, 0.f, 0.f )
	, m_vTo( 0.f, 0.f, 0.f )
{
}

SegmentUtil::SegmentUtil( const glm::vec3& vFrom, const glm::vec3& vTo )
	: m_vFrom( vFrom )
	, m_vTo( vTo )
{
}

PlaneUtil::PlaneUtil()
	: m_vOrigin( 0.f, 0.f, 0.f )
	, m_vNormal( 0.f, 0.f, 0.f )
{
}

PlaneUtil::PlaneUtil( const glm::vec3& vOrigin, const glm::vec3& vNormal )
	: m_vOrigin( vOrigin )
	, m_vNormal( glm::normalize( vNormal ) )
{
}
