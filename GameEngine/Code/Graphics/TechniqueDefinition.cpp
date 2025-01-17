#include "TechniqueDefinition.h"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_inverse.hpp>

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
	ASSERT( oTechnique.IsValid() );

	CreateDefinition( oTechnique );

	m_bValid = true;
}

bool TechniqueDefinitionBase::IsValid() const
{
	return m_bValid;
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const int iValue )
{
	ASSERT( iParameterID != -1 );
	glUniform1i( iParameterID, iValue );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const float fValue )
{
	ASSERT( iParameterID != -1 );
	glUniform1f( iParameterID, fValue );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::vec2& vValue )
{
	ASSERT( iParameterID != -1 );
	glUniform2fv( iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::vec3& vValue )
{
	ASSERT( iParameterID != -1 );
	glUniform3fv( iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::vec4& vValue )
{
	ASSERT( iParameterID != -1 );
	glUniform4fv( iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::mat3& mValue )
{
	ASSERT( iParameterID != -1 );
	glUniformMatrix3fv( iParameterID, 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::mat4& mValue )
{
	ASSERT( iParameterID != -1 );
	glUniformMatrix4fv( iParameterID, 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::uvec2& vValue )
{
	ASSERT( iParameterID != -1 );
	glUniform2uiv( iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::uvec3& vValue )
{
	ASSERT( iParameterID != -1 );
	glUniform3uiv( iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueDefinitionBase::SetParameterID( const GLint iParameterID, const glm::uvec4& vValue )
{
	ASSERT( iParameterID != -1 );
	glUniform4uiv( iParameterID, 1, glm::value_ptr( vValue ) );
}

ForwardOpaqueDefinition::ForwardOpaqueDefinition()
	: m_iModelViewProjectionUniform( -1 )
	, m_iModelUniform( -1 )
	, m_iModelInverseTransposeUniform( -1 )
	, m_iDiffuseColorUniform( -1 )
	, m_iDiffuseMapUniform( -1 )
	, m_iNormalMapUniform( -1 )
{
}

void ForwardOpaqueDefinition::SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection )
{
	SetParameterID( m_iModelViewProjectionUniform, mViewProjection * mModel );
	SetParameterID( m_iModelUniform, mModel );
	SetParameterID( m_iModelInverseTransposeUniform, glm::inverseTranspose( mModel ) );
}

void ForwardOpaqueDefinition::SetDiffuseColor( const glm::vec3& vColor )
{
	SetParameterID( m_iDiffuseColorUniform, vColor );
}

void ForwardOpaqueDefinition::SetDiffuseMap( const int iTextureUnit )
{
	SetParameterID( m_iDiffuseMapUniform, iTextureUnit );
}

void ForwardOpaqueDefinition::SetNormalMap( const int iTextureUnit )
{
	SetParameterID( m_iNormalMapUniform, iTextureUnit );
}

void ForwardOpaqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	LightingDefinition::CreateDefinition( oTechnique );

	m_iModelViewProjectionUniform = oTechnique.GetParameterID( "modelViewProjection" );
	m_iModelUniform = oTechnique.GetParameterID( "model" );
	m_iModelInverseTransposeUniform = oTechnique.GetParameterID( "modelInverseTranspose" );
	m_iDiffuseColorUniform = oTechnique.GetParameterID( "diffuseColor" );
	m_iDiffuseMapUniform = oTechnique.GetParameterID( "diffuseMap" );
	m_iNormalMapUniform = oTechnique.GetParameterID( "normalMap" );
}

DeferredMapsDefinition::DeferredMapsDefinition()
	: m_iModelViewProjectionUniform( -1 )
	, m_iModelInverseTransposeUniform( -1 )
	, m_iDiffuseColorUniform( -1 )
	, m_iDiffuseMapUniform( -1 )
	, m_iNormalMapUniform( -1 )
{
}

void DeferredMapsDefinition::SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection )
{
	SetParameterID( m_iModelViewProjectionUniform, mViewProjection * mModel );
	SetParameterID( m_iModelInverseTransposeUniform, glm::inverseTranspose( mModel ) );
}

void DeferredMapsDefinition::SetDiffuseColor( const glm::vec3& vColor )
{
	SetParameterID( m_iDiffuseColorUniform, vColor );
}

void DeferredMapsDefinition::SetDiffuseMap( const int iTextureUnit )
{
	SetParameterID( m_iDiffuseMapUniform, iTextureUnit );
}

void DeferredMapsDefinition::SetNormalMap( const int iTextureUnit )
{
	SetParameterID( m_iNormalMapUniform, iTextureUnit );
}

void DeferredMapsDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iModelViewProjectionUniform = oTechnique.GetParameterID( "modelViewProjection" );
	m_iModelInverseTransposeUniform = oTechnique.GetParameterID( "modelInverseTranspose" );
	m_iDiffuseColorUniform = oTechnique.GetParameterID( "diffuseColor" );
	m_iDiffuseMapUniform = oTechnique.GetParameterID( "diffuseMap" );
	m_iNormalMapUniform = oTechnique.GetParameterID( "normalMap" );
}

DeferredComposeDefinition::DeferredComposeDefinition()
	: m_iColorUniform( -1 )
	, m_iNormalUniform( -1 )
	, m_iDepthUniform( -1 )
	, m_iInverseViewProjectionUniform( -1 )
{
}

void DeferredComposeDefinition::SetColor( const int iTextureUnit )
{
	SetParameterID( m_iColorUniform, iTextureUnit );
}

void DeferredComposeDefinition::SetNormal( const int iTextureUnit )
{
	SetParameterID( m_iNormalUniform, iTextureUnit );
}

void DeferredComposeDefinition::SetDepth( const int iTextureUnit )
{
	SetParameterID( m_iDepthUniform, iTextureUnit );
}

void DeferredComposeDefinition::SetInverseViewProjection( const glm::mat4& mInverseViewProjection )
{
	SetParameterID( m_iInverseViewProjectionUniform, mInverseViewProjection );
}

void DeferredComposeDefinition::CreateDefinition( const Technique& oTechnique )
{
	LightingDefinition::CreateDefinition( oTechnique );

	m_iColorUniform = oTechnique.GetParameterID( "colorMap" );
	m_iNormalUniform = oTechnique.GetParameterID( "normalMap" );
	m_iDepthUniform = oTechnique.GetParameterID( "depthMap" );
	m_iInverseViewProjectionUniform = oTechnique.GetParameterID( "inverseViewProjection" );
}

PickingDefinition::PickingDefinition()
	: m_iModelViewProjectionUniform( -1 )
	, m_iColorIDUniform( -1 )
{
}

void PickingDefinition::SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection )
{
	SetParameterID( m_iModelViewProjectionUniform, mViewProjection * mModel );
}

void PickingDefinition::SetID( const uint64 uID )
{
	const uint16 uRed = ( uID >> 48 ) & 0xFFFF;
	const uint16 uGreen = ( uID >> 32 ) & 0xFFFF;
	const uint16 uBlue = ( uID >> 16 ) & 0xFFFF;
	const uint16 uAlpha = ( uID ) & 0xFFFF;

	const glm::uvec4 vColorID( uRed, uGreen, uBlue, uAlpha );
	SetParameterID( m_iColorIDUniform, vColorID );
}

void PickingDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iModelViewProjectionUniform = oTechnique.GetParameterID( "modelViewProjection" );
	m_iColorIDUniform = oTechnique.GetParameterID( "colorID" );
}

OutlineDefinition::OutlineDefinition()
	: m_iModelViewProjectionUniform( -1 )
	, m_iCameraPositionUniform( -1 )
	, m_iDisplacementUniform( -1 )
{
}

void OutlineDefinition::SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection )
{
	SetParameterID( m_iModelViewProjectionUniform, mViewProjection * mModel );
}

void OutlineDefinition::SetCameraPosition( const glm::vec3& vCameraPosition )
{
	SetParameterID( m_iCameraPositionUniform, vCameraPosition );
}

void OutlineDefinition::SetDisplacement( const float fDisplacement )
{
	SetParameterID( m_iDisplacementUniform, fDisplacement );
}

void OutlineDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iModelViewProjectionUniform = oTechnique.GetParameterID( "modelViewProjection" );
	m_iCameraPositionUniform = oTechnique.GetParameterID( "cameraPosition" );
	m_iDisplacementUniform = oTechnique.GetParameterID( "displacement" );
}

