#include "Skybox.h"

#include "Game/GameContext.h"
#include "Game/Entity.h"
#include "Game/ResourceLoader.h"
#include "Graphics/Renderer.h"
#include "Math/GLMHelpers.h"

REGISTER_COMPONENT( SkyboxComponent );

SkyboxComponent::SkyboxComponent( Entity* pEntity )
	: Component( pEntity )
	, m_pSky( nullptr )
{
}

void SkyboxComponent::Initialize()
{
	LoadCubeMapTextures();
}

bool SkyboxComponent::IsInitialized() const
{
	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
	{
		if( m_aTextures[ u ]->IsLoading() )
			return false;
	}

	return true;
}

void SkyboxComponent::Start()
{
	CreateCubeMap();

	m_pSky = g_pRenderer->m_oVisualStructure.AddSky();
	m_pSky->m_oCubeMap = m_oCubeMap;

	if( m_bActive )
		g_pRenderer->m_oVisualStructure.SetActiveSky( m_pSky );
}

void SkyboxComponent::Update( const GameContext& oGameContext )
{
	if( oGameContext.m_bEditing && AreCubeMapTexturesReady() )
	{
		m_oCubeMap.Destroy();
		CreateCubeMap();

		m_pSky->m_oCubeMap = m_oCubeMap;
	}

	m_bActive = g_pRenderer->m_oVisualStructure.GetActiveSky() == m_pSky;
}

void SkyboxComponent::Stop()
{
	m_oCubeMap.Destroy();

	g_pRenderer->m_oVisualStructure.RemoveSky( m_pSky );
}
void SkyboxComponent::Dispose()
{
	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
		m_aTextures[ u ] = nullptr;
}

#ifdef EDITOR
void SkyboxComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Active" )
	{
		if( m_bActive )
			g_pRenderer->m_oVisualStructure.SetActiveSky( m_pSky );
		else
			g_pRenderer->m_oVisualStructure.SetActiveSky( nullptr );
	}
	else
	{
		LoadCubeMapTextures();
	}
}
#endif

void SkyboxComponent::LoadCubeMapTextures()
{
	m_aTextures[ CubeMapDesc::POSITIVE_X ] = g_pResourceLoader->LoadTexture( m_sPositiveX.c_str() );
	m_aTextures[ CubeMapDesc::NEGATIVE_X ] = g_pResourceLoader->LoadTexture( m_sNegativeX.c_str() );
	m_aTextures[ CubeMapDesc::POSITIVE_Y ] = g_pResourceLoader->LoadTexture( m_sPositiveY.c_str() );
	m_aTextures[ CubeMapDesc::NEGATIVE_Y ] = g_pResourceLoader->LoadTexture( m_sNegativeY.c_str() );
	m_aTextures[ CubeMapDesc::POSITIVE_Z ] = g_pResourceLoader->LoadTexture( m_sPositiveZ.c_str() );
	m_aTextures[ CubeMapDesc::NEGATIVE_Z ] = g_pResourceLoader->LoadTexture( m_sNegativeZ.c_str() );
}

bool SkyboxComponent::AreCubeMapTexturesReady() const
{
	uint uReadyCount = 0;
	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
	{
		if( m_aTextures[ u ] != nullptr && m_aTextures[ u ]->IsLoading() == false )
			++uReadyCount;
	}

	return uReadyCount == CubeMapDesc::_COUNT;
}

void SkyboxComponent::CreateCubeMap()
{
	bool bCheck = false;
	int iWidth = 1;
	int iHeight = 1;
	TextureFormat eFormat = TextureFormat::RGBA;
	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
	{
		if( m_aTextures[ u ]->IsLoaded() )
		{
			if( bCheck )
			{
				ASSERT( m_aTextures[ u ]->GetTexture().GetWidth() == iWidth );
				ASSERT( m_aTextures[ u ]->GetTexture().GetHeight() == iHeight );
				ASSERT( m_aTextures[ u ]->GetTexture().GetFormat() == eFormat );
			}
			else
			{
				iWidth = m_aTextures[ u ]->GetTexture().GetWidth();
				iHeight = m_aTextures[ u ]->GetTexture().GetHeight();
				eFormat = m_aTextures[ u ]->GetTexture().GetFormat();
				bCheck = true;
			}
		}
	}

	Array< uint8 > aData[ CubeMapDesc::_COUNT ];

	CubeMapDesc oDesc = CubeMapDesc( iWidth, iHeight, eFormat ).Wrapping( TextureWrapping::CLAMP ).SRGB();
	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
	{
		if( m_aTextures[ u ]->IsLoaded() )
		{
			m_aTextures[ u ]->GetTexture().FetchData( aData[ u ] );
			oDesc.Data( aData[ u ].Data(), CubeMapDesc::Side( u ) );
		}
		else
		{
			aData[ u ].Resize( iWidth * iHeight * GetFormatBytes( eFormat ), 0 );
			oDesc.Data( aData[ u ].Data(), CubeMapDesc::Side( u ) );
		}
	}

	m_oCubeMap.Create( oDesc );

	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
		m_aTextures[ u ] = nullptr;
}
