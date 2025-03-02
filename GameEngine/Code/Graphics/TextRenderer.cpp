#include "TextRenderer.h"

#include "Core/Profiler.h"
#include "Renderer.h"

static const std::string PARAM_POSITION( "position" );
static const std::string PARAM_SIZE( "size" );
static const std::string PARAM_GLYPH_OFFSET( "glyphOffset" );
static const std::string PARAM_GLYPH_SIZE( "glyphSize" );
static const std::string PARAM_GLYPH_COLOR( "glyphColor" );
static const std::string PARAM_ATLAS_TEXTURE( "atlasTexture" );

glm::vec2 PositionOnScreen( const glm::vec2& vPosition, const RenderContext& oRenderContext )
{
	const float fX = ( 2.f * vPosition.x / ( float )oRenderContext.GetRenderRect().m_uWidth ) - 1.f;
	const float fY = ( 2.f * vPosition.y / ( float )oRenderContext.GetRenderRect().m_uHeight ) - 1.f;

	return glm::vec2( fX, -fY );
}

glm::vec2 SizeOnScreen( const glm::vec2& vSize, const RenderContext& oRenderContext )
{
	const float fX = 2.f * vSize.x / ( float )oRenderContext.GetRenderRect().m_uWidth;
	const float fY = 2.f * vSize.y / ( float )oRenderContext.GetRenderRect().m_uHeight;

	return glm::vec2( fX, fY );
}

TextRenderer::TextRenderer()
	: m_xFont( g_pResourceLoader->LoadFont( "Roboto-Bold.ttf" ) )
	, m_xTextTechnique( g_pResourceLoader->LoadTechnique( "Shader/text.tech" ) )
{
	Array< glm::vec3 > aVertices( 4 );
	aVertices[ 0 ] = glm::vec3( 0.f, 0.f, 0.f );
	aVertices[ 1 ] = glm::vec3( 1.f, 0.f, 0.f );
	aVertices[ 2 ] = glm::vec3( 1.f, 1.f, 0.f );
	aVertices[ 3 ] = glm::vec3( 0.f, 1.f, 0.f );

	Array< GLuint > aIndices( 6 );
	aIndices[ 0 ] = 0;
	aIndices[ 1 ] = 1;
	aIndices[ 2 ] = 2;
	aIndices[ 3 ] = 0;
	aIndices[ 4 ] = 2;
	aIndices[ 5 ] = 3;

	Array< glm::vec2 > aUVs( 4 );
	aUVs[ 0 ] = glm::vec2( 0.f, 0.f );
	aUVs[ 1 ] = glm::vec2( 1.f, 0.f );
	aUVs[ 2 ] = glm::vec2( 1.f, 1.f );
	aUVs[ 3 ] = glm::vec2( 0.f, 1.f );

	m_oTextQuad = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();
}

TextRenderer::~TextRenderer()
{
	m_oTextQuad.Destroy();
}

void TextRenderer::RenderText( const Array< Text >& aTexts, const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Text" );

	glEnable( GL_BLEND );
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );

	const Technique& oTechnique = m_xTextTechnique->GetTechnique();

	glUseProgram( oTechnique.m_uProgramID );

	glActiveTexture( GL_TEXTURE0 );
	glBindTexture( GL_TEXTURE_2D, m_xFont->GetAtlas().m_uTextureID );

 	for( const Text& oText : aTexts )
 		DrawText( oText, oRenderContext );

	glBindVertexArray( 0 );
	glBindTexture( GL_TEXTURE_2D, 0 );

	glUseProgram( 0 );

	glDisable( GL_BLEND );
}

bool TextRenderer::OnLoading()
{
	return m_xFont->IsLoaded() && m_xTextTechnique->IsLoaded();
}

void TextRenderer::DrawText( const Text& oText, const RenderContext& oRenderContext )
{
	const Texture& oAtlas = m_xFont->GetAtlas();
	const Array< stbtt_packedchar >& aPackedCharacters = m_xFont->GetGlyphs();

	float fX = oText.m_vPosition.x;
	float fY = oText.m_vPosition.y + FontResource::FONT_HEIGHT;

	Technique& oTechnique = m_xTextTechnique->GetTechnique();

	oTechnique.SetParameter( PARAM_GLYPH_COLOR, oText.m_vColor );
	oTechnique.SetParameter( PARAM_ATLAS_TEXTURE, 0 );

	for( const char cCharacter : oText.m_sText )
	{
		stbtt_aligned_quad oQuad;

		stbtt_GetPackedQuad( aPackedCharacters.Data(), oAtlas.GetWidth(), oAtlas.GetHeight(), cCharacter - FontResource::FIRST_GLYPH, &fX, &fY, &oQuad, 1 );

		const glm::vec2 vGlyphSize( oQuad.x1 - oQuad.x0, oQuad.y1 - oQuad.y0 );

		const glm::vec2 vOffsetInAtlas( oQuad.s0, oQuad.t0 );
		const glm::vec2 vSizeInAtlas( oQuad.s1 - oQuad.s0, oQuad.t1 - oQuad.t0 );

		oTechnique.SetParameter( PARAM_POSITION, PositionOnScreen( glm::vec2( oQuad.x0, oQuad.y1 ), oRenderContext ) );
		oTechnique.SetParameter( PARAM_SIZE, SizeOnScreen( vGlyphSize, oRenderContext ) );
		oTechnique.SetParameter( PARAM_GLYPH_OFFSET, vOffsetInAtlas );
		oTechnique.SetParameter( PARAM_GLYPH_SIZE, vSizeInAtlas );

		glBindVertexArray( m_oTextQuad.m_uVertexArrayID );
		glDrawElements( GL_TRIANGLES, m_oTextQuad.m_iIndexCount, GL_UNSIGNED_INT, nullptr );
	}
}
