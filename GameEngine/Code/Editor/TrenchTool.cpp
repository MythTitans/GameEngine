#include "TrenchTool.h"

#ifdef EDITOR

#include "Game/Entity.h"
#include "Game/ResourceLoader.h"
#include "Graphics/Renderer.h"
#include "Math/GLMHelpers.h"

inline constexpr int TRENCH_SIZE = 8192;

TrenchTool::TrenchTool()
	: m_xTrench( g_pResourceLoader->LoadTechnique( "Shader/trench.tech" ) )
{
	m_oTrenchRT.Create( RenderTargetDesc( TRENCH_SIZE, TRENCH_SIZE ).Depth() );
}

bool TrenchTool::OnLoading()
{
	return m_xTrench->IsLoading() == false;
}

void TrenchTool::OnLoaded()
{
}

void TrenchTool::Trench( const TerrainComponent* pTerrain, Texture& oTrench )
{
	GPUBlock oBlock( "Trench" );

	g_pRenderer->SetRenderTarget( m_oTrenchRT );

	glViewport( 0, 0, TRENCH_SIZE, TRENCH_SIZE );

	glEnable( GL_DEPTH_TEST );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	Technique& oTechnique = m_xTrench->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	Array< RoadTrenchComponent* > aRoadTrenches;
	g_pComponentManager->GetComponents< RoadTrenchComponent >( aRoadTrenches );

	const glm::vec3 vPosition = pTerrain->GetEntity()->GetWorldPosition();
	const float fHalfWidth = 0.5f * pTerrain->GetWidth();
	const float fHalfHeight = 0.5f * pTerrain->GetHeight();
	const float fElevation = pTerrain->GetIntensity();

	glm::mat4 proj = glm::orthoRH( -fHalfWidth, fHalfWidth, -fHalfHeight, fHalfHeight, 0.f, fElevation );
	glm::mat4 view = glm::lookAt( vPosition + glm::vec3( 0.f, fElevation, 0.f ), vPosition, glm::vec3( 0.f, 0.f, 1.f ) );

	const TerrainNode* pTerrainNode = pTerrain->GetTerrainNode();
	oTechnique.GetParameter( "modelViewProjection" ).SetValue( proj * view * ToMat4( pTerrainNode->m_mMatrix ) );

	for( const Mesh& oMesh : pTerrainNode->m_aMeshes )
		g_pRenderer->DrawMesh( oMesh );

	const Array< uint8 > aRawTerrainDepthData = m_oTrenchRT.GetDepthMap().FetchData();
	const float* aTerrainDepthData = ( float* )aRawTerrainDepthData.Data();

	glDepthFunc( GL_ALWAYS );

	for( const RoadTrenchComponent* pRoadTrench : aRoadTrenches )
	{
		const RoadNode* pRoadNode = pRoadTrench->GetRoadNode();

		oTechnique.GetParameter( "modelViewProjection" ).SetValue( proj * view * ToMat4( pRoadNode->m_mMatrix ) );

		g_pRenderer->DrawMesh( pRoadNode->m_oMesh );
	}

	glDepthFunc( GL_LESS );

	g_pRenderer->ClearRenderTarget();

	const Array< uint8 > aRawRoadDepthData = m_oTrenchRT.GetDepthMap().FetchData();
	const float* aRoadDepthData = ( float* )aRawRoadDepthData.Data();

	constexpr uint16 MAX = ( uint16 )-1;

	Array< uint16 > aDepth( aRawRoadDepthData.Count() / 4 );
	for( uint u = 0; u < aDepth.Count(); ++u )
	{
		const float fDelta = 0.5f * ( aRoadDepthData[ u ] - aTerrainDepthData[ u ] + 1.f );

		const uint uX = u % TRENCH_SIZE;
		const uint uY = u / TRENCH_SIZE;

		const uint uFlipped = uY * TRENCH_SIZE + ( TRENCH_SIZE - 1 - uX );

		aDepth[ uFlipped ] = ( uint16 )( ( 1.f - fDelta ) * MAX );
	}

	TextureDesc oTrenchDesc( TRENCH_SIZE, TRENCH_SIZE, TextureFormat::R16 );
	oTrenchDesc.Data( ( uint8* )aDepth.Data() );

	oTrench.Destroy();
	oTrench.Create( oTrenchDesc );
}

#endif