#include "Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>

#include "Game/Entity.h"
#include "Game/InputHandler.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "ImGui/imgui.h"

static const std::string PARAM_MODEL_VIEW_PROJECTION( "modelViewProjection" );
static const std::string PARAM_MODEL_INVERSE_TRANSPOSE( "modelInverseTranspose" );
static const std::string PARAM_MODEL( "model" );
static const std::string PARAM_INVERSE_VIEW_PROJECTION( "inverseViewProjection" );

static const std::string PARAM_DIFFUSE_COLOR( "diffuseColor" );
static const std::string PARAM_DIFFUSE_MAP( "diffuseMap" );
static const std::string PARAM_NORMAL_MAP( "normalMap" );
static const std::string PARAM_COLOR_MAP( "colorMap" );
static const std::string PARAM_DEPTH_MAP( "depthMap" );

static const std::string PARAM_COLOR( "color" );
static const std::string PARAM_COLOR_ID( "colorID" );
static const std::string PARAM_DISPLACEMENT( "displacement" );
static const std::string PARAM_CAMERA_POSITION( "cameraPosition" );

static const std::string PARAM_DIRECTIONAL_LIGHT_COUNT( "directionalLightCount" );
static const std::string PARAM_POINT_LIGHT_COUNT( "pointLightCount" );
static const std::string PARAM_SPOT_LIGHT_COUNT( "spotLightCount" );

static const std::string PARAM_DIRECTIONAL_LIGHT_DIRECTIONS( "directionalLightDirections" );
static const std::string PARAM_DIRECTIONAL_LIGHT_COLORS( "directionalLightColors" );
static const std::string PARAM_DIRECTIONAL_LIGHT_INTENSITIES( "directionalLightIntensities" );

static const std::string PARAM_POINT_LIGHT_POSITIONS( "pointLightPositions" );
static const std::string PARAM_POINT_LIGHT_COLORS( "pointLightColors" );
static const std::string PARAM_POINT_LIGHT_INTENSITIES( "pointLightIntensities" );
static const std::string PARAM_POINT_LIGHT_FALLOFF_MIN_DISTANCES( "pointLightFalloffMinDistances" );
static const std::string PARAM_POINT_LIGHT_FALLOFF_MAX_DISTANCES( "pointLightFalloffMaxDistances" );

static const std::string PARAM_SPOT_LIGHT_POSITIONS( "spotLightPositions" );
static const std::string PARAM_SPOT_LIGHT_DIRECTIONS( "spotLightDirections" );
static const std::string PARAM_SPOT_LIGHT_COLORS( "spotLightColors" );
static const std::string PARAM_SPOT_LIGHT_INTENSITIES( "spotLightIntensities" );
static const std::string PARAM_SPOT_LIGHT_OUTERRANGES( "spotLightOuterRanges" );
static const std::string PARAM_SPOT_LIGHT_RANGES( "spotLightRanges" );
static const std::string PARAM_SPOT_LIGHT_FALLOFF_MIN_DISTANCES( "spotLightFalloffMinDistances" );
static const std::string PARAM_SPOT_LIGHT_FALLOFF_MAX_DISTANCES( "spotLightFalloffMaxDistances" );

template < typename Technique >
void SetupLighting( Technique& oTechnique, const Array< DirectionalLight >& aDirectionalLights, const Array< PointLight >& aPointLights, const Array< SpotLight >& aSpotLights )
{
	oTechnique.SetParameter( PARAM_DIRECTIONAL_LIGHT_COUNT, ( int )aDirectionalLights.Count() );
	oTechnique.SetParameter( PARAM_POINT_LIGHT_COUNT, ( int )aPointLights.Count() );
	oTechnique.SetParameter( PARAM_SPOT_LIGHT_COUNT, ( int )aSpotLights.Count() );

	for( uint u = 0; u < aDirectionalLights.Count(); ++u )
	{
		oTechnique.SetArrayParameter( PARAM_DIRECTIONAL_LIGHT_DIRECTIONS, aDirectionalLights[ u ].m_vDirection, u );
		oTechnique.SetArrayParameter( PARAM_DIRECTIONAL_LIGHT_COLORS, aDirectionalLights[ u ].m_vColor, u );
		oTechnique.SetArrayParameter( PARAM_DIRECTIONAL_LIGHT_INTENSITIES, aDirectionalLights[ u ].m_fIntensity, u );
	}

	for( uint u = 0; u < aPointLights.Count(); ++u )
	{
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_POSITIONS, aPointLights[ u ].m_vPosition, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_COLORS, aPointLights[ u ].m_vColor, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_INTENSITIES, aPointLights[ u ].m_fIntensity, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_FALLOFF_MIN_DISTANCES, aPointLights[ u ].m_fFalloffMinDistance, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_FALLOFF_MAX_DISTANCES, aPointLights[ u ].m_fFalloffMaxDistance, u );
	}

	for( uint u = 0; u < aSpotLights.Count(); ++u )
	{
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_POSITIONS, aSpotLights[ u ].m_vPosition, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_DIRECTIONS, aSpotLights[ u ].m_vDirection, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_COLORS, aSpotLights[ u ].m_vColor, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_INTENSITIES, aSpotLights[ u ].m_fIntensity, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_OUTERRANGES, glm::cos( glm::radians( aSpotLights[ u ].m_fOuterAngle / 2.f ) ), u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_RANGES, glm::cos( glm::radians( aSpotLights[ u ].m_fInnerAngle / 2.f ) ) - glm::cos( glm::radians( aSpotLights[ u ].m_fOuterAngle / 2.f ) ), u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_FALLOFF_MIN_DISTANCES, aSpotLights[ u ].m_fFalloffMinDistance, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_FALLOFF_MAX_DISTANCES, aSpotLights[ u ].m_fFalloffMaxDistance, u );
	}
}

