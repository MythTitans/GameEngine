#include "Renderer.h"

#define NOMINMAX
#include <Windows.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtx/transform.hpp>

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "DebugDisplay.h"
#include "Game/Entity.h"
#include "Game/InputHandler.h"
#include "Game/ResourceLoader.h"
#include "Editor/Gizmo.h"
#include "ImGui/imgui.h"
#include "MaterialManager.h"
#include "Math/GLMHelpers.h"

static const std::string PARAM_MODEL_VIEW_PROJECTION( "modelViewProjection" );
static const std::string PARAM_MODEL_INVERSE_TRANSPOSE( "modelInverseTranspose" );
static const std::string PARAM_MODEL( "model" );
static const std::string PARAM_VIEW_POSITION( "viewPosition" );

static const std::string PARAM_DIFFUSE_MAP( "diffuseMap" );
static const std::string PARAM_NORMAL_MAP( "normalMap" );

static const std::string PARAM_USE_SKINNING( "useSkinning" );
static const std::string PARAM_SKINNING_OFFSET( "skinningOffset" );

static GPULightingDataBlock SetupLighting( const Array< DirectionalLightNode* >& aDirectionalLights, const Array< PointLightNode* >& aPointLights, const Array< SpotLightNode* >& aSpotLights )
{
	GPULightingDataBlock oLightingData;

	for( uint u = 0; u < aDirectionalLights.Count(); ++u )
	{
		oLightingData.m_aDirectionalLights[ u ].m_vDirection = aDirectionalLights[ u ]->m_vDirection;
		oLightingData.m_aDirectionalLights[ u ].m_vColor = aDirectionalLights[ u ]->m_vColor;
		oLightingData.m_aDirectionalLights[ u ].m_fIntensity = aDirectionalLights[ u ]->m_fIntensity;
	}
	oLightingData.m_uDirectionalLightCount = aDirectionalLights.Count();

	for( uint u = 0; u < aPointLights.Count(); ++u )
	{
		oLightingData.m_aPointLights[ u ].m_vPosition = aPointLights[ u ]->m_vPosition;
		oLightingData.m_aPointLights[ u ].m_vColor = aPointLights[ u ]->m_vColor;
		oLightingData.m_aPointLights[ u ].m_fIntensity = aPointLights[ u ]->m_fIntensity;
		oLightingData.m_aPointLights[ u ].m_fFalloffMinDistance = aPointLights[ u ]->m_fFalloffMinDistance;
		oLightingData.m_aPointLights[ u ].m_fFalloffMaxDistance = aPointLights[ u ]->m_fFalloffMaxDistance;
	}
	oLightingData.m_uPointLightCount = aPointLights.Count();

	for( uint u = 0; u < aSpotLights.Count(); ++u )
	{
		oLightingData.m_aSpotLights[ u ].m_vDirection = aSpotLights[ u ]->m_vDirection;
		oLightingData.m_aSpotLights[ u ].m_vPosition = aSpotLights[ u ]->m_vPosition;
		oLightingData.m_aSpotLights[ u ].m_vColor = aSpotLights[ u ]->m_vColor;
		oLightingData.m_aSpotLights[ u ].m_fIntensity = aSpotLights[ u ]->m_fIntensity;
		oLightingData.m_aSpotLights[ u ].m_fInnerRange = glm::cos( glm::radians( aSpotLights[ u ]->m_fInnerAngle / 2.f ) ) - glm::cos( glm::radians( aSpotLights[ u ]->m_fOuterAngle / 2.f ) );
		oLightingData.m_aSpotLights[ u ].m_fOuterRange = glm::cos( glm::radians( aSpotLights[ u ]->m_fOuterAngle / 2.f ) );
		oLightingData.m_aSpotLights[ u ].m_fFalloffMinDistance = aSpotLights[ u ]->m_fFalloffMinDistance;
		oLightingData.m_aSpotLights[ u ].m_fFalloffMaxDistance = aSpotLights[ u ]->m_fFalloffMaxDistance;
	}
	oLightingData.m_uSpotLightCount = aSpotLights.Count();

	return oLightingData;
}

