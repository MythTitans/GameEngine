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
static const std::string PARAM_VIEW_POSITION( "viewPosition" );

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

static const std::string PARAM_TEXTURE_A( "textureA" );
static const std::string PARAM_TEXTURE_B( "textureB" );

static const std::string PARAM_BONE_MATRICES( "boneMatrices" );

template < typename Technique >
static void SetupLighting( Technique& oTechnique, const Array< DirectionalLight* >& aDirectionalLights, const Array< PointLight* >& aPointLights, const Array< SpotLight* >& aSpotLights )
{
	oTechnique.SetParameter( PARAM_DIRECTIONAL_LIGHT_COUNT, ( int )aDirectionalLights.Count() );
	oTechnique.SetParameter( PARAM_POINT_LIGHT_COUNT, ( int )aPointLights.Count() );
	oTechnique.SetParameter( PARAM_SPOT_LIGHT_COUNT, ( int )aSpotLights.Count() );

	for( uint u = 0; u < aDirectionalLights.Count(); ++u )
	{
		oTechnique.SetArrayParameter( PARAM_DIRECTIONAL_LIGHT_DIRECTIONS, aDirectionalLights[ u ]->m_vDirection, u );
		oTechnique.SetArrayParameter( PARAM_DIRECTIONAL_LIGHT_COLORS, aDirectionalLights[ u ]->m_vColor, u );
		oTechnique.SetArrayParameter( PARAM_DIRECTIONAL_LIGHT_INTENSITIES, aDirectionalLights[ u ]->m_fIntensity, u );
	}

	for( uint u = 0; u < aPointLights.Count(); ++u )
	{
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_POSITIONS, aPointLights[ u ]->m_vPosition, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_COLORS, aPointLights[ u ]->m_vColor, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_INTENSITIES, aPointLights[ u ]->m_fIntensity, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_FALLOFF_MIN_DISTANCES, aPointLights[ u ]->m_fFalloffMinDistance, u );
		oTechnique.SetArrayParameter( PARAM_POINT_LIGHT_FALLOFF_MAX_DISTANCES, aPointLights[ u ]->m_fFalloffMaxDistance, u );
	}

	for( uint u = 0; u < aSpotLights.Count(); ++u )
	{
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_POSITIONS, aSpotLights[ u ]->m_vPosition, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_DIRECTIONS, aSpotLights[ u ]->m_vDirection, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_COLORS, aSpotLights[ u ]->m_vColor, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_INTENSITIES, aSpotLights[ u ]->m_fIntensity, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_OUTERRANGES, glm::cos( glm::radians( aSpotLights[ u ]->m_fOuterAngle / 2.f ) ), u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_RANGES, glm::cos( glm::radians( aSpotLights[ u ]->m_fInnerAngle / 2.f ) ) - glm::cos( glm::radians( aSpotLights[ u ]->m_fOuterAngle / 2.f ) ), u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_FALLOFF_MIN_DISTANCES, aSpotLights[ u ]->m_fFalloffMinDistance, u );
		oTechnique.SetArrayParameter( PARAM_SPOT_LIGHT_FALLOFF_MAX_DISTANCES, aSpotLights[ u ]->m_fFalloffMaxDistance, u );
	}
}

template < typename Technique >
static void DrawMeshes( const Array< VisualNode* >& aVisualNodes, Technique& oTechnique )
{
	for( const VisualNode* pVisualNode : aVisualNodes )
	{
		if( oTechnique.HasArrayParameter( PARAM_BONE_MATRICES ) )
		{
			const Array< glm::mat4 >& aBoneMatrices = pVisualNode->m_aBoneMatrices;
			for( uint u = 0; u < aBoneMatrices.Count(); ++u )
				oTechnique.SetArrayParameter( PARAM_BONE_MATRICES, aBoneMatrices[ u ], u );
			for( uint u = aBoneMatrices.Count(); u < MAX_BONE_COUNT; ++u )
				oTechnique.SetArrayParameter( PARAM_BONE_MATRICES, glm::mat4( 1.f ), u );
		}

		oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() * pVisualNode->m_mMatrix );

		if( oTechnique.HasParameter( PARAM_MODEL_INVERSE_TRANSPOSE ) )
			oTechnique.SetParameter( PARAM_MODEL_INVERSE_TRANSPOSE, glm::inverseTranspose( pVisualNode->m_mMatrix ) );

		if( oTechnique.HasParameter( PARAM_MODEL ) )
			oTechnique.SetParameter( PARAM_MODEL, pVisualNode->m_mMatrix );

		const Array< Mesh >& aMeshes = pVisualNode->m_aMeshes;
		for( const Mesh& oMesh : aMeshes )
		{
			g_pMaterialManager->ApplyMaterial( oMesh.m_oMaterial, oTechnique );

			const Array< const Texture* >& aTextures = oTechnique.m_aTextures;
			for( uint u = 0; u < aTextures.Count(); ++u )
				g_pRenderer->SetTextureSlot( *aTextures[ u ], ( int )u );

			g_pRenderer->DrawMesh( oMesh );

			oTechnique.m_aTextures.Clear();
		}
	}
}

