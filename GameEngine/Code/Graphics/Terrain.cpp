#include "Terrain.h"

#include "Game/Entity.h"
#include "Game/GameEngine.h"
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
	oTechnique.GetParameter( "diffuseColor" ).SetValue( glm::vec3( 1.f, 1.f, 1.f ) );

	g_pRenderer->SetTextureSlot( pTerrain->m_oDiffuse, 0 );
	oTechnique.GetParameter( "diffuseMap" ).SetValue( 0 );

	for( const Mesh& oMesh : pTerrain->m_aMeshes )
		g_pRenderer->DrawMesh( oMesh );

	g_pRenderer->ClearTextureSlot( 0 );
}

void Terrain::RenderWireframe( const TerrainNode* pTerrain, const RenderContext& oRenderContext, const glm::vec3 vColor )
{
	Technique& oTechnique = m_xTerrain->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	oTechnique.GetParameter( "modelViewProjection" ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pTerrain->m_mMatrix ) );

	g_pRenderer->SetTextureSlot( *g_pRenderer->GetDefaultDiffuseMap(), 0 );
	oTechnique.GetParameter( "diffuseMap" ).SetValue( 0 );

	oTechnique.GetParameter( "diffuseColor" ).SetValue( vColor );

	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

	for( const Mesh& oMesh : pTerrain->m_aMeshes )
		g_pRenderer->DrawMesh( oMesh );

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	g_pRenderer->ClearTextureSlot( 0 );
}

bool Terrain::OnLoading()
{
	return m_xTerrain->IsLoaded();
}

REGISTER_COMPONENT( TerrainComponent );

TerrainComponent::TerrainComponent( Entity* pEntity )
	: Component( pEntity )
	, m_xDiffuseMap( g_pResourceLoader->LoadTexture( "Liege-Diffuse-1024.png" ) )
	, m_xHeightMap( g_pResourceLoader->LoadTexture( "Liege-Heightmap-1024.png", false, true ) )
	, m_pTerrain( nullptr )
	, m_pRigidStatic( nullptr )
{
}

void TerrainComponent::Initialize()
{
	m_pRigidStatic = g_pPhysics->m_pPhysics->createRigidStatic( PxTransform( PxIdentity ) );

	const uint uChunkCount = m_uWidthChunks * m_uHeightChunks;
	m_aTerrainChunks.Resize( uChunkCount, nullptr );

	m_pTerrain = g_pRenderer->m_oVisualStructure.AddTerrain();
	m_pTerrain->m_aMeshes.Resize( uChunkCount );

	m_pTerrain->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTR();
}

bool TerrainComponent::IsInitialized() const
{
	return m_xHeightMap->IsLoading() == false;
}

void TerrainComponent::Start()
{
	g_pPhysics->m_pScene->addActor( *m_pRigidStatic );

	Entity* pEntity = GetEntity();

	const glm::vec3 vPosition = pEntity->GetWorldPosition();
	const glm::quat qRotation = pEntity->GetRotation();

	m_pRigidStatic->setGlobalPose( PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) ) );

	m_pTerrain->m_oDiffuse = m_xDiffuseMap->GetTexture();
}

void TerrainComponent::Update( const GameContext& oGameContext )
{
	Entity* pEntity = GetEntity();

	if( oGameContext.m_bEditing )
	{
		const glm::vec3 vPosition = pEntity->GetWorldPosition();
		const glm::quat qRotation = pEntity->GetRotation();

		m_pRigidStatic->setGlobalPose( PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) ) );
	}

	m_pTerrain->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTR();
}

void TerrainComponent::Stop()
{
	g_pPhysics->m_pScene->removeActor( *m_pRigidStatic );
}

void TerrainComponent::Dispose()
{
	g_pRenderer->m_oVisualStructure.RemoveTerrain( m_pTerrain );

	PX_RELEASE( m_pRigidStatic );

	m_xHeightMap = nullptr;
}

void TerrainComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
	{
		for( const Entity* pEntity : m_aTerrainChunks )
		{
			if( pEntity != nullptr )
				g_pComponentManager->GetComponent< TerrainChunkComponent >( pEntity )->DisplayWireMesh( glm::vec3( 0.f, 0.25f, 0.5f ) );
		}
	}
}

