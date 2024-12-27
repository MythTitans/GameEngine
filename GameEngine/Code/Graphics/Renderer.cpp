#include "Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Game/Component.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Game/GameEngine.h"
#include "ImGui/imgui.h"

RenderRect::RenderRect()
	: m_uX( 0 )
	, m_uY( 0 )
	, m_uWidth( 0 )
	, m_uHeight( 0 )
{
}

void RenderContext::OnFrameBufferResized( int iWidth, int iHeight )
{
	if( iWidth <= 0 )
	{
		LOG_ERROR( "Framebuffer resized with width=[{}]", iWidth );
		iWidth = 0;
	}

	if( iHeight <= 0 )
	{
		LOG_ERROR( "Framebuffer resized with height=[{}]", iHeight );
		iHeight = 0;
	}

	m_oRenderRect.m_uWidth = ( uint )iWidth;
	m_oRenderRect.m_uHeight = ( uint )iHeight;

	ASSERT( m_oRenderRect.m_uWidth >= 0 );
	ASSERT( m_oRenderRect.m_uHeight >= 0 );
}

float RenderContext::ComputeAspectRatio() const
{
	return ( float )m_oRenderRect.m_uWidth / ( float )m_oRenderRect.m_uHeight;
}

Renderer* g_pRenderer = nullptr;

