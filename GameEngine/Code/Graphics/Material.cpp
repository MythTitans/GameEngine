#include "Material.h"

#include "Graphics/Renderer.h"

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
	s_oMaterialSheet.BindParameter( LitMaterialParam::MATERIAL_ID, "materialID" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::DIFFUSE_MAP, "diffuseMap" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::NORMAL_MAP, "normalMap" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::SPECULAR_MAP, "specularMap" );
	s_oMaterialSheet.BindParameter( LitMaterialParam::EMISSIVE_MAP, "emissiveMap" );
}

void LitMaterialData::ApplyMaterial( const uint uMaterialID, Technique& oTechnique )
{
	ASSERT( uMaterialID <= MAX_MATERIAL_COUNT );

	if( uMaterialID <= MAX_MATERIAL_COUNT )
		s_oMaterialSheet.GetParameter( LitMaterialParam::MATERIAL_ID ).SetValue( uMaterialID );
	else
		s_oMaterialSheet.GetParameter( LitMaterialParam::MATERIAL_ID ).SetValue( 0u );

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

void LitMaterialData::ExportToGPU( GPULitMaterialData& oMaterialData ) const
{
	oMaterialData.m_vDiffuseColor = m_vDiffuseColor;
	oMaterialData.m_vSpecularColor = m_vSpecularColor;
	oMaterialData.m_vEmissiveColor = m_vEmissiveColor;
	oMaterialData.m_fShininess = m_fShininess;
}

PARAM_SHEET( LitMaterialData::LitMaterialParam ) LitMaterialData::s_oMaterialSheet;

UnlitMaterialData::UnlitMaterialData()
	: m_vDiffuseColor( 1.f, 1.f, 1.f )
{
}

void UnlitMaterialData::PrepareMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.Init( oTechnique );
	s_oMaterialSheet.BindParameter( UnlitMaterialParam::MATERIAL_ID, "materialID" );
	s_oMaterialSheet.BindParameter( UnlitMaterialParam::DIFFUSE_MAP, "diffuseMap" );
}

void UnlitMaterialData::ApplyMaterial( const uint uMaterialID, Technique& oTechnique )
{
	ASSERT( uMaterialID <= MAX_MATERIAL_COUNT );

	if( uMaterialID <= MAX_MATERIAL_COUNT )
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::MATERIAL_ID ).SetValue( uMaterialID );
	else
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::MATERIAL_ID ).SetValue( 0u );
	
	if( m_xDiffuseTextureResource != nullptr )
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::DIFFUSE_MAP ).SetValue( &m_xDiffuseTextureResource->GetTexture(), oTechnique );
	else
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::DIFFUSE_MAP ).SetValue( g_pRenderer->GetDefaultDiffuseMap(), oTechnique );
}

void UnlitMaterialData::ExportToGPU( GPUUnlitMaterialData& oMaterialData ) const
{
	oMaterialData.m_vDiffuseColor = m_vDiffuseColor;
}

PARAM_SHEET( UnlitMaterialData::UnlitMaterialParam ) UnlitMaterialData::s_oMaterialSheet;
