#include "Skybox.h"

#include "Game/GameEngine.h"
#include "Game/Entity.h"
#include "Math/GLMHelpers.h"
#include "Renderer.h"
#include "Technique.h"

Skybox::Skybox()
	: m_xSkybox( g_pResourceLoader->LoadTechnique( "Shader/skybox.tech" ) )
{
	Array< glm::vec3 > aVertices( 8 );
	aVertices[ 0 ] = glm::vec3( 1.f, 1.f, 1.f );
	aVertices[ 1 ] = glm::vec3( -1.f, 1.f, 1.f );
	aVertices[ 2 ] = glm::vec3( -1.f, 1.f, -1.f );
	aVertices[ 3 ] = glm::vec3( 1.f, 1.f, -1.f );
	aVertices[ 4 ] = glm::vec3( 1.f, -1.f, 1.f );
	aVertices[ 5 ] = glm::vec3( -1.f, -1.f, 1.f );
	aVertices[ 6 ] = glm::vec3( -1.f, -1.f, -1.f );
	aVertices[ 7 ] = glm::vec3( 1.f, -1.f, -1.f );

	Array< GLuint > aIndices( 36 );
	aIndices[ 0 ] = 0; aIndices[ 1 ] = 1; aIndices[ 2 ] = 2;
	aIndices[ 3 ] = 0; aIndices[ 4 ] = 2; aIndices[ 5 ] = 3;
	aIndices[ 6 ] = 6; aIndices[ 7 ] = 5; aIndices[ 8 ] = 4;
	aIndices[ 9 ] = 7; aIndices[ 10 ] = 6; aIndices[ 11 ] = 4;
	aIndices[ 12 ] = 3; aIndices[ 13 ] = 2; aIndices[ 14 ] = 6;
	aIndices[ 15 ] = 3; aIndices[ 16 ] = 6; aIndices[ 17 ] = 7;
	aIndices[ 18 ] = 5; aIndices[ 19 ] = 1; aIndices[ 20 ] = 0;
	aIndices[ 21 ] = 4; aIndices[ 22 ] = 5; aIndices[ 23 ] = 0;
	aIndices[ 24 ] = 6; aIndices[ 25 ] = 2; aIndices[ 26 ] = 1;
	aIndices[ 27 ] = 5; aIndices[ 28 ] = 6; aIndices[ 29 ] = 1;
	aIndices[ 30 ] = 0; aIndices[ 31 ] = 3; aIndices[ 32 ] = 7;
	aIndices[ 33 ] = 0; aIndices[ 34 ] = 7; aIndices[ 35 ] = 4;

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).Build();
}

void Skybox::Render( const Sky* pSky, const RenderContext& /*oRenderContext*/ )
{
	glDepthMask( GL_FALSE );

	Technique& oTechnique = m_xSkybox->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	const glm::mat4 mViewProjection = g_pRenderer->m_oCamera.GetProjectionMatrix() * ToMat4( glm::mat3( g_pRenderer->m_oCamera.GetViewMatrix() ) );
	m_oSkyboxSheet.GetParameter( SkyboxParam::VIEW_PROJECTION ).SetValue( mViewProjection );

	g_pRenderer->SetCubeMapSlot( pSky->m_oCubeMap, 0 );
	m_oSkyboxSheet.GetParameter( SkyboxParam::CUBE_MAP ).SetValue( 0 );

	g_pRenderer->DrawMesh( m_oMesh );

	g_pRenderer->ClearCubeMapSlot( 0 );

	glDepthMask( GL_TRUE );
}

bool Skybox::OnLoading()
{
	return m_xSkybox->IsLoaded();
}

void Skybox::OnLoaded()
{
	m_oSkyboxSheet.Init( m_xSkybox->GetTechnique() );
	m_oSkyboxSheet.BindParameter( SkyboxParam::VIEW_PROJECTION, "viewProjection" );
	m_oSkyboxSheet.BindParameter( SkyboxParam::CUBE_MAP, "cubeMap" );
}

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

	Array< uint8, ArrayFlags::FAST_RESIZE > aData[ CubeMapDesc::_COUNT ];

	CubeMapDesc oDesc = CubeMapDesc( iWidth, iHeight, eFormat ).Wrapping( TextureWrapping::CLAMP ).SRGB();
	for( uint u = 0; u < CubeMapDesc::_COUNT; ++u )
	{
		if( m_aTextures[ u ]->IsLoaded() )
		{
			aData[ u ] = m_aTextures[ u ]->GetTexture().FetchData();
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
