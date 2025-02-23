#include "Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/transform.hpp>

#include "Game/Component.h"
#include "Game/ComponentManager.h"
#include "Game/Entity.h"
#include "Game/InputHandler.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "ImGui/imgui.h"

template < typename LightingDefinition >
void SetupLighting( LightingDefinition& oLightingDefinition )
{
	{
		ArrayView< DirectionalLightComponent > aDirectionalLightComponents = g_pComponentManager->GetComponents< DirectionalLightComponent >();
		Array< glm::vec3 > aLightDirections( aDirectionalLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aDirectionalLightComponents.Count() );
		Array< float > aLightIntensities( aDirectionalLightComponents.Count() );
		for( uint u = 0; u < aDirectionalLightComponents.Count(); ++u )
		{
			aLightDirections[ u ] = aDirectionalLightComponents[ u ].m_vDirection;
			aLightColors[ u ] = aDirectionalLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aDirectionalLightComponents[ u ].m_fIntensity;
		}

		oLightingDefinition.SetDirectionalLights( aLightDirections, aLightColors, aLightIntensities );
	}

	{
		ArrayView< PointLightComponent > aPointLightComponents = g_pComponentManager->GetComponents< PointLightComponent >();
		Array< glm::vec3 > aLightPositions( aPointLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aPointLightComponents.Count() );
		Array< float > aLightIntensities( aPointLightComponents.Count() );
		Array< float > aLightFalloffMinDistances( aPointLightComponents.Count() );
		Array< float > aLightFalloffMaxDistances( aPointLightComponents.Count() );
		for( uint u = 0; u < aPointLightComponents.Count(); ++u )
		{
			aLightPositions[ u ] = aPointLightComponents[ u ].GetPosition();
			aLightColors[ u ] = aPointLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aPointLightComponents[ u ].m_fIntensity;
			aLightFalloffMinDistances[ u ] = aPointLightComponents[ u ].m_fFalloffMinDistance;
			aLightFalloffMaxDistances[ u ] = aPointLightComponents[ u ].m_fFalloffMaxDistance;
		}

		oLightingDefinition.SetPointLights( aLightPositions, aLightColors, aLightIntensities, aLightFalloffMinDistances, aLightFalloffMaxDistances );
	}

	{
		ArrayView< SpotLightComponent > aLightComponents = g_pComponentManager->GetComponents< SpotLightComponent >();
		Array< glm::vec3 > aLightPositions( aLightComponents.Count() );
		Array< glm::vec3 > aLightDirections( aLightComponents.Count() );
		Array< glm::vec3 > aLightColors( aLightComponents.Count() );
		Array< float > aLightIntensities( aLightComponents.Count() );
		Array< float > aLightInnerAngles( aLightComponents.Count() );
		Array< float > aLightOuterAngles( aLightComponents.Count() );
		Array< float > aLightFalloffMinDistances( aLightComponents.Count() );
		Array< float > aLightFalloffMaxDistances( aLightComponents.Count() );
		for( uint u = 0; u < aLightComponents.Count(); ++u )
		{
			aLightPositions[ u ] = aLightComponents[ u ].GetPosition();
			aLightDirections[ u ] = aLightComponents[ u ].m_vDirection;
			aLightColors[ u ] = aLightComponents[ u ].m_vColor;
			aLightIntensities[ u ] = aLightComponents[ u ].m_fIntensity;
			aLightInnerAngles[ u ] = aLightComponents[ u ].m_fInnerAngle;
			aLightOuterAngles[ u ] = aLightComponents[ u ].m_fOuterAngle;
			aLightFalloffMinDistances[ u ] = aLightComponents[ u ].m_fFalloffMinDistance;
			aLightFalloffMaxDistances[ u ] = aLightComponents[ u ].m_fFalloffMaxDistance;
		}

		oLightingDefinition.SetSpotLights( aLightPositions, aLightDirections, aLightColors, aLightIntensities, aLightInnerAngles, aLightOuterAngles, aLightFalloffMinDistances, aLightFalloffMaxDistances );
	}
}

