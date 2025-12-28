#include "PickingTool.h"

#ifdef EDITOR

#include "Game/Entity.h"
#include "Game/ResourceLoader.h"
#include "Graphics/Renderer.h"
#include "Math/GLMHelpers.h"

PickingTool::PickingTool()
	: m_xPicking( g_pResourceLoader->LoadTechnique( "Shader/picking.tech" ) )
{
}

bool PickingTool::OnLoading()
{
	return m_xPicking->IsLoading() == false;
}

void PickingTool::OnLoaded()
{
	m_oPickingSheet.Init( m_xPicking->GetTechnique() );
	m_oPickingSheet.BindParameter( PickingParam::USE_SKINNING, "useSkinning" );
	m_oPickingSheet.BindArrayParameter( PickingParam::BONE_MATRICES, "boneMatrices" );
	m_oPickingSheet.BindParameter( PickingParam::MODEL_VIEW_PROJECTION, "modelViewProjection" );
	m_oPickingSheet.BindParameter( PickingParam::COLOR_ID, "colorID" );
}

uint64 PickingTool::Pick( const RenderContext& oRenderContext, const int iCursorX, const int iCursorY, const bool bAllowGizmos )
{
	GPUBlock oGPUBlock( "Picking" );

	const RenderRect& oRenderRect = oRenderContext.GetRenderRect();
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );

	if( oRenderRect.m_uWidth != m_oPickingTarget.GetWidth() || oRenderRect.m_uHeight != m_oPickingTarget.GetHeight() )
	{
		m_oPickingTarget.Destroy();

		m_oPickingTarget.Create( RenderTargetDesc( oRenderRect.m_uWidth, oRenderRect.m_uHeight, TextureFormat::ID ).Depth() );

		g_pRenderer->m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );
	}

	g_pRenderer->SetRenderTarget( m_oPickingTarget );

	glClearColor( 1.f, 1.f, 1.f, 1.f );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	Technique& oTechnique = m_xPicking->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	auto BuildColorID = []( const uint64 uID ) {
		const uint16 uRed = ( uID >> 48 ) & 0xFFFF;
		const uint16 uGreen = ( uID >> 32 ) & 0xFFFF;
		const uint16 uBlue = ( uID >> 16 ) & 0xFFFF;
		const uint16 uAlpha = ( uID ) & 0xFFFF;

		return glm::uvec4( uRed, uGreen, uBlue, uAlpha );
	};

	Array< VisualNode* > aVisualNodes;
	Array< VisualNode* > aTemporaryVisualNodes;
	g_pRenderer->m_oVisualStructure.GetVisualNodes( aVisualNodes, aTemporaryVisualNodes );

	for( const VisualNode* pVisualNode : aVisualNodes )
	{
		m_oPickingSheet.GetParameter( PickingParam::USE_SKINNING ).SetValue( true );

		TechniqueArrayParameter& oParamBoneMatrices = m_oPickingSheet.GetArrayParameter( PickingParam::BONE_MATRICES );

		const Array< glm::mat4x3 >& aBoneMatrices = pVisualNode->m_aBoneMatrices;
		for( uint u = 0; u < aBoneMatrices.Count(); ++u )
			oParamBoneMatrices.SetValue( ToMat4( aBoneMatrices[ u ] ), u );
		for( uint u = aBoneMatrices.Count(); u < MAX_BONE_COUNT; ++u )
			oParamBoneMatrices.SetValue( glm::mat4( 1.f ), u );

		m_oPickingSheet.GetParameter( PickingParam::MODEL_VIEW_PROJECTION ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pVisualNode->m_mMatrix ) );
		m_oPickingSheet.GetParameter( PickingParam::COLOR_ID ).SetValue( BuildColorID( pVisualNode->m_uEntityID ) );

		const Array< Mesh >& aMeshes = pVisualNode->m_aMeshes;
		for( const Mesh& oMesh : aMeshes )
			g_pRenderer->DrawMesh( oMesh );
	}

	for( const VisualNode* pVisualNode : aTemporaryVisualNodes )
	{
		m_oPickingSheet.GetParameter( PickingParam::USE_SKINNING ).SetValue( false );

		m_oPickingSheet.GetParameter( PickingParam::MODEL_VIEW_PROJECTION ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pVisualNode->m_mMatrix ) );
		m_oPickingSheet.GetParameter( PickingParam::COLOR_ID ).SetValue( BuildColorID( pVisualNode->m_uEntityID ) );

		const Array< Mesh >& aMeshes = pVisualNode->m_aMeshes;
		for( const Mesh& oMesh : aMeshes )
			g_pRenderer->DrawMesh( oMesh );
	}

	TerrainNode* pTerrain = g_pRenderer->m_oVisualStructure.GetTerrain();
	if( pTerrain != nullptr )
	{
		m_oPickingSheet.GetParameter( PickingParam::USE_SKINNING ).SetValue( false );

		m_oPickingSheet.GetParameter( PickingParam::MODEL_VIEW_PROJECTION ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pTerrain->m_mMatrix ) );

		const Array< Mesh >& aMeshes = pTerrain->m_aMeshes;
		for( uint u = 0; u < aMeshes.Count(); ++u )
		{
			const Mesh& oMesh = aMeshes[ u ];
			m_oPickingSheet.GetParameter( PickingParam::COLOR_ID ).SetValue( BuildColorID( pTerrain->m_aEntitiesIDs[ u ] ) );
			g_pRenderer->DrawMesh( oMesh );
		}
	}

	Array< RoadNode* > aRoads;
	g_pRenderer->m_oVisualStructure.GetRoads( aRoads );

	for( RoadNode* pRoad : aRoads )
	{
		m_oPickingSheet.GetParameter( PickingParam::USE_SKINNING ).SetValue( false );

		m_oPickingSheet.GetParameter( PickingParam::MODEL_VIEW_PROJECTION ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pRoad->m_mMatrix ) );
		m_oPickingSheet.GetParameter( PickingParam::COLOR_ID ).SetValue( BuildColorID( pRoad->m_uEntityID ) );

		g_pRenderer->DrawMesh( pRoad->m_oMesh );
	}

	if( bAllowGizmos )
	{
		glClear( GL_DEPTH_BUFFER_BIT );

		Array< GizmoComponent* > aComponents;
		g_pComponentManager->GetComponents< GizmoComponent >( aComponents );
		for( const GizmoComponent* pComponent : aComponents )
		{
			m_oPickingSheet.GetParameter( PickingParam::USE_SKINNING ).SetValue( false );

			m_oPickingSheet.GetParameter( PickingParam::MODEL_VIEW_PROJECTION ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pComponent->GetWorldMatrix() ) );
			m_oPickingSheet.GetParameter( PickingParam::COLOR_ID ).SetValue( BuildColorID( pComponent->GetEntity()->GetID() ) );

			g_pRenderer->m_oGizmoRenderer.RenderGizmo( pComponent->GetType(), pComponent->GetAxis(), oRenderContext );
		}
	}

	g_pRenderer->ClearTechnique();

	GLushort aChannels[ 4 ];
	glReadPixels( iCursorX, oRenderRect.m_uHeight - iCursorY - 1, 1, 1, GL_RGBA_INTEGER, GL_UNSIGNED_SHORT, &aChannels[ 0 ] );

	g_pRenderer->ClearRenderTarget();

	const uint64 uRed = ( ( uint64 )aChannels[ 0 ] << 48 ) & 0xFFFF'0000'0000'0000;
	const uint64 uGreen = ( ( uint64 )aChannels[ 1 ] << 32 ) & 0x0000'FFFF'0000'0000;
	const uint64 uBlue = ( ( uint64 )aChannels[ 2 ] << 16 ) & 0x0000'0000'FFFF'0000;
	const uint64 uAlpha = ( ( uint64 )aChannels[ 3 ] ) & 0x0000'0000'0000'FFFF;

	const uint64 uID = uRed | uGreen | uBlue | uAlpha;

	return uID;
}

#endif