Array< VisualNode* > BuildTemporaryVisualNodesArray( ArrayView< VisualNode > aVisualNodes )
{
	Array< VisualNode* > aTemporaryVisualNodes( aVisualNodes.Count() );
	for( uint u = 0; u < aVisualNodes.Count(); ++u )
		aTemporaryVisualNodes[ u ] = &aVisualNodes[ u ];

	return aTemporaryVisualNodes;
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

GPUBlock::GPUBlock( const char* sName )
{
	glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, -1, sName );
}

GPUBlock::~GPUBlock()
{
	glPopDebugGroup();
}

Renderer* g_pRenderer = nullptr;

Renderer::Renderer()
	: m_xDefaultDiffuseMap( g_pResourceLoader->LoadTexture( "Default_diffuse.png", true ) )
	, m_xDefaultNormalMap( g_pResourceLoader->LoadTexture( "Default_normal.png" ) )
	, m_xDeferredMaps( g_pResourceLoader->LoadTechnique( "Shader/deferred_maps.tech" ) )
	, m_xDeferredCompose( g_pResourceLoader->LoadTechnique( "Shader/deferred_compose.tech" ) )
	, m_xBlend( g_pResourceLoader->LoadTechnique( "Shader/blend.tech" ) )
	, m_xPicking( g_pResourceLoader->LoadTechnique( "Shader/picking.tech" ) )
	, m_xOutline( g_pResourceLoader->LoadTechnique( "Shader/outline.tech" ) )
	, m_xGizmo( g_pResourceLoader->LoadTechnique( "Shader/gizmo.tech" ) )
	, m_eRenderingMode( RenderingMode::FORWARD )
	, m_eMSAALevel( MSAALevel::MSAA_8X )
	, m_bSRGB( true )
	, m_bUpdateRenderPipeline( false )
	, m_bDisplayDebug( false )
{
	glEnable( GL_CULL_FACE );
	
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

	m_oFramebuffer.m_uFrameBufferID = 0;

	g_pRenderer = this;
}

Renderer::~Renderer()
{
	g_pRenderer = nullptr;
}

void Renderer::Render( const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Renderer" );

	if( m_bSRGB )
		glEnable( GL_FRAMEBUFFER_SRGB );

	glClearColor( 0.f, 0.f, 0.f, 0.f );

	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	UpdateRenderPipeline( oRenderContext );

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
	bLoaded &= m_xDeferredMaps->IsLoaded() && m_xDeferredCompose->IsLoaded() && m_xBlend->IsLoaded() && m_xPicking->IsLoaded() && m_xOutline->IsLoaded() && m_xGizmo->IsLoaded();
	bLoaded &= m_oTextRenderer.OnLoading() && m_oDebugRenderer.OnLoading() && m_oBloom.OnLoading();

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
		for( uint u = 0; u < ( uint )RenderingMode::_COUNT; ++u )
		{
			RenderingMode eRenderingMode = RenderingMode( u );
			if( ImGui::Selectable( GetRenderingModeName( eRenderingMode ), eRenderingMode == m_eRenderingMode ) )
				m_eRenderingMode = eRenderingMode;
		}
		ImGui::EndCombo();
	}

	if( m_eRenderingMode == RenderingMode::FORWARD )
	{
		auto GetMSAALevelName = []( const MSAALevel eMSAALevel ) {
			switch( eMSAALevel )
			{
			case MSAALevel::MSAA_NONE:
				return "None";
			case MSAALevel::MSAA_2X:
				return "2x";
			case MSAALevel::MSAA_4X:
				return "4x";
			case MSAALevel::MSAA_8X:
				return "8x";
			case MSAALevel::_COUNT:
				return "";
			}

			return "";
		};

		if( ImGui::BeginCombo( "MSAA", GetMSAALevelName( m_eMSAALevel ) ) )
		{
			for( uint u = 0; u < ( uint )MSAALevel::_COUNT; ++u )
			{
				MSAALevel eMSAALevel = MSAALevel( u );
				if( ImGui::Selectable( GetMSAALevelName( eMSAALevel ), eMSAALevel == m_eMSAALevel ) )
				{
					m_eMSAALevel = eMSAALevel;
					m_bUpdateRenderPipeline = true;
				}
			}

			ImGui::EndCombo();
		}
	}

	ImGui::Checkbox( "SRGB", &m_bSRGB );

	ImGui::InputInt( "Bloom iterations", &m_oBloom.m_iIterations );

	ImGui::End();
}

