#include "Material.h"

#include "Graphics/Renderer.h"

static const std::string PARAM_DIFFUSE_COLOR( "diffuseColor" );
static const std::string PARAM_DIFFUSE_MAP( "diffuseMap" );
static const std::string PARAM_NORMAL_MAP( "normalMap" );

LitMaterialData::LitMaterialData()
	: m_vDiffuseColor( 1.f, 1.f, 1.f )
{
}

void LitMaterialData::ApplyMaterial( Technique& oTechnique )
{
	oTechnique.SetParameter( PARAM_DIFFUSE_COLOR, m_vDiffuseColor );

	if( m_xDiffuseTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_DIFFUSE_MAP, &m_xDiffuseTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_DIFFUSE_MAP, g_pRenderer->GetDefaultDiffuseMap() );

	if( m_xNormalTextureResource != nullptr )
		oTechnique.SetParameter( PARAM_NORMAL_MAP, &m_xNormalTextureResource->GetTexture() );
	else
		oTechnique.SetParameter( PARAM_NORMAL_MAP, g_pRenderer->GetDefaultNormalMap() );
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
