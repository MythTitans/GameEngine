#pragma once

#include <glm/glm.hpp>

struct SegmentUtil;
struct RayUtil;
struct PlaneUtil;

glm::vec3	Project( const SegmentUtil& oSegment, const glm::vec3& vDirection );
bool		Intersect( const RayUtil& oRay, const PlaneUtil& oPlane, glm::vec3& vIntersection );

struct RayUtil
{
	RayUtil();
	RayUtil( const glm::vec3& vOrigin, const glm::vec3& vDirection );

	glm::vec3	m_vOrigin;
	glm::vec3	m_vDirection;
};

struct SegmentUtil
{
	SegmentUtil();
	SegmentUtil( const glm::vec3& vFrom, const glm::vec3& vTo );

	glm::vec3 m_vFrom;
	glm::vec3 m_vTo;
};

struct PlaneUtil
{
	PlaneUtil();
	PlaneUtil( const glm::vec3& vOrigin, const glm::vec3& vNormal );

	glm::vec3 m_vOrigin;
	glm::vec3 m_vNormal;
};