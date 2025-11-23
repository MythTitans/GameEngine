#pragma once

#include "Game/ResourceTypes.h"
#include "Mesh.h"
#include "Technique.h"

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

	bool OnLoading();
	void OnLoaded();

private:
	enum class TextParam : uint8
	{
		POSITION,
		SIZE,
		GLYPH_OFFSET,
		GLYPH_SIZE,
		GLYPH_COLOR,
		ATLAS_TEXTURE,
		_COUNT
	};

	void DrawText( const Text& oText, const RenderContext& oRenderContext );

	FontResPtr					m_xFont;
	TechniqueResPtr				m_xTextTechnique;
	PARAM_SHEET( TextParam )	m_oTextSheet;
	Mesh						m_oTextQuad;
};