#pragma once

#include "Game/ResourceLoader.h"
#include "TechniqueDefinition.h"

class RenderContext;

struct Text
{
	Text( const std::string& sText, const glm::vec2& vPosition, const glm::vec4& vColor )
		: m_sText( sText )
		, m_vPosition( vPosition )
		, m_vColor( vColor )
	{
	}

	std::string m_sText;
	glm::vec2	m_vPosition;
	glm::vec4	m_vColor;
};

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void RenderText( const Array< Text >& aTexts, const RenderContext& oRenderContext );
	void RenderText( const Text& oText, const RenderContext& oRenderContext );

	bool OnLoading();

private:
	void DrawText( const Text& oText, const RenderContext& oRenderContext );

	FontResPtr				m_xFont;
	TechniqueResPtr			m_xTextTechnique;
	TextTechniqueDefinition	m_oTextTechniqueDefinition;
	Mesh					m_oTextQuad;
};