#pragma once

#include "Game/ResourceLoader.h"

class RenderContext;

struct Line
{
	Line( const glm::vec3& vFrom, const glm::vec3& vTo, const glm::vec3& vColor )
		: m_vFrom( vFrom )
		, m_vTo( vTo )
		, m_vColor( vColor )
	{
	}

	glm::vec3 m_vFrom;
	glm::vec3 m_vTo;
	glm::vec3 m_vColor;
};

struct Sphere
{
	Sphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor )
		: m_vPosition( vPosition )
		, m_fRadius( fRadius )
		, m_vColor( vColor )
	{
	}

	glm::vec3	m_vPosition;
	float		m_fRadius;
	glm::vec3	m_vColor;
};

struct Cylinder
{
	glm::vec3	m_vFrom;
	glm::vec3	m_vTo;
	float		m_fRadius;
	glm::vec3	m_vColor;
};

class DebugRenderer
{
public:
	DebugRenderer();
	~DebugRenderer();

	void RenderLines( const Array< Line >& aLines, const RenderContext& oRenderContext );
	void RenderSpheres( const Array< Sphere >& aSpheres, const RenderContext& oRenderContext );
	void RenderWireSpheres( const Array< Sphere >& aSpheres, const RenderContext& oRenderContext );
	void RenderWireCylinders( const Array< Cylinder >& aCylinders, const RenderContext& oRenderContext );
	void RenderWireCones( const Array< Cylinder >& aCylinders, const RenderContext& oRenderContext );

	bool OnLoading();

private:
	Array< GLfloat > GenerateSphereEquator();
	Array< GLfloat > GenerateSphereMeridians();
	Array< GLfloat > GenerateCylinderEquator( const glm::vec3& vNormal, const float fRadius );

	TechniqueResPtr	m_xLine;
	TechniqueResPtr	m_xSphere;

	GLuint			m_uVertexArrayID;
	GLuint			m_uVertexBufferID;
};