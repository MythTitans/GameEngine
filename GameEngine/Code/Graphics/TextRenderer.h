#pragma once

#include "Core/ResourceLoader.h"
#include "TechniqueDefinition.h"

class RenderContext;

struct Text
{
	Text( const std::string& sText, const glm::vec2& vPosition )
		: m_sText( sText )
		, m_vPosition( vPosition )
	{
	}

	std::string m_sText;
	glm::vec2	m_vPosition;
};

class TextRenderer
{
public:
	TextRenderer();
	~TextRenderer();

	void Render( const RenderContext& oRenderContext );

private:
	Array< Text >			m_aTexts;

	FontResPtr				m_xFont;
	TechniqueResPtr			m_xTextTechnique;
	TextTechniqueDefinition	m_oTextTechniqueDefinition;
	Mesh					m_oTextQuad;
};