template < typename Technique >
static void DrawMeshes( const Array< VisualNode* >& aVisualNodes, Technique& oTechnique )
{
	TechniqueParameter oParamUseSkinning = oTechnique.GetParameter( PARAM_USE_SKINNING );
	TechniqueParameter oParamSkinningOffset = oTechnique.GetParameter( PARAM_SKINNING_OFFSET );
	TechniqueParameter oParamModelViewProjection = oTechnique.GetParameter( PARAM_MODEL_VIEW_PROJECTION );
	TechniqueParameter oParamModelInverseTranspose = oTechnique.GetParameter( PARAM_MODEL_INVERSE_TRANSPOSE );
	TechniqueParameter oParamModel = oTechnique.GetParameter( PARAM_MODEL );

	const glm::mat4 mViewProjectionMatrix = g_pRenderer->m_oCamera.GetViewProjectionMatrix();

	const Frustum oFrustum = Frustum::FromViewProjection( mViewProjectionMatrix );

	Array< VisualNode* > aVisibleNodes;

	if( g_pRenderer->m_bEnableFrustumCulling )
	{
		ProfilerBlock oBlock( "FrustumCulling" );

		aVisibleNodes.Reserve( aVisualNodes.Count() );
		for( VisualNode* pVisualNode : aVisualNodes )
		{
			if( oFrustum.IsVisible( pVisualNode->m_oAABB ) )
				aVisibleNodes.PushBack( pVisualNode );
		}
	}
	else
	{
		aVisibleNodes = aVisualNodes;
	}

	{
		ProfilerBlock oBlock( "Draw" );

		for( const VisualNode* pVisualNode : aVisibleNodes )
		{
			if( g_pRenderer->m_bEnableFrustumCulling )
			{
				if( oFrustum.IsVisible( pVisualNode->m_oAABB ) == false )
					continue;
			}

			if( oParamSkinningOffset.IsValid() )
				oParamSkinningOffset.SetValue( pVisualNode->m_uBoneStorageIndex );

			if( oParamUseSkinning.IsValid() )
				oParamUseSkinning.SetValue( pVisualNode->m_uBoneCount > 0 );

			const glm::mat4 mMatrix = pVisualNode->m_mMatrix;

			oParamModelViewProjection.SetValue( mViewProjectionMatrix * mMatrix );

			if( oParamModelInverseTranspose.IsValid() )
				oParamModelInverseTranspose.SetValue( pVisualNode->m_InverseTransposeMatrix );

			if( oParamModel.IsValid() )
				oParamModel.SetValue( mMatrix );

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
	if( m_oRenderRect.m_uHeight == 0 )
		return 1.f;

	return ( float )m_oRenderRect.m_uWidth / ( float )m_oRenderRect.m_uHeight;
}

GPUMarker::GPUMarker( const char* sName )
{
	glPushDebugGroup( GL_DEBUG_SOURCE_APPLICATION, 0, -1, sName );
}

GPUMarker::~GPUMarker()
{
	glPopDebugGroup();
}

RendererStatistics::RendererStatistics()
	: m_uTriangleCount( 0 )
	, m_uDrawCallCount( 0 )
{
}

Renderer* g_pRenderer = nullptr;

Renderer::Renderer()
	: m_xDefaultDiffuseMap( g_pResourceLoader->LoadTexture( "Default_diffuse.png", true ) )
	, m_xDefaultNormalMap( g_pResourceLoader->LoadTexture( "Default_normal.png" ) )
	, m_xDeferredMaps( g_pResourceLoader->LoadTechnique( "Shader/deferred_maps.tech" ) )
	, m_xDeferredCompose( g_pResourceLoader->LoadTechnique( "Shader/deferred_compose.tech" ) )
	, m_xBlend( g_pResourceLoader->LoadTechnique( "Shader/blend.tech" ) )
	, m_xOutline( g_pResourceLoader->LoadTechnique( "Shader/outline.tech" ) )
	, m_xGizmo( g_pResourceLoader->LoadTechnique( "Shader/gizmo.tech" ) )
	, m_eRenderingMode( RenderingMode::FORWARD )
	, m_eMSAALevel( MSAALevel::MSAA_8X )
	, m_bSRGB( true )
	, m_bEnableFrustumCulling( true )
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

	m_oSkinningBuffer.Create( ShaderBufferDesc().Dynamic() );
	m_oMaterialBuffer.Create( ShaderBufferDesc().Dynamic() );
	m_oLightingBuffer.Create( ShaderBufferDesc().Dynamic() );

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
	GPUProfilerBlock oGPUBlock( "Renderer" );

	glClearColor( 0.f, 0.f, 0.f, 0.f );

	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	UpdateRenderPipeline( oRenderContext );

	GPUMaterialDataBlock oMaterialData;
	g_pMaterialManager->ExportMaterialsToGPU< LitMaterialData >( oMaterialData.m_aLitMaterialData );
	g_pMaterialManager->ExportMaterialsToGPU< UnlitMaterialData >( oMaterialData.m_aUnlitMaterialData );

	m_oSkinningBuffer.Update( m_oGPUSkinningStorage.GetSkinningData() );
	SetShaderBufferSlot( m_oSkinningBuffer, 0 );

	m_oMaterialBuffer.Update( oMaterialData );
	SetShaderBufferSlot( m_oMaterialBuffer, 1 );

	GPULightingDataBlock oLightingData = SetupLighting( m_oVisualStructure.m_aDirectionalLights, m_oVisualStructure.m_aPointLights, m_oVisualStructure.m_aSpotLights );
	m_oLightingBuffer.Update( oLightingData );
	SetShaderBufferSlot( m_oLightingBuffer, 2 );

	switch( m_eRenderingMode )
	{
	case RenderingMode::FORWARD:
		RenderForward( oRenderContext );
		break;
	case RenderingMode::DEFERRED:
		RenderDeferred( oRenderContext );
		break;
	}

	auto FormatNumber = []( const uint64 uNumber )
	{
		std::string sResult;


		uint64 uRemaining = uNumber;
		while( uRemaining != 0 )
		{
			if( sResult.empty() == false )
				sResult = "'" + sResult;

			const uint64 uPart = uRemaining % 1000;
			uRemaining = ( uRemaining - uPart ) / 1000;

			if( uRemaining != 0 )
				sResult = std::format( "{:03d}", uPart ) + sResult;
			else
				sResult = std::format( "{}", uPart ) + sResult;
		}

		return sResult;
	};

	const RendererStatistics& oRendererStatistics = g_pRenderer->GetStatistics();
	g_pDebugDisplay->DisplayText( std::format( "Renderer statistics : {} triangles, {} draw calls", FormatNumber( oRendererStatistics.m_uTriangleCount ), FormatNumber( oRendererStatistics.m_uDrawCallCount ) ) );
}

void Renderer::Clear()
{
	m_oStatistics = RendererStatistics();

	m_oVisualStructure.Clear();
	m_oGPUSkinningStorage.Reset();
}

bool Renderer::OnLoading()
{
	bool bLoaded = m_xDefaultDiffuseMap->IsLoaded() && m_xDefaultNormalMap->IsLoaded();
	bLoaded &= m_xDeferredMaps->IsLoaded() && m_xDeferredCompose->IsLoaded() && m_xBlend->IsLoaded() && m_xOutline->IsLoaded() && m_xGizmo->IsLoaded();
	bLoaded &= m_oTextRenderer.OnLoading() && m_oDebugRenderer.OnLoading() && m_oSkybox.OnLoading() && m_oTerrain.OnLoading() && m_oRoad.OnLoading() && m_oBloom.OnLoading();

	return bLoaded;
}

void Renderer::OnLoaded()
{
	m_oTextRenderer.OnLoaded();
	m_oDebugRenderer.OnLoaded();

	m_oDeferredComposeSheet.Init( m_xDeferredCompose->GetTechnique() );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::VIEW_POSITION, "viewPosition" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::INVERSE_VIEW_PROJECTION, "inverseViewProjection" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::DIFFUSE, "diffuseMap" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::NORMAL, "normalMap" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::SPECULAR, "specularMap" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::EMISSIVE, "emissiveMap" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::MATERIAL_ID, "materialMap" );
	m_oDeferredComposeSheet.BindParameter( DeferredComposeParam::DEPTH, "depthMap" );

	m_oBlendSheet.Init( m_xBlend->GetTechnique() );
	m_oBlendSheet.BindParameter( BlendParam::TEXTURE_A, "textureA" );
	m_oBlendSheet.BindParameter( BlendParam::TEXTURE_B, "textureB" );

	m_oOutlineSheet.Init( m_xOutline->GetTechnique() );
	m_oOutlineSheet.BindParameter( OutlineParam::MODEL, "model" );
	m_oOutlineSheet.BindParameter( OutlineParam::MODEL_VIEW_PROJECTION, "modelViewProjection" );
	m_oOutlineSheet.BindParameter( OutlineParam::DISPLACEMENT, "displacement" );
	m_oOutlineSheet.BindParameter( OutlineParam::CAMERA_POSITION, "cameraPosition" );
	m_oOutlineSheet.BindParameter( OutlineParam::COLOR, "color" );
	m_oOutlineSheet.BindParameter( OutlineParam::SKINNING_OFFSET, "skinningOffset" );

	m_oGizmoSheet.Init( m_xGizmo->GetTechnique() );
	m_oGizmoSheet.BindParameter( GizmoParam::MODEL_VIEW_PROJECTION, "modelViewProjection" );
	m_oGizmoSheet.BindParameter( GizmoParam::COLOR, "color" );

	m_oSkybox.OnLoaded();
	m_oBloom.OnLoaded();
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
	ImGui::Checkbox( "Enable frustum culling", &m_bEnableFrustumCulling );

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
	glBindTexture( GL_TEXTURE_2D, oTexture.GetID() );
}

