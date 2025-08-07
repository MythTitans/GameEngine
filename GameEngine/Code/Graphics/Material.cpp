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

void LitMaterialData::PrepareMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.Init( oTechnique );
	s_oMaterialSheet.BindParameter( LitMaterialParam::DIFFUSE_COLOR, "diffuseColor" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::SPECULAR_COLOR, "specularColor" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::EMISSIVE_COLOR, "emissiveColor" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::SHININESS, "shininess" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::DIFFUSE_MAP, "diffuseMap" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::NORMAL_MAP, "normalMap" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::SPECULAR_MAP, "specularMap" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::EMISSIVE_MAP, "emissiveMap" );
}

void LitMaterialData::ApplyMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.GetParameter( LitMaterialParam::DIFFUSE_COLOR ).SetValue( m_vDiffuseColor );
	s_oMaterialSheet.GetParameter( LitMaterialParam::SPECULAR_COLOR ).SetValue( m_vSpecularColor );
	s_oMaterialSheet.GetParameter( LitMaterialParam::EMISSIVE_COLOR ).SetValue( m_vEmissiveColor );
	s_oMaterialSheet.GetParameter( LitMaterialParam::SHININESS ).SetValue( glm::max( m_fShininess, 1.0f ) );

	if( m_xDiffuseTextureResource != nullptr )
		s_oMaterialSheet.GetParameter( LitMaterialParam::DIFFUSE_MAP ).SetValue( &m_xDiffuseTextureResource->GetTexture(), oTechnique );
	else
		s_oMaterialSheet.GetParameter( LitMaterialParam::DIFFUSE_MAP ).SetValue( g_pRenderer->GetDefaultDiffuseMap(), oTechnique );

	if( m_xNormalTextureResource != nullptr )
		s_oMaterialSheet.GetParameter( LitMaterialParam::NORMAL_MAP ).SetValue( &m_xNormalTextureResource->GetTexture(), oTechnique );
	else
		s_oMaterialSheet.GetParameter( LitMaterialParam::NORMAL_MAP ).SetValue( g_pRenderer->GetDefaultNormalMap(), oTechnique );

	if( m_xSpecularTextureResource != nullptr )
		s_oMaterialSheet.GetParameter( LitMaterialParam::SPECULAR_MAP ).SetValue( &m_xSpecularTextureResource->GetTexture(), oTechnique );
	else
		s_oMaterialSheet.GetParameter( LitMaterialParam::SPECULAR_MAP ).SetValue( g_pRenderer->GetDefaultDiffuseMap(), oTechnique );

	if( m_xEmissiveTextureResource != nullptr )
		s_oMaterialSheet.GetParameter( LitMaterialParam::EMISSIVE_MAP ).SetValue( &m_xEmissiveTextureResource->GetTexture(), oTechnique );
	else
		s_oMaterialSheet.GetParameter( LitMaterialParam::EMISSIVE_MAP ).SetValue( g_pRenderer->GetDefaultDiffuseMap(), oTechnique );
}

PARAM_SHEET( LitMaterialData::LitMaterialParam ) LitMaterialData::s_oMaterialSheet;

UnlitMaterialData::UnlitMaterialData()
	: m_vDiffuseColor( 1.f, 1.f, 1.f )
{
}

void UnlitMaterialData::PrepareMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.Init( oTechnique );
	s_oMaterialSheet.BindParameter( UnlitMaterialParam::DIFFUSE_COLOR, "diffuseColor" );
	s_oMaterialSheet.BindParameter( UnlitMaterialParam::DIFFUSE_MAP, "diffuseMap" );
}

void UnlitMaterialData::ApplyMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.GetParameter( UnlitMaterialParam::DIFFUSE_COLOR ).SetValue( m_vDiffuseColor );
	
	if( m_xDiffuseTextureResource != nullptr )
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::DIFFUSE_MAP ).SetValue( &m_xDiffuseTextureResource->GetTexture(), oTechnique );
	else
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::DIFFUSE_MAP ).SetValue( g_pRenderer->GetDefaultDiffuseMap(), oTechnique );
}

PARAM_SHEET( UnlitMaterialData::UnlitMaterialParam ) UnlitMaterialData::s_oMaterialSheet;
