#include "Material.h"

#include "Graphics/Renderer.h"

LitMaterialData::LitMaterialData()
	: m_oDiffuseColor( Color::White() )
	, m_oSpecularColor( Color::White() )
	, m_oEmissiveColor( Color::Black() )
	, m_fShininess( 32.f )
{
}

void LitMaterialData::PrepareMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.Init( oTechnique );
	s_oMaterialSheet.BindParameter( LitMaterialParam::MATERIAL_ID, "materialID" );
	oTechnique.SetUsedTextureCount( 5 );
}

void LitMaterialData::ApplyMaterial( const uint uMaterialID, Technique& oTechnique )
{
	ASSERT( uMaterialID <= MAX_MATERIAL_COUNT );

	if( uMaterialID <= MAX_MATERIAL_COUNT )
		s_oMaterialSheet.GetParameter( LitMaterialParam::MATERIAL_ID ).SetValue( uMaterialID );
	else
		s_oMaterialSheet.GetParameter( LitMaterialParam::MATERIAL_ID ).SetValue( 0u );

	if( m_xDiffuseTextureResource != nullptr )
		oTechnique.AddUsedTexture( &m_xDiffuseTextureResource->GetTexture() );
	else
		oTechnique.AddUsedTexture( g_pRenderer->GetDefaultDiffuseMap() );

	if( m_xNormalTextureResource != nullptr )
		oTechnique.AddUsedTexture( &m_xNormalTextureResource->GetTexture() );
	else
		oTechnique.AddUsedTexture( g_pRenderer->GetDefaultNormalMap() );

	if( m_xSpecularTextureResource != nullptr )
		oTechnique.AddUsedTexture( &m_xSpecularTextureResource->GetTexture() );
	else
		oTechnique.AddUsedTexture( g_pRenderer->GetDefaultDiffuseMap() );

	if( m_xEmissiveTextureResource != nullptr )
		oTechnique.AddUsedTexture( &m_xEmissiveTextureResource->GetTexture() );
	else
		oTechnique.AddUsedTexture( g_pRenderer->GetDefaultDiffuseMap() );
}

void LitMaterialData::ExportToGPU( GPULitMaterialData& oMaterialData ) const
{
	oMaterialData.m_vDiffuseColor = m_oDiffuseColor.m_vColor;
	oMaterialData.m_vSpecularColor = m_oSpecularColor.m_vColor;
	oMaterialData.m_vEmissiveColor = m_oEmissiveColor.m_vColor;
	oMaterialData.m_fShininess = m_fShininess;
}

PARAM_SHEET( LitMaterialData::LitMaterialParam ) LitMaterialData::s_oMaterialSheet;

UnlitMaterialData::UnlitMaterialData()
	: m_oDiffuseColor( Color::White() )
{
}

void UnlitMaterialData::PrepareMaterial( Technique& oTechnique )
{
	s_oMaterialSheet.Init( oTechnique );
	s_oMaterialSheet.BindParameter( UnlitMaterialParam::MATERIAL_ID, "materialID" );
	oTechnique.SetUsedTextureCount( 5 );
}

void UnlitMaterialData::ApplyMaterial( const uint uMaterialID, Technique& oTechnique )
{
	ASSERT( uMaterialID <= MAX_MATERIAL_COUNT );

	if( uMaterialID <= MAX_MATERIAL_COUNT )
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::MATERIAL_ID ).SetValue( uMaterialID );
	else
		s_oMaterialSheet.GetParameter( UnlitMaterialParam::MATERIAL_ID ).SetValue( 0u );
	
	if( m_xDiffuseTextureResource != nullptr )
		oTechnique.AddUsedTexture( &m_xDiffuseTextureResource->GetTexture() );
	else
		oTechnique.AddUsedTexture( g_pRenderer->GetDefaultDiffuseMap() );

	// TODO #eric find a better way to make deferred renderer happy
	oTechnique.AddUsedTexture( g_pRenderer->GetDefaultNormalMap() );
	oTechnique.AddUsedTexture( g_pRenderer->GetDefaultDiffuseMap() );
	oTechnique.AddUsedTexture( g_pRenderer->GetDefaultDiffuseMap() );
}

void UnlitMaterialData::ExportToGPU( GPUUnlitMaterialData& oMaterialData ) const
{
	oMaterialData.m_vDiffuseColor = m_oDiffuseColor.m_vColor;
}

PARAM_SHEET( UnlitMaterialData::UnlitMaterialParam ) UnlitMaterialData::s_oMaterialSheet;