TextTechniqueDefinition::TextTechniqueDefinition()
	: m_iPositionUniform( -1 )
	, m_iSizeUniform( -1 )
	, m_iGlyphOffsetUniform( -1 )
	, m_iGlyphSizeUniform( -1 )
	, m_iGlyphColorUniform( -1 )
	, m_iAtlasTextureUniform( -1 )
{
}

void TextTechniqueDefinition::SetScreenPosition( const glm::vec2& vPosition )
{
	SetParameterID( m_iPositionUniform, vPosition );
}

void TextTechniqueDefinition::SetScreenSize( const glm::vec2& vSize )
{
	SetParameterID( m_iSizeUniform, vSize );
}

void TextTechniqueDefinition::SetGlyph( const glm::vec2& vFromUVs, const glm::vec2& vToUVs )
{
	SetParameterID( m_iGlyphOffsetUniform, vFromUVs );
	SetParameterID( m_iGlyphSizeUniform, vToUVs );
}

void TextTechniqueDefinition::SetColor( const glm::vec4& vColor )
{
	SetParameterID( m_iGlyphColorUniform, vColor );
}

void TextTechniqueDefinition::SetTexture( const GLint iTexture )
{
	SetParameterID( m_iAtlasTextureUniform, iTexture );
}

void TextTechniqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iPositionUniform = oTechnique.GetParameterID( "position" );
	m_iSizeUniform = oTechnique.GetParameterID( "size" );
	m_iGlyphOffsetUniform = oTechnique.GetParameterID( "glyphOffset" );
	m_iGlyphSizeUniform = oTechnique.GetParameterID( "glyphSize" );
	m_iGlyphColorUniform = oTechnique.GetParameterID( "glyphColor" );
	m_iAtlasTextureUniform = oTechnique.GetParameterID( "atlasTexture" );
}