template < typename Technique >
void DrawMeshes( Technique& oTechnique )
{
	for( const VisualNode& oVisualNode : g_pRenderer->m_oVisualStructure )
	{
		oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
		oTechnique.SetParameter( PARAM_MODEL_INVERSE_TRANSPOSE, glm::inverseTranspose( oVisualNode.m_mMatrix ) );

		if( oTechnique.HasParameter( PARAM_MODEL ) )
			oTechnique.SetParameter( PARAM_MODEL, oVisualNode.m_mMatrix );

		const Array< Mesh >& aMeshes = *oVisualNode.m_pMeshes;
		for( const Mesh& oMesh : aMeshes )
		{
			if( oMesh.m_pMaterial != nullptr )
			{
				oTechnique.SetParameter( PARAM_DIFFUSE_COLOR, oMesh.m_pMaterial->m_vDiffuseColor );

				if( oMesh.m_pMaterial->m_xDiffuseTextureResource != nullptr )
				{
					g_pRenderer->SetTextureSlot( oMesh.m_pMaterial->m_xDiffuseTextureResource->GetTexture(), 0 );
					oTechnique.SetParameter( PARAM_DIFFUSE_MAP, 0 );
				}
				else
				{
					g_pRenderer->SetTextureSlot( g_pRenderer->m_xDefaultDiffuseMap->GetTexture(), 0 );
					oTechnique.SetParameter( PARAM_DIFFUSE_MAP, 0 );
				}

				if( oMesh.m_pMaterial->m_xNormalTextureResource != nullptr )
				{
					g_pRenderer->SetTextureSlot( oMesh.m_pMaterial->m_xNormalTextureResource->GetTexture(), 1 );
					oTechnique.SetParameter( PARAM_NORMAL_MAP, 1 );
				}
				else
				{
					g_pRenderer->SetTextureSlot( g_pRenderer->m_xDefaultNormalMap->GetTexture(), 1 );
					oTechnique.SetParameter( PARAM_NORMAL_MAP, 1 );
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
	, m_xForwardOpaque( g_pResourceLoader->LoadTechnique( "Shader/forward_opaque.tech" ) )
	, m_xDeferredMaps( g_pResourceLoader->LoadTechnique( "Shader/deferred_maps.tech" ) )
	, m_xDeferredCompose( g_pResourceLoader->LoadTechnique( "Shader/deferred_compose.tech" ) )
	, m_xGizmo( g_pResourceLoader->LoadTechnique( "Shader/gizmo.tech" ) )
	, m_xPicking( g_pResourceLoader->LoadTechnique( "Shader/picking.tech" ) )
	, m_xOutline( g_pResourceLoader->LoadTechnique( "Shader/outline.tech" ) )
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

void Renderer::Clear()
{
	m_oVisualStructure.Clear();
}

bool Renderer::OnLoading()
{
	bool bLoaded = m_xDefaultDiffuseMap->IsLoaded() && m_xDefaultNormalMap->IsLoaded();
	bLoaded &= m_xForwardOpaque->IsLoaded() && m_xDeferredMaps->IsLoaded() && m_xDeferredCompose->IsLoaded() && m_xPicking->IsLoaded() && m_xOutline->IsLoaded() && m_xGizmo->IsLoaded();
	bLoaded &= m_oTextRenderer.OnLoading() && m_oDebugRenderer.OnLoading();

	return bLoaded;
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

	Technique& oTechnique = m_xForwardOpaque->GetTechnique();
	SetTechnique( oTechnique );

	SetupLighting( oTechnique, m_oVisualStructure.m_aDirectionalLights, m_oVisualStructure.m_aPointLights, m_oVisualStructure.m_aSpotLights );
	DrawMeshes( oTechnique );

	ClearTechnique();
}

void Renderer::RenderDeferred( const RenderContext& oRenderContext )
{
	SetRenderTarget( m_oRenderTarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	Technique& oMapsTechnique = m_xDeferredMaps->GetTechnique();
	SetTechnique( oMapsTechnique );

	DrawMeshes( oMapsTechnique );

	ClearRenderTarget();

	glClear( GL_COLOR_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	Technique& oComposeTechnique = m_xDeferredCompose->GetTechnique();
	SetTechnique( oComposeTechnique );

	SetTextureSlot( m_oRenderTarget.GetColorMap( 0 ), 0 );
	oComposeTechnique.SetParameter( PARAM_COLOR_MAP, 0 );
	SetTextureSlot( m_oRenderTarget.GetColorMap( 1 ), 1 );
	oComposeTechnique.SetParameter( PARAM_NORMAL_MAP, 1 );
	SetTextureSlot( m_oRenderTarget.GetDepthMap(), 2 );
	oComposeTechnique.SetParameter( PARAM_DEPTH_MAP, 2 );
	oComposeTechnique.SetParameter( PARAM_INVERSE_VIEW_PROJECTION, m_oCamera.GetInverseViewProjectionMatrix() );

	SetupLighting( oComposeTechnique, m_oVisualStructure.m_aDirectionalLights, m_oVisualStructure.m_aPointLights, m_oVisualStructure.m_aSpotLights );

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

	Technique& oTechnique = m_xPicking->GetTechnique();
	SetTechnique( oTechnique );

	auto BuildColorID = []( const uint64 uID ) {
		const uint16 uRed = ( uID >> 48 ) & 0xFFFF;
		const uint16 uGreen = ( uID >> 32 ) & 0xFFFF;
		const uint16 uBlue = ( uID >> 16 ) & 0xFFFF;
		const uint16 uAlpha = ( uID ) & 0xFFFF;

		return glm::uvec4( uRed, uGreen, uBlue, uAlpha );
	};

	for( const VisualNode& oVisualNode : g_pRenderer->m_oVisualStructure )
	{
		oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
		oTechnique.SetParameter( PARAM_COLOR_ID, BuildColorID( oVisualNode.m_uEntityID ) );

		const Array< Mesh >& aMeshes = *oVisualNode.m_pMeshes;
		for( const Mesh& oMesh : aMeshes )
			DrawMesh( oMesh );
	}

	if( bAllowGizmos )
	{
		glClear( GL_DEPTH_BUFFER_BIT );

		ArrayView< GizmoComponent > aComponents = g_pComponentManager->GetComponents< GizmoComponent >();
		for( const GizmoComponent& oComponent : aComponents )
		{
			oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * oComponent.GetWorldMatrix() );
			oTechnique.SetParameter( PARAM_COLOR_ID, BuildColorID( oComponent.GetEntity()->GetID() ) );

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

void Renderer::RenderOutline( const RenderContext& oRenderContext, const VisualNode& oVisualNode )
{
	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_STENCIL_TEST );

	glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	glStencilFunc( GL_ALWAYS, 1, 0xFF );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	Technique& oTechnique = m_xOutline->GetTechnique();
	SetTechnique( oTechnique );

	oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
	oTechnique.SetParameter( PARAM_DISPLACEMENT, 0.f );

	const Array< Mesh >& aMeshes = *oVisualNode.m_pMeshes;
	for( const Mesh& oMesh : aMeshes )
		DrawMesh( oMesh );

	glDisable( GL_DEPTH_TEST );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glStencilFunc( GL_NOTEQUAL, 1, 0xFF );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
	oTechnique.SetParameter( PARAM_CAMERA_POSITION, glm::vec3( m_oCamera.GetPosition() ) );
	oTechnique.SetParameter( PARAM_DISPLACEMENT, 0.004f );

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

	Technique& oTechnique = m_xGizmo->GetTechnique();
	SetTechnique( oTechnique );

	ArrayView< GizmoComponent > aGizmoComponents = g_pComponentManager->GetComponents< GizmoComponent >();
	for( const GizmoComponent& oGizmoComponent : aGizmoComponents )
	{
		oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * oGizmoComponent.GetWorldMatrix() );
		oTechnique.SetParameter( PARAM_COLOR, oGizmoComponent.GetColor() );

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