template < typename MeshesDefinition >
void DrawMeshes( MeshesDefinition& oMeshesDefinition )
{
	ArrayView< VisualComponent > aVisualComponents = g_pComponentManager->GetComponents< VisualComponent >();
	for( const VisualComponent& oVisualComponent : aVisualComponents )
	{
		oMeshesDefinition.SetModelAndViewProjection( oVisualComponent.GetWorldMatrix(), g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

		const Array< Mesh >& aMeshes = oVisualComponent.GetResource()->GetMeshes();
		for( const Mesh& oMesh : aMeshes )
		{
			if( oMesh.m_pMaterial != nullptr )
			{
				oMeshesDefinition.SetDiffuseColor( oMesh.m_pMaterial->m_vDiffuseColor );

				if( oMesh.m_pMaterial->m_xDiffuseTextureResource != nullptr )
				{
					g_pRenderer->SetTextureSlot( oMesh.m_pMaterial->m_xDiffuseTextureResource->GetTexture(), 0 );
					oMeshesDefinition.SetDiffuseMap( 0 );
				}
				else
				{
					g_pRenderer->SetTextureSlot( g_pRenderer->m_xDefaultDiffuseMap->GetTexture(), 0 );
					oMeshesDefinition.SetDiffuseMap( 0 );
				}

				if( oMesh.m_pMaterial->m_xNormalTextureResource != nullptr )
				{
					g_pRenderer->SetTextureSlot( oMesh.m_pMaterial->m_xNormalTextureResource->GetTexture(), 1 );
					oMeshesDefinition.SetNormalMap( 1 );
				}
				else
				{
					g_pRenderer->SetTextureSlot( g_pRenderer->m_xDefaultNormalMap->GetTexture(), 1 );
					oMeshesDefinition.SetNormalMap( 1 );
				}
			}
			else
			{
				g_pRenderer->ClearTextureSlot( 0 );
				g_pRenderer->ClearTextureSlot( 1 );
			}

			g_pRenderer->DrawMesh( oMesh );
		}
	}
}

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
	: m_xDefaultDiffuseMap( g_pResourceLoader->LoadTexture( "Default_diffuse.png" ) )
	, m_xDefaultNormalMap( g_pResourceLoader->LoadTexture( "Default_normal.png" ) )
	, m_xForwardOpaque( g_pResourceLoader->LoadTechnique( "Shader/forward_opaque" ) )
	, m_xDeferredMaps( g_pResourceLoader->LoadTechnique( "Shader/deferred_maps" ) )
	, m_xDeferredCompose( g_pResourceLoader->LoadTechnique( "Shader/deferred_compose" ) )
	, m_xGizmo( g_pResourceLoader->LoadTechnique( "Shader/gizmo" ) )
	, m_xPicking( g_pResourceLoader->LoadTechnique( "Shader/picking" ) )
	, m_xOutline( g_pResourceLoader->LoadTechnique( "Shader/outline" ) )
	, m_eRenderingMode( RenderingMode::FORWARD )
	, m_bMSAA( false )
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

	glClearColor( 0.f, 0.f, 0.f, 1.f );

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

bool Renderer::OnLoading()
{
	if( m_xForwardOpaque->IsLoaded() && m_oForwardOpaque.IsValid() == false )
		m_oForwardOpaque.Create( m_xForwardOpaque->GetTechnique() );

	if( m_xDeferredMaps->IsLoaded() && m_oDeferredMaps.IsValid() == false )
		m_oDeferredMaps.Create( m_xDeferredMaps->GetTechnique() );

	if( m_xDeferredCompose->IsLoaded() && m_oDeferredCompose.IsValid() == false )
		m_oDeferredCompose.Create( m_xDeferredCompose->GetTechnique() );

	if( m_xPicking->IsLoaded() && m_oPicking.IsValid() == false )
		m_oPicking.Create( m_xPicking->GetTechnique() );

	if( m_xOutline->IsLoaded() && m_oOutline.IsValid() == false )
		m_oOutline.Create( m_xOutline->GetTechnique() );

	if( m_xGizmo->IsLoaded() && m_oGizmo.IsValid() == false )
		m_oGizmo.Create( m_xGizmo->GetTechnique() );

	return m_xDefaultDiffuseMap->IsLoaded() && m_xDefaultNormalMap->IsLoaded() && m_xForwardOpaque->IsLoaded() && m_xDeferredMaps->IsLoaded() && m_xDeferredCompose->IsLoaded() && m_xPicking->IsLoaded() && m_xOutline->IsLoaded() && m_xGizmo->IsLoaded() && m_oTextRenderer.OnLoading() &&m_oDebugRenderer.OnLoading();
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

	if( m_eRenderingMode == FORWARD )
		ImGui::Checkbox( "MSAA", &m_bMSAA );

	ImGui::End();
}

void Renderer::RenderForward( const RenderContext& oRenderContext )
{
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	if( m_bMSAA )
		glEnable( GL_MULTISAMPLE );
	else
		glDisable( GL_MULTISAMPLE );

	SetTechnique( m_xForwardOpaque->GetTechnique() );

	SetupLighting( m_oForwardOpaque );
	DrawMeshes( m_oForwardOpaque );

	ClearTechnique();
}

void Renderer::RenderDeferred( const RenderContext& oRenderContext )
{
	SetRenderTarget( m_oRenderTarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	SetTechnique( m_xDeferredMaps->GetTechnique() );

	DrawMeshes( m_oDeferredMaps );

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

	SetupLighting( m_oDeferredCompose );

	DrawMesh( m_oRenderMesh );

	ClearTextureSlot( 0 );
	ClearTechnique();

	CopyDepthToBackBuffer( m_oRenderTarget, oRenderContext.GetRenderRect() );

}

uint64 Renderer::RenderPicking( const RenderContext& oRenderContext, const int iCursorX, const int iCursorY, const bool bAllowGizmos )
{
	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	if( oRenderRect.m_uWidth != m_oPickingTarget.GetWidth() || oRenderRect.m_uHeight != m_oPickingTarget.GetHeight() )
	{
		m_oPickingTarget.Destroy();

		Array< TextureFormat > aFormats( 1 );
		aFormats[ 0 ] = TextureFormat::ID;
		m_oPickingTarget.Create( oRenderRect.m_uWidth, oRenderRect.m_uHeight, aFormats, true );

		m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );
	}

	SetRenderTarget( m_oPickingTarget );

	glClearColor( 1.f, 1.f, 1.f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	SetTechnique( m_xPicking->GetTechnique() );

	{
		ArrayView< VisualComponent > aComponents = g_pComponentManager->GetComponents< VisualComponent >();
		for( const VisualComponent& oComponent : aComponents )
		{
			m_oPicking.SetModelAndViewProjection( oComponent.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );
			m_oPicking.SetID( oComponent.GetEntity()->GetID() );

			const Array< Mesh >& aMeshes = oComponent.GetResource()->GetMeshes();
			for( const Mesh& oMesh : aMeshes )
				DrawMesh( oMesh );
		}
	}

	if( bAllowGizmos )
	{
		glClear( GL_DEPTH_BUFFER_BIT );

		ArrayView< GizmoComponent > aComponents = g_pComponentManager->GetComponents< GizmoComponent >();
		for( const GizmoComponent& oComponent : aComponents )
		{
			m_oPicking.SetModelAndViewProjection( oComponent.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );
			m_oPicking.SetID( oComponent.GetEntity()->GetID() );

			m_oGizmoRenderer.RenderGizmo( oComponent.GetType(), oComponent.GetAxis(), oRenderContext );
		}
	}

	ClearTechnique();

	GLushort aChannels[ 4 ];
	glReadPixels( iCursorX, oRenderRect.m_uHeight - iCursorY - 1, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, &aChannels[ 0 ] );

	ClearRenderTarget();

	const uint64 uRed = ( ( uint64 )aChannels[ 0 ] << 48 ) & 0xFFFF'0000'0000'0000;
	const uint64 uGreen = ( ( uint64 )aChannels[ 1 ] << 32 ) & 0x0000'FFFF'0000'0000;
	const uint64 uBlue = ( ( uint64 )aChannels[ 2 ] << 16 ) & 0x0000'0000'FFFF'0000;
	const uint64 uAlpha = ( ( uint64 )aChannels[ 3 ] ) & 0x0000'0000'0000'FFFF;

	const uint64 uID = uRed | uGreen | uBlue | uAlpha;

	return uID;
}

void Renderer::RenderOutline( const RenderContext& oRenderContext, const VisualComponent& oObject )
{
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_STENCIL_TEST );

	glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	glStencilFunc( GL_ALWAYS, 1, 0xFF );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	SetTechnique( m_xOutline->GetTechnique() );

	m_oOutline.SetModelAndViewProjection( oObject.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );
	m_oOutline.SetDisplacement( 0.f );

	const Array< Mesh >& aMeshes = oObject.GetResource()->GetMeshes();
	for( const Mesh& oMesh : aMeshes )
		DrawMesh( oMesh );

	glDisable( GL_DEPTH_TEST );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glStencilFunc( GL_NOTEQUAL, 1, 0xFF );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	m_oOutline.SetModelAndViewProjection( oObject.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );
	m_oOutline.SetCameraPosition( glm::vec3( m_oCamera.GetPosition() ) );
	m_oOutline.SetDisplacement( 0.004f );

	for( const Mesh& oMesh : aMeshes )
		DrawMesh( oMesh );

	ClearTechnique();

	glDisable( GL_STENCIL_TEST );
	glEnable( GL_CULL_FACE );
}

void Renderer::RenderGizmos( const RenderContext& oRenderContext )
{
	glEnable( GL_DEPTH_TEST );

	glClear( GL_DEPTH_BUFFER_BIT );

	SetTechnique( m_xGizmo->GetTechnique() );

	ArrayView< GizmoComponent > aGizmoComponents = g_pComponentManager->GetComponents< GizmoComponent >();
	for( const GizmoComponent& oGizmoComponent : aGizmoComponents )
	{
		m_oGizmo.SetModelAndViewProjection( oGizmoComponent.GetWorldMatrix(), m_oCamera.GetViewProjectionMatrix() );
		m_oGizmo.SetColor( oGizmoComponent.GetColor() );

		m_oGizmoRenderer.RenderGizmo( oGizmoComponent.GetType(), oGizmoComponent.GetAxis(), oRenderContext );
	}

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

void Renderer::CopyDepthToBackBuffer( const RenderTarget& oRenderTarget, const RenderRect& oRect )
{
	glBindFramebuffer( GL_READ_FRAMEBUFFER, oRenderTarget.m_uFrameBufferID );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, 0 );

	const int iX = ( int )oRect.m_uX;
	const int iY = ( int )oRect.m_uY;
	const int iWidth = ( int )oRect.m_uWidth;
	const int iHeight = ( int )oRect.m_uHeight;
	glBlitFramebuffer( iX, iY, iWidth, iHeight, iX, iY, iWidth, iHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Renderer::DrawMesh( const Mesh& oMesh )
{
	glBindVertexArray( oMesh.m_uVertexArrayID );
	glDrawElements( GL_TRIANGLES, oMesh.m_iIndexCount, GL_UNSIGNED_INT, nullptr );
	glBindVertexArray( 0 );
}