GizmoDefinition::GizmoDefinition()
	: m_iModelViewProjectionUniform( -1 )
	, m_iColorUniform( -1 )
{
}

void GizmoDefinition::SetModelAndViewProjection( const glm::mat4& mModel, const glm::mat4& mViewProjection )
{
	SetParameterID( m_iModelViewProjectionUniform, mViewProjection * mModel );
}

void GizmoDefinition::SetColor( const glm::vec3& vColor )
{
	SetParameterID( m_iColorUniform, vColor );
}

void GizmoDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iModelViewProjectionUniform = oTechnique.GetParameterID( "modelViewProjection" );
	m_iColorUniform = oTechnique.GetParameterID( "color" );
}

LineTechniqueDefinition::LineTechniqueDefinition()
	: m_iViewProjectionUniform( -1 )
	, m_iColorUniform( -1 )
{
}

void LineTechniqueDefinition::SetViewProjection( const glm::mat4& mViewProjection )
{
	SetParameterID( m_iViewProjectionUniform, mViewProjection );
}

void LineTechniqueDefinition::SetColor( const glm::vec3& vColor )
{
	SetParameterID( m_iColorUniform, vColor );
}

void LineTechniqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iViewProjectionUniform = oTechnique.GetParameterID( "viewProjection" );
	m_iColorUniform = oTechnique.GetParameterID( "color" );
}

SphereTechniqueDefinition::SphereTechniqueDefinition()
	: m_iViewProjectionUniform( -1 )
	, m_iPositionUniform( -1 )
	, m_iRadiusUniform( -1 )
	, m_iColorUniform( -1 )
{
}

void SphereTechniqueDefinition::SetViewProjection( const glm::mat4& mViewProjection )
{
	SetParameterID( m_iViewProjectionUniform, mViewProjection );
}

void SphereTechniqueDefinition::SetPosition( const glm::vec3& vPosition )
{
	SetParameterID( m_iPositionUniform, vPosition );
}

void SphereTechniqueDefinition::SetRadius( const float fRadius )
{
	SetParameterID( m_iRadiusUniform, fRadius );
}

void SphereTechniqueDefinition::SetColor( const glm::vec3& vColor )
{
	SetParameterID( m_iColorUniform, vColor );
}

void SphereTechniqueDefinition::CreateDefinition( const Technique& oTechnique )
{
	m_iViewProjectionUniform = oTechnique.GetParameterID( "viewProjection" );
	m_iPositionUniform = oTechnique.GetParameterID( "position" );
	m_iRadiusUniform = oTechnique.GetParameterID( "radius" );
	m_iColorUniform = oTechnique.GetParameterID( "color" );
}
