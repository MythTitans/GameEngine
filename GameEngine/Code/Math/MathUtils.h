#pragma once

#include <glm/glm.hpp>

struct Segment;
struct Ray;
struct Plane;

glm::vec3	Project( const Segment& oSegment, const glm::vec3& vDirection );
bool		Intersect( const Ray& oRay, const Plane& oPlane, glm::vec3& vIntersection );

struct Ray
{
	Ray();
	Ray( const glm::vec3& vOrigin, const glm::vec3& vDirection );

	glm::vec3	m_vOrigin;
	glm::vec3	m_vDirection;
};

struct Segment
{
	Segment();
	Segment( const glm::vec3& vFrom, const glm::vec3& vTo );

	glm::vec3 m_vFrom;
	glm::vec3 m_vTo;
};

struct Plane
{
	Plane();
	Plane( const glm::vec3& vOrigin, const glm::vec3& vNormal );

	glm::vec3 m_vOrigin;
	glm::vec3 m_vNormal;
};