void TerrainComponent::DisplayInspector()
{
	if( ImGui::Button( "Generate chunks" ) )
	{
		for( Entity* pChunk : m_aTerrainChunks )
		{
			if( pChunk != nullptr )
				g_pGameWorld->RemoveEntity( pChunk );
		}

		const uint uChunkCount = m_uWidthChunks * m_uHeightChunks;

		m_aTerrainChunks.Clear();
		m_aTerrainChunks.Resize( uChunkCount, nullptr );

		m_pTerrain->m_aMeshes.Resize( uChunkCount );

		for( uint uY = 0; uY < m_uHeightChunks; ++uY )
		{
			for( uint uX = 0; uX < m_uWidthChunks; ++uX )
			{
				const uint uChunkIndex = uX + uY * m_uWidthChunks;

				Entity* pEntity = g_pGameWorld->CreateEntity( std::format( "Chunk_{}_{}", uX, uY ), GetEntity() );
				TerrainChunkComponent* pChunkComponent = g_pComponentManager->CreateComponent< TerrainChunkComponent >( pEntity );
				pChunkComponent->m_iChunkIndex = uChunkIndex;
			}
		}
	}

	if( ImGui::Button( "Update chunks" ) )
	{
		for( Entity* pChunk : m_aTerrainChunks )
		{
			if( pChunk != nullptr )
				g_pComponentManager->GetComponent< TerrainChunkComponent >( pChunk )->UpdateChunk();
		}
	}
}

void TerrainComponent::RegisterChunk( TerrainChunkComponent* pChunkComponent )
{
	m_aTerrainChunks[ pChunkComponent->m_iChunkIndex ] = pChunkComponent->GetEntity();
}

void TerrainComponent::UnRegisterChunk( TerrainChunkComponent* pChunkComponent )
{
	m_aTerrainChunks[ pChunkComponent->m_iChunkIndex ] = nullptr;
}

REGISTER_COMPONENT( TerrainChunkComponent );

TerrainChunkComponent::TerrainChunkComponent( Entity* pEntity )
	: Component( pEntity )
	, m_pShape( nullptr )
{
}

void TerrainChunkComponent::Initialize()
{
	m_hTerrainComponent = g_pComponentManager->GetComponent< TerrainComponent >( GetEntity()->GetParent() );
	ASSERT( m_hTerrainComponent.IsValid() );
}

bool TerrainChunkComponent::IsInitialized() const
{
	return m_hTerrainComponent->IsInitialized();
}

void TerrainChunkComponent::Start()
{
	TerrainComponent* pTerrainComponent = m_hTerrainComponent;

	// TODO #eric this could be done in Initialize() if we had a priority system
	pTerrainComponent->RegisterChunk( this );

	GenerateTerrain();

	pTerrainComponent->m_pTerrain->m_oDiffuse = pTerrainComponent->m_xDiffuseMap->GetTexture();
	pTerrainComponent->m_pTerrain->m_aMeshes[ m_iChunkIndex ] = m_oMesh;
}

void TerrainChunkComponent::Stop()
{
	TerrainComponent* pTerrainComponent = m_hTerrainComponent;

	if( pTerrainComponent != nullptr )
	{
		pTerrainComponent->UnRegisterChunk( this );
		pTerrainComponent->m_pRigidStatic->detachShape( *m_pShape );
		pTerrainComponent->m_pTerrain->m_aMeshes[ m_iChunkIndex ] = Mesh();
	}
}

void TerrainChunkComponent::Dispose()
{
	PX_RELEASE( m_pShape );

	m_oMesh.Destroy();
}

void TerrainChunkComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
		DisplayWireMesh( glm::vec3( 1.f, 0.8f, 0.f ) );
}

void TerrainChunkComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Width resolution" || sProperty == "Height resolution" )
		UpdateChunk();
}

void TerrainChunkComponent::UpdateChunk()
{
	TerrainComponent* pTerrainComponent = m_hTerrainComponent;

	pTerrainComponent->m_pRigidStatic->detachShape( *m_pShape );
	PX_RELEASE( m_pShape );

	m_oMesh.Destroy();

	GenerateTerrain();

	pTerrainComponent->m_pTerrain->m_aMeshes[ m_iChunkIndex ] = m_oMesh;
}