void Renderer::ClearTextureSlot( const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Renderer::SetCubeMapSlot( const CubeMap& oCubeMap, const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_CUBE_MAP, oCubeMap.m_uTextureID );
}

void Renderer::ClearCubeMapSlot( const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_CUBE_MAP, 0 );
}

void Renderer::SetShaderBufferSlot( const ShaderBufferBase& oBuffer, const uint uBinding )
{
	glBindBufferBase( GL_UNIFORM_BUFFER, uBinding, oBuffer.GetID() );
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
	m_oStatistics.m_uTriangleCount += oMesh.m_iIndexCount / 3;
	m_oStatistics.m_uDrawCallCount += 1;

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
	m_oBlendSheet.GetParameter( BlendParam::TEXTURE_A ).SetValue( 0 );
	g_pRenderer->SetTextureSlot( oTextureB, 1 );
	m_oBlendSheet.GetParameter( BlendParam::TEXTURE_B ).SetValue( 1 );

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
		ASSERT( oSource.m_aTextures[ u ].GetWidth() == oDestination.m_aTextures[ u ].GetWidth() );
		ASSERT( oSource.m_aTextures[ u ].GetHeight() == oDestination.m_aTextures[ u ].GetHeight() );
		ASSERT( oSource.m_aTextures[ u ].GetFormat() == oDestination.m_aTextures[u].GetFormat());

		glReadBuffer( GL_COLOR_ATTACHMENT0 + u );
		glDrawBuffer( GL_COLOR_ATTACHMENT0 + u );

		glBlitFramebuffer( 0, 0, oSource.m_iWidth, oSource.m_iHeight, 0, 0, oDestination.m_iWidth, oDestination.m_iHeight, GL_COLOR_BUFFER_BIT, GL_NEAREST );
	}

	if( oSource.m_bDepthMap )
	{
		ASSERT( oSource.m_aTextures.Back().GetFormat() == oDestination.m_aTextures.Back().GetFormat() );

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

const RendererStatistics& Renderer::GetStatistics() const
{
	return m_oStatistics;
}

void Renderer::RenderForward( const RenderContext& oRenderContext )
{
	GPUMarker oGPUMarker( "Forward" );

	SetRenderTarget( m_oForwardMSAATarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	{
		GPUProfilerBlock oBlock( "Sky" );

		const SkyNode* pActiveSky = g_pRenderer->m_oVisualStructure.GetActiveSky();
		if( pActiveSky != nullptr )
			m_oSkybox.Render( pActiveSky, oRenderContext );
	}

	{
		GPUProfilerBlock oBlock( "Terrain" );

		const TerrainNode* pTerrain = g_pRenderer->m_oVisualStructure.m_pTerrain;
		if( pTerrain != nullptr )
			m_oTerrain.Render( pTerrain, oRenderContext );
	}

	{
		GPUProfilerBlock oBlock( "Roads" );

		const Array< RoadNode* >& aRoads = g_pRenderer->m_oVisualStructure.m_aRoads;
		if( aRoads.Empty() == false )
			m_oRoad.Render( aRoads, oRenderContext );
	}

	{
		GPUProfilerBlock oBlock( "Meshes" );

		for( uint u = 0; u < m_oVisualStructure.m_aTechniques.Count(); ++u )
		{
			Technique& oTechnique = *m_oVisualStructure.m_aTechniques[ u ];
			SetTechnique( oTechnique );

			g_pMaterialManager->PrepareMaterials( oTechnique );

			TechniqueParameter oParamViewPosition = oTechnique.GetParameter( PARAM_VIEW_POSITION );

			if( oParamViewPosition.IsValid() )
				oParamViewPosition.SetValue( m_oCamera.m_vPosition );

			DrawMeshes( m_oVisualStructure.m_aVisuals[ u ], oTechnique );
		}
	}

	{
		GPUProfilerBlock oBlock( "TemporaryMeshes" );

		for( uint u = 0; u < m_oVisualStructure.m_aTemporaryTechniques.Count(); ++u )
		{
			Technique& oTechnique = *m_oVisualStructure.m_aTemporaryTechniques[ u ];
			SetTechnique( oTechnique );

			g_pMaterialManager->PrepareMaterials( oTechnique );

			TechniqueParameter oParamViewPosition = oTechnique.GetParameter( PARAM_VIEW_POSITION );

			if( oParamViewPosition.IsValid() )
				oParamViewPosition.SetValue( m_oCamera.m_vPosition );

			const Array< VisualNode* > aTemporaryVisualNodes = BuildTemporaryVisualNodesArray( m_oVisualStructure.m_aTemporaryVisuals[ u ] );
			DrawMeshes( aTemporaryVisualNodes, oTechnique );
		}
	}

	glDisable( GL_DEPTH_TEST );

	CopyRenderTarget( m_oForwardMSAATarget, m_oForwardTarget );

	m_oBloom.Render( m_oForwardTarget, m_oPostProcessTarget, oRenderContext );

	if( m_bSRGB )
		glEnable( GL_FRAMEBUFFER_SRGB );
	CopyRenderTargetColor( m_oPostProcessTarget, 0, m_oFramebuffer, 0 );
	glDisable( GL_FRAMEBUFFER_SRGB );
	CopyRenderTargetDepth( m_oForwardTarget, m_oFramebuffer );

	ClearTextureSlot( 0 );
	ClearTechnique();
}

void Renderer::RenderDeferred( const RenderContext& oRenderContext )
{
	SetRenderTarget( m_oDeferredMapsTarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	Technique& oMapsTechnique = m_xDeferredMaps->GetTechnique();
	SetTechnique( oMapsTechnique );

	g_pMaterialManager->PrepareMaterials( oMapsTechnique );

	{
		GPUProfilerBlock oGPUBlock( "Meshes" );

		for( const Array< VisualNode* >& aVisualNodes : m_oVisualStructure.m_aVisuals )
			DrawMeshes( aVisualNodes, oMapsTechnique );
	}

	{
		GPUProfilerBlock oGPUBlock( "TemporaryMeshes" );

		for( Array< VisualNode >& aVisualNodes : m_oVisualStructure.m_aTemporaryVisuals )
		{
			const Array< VisualNode* > aTemporaryVisualNodes = BuildTemporaryVisualNodesArray( aVisualNodes );
			DrawMeshes( aTemporaryVisualNodes, oMapsTechnique );
		}
	}

	SetRenderTarget( m_oDeferredComposeTarget );

	glClear( GL_COLOR_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	Technique& oComposeTechnique = m_xDeferredCompose->GetTechnique();
	SetTechnique( oComposeTechnique );

	SetTextureSlot( m_oDeferredMapsTarget.GetColorMap( 0 ), 0 );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::DIFFUSE ).SetValue( 0 );
	SetTextureSlot( m_oDeferredMapsTarget.GetColorMap( 1 ), 1 );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::NORMAL ).SetValue( 1 );
	SetTextureSlot( m_oDeferredMapsTarget.GetColorMap( 2 ), 2 );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::SPECULAR ).SetValue( 2 );
	SetTextureSlot( m_oDeferredMapsTarget.GetColorMap( 3 ), 3 );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::EMISSIVE ).SetValue( 3 );
	SetTextureSlot( m_oDeferredMapsTarget.GetColorMap( 4 ), 4 );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::MATERIAL_ID ).SetValue( 4 );
	SetTextureSlot( m_oDeferredMapsTarget.GetDepthMap(), 5 );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::DEPTH ).SetValue( 5 );

	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::VIEW_POSITION ).SetValue( m_oCamera.GetPosition() );
	m_oDeferredComposeSheet.GetParameter( DeferredComposeParam::INVERSE_VIEW_PROJECTION ).SetValue( m_oCamera.GetInverseViewProjectionMatrix() );

	{
		GPUProfilerBlock oGPUBlock( "Lighting" );

		RenderQuad();
	}

	m_oBloom.Render( m_oDeferredComposeTarget, m_oPostProcessTarget, oRenderContext );

	if( m_bSRGB )
		glEnable( GL_FRAMEBUFFER_SRGB );
	CopyRenderTargetColor( m_oPostProcessTarget, 0, m_oFramebuffer, 0 );
	glDisable( GL_FRAMEBUFFER_SRGB );
	CopyRenderTargetDepth( m_oDeferredMapsTarget, m_oFramebuffer );

	ClearTextureSlot( 0 );
	ClearTechnique();
}

