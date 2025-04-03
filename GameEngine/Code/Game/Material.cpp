#include "Material.h"

#include "Graphics/Renderer.h"

static const std::string PARAM_DIFFUSE_COLOR( "diffuseColor" );
static const std::string PARAM_SPECULAR_COLOR( "specularColor" );
static const std::string PARAM_EMISSIVE_COLOR( "emissiveColor" );
static const std::string PARAM_SHININESS( "shininess" );
static const std::string PARAM_DIFFUSE_MAP( "diffuseMap" );
static const std::string PARAM_NORMAL_MAP( "normalMap" );
static const std::string PARAM_SPECULAR_MAP( "specularMap" );
static const std::string PARAM_EMISSIVE_MAP( "emissiveMap" );

LitMaterialData::LitMaterialData()
	: m_vDiffuseColor( 1.f, 1.f, 1.f )
	, m_vSpecularColor( 1.f, 1.f, 1.f )
	, m_vEmissiveColor( 0.f, 0.f, 0.f )
	, m_fShininess( 32.f )
{
}

void LitMaterialData::ApplyMaterial( Technique& oTechnique )
{
	oTechnique.SetParameter( PARAM_DIFFUSE_COLOR, m_vDiffuseColor );
	oTechnique.SetParameter( PARAM_SPECULAR_COLOR, m_vSpecularColor );
	oTechnique.SetParameter( PARAM_EMISSIVE_COLOR, m_vEmissiveColor );
	oTechnique.SetParameter( PARAM_SHININESS, max( m_fShininess, 1.0f)  );

	if( m_xDiffuseTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_DIFFUSE_MAP, &m_xDiffuseTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_DIFFUSE_MAP, g_pRenderer->GetDefaultDiffuseMap() );

	if( m_xNormalTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_NORMAL_MAP, &m_xNormalTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_NORMAL_MAP, g_pRenderer->GetDefaultNormalMap() );

	if( m_xSpecularTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_SPECULAR_MAP, &m_xSpecularTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_SPECULAR_MAP, g_pRenderer->GetDefaultDiffuseMap() );

	if( m_xEmissiveTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_EMISSIVE_MAP, &m_xEmissiveTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_EMISSIVE_MAP, g_pRenderer->GetDefaultDiffuseMap() );
}

UnlitMaterialData::UnlitMaterialData()
	: m_vDiffuseColor( 1.f, 1.f, 1.f )
{
}

void UnlitMaterialData::ApplyMaterial( Technique& oTechnique )
{
	oTechnique.SetParameter( PARAM_DIFFUSE_COLOR, m_vDiffuseColor );

	if( m_xDiffuseTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_DIFFUSE_MAP, &m_xDiffuseTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_DIFFUSE_MAP, g_pRenderer->GetDefaultDiffuseMap() );
}