Renderer::Renderer()
	: m_xDefaultDiffuseMap( g_pResourceLoader->LoadTexture( std::filesystem::path( "Data/Default_diffuse.png" ) ) )
	, m_xDefaultNormalMap( g_pResourceLoader->LoadTexture( std::filesystem::path( "Data/Default_normal.png" ) ) )
	, m_xForwardOpaque( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/Shader/forward_opaque" ) ) )
	, m_xDeferredMaps( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/Shader/deferred_maps" ) ) )
	, m_xDeferredCompose( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/Shader/deferred_compose" ) ) )
	, m_eRenderingMode( RenderingMode::FORWARD )
	, m_bDisplayDebug( false )
{
	glEnable( GL_CULL_FACE );
	//glEnable( GL_FRAMEBUFFER_SRGB );

	glClearColor( 0.f, 0.f, 0.f, 1.f );

	Array< glm::vec3 > aVertices( 3 );
	aVertices[ 0 ] = glm::vec3( -3.f, -1.f, 0.f );
	aVertices[ 1 ] = glm::vec3( 1.f, -1.f, 0.f );
	aVertices[ 2 ] = glm::vec3( 1.f, 3.f, 0.f );

	Array< GLuint > aIndices( 3 );
	aIndices[ 0 ] = 0;
	aIndices[ 1 ] = 1;
	aIndices[ 2 ] = 2;

	Array< glm::vec2 > aUVs( 3 );
	aUVs[ 0 ] = glm::vec2( -1.f, 0.f );
	aUVs[ 1 ] = glm::vec2( 1.f, 0.f );
	aUVs[ 2 ] = glm::vec2( 1.f, 2.f );

	m_oRenderMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();

	g_pRenderer = this;
}

Renderer::~Renderer()
{
	g_pRenderer = nullptr;
}

void Renderer::Render( const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Renderer" );

	switch( m_eRenderingMode )
	{
	case RenderingMode::FORWARD:
		RenderForward( oRenderContext );
		break;
	case RenderingMode::DEFERRED:
		RenderDeferred( oRenderContext );
		break;
	}
}

Camera& Renderer::GetCamera()
{
	return m_oCamera;
}

const Camera& Renderer::GetCamera() const
{
	return m_oCamera;
}

TextRenderer& Renderer::GetTextRenderer()
{
	return m_oTextRenderer;
}

const TextRenderer& Renderer::GetTextRenderer() const
{
	return m_oTextRenderer;
}

bool Renderer::OnLoading()
{
	if( m_xForwardOpaque->IsLoaded() && m_oForwardOpaque.IsValid() == false )
		m_oForwardOpaque.Create( m_xForwardOpaque->GetTechnique() );

	if( m_xDeferredMaps->IsLoaded() && m_oDeferredMaps.IsValid() == false )
		m_oDeferredMaps.Create( m_xDeferredMaps->GetTechnique() );

	if( m_xDeferredCompose->IsLoaded() && m_oDeferredCompose.IsValid() == false )
		m_oDeferredCompose.Create( m_xDeferredCompose->GetTechnique() );

	return m_xDefaultDiffuseMap->IsLoaded() && m_xDefaultNormalMap->IsLoaded() && m_xForwardOpaque->IsLoaded() && m_xDeferredMaps->IsLoaded() && m_xDeferredCompose->IsLoaded() && m_oTextRenderer.OnLoading();
}

void Renderer::DisplayDebug()
{
	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_RENDERER_DEBUG ) )
		m_bDisplayDebug = !m_bDisplayDebug;

	if( m_bDisplayDebug == false )
		return;

	auto GetRenderingModeName = []( const RenderingMode eRenderingType ) -> const char* {
		switch( eRenderingType )
		{
		case RenderingMode::FORWARD:
			return "Forward";
		case RenderingMode::DEFERRED:
			return "Deferred";
		case RenderingMode::_COUNT:
			return "";
		}

		return "";
	};

	ImGui::Begin( "Renderer" );

	if( ImGui::BeginCombo( "Rendering mode", GetRenderingModeName( m_eRenderingMode ) ) )
	{
		for( uint u = 0; u < RenderingMode::_COUNT; ++u )
		{
			RenderingMode eRenderingMode = RenderingMode( u );
			if( ImGui::Selectable( GetRenderingModeName( eRenderingMode ), eRenderingMode == m_eRenderingMode ) )
				m_eRenderingMode = eRenderingMode;
		}
		ImGui::EndCombo();
	}

	ImGui::End();
}

void Renderer::RenderForward( const RenderContext& oRenderContext )
{
	const RenderRect& oRenderRect = oRenderContext.m_oRenderRect;
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	if( oRenderRect.m_uWidth != m_oRenderTarget.GetWidth() || oRenderRect.m_uHeight != m_oRenderTarget.GetHeight() )
		m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	SetTechnique( m_xForwardOpaque->GetTechnique() );

	{
		ArrayView< DirectionalLightComponent > aDirectionalLightComponents = g_pGameEngine->GetComponentManager().GetComponents< DirectionalLightComponent >();
		Array< glm::vec3 > aLightDirections( aDirectionalLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aDirectionalLightComponents.Count() );
		Array< float > aLightIntensities( aDirectionalLightComponents.Count() );
		for( uint u = 0; u < aDirectionalLightComponents.Count(); ++u )
		{
			aLightDirections[ u ] = aDirectionalLightComponents[ u ].m_vDirection;
			aLightColors[ u ] = aDirectionalLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aDirectionalLightComponents[ u ].m_fIntensity;
		}
		m_oForwardOpaque.SetDirectionalLights( aLightDirections, aLightColors, aLightIntensities );
	}

	{
		ArrayView< PointLightComponent > aPointLightComponents = g_pGameEngine->GetComponentManager().GetComponents< PointLightComponent >();
		Array< glm::vec3 > aLightPositions( aPointLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aPointLightComponents.Count() );
		Array< float > aLightIntensities( aPointLightComponents.Count() );
		Array< float > aLightFalloffFactors( aPointLightComponents.Count() );
		for( uint u = 0; u < aPointLightComponents.Count(); ++u )
		{
			aLightPositions[ u ] = aPointLightComponents[ u ].GetPosition();
			aLightColors[ u ] = aPointLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aPointLightComponents[ u ].m_fIntensity;
			aLightFalloffFactors[ u ] = aPointLightComponents[ u ].m_fFalloffFactor;
		}
		m_oForwardOpaque.SetPointLights( aLightPositions, aLightColors, aLightIntensities, aLightFalloffFactors );
	}

	{
		ArrayView< SpotLightComponent > aLightComponents = g_pGameEngine->GetComponentManager().GetComponents< SpotLightComponent >();
		Array< glm::vec3 > aLightPositions( aLightComponents.Count() );
		Array< glm::vec3 > aLightDirections( aLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aLightComponents.Count() );
		Array< float > aLightIntensities( aLightComponents.Count() );
		Array< float > aLightInnerAngles( aLightComponents.Count() );
		Array< float > aLightOuterAngles( aLightComponents.Count() );
		Array< float > aLightFalloffFactors( aLightComponents.Count() );
		for( uint u = 0; u < aLightComponents.Count(); ++u )
		{
			aLightPositions[ u ] = aLightComponents[ u ].GetPosition();
			aLightDirections[ u ] = aLightComponents[ u ].m_vDirection;
			aLightColors[ u ] = aLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aLightComponents[ u ].m_fIntensity;
			aLightInnerAngles[ u ] = aLightComponents[ u ].m_fInnerAngle;
			aLightOuterAngles[ u ] = aLightComponents[ u ].m_fOuterAngle;
			aLightFalloffFactors[ u ] = aLightComponents[ u ].m_fFalloffFactor;
		}
		m_oForwardOpaque.SetSpotLights( aLightPositions, aLightDirections, aLightColors, aLightIntensities, aLightInnerAngles, aLightOuterAngles, aLightFalloffFactors );
	}

	ArrayView< VisualComponent > aVisualComponents = g_pGameEngine->GetComponentManager().GetComponents< VisualComponent >();
	for( const VisualComponent& oVisualComponent : aVisualComponents )
	{
		m_oForwardOpaque.SetModelAndViewProjection( oVisualComponent.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );

		const Array< Mesh >& aMeshes = oVisualComponent.GetResource()->GetMeshes();
		for( const Mesh& oMesh : aMeshes )
		{
			if( oMesh.m_pMaterial != nullptr )
			{
				m_oForwardOpaque.SetDiffuseColor( oMesh.m_pMaterial->m_vDiffuseColor );

				if( oMesh.m_pMaterial->m_xDiffuseTextureResource != nullptr )
				{
					SetTextureSlot( oMesh.m_pMaterial->m_xDiffuseTextureResource->GetTexture(), 0 );
					m_oForwardOpaque.SetDiffuseMap( 0 );
				}
				else
				{
					SetTextureSlot( m_xDefaultDiffuseMap->GetTexture(), 0 );
					m_oForwardOpaque.SetDiffuseMap( 0 );
				}

				if( oMesh.m_pMaterial->m_xNormalTextureResource != nullptr )
				{
					SetTextureSlot( oMesh.m_pMaterial->m_xNormalTextureResource->GetTexture(), 1 );
					m_oForwardOpaque.SetNormalMap( 1 );
				}
				else
				{
					SetTextureSlot( m_xDefaultNormalMap->GetTexture(), 1 );
					m_oForwardOpaque.SetNormalMap( 1 );
				}
			}
			else
			{
				ClearTextureSlot( 0 );
				ClearTextureSlot( 1 );
			}

			DrawMesh( oMesh );
		}
	}

	ClearTechnique();
}

void Renderer::RenderDeferred( const RenderContext& oRenderContext )
{
	const RenderRect& oRenderRect = oRenderContext.m_oRenderRect;
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	if( oRenderRect.m_uWidth != m_oRenderTarget.GetWidth() || oRenderRect.m_uHeight != m_oRenderTarget.GetHeight() )
	{
		m_oRenderTarget.Destroy();

		Array< TextureFormat > aFormats( 2 );
		aFormats[ 0 ] = TextureFormat::RGB;
		aFormats[ 1 ] = TextureFormat::NORMAL;
		m_oRenderTarget.Create( oRenderRect.m_uWidth, oRenderRect.m_uHeight, aFormats, true );

		m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );
	}

	SetRenderTarget( m_oRenderTarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	SetTechnique( m_xDeferredMaps->GetTechnique() );

	ArrayView< VisualComponent > aVisualComponents = g_pGameEngine->GetComponentManager().GetComponents< VisualComponent >();
	for( const VisualComponent& oVisualComponent : aVisualComponents )
	{
		m_oDeferredMaps.SetModelAndViewProjection( oVisualComponent.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );

		const Array< Mesh >& aMeshes = oVisualComponent.GetResource()->GetMeshes();
		for( const Mesh& oMesh : aMeshes )
		{
			if( oMesh.m_pMaterial != nullptr )
			{
				m_oDeferredMaps.SetDiffuseColor( oMesh.m_pMaterial->m_vDiffuseColor );

				if( oMesh.m_pMaterial->m_xDiffuseTextureResource != nullptr )
				{
					SetTextureSlot( oMesh.m_pMaterial->m_xDiffuseTextureResource->GetTexture(), 0 );
					m_oDeferredMaps.SetDiffuseMap( 0 );
				}
				else
				{
					SetTextureSlot( m_xDefaultDiffuseMap->GetTexture(), 0 );
					m_oDeferredMaps.SetDiffuseMap( 0 );
				}

				if( oMesh.m_pMaterial->m_xNormalTextureResource != nullptr )
				{
					SetTextureSlot( oMesh.m_pMaterial->m_xNormalTextureResource->GetTexture(), 1 );
					m_oDeferredMaps.SetNormalMap( 1 );
				}
				else
				{
					SetTextureSlot( m_xDefaultNormalMap->GetTexture(), 1 );
					m_oDeferredMaps.SetNormalMap( 1 );
				}
			}
			else
			{
				ClearTextureSlot( 0 );
				ClearTextureSlot( 1 );
			}

			DrawMesh( oMesh );
		}
	}

	ClearRenderTarget();

	glClear( GL_COLOR_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	SetTechnique( m_xDeferredCompose->GetTechnique() );

	SetTextureSlot( m_oRenderTarget.GetColorMap( 0 ), 0 );
	m_oDeferredCompose.SetColor( 0 );
	SetTextureSlot( m_oRenderTarget.GetColorMap( 1 ), 1 );
	m_oDeferredCompose.SetNormal( 1 );
	SetTextureSlot( m_oRenderTarget.GetDepthMap(), 2 );
	m_oDeferredCompose.SetDepth( 2 );
	m_oDeferredCompose.SetInverseViewProjection( m_oCamera.GetInverseViewProjectionMatrix() );

	{
		ArrayView< DirectionalLightComponent > aLightComponents = g_pGameEngine->GetComponentManager().GetComponents< DirectionalLightComponent >();
		Array< glm::vec3 > aLightDirections( aLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aLightComponents.Count() );
		Array< float > aLightIntensities( aLightComponents.Count() );
		for( uint u = 0; u < aLightComponents.Count(); ++u )
		{
			aLightDirections[ u ] = aLightComponents[ u ].m_vDirection;
			aLightColors[ u ] = aLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aLightComponents[ u ].m_fIntensity;
		}
		m_oDeferredCompose.SetDirectionalLights( aLightDirections, aLightColors, aLightIntensities );
	}

	{
		ArrayView< PointLightComponent > aLightComponents = g_pGameEngine->GetComponentManager().GetComponents< PointLightComponent >();
		Array< glm::vec3 > aLightPositions( aLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aLightComponents.Count() );
		Array< float > aLightIntensities( aLightComponents.Count() );
		Array< float > aLightFalloffFactors( aLightComponents.Count() );
		for( uint u = 0; u < aLightComponents.Count(); ++u )
		{
			aLightPositions[ u ] = aLightComponents[ u ].GetPosition();
			aLightColors[ u ] = aLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aLightComponents[ u ].m_fIntensity;
			aLightFalloffFactors[ u ] = aLightComponents[ u ].m_fFalloffFactor;
		}
		m_oDeferredCompose.SetPointLights( aLightPositions, aLightColors, aLightIntensities, aLightFalloffFactors );
	}

	{
		ArrayView< SpotLightComponent > aLightComponents = g_pGameEngine->GetComponentManager().GetComponents< SpotLightComponent >();
		Array< glm::vec3 > aLightPositions( aLightComponents.Count() );
		Array< glm::vec3 > aLightDirections( aLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aLightComponents.Count() );
		Array< float > aLightIntensities( aLightComponents.Count() );
		Array< float > aLightInnerAngles( aLightComponents.Count() );
		Array< float > aLightOuterAngles( aLightComponents.Count() );
		Array< float > aLightFalloffFactors( aLightComponents.Count() );
		for( uint u = 0; u < aLightComponents.Count(); ++u )
		{
			aLightPositions[ u ] = aLightComponents[ u ].GetPosition();
			aLightDirections[ u ] = aLightComponents[ u ].m_vDirection;
			aLightColors[ u ] = aLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aLightComponents[ u ].m_fIntensity;
			aLightInnerAngles[ u ] = aLightComponents[ u ].m_fInnerAngle;
			aLightOuterAngles[ u ] = aLightComponents[ u ].m_fOuterAngle;
			aLightFalloffFactors[ u ] = aLightComponents[ u ].m_fFalloffFactor;
		}
		m_oDeferredCompose.SetSpotLights( aLightPositions, aLightDirections, aLightColors, aLightIntensities, aLightInnerAngles, aLightOuterAngles, aLightFalloffFactors );
	}

	DrawMesh( m_oRenderMesh );

	ClearTextureSlot( 0 );
	ClearTechnique();
}

void Renderer::SetTechnique( const Technique& oTechnique )
{
	glUseProgram( oTechnique.m_uProgramID );
}

void Renderer::ClearTechnique()
{
	glUseProgram( 0 );
}

void Renderer::SetTextureSlot( const Texture& oTexture, const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_2D, oTexture.m_uTextureID );
}

void Renderer::ClearTextureSlot( const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Renderer::SetRenderTarget( const RenderTarget& oRenderTarget )
{
	glBindFramebuffer( GL_FRAMEBUFFER, oRenderTarget.m_uFrameBufferID );
}

void Renderer::ClearRenderTarget()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Renderer::DrawMesh( const Mesh& oMesh )
{
	glBindVertexArray( oMesh.m_uVertexArrayID );
	glDrawElements( GL_TRIANGLES, oMesh.m_iIndexCount, GL_UNSIGNED_INT, nullptr );
	glBindVertexArray( 0 );
}