#ifdef EDITOR
void Renderer::RenderOutline( const RenderContext& oRenderContext, const VisualNode& oVisualNode )
{
	GPUMarker oGPUMarker( "Outline" );
	GPUProfilerBlock oGPUBlock( "Outline" );

	glDisable( GL_CULL_FACE );
	glEnable( GL_DEPTH_TEST );
	glEnable( GL_STENCIL_TEST );

	glClear( GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT );

	glColorMask( GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE );

	glStencilFunc( GL_ALWAYS, 1, 0xFF );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	Technique& oTechnique = m_xOutline->GetTechnique();
	SetTechnique( oTechnique );

	m_oOutlineSheet.GetParameter( OutlineParam::MODEL ).SetValue( oVisualNode.m_mMatrix );
	m_oOutlineSheet.GetParameter( OutlineParam::MODEL_VIEW_PROJECTION ).SetValue( m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
	m_oOutlineSheet.GetParameter( OutlineParam::DISPLACEMENT ).SetValue( 0.f );
	m_oOutlineSheet.GetParameter( OutlineParam::COLOR ).SetValue( glm::vec3( 1.f, 0.8f, 0.f ) );
	m_oOutlineSheet.GetParameter( OutlineParam::SKINNING_OFFSET ).SetValue( oVisualNode.m_uBoneStorageIndex );

	const Array< Mesh >& aMeshes = oVisualNode.m_aMeshes;
	for( const Mesh& oMesh : aMeshes )
		DrawMesh( oMesh );

	glDisable( GL_DEPTH_TEST );

	glColorMask( GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE );

	glStencilFunc( GL_NOTEQUAL, 1, 0xFF );
	glStencilOp( GL_KEEP, GL_KEEP, GL_REPLACE );

	m_oOutlineSheet.GetParameter( OutlineParam::MODEL_VIEW_PROJECTION ).SetValue( m_oCamera.GetViewProjectionMatrix() * oVisualNode.m_mMatrix );
	m_oOutlineSheet.GetParameter( OutlineParam::CAMERA_POSITION ).SetValue( glm::vec3( m_oCamera.GetPosition() ) );
	m_oOutlineSheet.GetParameter( OutlineParam::DISPLACEMENT ).SetValue( 0.004f );

	for( const Mesh& oMesh : aMeshes )
		DrawMesh( oMesh );

	ClearTechnique();

	glDisable( GL_STENCIL_TEST );
	glEnable( GL_CULL_FACE );
}

void Renderer::RenderGizmos( const RenderContext& oRenderContext )
{
	GPUMarker oGPUMarker( "Gizmos" );
	GPUProfilerBlock oGPUBlock( "Gizmos" );

	glEnable( GL_DEPTH_TEST );

	glClear( GL_DEPTH_BUFFER_BIT );

	Technique& oTechnique = m_xGizmo->GetTechnique();
	SetTechnique( oTechnique );

	Array< GizmoComponent* > aGizmoComponents = g_pComponentManager->GetComponents< GizmoComponent >();
	for( const GizmoComponent* pGizmoComponent : aGizmoComponents )
	{
		m_oGizmoSheet.GetParameter( GizmoParam::MODEL_VIEW_PROJECTION ).SetValue( m_oCamera.GetViewProjectionMatrix() * ToMat4( pGizmoComponent->GetWorldMatrix() ) );
		m_oGizmoSheet.GetParameter( GizmoParam::COLOR ).SetValue( pGizmoComponent->GetColor() );

		m_oGizmoRenderer.RenderGizmo( pGizmoComponent->GetType(), pGizmoComponent->GetAxis(), oRenderContext );
	}

	ClearTechnique();
}
#endif

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
		m_oForwardMSAATarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight )
									 .AddColor( TextureFormat::RGB16 )
									 .AddColor( TextureFormat::RGB16 )
									 .Depth()
									 .Multisample( GetMSAALEvelSamples( m_eMSAALevel ) ) );

		m_oForwardTarget.Destroy();
		m_oForwardTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight )
								 .AddColor( TextureFormat::RGB16 )
								 .AddColor( TextureFormat::RGB16 )
								 .Depth() );

		m_oPostProcessTarget.Destroy();
		m_oPostProcessTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight )
									 .AddColor( TextureFormat::RGB16 )
									 .AddColor( TextureFormat::RGB16 )
									 .Depth() );

		m_oDeferredMapsTarget.Destroy();
		m_oDeferredMapsTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight )
									  .AddColor( TextureFormat::RGB )
									  .AddColor( TextureFormat::NORMAL )
									  .AddColor( TextureFormat::RGB )
									  .AddColor( TextureFormat::RGB )
									  .AddColor( TextureFormat::ID8 ) // TODO #eric switch to 16 bits at some point
									  .Depth() );

		m_oDeferredComposeTarget.Destroy();
		m_oDeferredComposeTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight )
										 .AddColor( TextureFormat::RGB16 )
										 .AddColor( TextureFormat::RGB16 ) );

		m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );

		m_bUpdateRenderPipeline = false;
	}
}
