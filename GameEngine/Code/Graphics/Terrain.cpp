#include "Terrain.h"

#include "Game/Entity.h"
#include "Math/GLMHelpers.h"
#include "Physics/Physics.h"
#include "Renderer.h"

using namespace physx;

Terrain::Terrain()
	: m_xTerrain( g_pResourceLoader->LoadTechnique( "Shader/terrain.tech" ) )
{
}

void Terrain::Render( const TerrainNode* pTerrain, const RenderContext& oRenderContext )
{
	Technique& oTechnique = m_xTerrain->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	oTechnique.GetParameter( "modelViewProjection" ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pTerrain->m_mMatrix ) );
	oTechnique.GetParameter( "diffuseColor" ).SetValue( glm::vec3( 1.f, 0.5f, 0.25f ) );

	g_pRenderer->DrawMesh( pTerrain->m_oMesh );

	oTechnique.GetParameter( "diffuseColor" ).SetValue( glm::vec3( 0.f, 0.5f, 0.75f ) );
	//glDisable( GL_DEPTH_TEST );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
	g_pRenderer->DrawMesh( pTerrain->m_oMesh );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	//glEnable( GL_DEPTH_TEST );
}

bool Terrain::OnLoading()
{
	return m_xTerrain->IsLoaded();
}

REGISTER_COMPONENT( TerrainComponent );

TerrainComponent::TerrainComponent( Entity* pEntity )
	: Component( pEntity )
	, m_xHeightMap( g_pResourceLoader->LoadTexture( "heightmap-test.png" ) )
	, m_pTerrain( nullptr )
{
}

void TerrainComponent::Initialize()
{
	m_pRigidStatic = g_pPhysics->m_pPhysics->createRigidStatic( PxTransform( PxIdentity ) );
	g_pPhysics->m_pScene->addActor( *m_pRigidStatic );
}

bool TerrainComponent::IsInitialized() const
{
	return m_xHeightMap->IsLoading() == false;
}

void TerrainComponent::Start()
{
	GenerateTerrain();

	m_pTerrain = g_pRenderer->m_oVisualStructure.AddTerrain();
	m_pTerrain->m_oMesh = m_oMesh;
}

void TerrainComponent::Update( const GameContext& oGameContext )
{
	m_pTerrain->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTRS();
}

void TerrainComponent::GenerateTerrain()
{
	const uint uXQuads = 100;
	const uint uYQuads = 100;

	const float fHalfXQuads = 0.5f * uXQuads;
	const float fHalfYQuads = 0.5f * uYQuads;

	const Texture& oTexture = m_xHeightMap->GetTexture();
	Array< uint8 > aData = oTexture.FetchData();

	Array< glm::vec2 > aUVs( ( uXQuads + 1 ) * ( uYQuads + 1 ) );
	for( uint uYQuad = 0; uYQuad <= uYQuads; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad <= uXQuads; ++uXQuad )
			aUVs[ uXQuad + ( uXQuads + 1 ) * uYQuad ] = glm::vec2( uXQuad / ( float )uXQuads, uYQuad / ( float )uYQuads );
	}

	auto SampleHeightMap = [ & ]( const glm::vec2 vUV ) {
		const int iWidth = oTexture.GetWidth();
		const int iHeight = oTexture.GetHeight();

		const float fX = vUV.x * iWidth - 0.5f;
		const float fY = vUV.y * iHeight - 0.5f;

		int iX0 = ( int )fX;
		int iY0 = ( int )fY;
		int iX1 = iX0 + 1;
		int iY1 = iY0 + 1;

		const float fXRatio = fX - iX0;
		const float fYRatio = fY - iY0;

		iX0 = glm::clamp( iX0, 0, iWidth - 1 );
		iX1 = glm::clamp( iX1, 0, iWidth - 1 );
		iY0 = glm::clamp( iY0, 0, iHeight - 1 );
		iY1 = glm::clamp( iY1, 0, iHeight - 1 );

		const uint8 uValue00 = aData[ 3 * ( iY0 * iWidth + iX0 ) ];
		const uint8 uValue10 = aData[ 3 * ( iY0 * iWidth + iX1 ) ];
		const uint8 uValue01 = aData[ 3 * ( iY1 * iWidth + iX0 ) ];
		const uint8 uValue11 = aData[ 3 * ( iY1 * iWidth + iX1 ) ];

		const float uValue0 = uValue00 * ( 1.f - fXRatio ) + uValue10 * fXRatio;
		const float uValue1 = uValue01 * ( 1.f - fXRatio ) + uValue11 * fXRatio;

		return uValue0 * ( 1 - fYRatio ) + uValue1 * fYRatio;
	};

	Array< glm::vec3 > aVertices( ( uXQuads + 1 ) * ( uYQuads + 1 ) );
	for( uint uYQuad = 0; uYQuad <= uYQuads; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad <= uXQuads; ++uXQuad )
		{
			const uint uVerticeIndex = uXQuad + ( uXQuads + 1 ) * uYQuad;
			const float fHeight = 10.f * ( 255 - SampleHeightMap( aUVs[ uVerticeIndex ] ) ) / 255.f;
			aVertices[ uVerticeIndex ] = glm::vec3( uXQuad - fHalfXQuads, fHeight, uYQuad - fHalfYQuads );
		}
	}

	Array< uint > aIndices( 6 * uXQuads * uYQuads );
	for( uint uYQuad = 0; uYQuad < uYQuads; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad < uXQuads; ++uXQuad )
		{
			const uint uQuadIndex = uXQuad + uXQuads * uYQuad;
			aIndices[ 6 * uQuadIndex ] = uQuadIndex + uYQuad;
			aIndices[ 6 * uQuadIndex + 1 ] = uXQuads + uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 2 ] = uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 3 ] = uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 4 ] = uXQuads + uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 5 ] = uXQuads + uQuadIndex + uYQuad + 2;
		}
	}

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();
}
