#include "TechniqueDefinition.h"

#include <glm/gtc/type_ptr.hpp>

#include "Technique.h"

TechniqueDefinitionBase::TechniqueDefinitionBase()
	: m_bValid( false )
{
}

TechniqueDefinitionBase::~TechniqueDefinitionBase()
{
	m_bValid = false;
}

void TechniqueDefinitionBase::Create( const Technique& oTechnique )
{
	CreateDefinition( oTechnique );

	m_bValid = true;
}

bool TechniqueDefinitionBase::IsValid() const
{
	return m_bValid;
}

BasicTechniqueDefinition::BasicTechniqueDefinition()
	: m_uViewUniform( GL_INVALID_VALUE )
	, m_uProjectionUniform( GL_INVALID_VALUE )
{
}

void BasicTechniqueDefinition::SetView( const glm::mat4& mView )
{
	glUniformMatrix4fv( m_uViewUniform, 1, GL_FALSE, glm::value_ptr( mView ) );
}

void BasicTechniqueDefinition::SetProjection( const glm::mat4& mProjection )
{
	glUniformMatrix4fv( m_uProjectionUniform, 1, GL_FALSE, glm::value_ptr( mProjection ) );
}

void BasicTechniqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_uViewUniform = oTechnique.GetParameterID( "view" );
	m_uProjectionUniform = oTechnique.GetParameterID( "projection" );
}

TextTechniqueDefinition::TextTechniqueDefinition()
	: m_uPositionUniform( GL_INVALID_VALUE )
	, m_uSizeUniform( GL_INVALID_VALUE )
	, m_vGlyphOffsetUniform( GL_INVALID_VALUE )
	, m_vGlyphSizeUniform( GL_INVALID_VALUE )
	, m_vGlyphColorUniform( GL_INVALID_VALUE )
	, m_uAtlasTextureUniform( GL_INVALID_VALUE )
{
}

void TextTechniqueDefinition::SetScreenPosition( const glm::vec2& vPosition )
{
	glUniform2fv( m_uPositionUniform, 1, glm::value_ptr( vPosition ) );
}

void TextTechniqueDefinition::SetScreenSize( const glm::vec2& vSize )
{
	glUniform2fv( m_uSizeUniform, 1, glm::value_ptr( vSize ) );
}

void TextTechniqueDefinition::SetGlyph( const glm::vec2& vFromUVs, const glm::vec2& vToUVs )
{
	glUniform2fv( m_vGlyphOffsetUniform, 1, glm::value_ptr( vFromUVs ) );
	glUniform2fv( m_vGlyphSizeUniform, 1, glm::value_ptr( vToUVs ) );
}

void TextTechniqueDefinition::SetColor( const glm::vec4& vColor )
{
	glUniform4fv( m_vGlyphColorUniform, 1, glm::value_ptr( vColor ) );
}

void TextTechniqueDefinition::SetTexture( const GLint iTexture )
{
	glUniform1i( m_uAtlasTextureUniform, iTexture );
}

void TextTechniqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_uPositionUniform = oTechnique.GetParameterID( "position" );
	m_uSizeUniform = oTechnique.GetParameterID( "size" );
	m_vGlyphOffsetUniform = oTechnique.GetParameterID( "glyphOffset" );
	m_vGlyphSizeUniform = oTechnique.GetParameterID( "glyphSize" );
	m_vGlyphColorUniform = oTechnique.GetParameterID( "glyphColor" );
	m_uAtlasTextureUniform = oTechnique.GetParameterID( "atlasTexture" );
}
