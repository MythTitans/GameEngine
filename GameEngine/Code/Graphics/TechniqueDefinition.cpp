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

DeferredMapsDefinition::DeferredMapsDefinition()
	: m_uModelViewProjectionUniform( GL_INVALID_VALUE )
	, m_uDiffuseColorUniform( GL_INVALID_VALUE )
	, m_uDiffuseTextureUniform( GL_INVALID_VALUE )
{
}

void DeferredMapsDefinition::SetModelViewProjection( const glm::mat4& mViewProjection )
{
	glUniformMatrix4fv( m_uModelViewProjectionUniform, 1, GL_FALSE, glm::value_ptr( mViewProjection ) );
}

void DeferredMapsDefinition::SetDiffuseColor( const glm::vec3& vColor )
{
	glUniform3fv( m_uDiffuseColorUniform, 1, glm::value_ptr( vColor ) );
}

void DeferredMapsDefinition::SetDiffuseTexture( const int iTextureUnit )
{
	glUniform1i( m_uDiffuseTextureUniform, iTextureUnit );
}

void DeferredMapsDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_uModelViewProjectionUniform = oTechnique.GetParameterID( "modelViewProjection" );
	m_uDiffuseColorUniform = oTechnique.GetParameterID( "diffuseColor" );
	m_uDiffuseTextureUniform = oTechnique.GetParameterID( "diffuseTexture" );
}

DeferredComposeDefinition::DeferredComposeDefinition()
	: m_uColorUniform( GL_INVALID_VALUE )
	, m_uNormalUniform( GL_INVALID_VALUE )
	, m_uDepthUniform( GL_INVALID_VALUE )
	, m_uInverseViewProjectionUniform( GL_INVALID_VALUE )
{
}

void DeferredComposeDefinition::SetColor( const int iTextureUnit )
{
	glUniform1i( m_uColorUniform, iTextureUnit );
}

void DeferredComposeDefinition::SetNormal( const int iTextureUnit )
{
	glUniform1i( m_uNormalUniform, iTextureUnit );
}

void DeferredComposeDefinition::SetDepth( const int iTextureUnit )
{
	glUniform1i( m_uDepthUniform, iTextureUnit );
}

void DeferredComposeDefinition::SetInverseViewProjection( const glm::mat4& mInverseViewProjection )
{
	glUniformMatrix4fv( m_uInverseViewProjectionUniform, 1, GL_FALSE, glm::value_ptr( mInverseViewProjection ) );
}

void DeferredComposeDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_uColorUniform = oTechnique.GetParameterID( "colorMap" );
	m_uNormalUniform = oTechnique.GetParameterID( "normalMap" );
	m_uDepthUniform = oTechnique.GetParameterID( "depthMap" );
	m_uInverseViewProjectionUniform = oTechnique.GetParameterID( "inverseViewProjection" );
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