const Texture* Renderer::GetDefaultDiffuseMap() const
{
	return &m_xDefaultDiffuseMap->GetTexture();
}

const Texture* Renderer::GetDefaultNormalMap() const
{
	return &m_xDefaultNormalMap->GetTexture();
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

void Renderer::RenderQuad()
{
	g_pRenderer->DrawMesh( m_oRenderMesh );
}

void Renderer::BlendTextures( const Texture& oTextureA, const Texture& oTextureB )
{
	Technique& oBlendTechnique = m_xBlend->GetTechnique();
	g_pRenderer->SetTechnique( oBlendTechnique );

	g_pRenderer->SetTextureSlot( oTextureA, 0 );
	oBlendTechnique.SetParameter( PARAM_TEXTURE_A, 0 );
	g_pRenderer->SetTextureSlot( oTextureB, 1 );
	oBlendTechnique.SetParameter( PARAM_TEXTURE_B, 1 );

	RenderQuad();

	g_pRenderer->ClearTextureSlot( 0 );
	g_pRenderer->ClearTextureSlot( 1 );
}

void Renderer::CopyRenderTarget( const RenderTarget& oSource, const RenderTarget& oDestination )
{
	ASSERT( oSource.m_aTextures.Count() == oDestination.m_aTextures.Count() );
	ASSERT( oSource.m_bDepthMap == oDestination.m_bDepthMap );

	glBindFramebuffer( GL_READ_FRAMEBUFFER, oSource.m_uFrameBufferID );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, oDestination.m_uFrameBufferID );

	const uint uTextureCount = oSource.m_bDepthMap ? oSource.m_aTextures.Count() - 1 : oSource.m_aTextures.Count();

	for( uint u = 0; u < uTextureCount; ++u )
	{
		ASSERT( oSource.m_aTextures[ u ].m_iWidth == oDestination.m_aTextures[ u ].m_iWidth );
		ASSERT( oSource.m_aTextures[ u ].m_iHeight == oDestination.m_aTextures[ u ].m_iHeight );
		ASSERT( oSource.m_aTextures[ u ].m_eFormat == oDestination.m_aTextures[ u ].m_eFormat );

		glReadBuffer( GL_COLOR_ATTACHMENT0 + u );
		glDrawBuffer( GL_COLOR_ATTACHMENT0 + u );

		glBlitFramebuffer( 0, 0, oSource.m_iWidth, oSource.m_iHeight, 0, 0, oDestination.m_iWidth, oDestination.m_iHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	}

	if( oSource.m_bDepthMap )
	{
		ASSERT( oSource.m_aTextures.Back().m_eFormat == oDestination.m_aTextures.Back().m_eFormat );

		glReadBuffer( GL_DEPTH_ATTACHMENT );
		glDrawBuffer( GL_DEPTH_ATTACHMENT );

		glBlitFramebuffer( 0, 0, oSource.m_iWidth, oSource.m_iHeight, 0, 0, oDestination.m_iWidth, oDestination.m_iHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST );
	}

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Renderer::CopyRenderTargetColor( const RenderTarget& oSource, const uint uSourceColorIndex, const RenderTarget& oDestination, const uint uDestinationColorIndex )
{
	glBindFramebuffer( GL_READ_FRAMEBUFFER, oSource.m_uFrameBufferID );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, oDestination.m_uFrameBufferID );

	glReadBuffer( GL_COLOR_ATTACHMENT0 + uSourceColorIndex );
	glDrawBuffer( GL_COLOR_ATTACHMENT0 + uDestinationColorIndex );

	glBlitFramebuffer( 0, 0, oSource.m_iWidth, oSource.m_iHeight, 0, 0, oDestination.m_iWidth, oDestination.m_iHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Renderer::CopyRenderTargetDepth( const RenderTarget& oSource, const RenderTarget& oDestination )
{
	glBindFramebuffer( GL_READ_FRAMEBUFFER, oSource.m_uFrameBufferID );
	glBindFramebuffer( GL_DRAW_FRAMEBUFFER, oDestination.m_uFrameBufferID );

	glReadBuffer( GL_DEPTH_ATTACHMENT );
	glDrawBuffer( GL_DEPTH_ATTACHMENT );

	glBlitFramebuffer( 0, 0, oSource.m_iWidth, oSource.m_iHeight, 0, 0, oDestination.m_iWidth, oDestination.m_iHeight, GL_DEPTH_BUFFER_BIT, GL_NEAREST );

	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Renderer::RenderForward( const RenderContext& oRenderContext )
{
	GPUBlock oGPUBlock( "Forward" );

	SetRenderTarget( m_oForwardMSAATarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	for( uint u = 0; u < m_oVisualStructure.m_aTechniques.Count(); ++u )
	{
		Technique& oTechnique = *m_oVisualStructure.m_aTechniques[ u ];
		SetTechnique( oTechnique );

		if( oTechnique.HasParameter( PARAM_DIRECTIONAL_LIGHT_COUNT ) )
			SetupLighting( oTechnique, m_oVisualStructure.m_aDirectionalLights, m_oVisualStructure.m_aPointLights, m_oVisualStructure.m_aSpotLights );

		if( oTechnique.HasParameter( PARAM_VIEW_POSITION ) )
			oTechnique.SetParameter( PARAM_VIEW_POSITION, m_oCamera.m_vPosition );

		DrawMeshes( m_oVisualStructure.m_aVisualNodes[ u ], oTechnique );
	}

	for( uint u = 0; u < m_oVisualStructure.m_aTemporaryTechniques.Count(); ++u )
	{
		Technique& oTechnique = *m_oVisualStructure.m_aTemporaryTechniques[ u ];
		SetTechnique( oTechnique );

		if( oTechnique.HasParameter( PARAM_DIRECTIONAL_LIGHT_COUNT ) )
			SetupLighting( oTechnique, m_oVisualStructure.m_aDirectionalLights, m_oVisualStructure.m_aPointLights, m_oVisualStructure.m_aSpotLights );

		if( oTechnique.HasParameter( PARAM_VIEW_POSITION ) )
			oTechnique.SetParameter( PARAM_VIEW_POSITION, m_oCamera.m_vPosition );

		const Array< VisualNode* > aTemporaryVisualNodes = BuildTemporaryVisualNodesArray( m_oVisualStructure.m_aTemporaryVisualNodes[ u ] );
		DrawMeshes( aTemporaryVisualNodes, oTechnique );
	}

	glDisable( GL_DEPTH_TEST );

	CopyRenderTarget( m_oForwardMSAATarget, m_oForwardTarget );

	m_oBloom.Render( m_oForwardTarget, m_oPostProcessTarget, oRenderContext );

	CopyRenderTargetColor( m_oPostProcessTarget, 0, m_oFramebuffer, 0 );
	CopyRenderTargetDepth( m_oForwardTarget, m_oFramebuffer );

	ClearTextureSlot( 0 );
	ClearTechnique();
}

void Renderer::RenderDeferred( const RenderContext& oRenderContext )
{
	SetRenderTarget( m_oDeferredTarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	Technique& oMapsTechnique = m_xDeferredMaps->GetTechnique();
	SetTechnique( oMapsTechnique );

	for( const Array< VisualNode* >& aVisualNodes : m_oVisualStructure.m_aVisualNodes )
		DrawMeshes( aVisualNodes, oMapsTechnique );

	for( Array< VisualNode >& aVisualNodes : m_oVisualStructure.m_aTemporaryVisualNodes )
	{
		const Array< VisualNode* > aTemporaryVisualNodes = BuildTemporaryVisualNodesArray( aVisualNodes );
		DrawMeshes( aTemporaryVisualNodes, oMapsTechnique );
	}

	ClearRenderTarget();

	glClear( GL_COLOR_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	Technique& oComposeTechnique = m_xDeferredCompose->GetTechnique();
	SetTechnique( oComposeTechnique );

	SetTextureSlot( m_oDeferredTarget.GetColorMap( 0 ), 0 );
	oComposeTechnique.SetParameter( PARAM_COLOR_MAP, 0 );
	SetTextureSlot( m_oDeferredTarget.GetColorMap( 1 ), 1 );
	oComposeTechnique.SetParameter( PARAM_NORMAL_MAP, 1 );
	SetTextureSlot( m_oDeferredTarget.GetDepthMap(), 2 );
	oComposeTechnique.SetParameter( PARAM_DEPTH_MAP, 2 );
	oComposeTechnique.SetParameter( PARAM_INVERSE_VIEW_PROJECTION, m_oCamera.GetInverseViewProjectionMatrix() );

	SetupLighting( oComposeTechnique, m_oVisualStructure.m_aDirectionalLights, m_oVisualStructure.m_aPointLights, m_oVisualStructure.m_aSpotLights );

	RenderQuad();
	CopyRenderTargetDepth( m_oDeferredTarget, m_oFramebuffer );

	ClearTextureSlot( 0 );
	ClearTechnique();
}

uint64 Renderer::RenderPicking( const RenderContext& oRenderContext, const int iCursorX, const int iCursorY, const bool bAllowGizmos )
{
	GPUBlock oGPUBlock( "Picking" );

	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	if( oRenderRect.m_uWidth != m_oPickingTarget.GetWidth() || oRenderRect.m_uHeight != m_oPickingTarget.GetHeight() )
	{
		m_oPickingTarget.Destroy();

		m_oPickingTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight, TextureFormat::ID ).Depth() );

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

	for( const Array< VisualNode* >& aVisualNodes : g_pRenderer->m_oVisualStructure.m_aVisualNodes )
	{
		for( const VisualNode* pVisualNode : aVisualNodes )
		{
			oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * pVisualNode->m_mMatrix );
			oTechnique.SetParameter( PARAM_COLOR_ID, BuildColorID( pVisualNode->m_uEntityID ) );

			const Array< Mesh >& aMeshes = pVisualNode->m_aMeshes;
			for( const Mesh& oMesh : aMeshes )
				DrawMesh( oMesh );
		}
	}

	for( const Array< VisualNode >& aVisualNodes : g_pRenderer->m_oVisualStructure.m_aTemporaryVisualNodes )
	{
		for( const VisualNode& oVisualNode : aVisualNodes )
		{
			oTechnique.SetParameter( PARAM_MODEL_VIEW_PROJECTION, m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
			oTechnique.SetParameter( PARAM_COLOR_ID, BuildColorID( oVisualNode.m_uEntityID ) );

			const Array< Mesh >& aMeshes = oVisualNode.m_aMeshes;
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
	GPUBlock oGPUBlock( "Outline" );

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

	const Array< Mesh >& aMeshes = oVisualNode.m_aMeshes;
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
	GPUBlock oGPUBlock( "Gizmos" );

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

void Renderer::UpdateRenderPipeline( const RenderContext& oRenderContext )
{
	const RenderRect& oRenderRect = oRenderContext.m_oRenderRect;

	m_bUpdateRenderPipeline |= ( oRenderRect.m_uWidth != m_oFramebuffer.GetWidth() );
	m_bUpdateRenderPipeline |= ( oRenderRect.m_uHeight != m_oFramebuffer.GetHeight() );

	if( m_bUpdateRenderPipeline )
	{
		m_oFramebuffer.m_iWidth = oRenderRect.m_uWidth;
		m_oFramebuffer.m_iHeight = oRenderRect.m_uHeight;

		auto GetMSAALEvelSamples = []( const MSAALevel eMSAALevel ) {
			switch( eMSAALevel )
			{
			case Renderer::MSAALevel::MSAA_NONE:
				return 1;
			case Renderer::MSAALevel::MSAA_2X:
				return 2;
			case Renderer::MSAALevel::MSAA_4X:
				return 4;
			case Renderer::MSAALevel::MSAA_8X:
				return 8;
			default:
				return 1;
			}
		};

		m_oForwardMSAATarget.Destroy();
		m_oForwardMSAATarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight ).AddColor( TextureFormat::RGB16 ).AddColor( TextureFormat::RGB16 ).Depth().Multisample( GetMSAALEvelSamples( m_eMSAALevel ) ) );

		m_oForwardTarget.Destroy();
		m_oForwardTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight ).AddColor( TextureFormat::RGB16 ).AddColor( TextureFormat::RGB16 ).Depth() );

		m_oPostProcessTarget.Destroy();
		m_oPostProcessTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight ).AddColor( TextureFormat::RGB16 ).AddColor( TextureFormat::RGB16 ).Depth() );

		m_oDeferredTarget.Destroy();
		m_oDeferredTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight ).AddColor( TextureFormat::RGB ).AddColor( TextureFormat::NORMAL ).Depth() );

		m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );

		m_bUpdateRenderPipeline = false;
	}
}