void TerrainChunkComponent::GenerateTerrain()
{
	TerrainComponent* pTerrainComponent = m_hTerrainComponent;

	const uint uChunkY = m_iChunkIndex / pTerrainComponent->m_uWidthChunks;
	const uint uChunkX = m_iChunkIndex - uChunkY * pTerrainComponent->m_uWidthChunks;

	const float fChunkWidth = pTerrainComponent->m_fWidth / pTerrainComponent->m_uWidthChunks;
	const float fChunkHeight = pTerrainComponent->m_fHeight / pTerrainComponent->m_uHeightChunks;
	const float fChunkStartX = uChunkX * fChunkWidth;
	const float fChunkStartY = uChunkY * fChunkHeight;
	const float fChunkWidthRatio = fChunkWidth / pTerrainComponent->m_fWidth;
	const float fChunkHeightRatio = fChunkHeight / pTerrainComponent->m_fHeight;
	const float fChunkStartXRatio = fChunkStartX / pTerrainComponent->m_fWidth;
	const float fChunkStartYRatio = fChunkStartY / pTerrainComponent->m_fHeight;

	const uint uXQuads = m_uWidthResolution;
	const uint uYQuads = m_uHeightResolution;

	const Texture& oTexture = pTerrainComponent->m_xHeightMap->GetTexture();
	const Array< uint8 > aRawData = oTexture.FetchData();

	const uint16* aData = ( uint16* )aRawData.Data();

	Array< glm::vec2 > aUVs( ( uXQuads + 1 ) * ( uYQuads + 1 ) );
	for( uint uYQuad = 0; uYQuad <= uYQuads; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad <= uXQuads; ++uXQuad )
			aUVs[ uXQuad + ( uXQuads + 1 ) * uYQuad ] = glm::vec2( fChunkStartXRatio + ( fChunkWidthRatio * uXQuad ) / uXQuads, fChunkStartYRatio + ( fChunkHeightRatio * uYQuad ) / uYQuads );
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

		const uint16 uValue00 = aData[ iY0 * iWidth + iX0 ];
		const uint16 uValue10 = aData[ iY0 * iWidth + iX1 ];
		const uint16 uValue01 = aData[ iY1 * iWidth + iX0 ];
		const uint16 uValue11 = aData[ iY1 * iWidth + iX1 ];

		const float uValue0 = uValue00 * ( 1.f - fXRatio ) + uValue10 * fXRatio;
		const float uValue1 = uValue01 * ( 1.f - fXRatio ) + uValue11 * fXRatio;

		return uValue0 * ( 1 - fYRatio ) + uValue1 * fYRatio;
	};

	const float fXOffset = fChunkWidth / uXQuads;
	const float fYOffset = fChunkHeight / uYQuads;
	const float fHalfWidth = pTerrainComponent->m_fWidth / 2.f;
	const float fHalfHeight = pTerrainComponent->m_fHeight / 2.f;
	const float fNormalizeFactor = 1.f / ( float )( glm::pow( 2, 16 ) - 1 );

	Array< glm::vec3 > aVertices( ( uXQuads + 1 ) * ( uYQuads + 1 ) );
	for( uint uYQuad = 0; uYQuad <= uYQuads; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad <= uXQuads; ++uXQuad )
		{
			const uint uVerticeIndex = uXQuad + ( uXQuads + 1 ) * uYQuad;
			const float fHeight = pTerrainComponent->m_fIntensity * SampleHeightMap( aUVs[ uVerticeIndex ] ) * fNormalizeFactor;
			aVertices[ uVerticeIndex ] = glm::vec3( uXQuad * fXOffset - fHalfWidth + fChunkStartX, fHeight, uYQuad * fYOffset - fHalfHeight + fChunkStartY );
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

	GenerateShape( aVertices, aIndices );

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();
}

void TerrainChunkComponent::GenerateShape( const Array< glm::vec3 >& aVertices, const Array< uint >& aIndices )
{
	TerrainComponent* pTerrainComponent = m_hTerrainComponent;

	PxTriangleMeshDesc oMeshDesc;
	oMeshDesc.points.count = aVertices.Count();
	oMeshDesc.points.stride = sizeof( glm::vec3 );
	oMeshDesc.points.data = aVertices.Data();

	oMeshDesc.triangles.count = aIndices.Count() / 3;
	oMeshDesc.triangles.stride = 3 * sizeof( uint );
	oMeshDesc.triangles.data = aIndices.Data();

	const PxCookingParams oParams( g_pPhysics->m_pPhysics->getTolerancesScale() );

	PxDefaultMemoryOutputStream oWriteBuffer;
	PxTriangleMeshCookingResult::Enum oResult;
	PxCookTriangleMesh( oParams, oMeshDesc, oWriteBuffer, &oResult );

	PxDefaultMemoryInputData oReadBuffer( oWriteBuffer.getData(), oWriteBuffer.getSize() );
	PxTriangleMesh* pTriangleMesh = g_pPhysics->m_pPhysics->createTriangleMesh( oReadBuffer );

	m_pShape = g_pPhysics->m_pPhysics->createShape( PxTriangleMeshGeometry( pTriangleMesh ), *g_pPhysics->m_pMaterial );
	pTerrainComponent->m_pRigidStatic->attachShape( *m_pShape );
}

void TerrainChunkComponent::DisplayWireMesh( const glm::vec3& vColor )
{
	TerrainComponent* pTerrainComponent = m_hTerrainComponent;

	g_pDebugDisplay->DisplayWireMesh( m_oMesh, pTerrainComponent->GetEntity()->GetWorldTransform().GetMatrixTR(), vColor );
}
