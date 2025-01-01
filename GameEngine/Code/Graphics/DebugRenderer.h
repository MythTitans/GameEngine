#pragma once

#include "Game/ResourceLoader.h"
#include "TechniqueDefinition.h"

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

class DebugRenderer
{
public:
	DebugRenderer();
	~DebugRenderer();

	void RenderLines( const Array< Line >& aLines, const RenderContext& oRenderContext );

	bool OnLoading();

private:
	TechniqueResPtr			m_xLine;
	LineTechniqueDefinition m_oLine;

	GLuint					m_uVertexArrayID;
	GLuint					m_uVertexBufferID;
};