#include "Terrain.h"

#include "Core/ArrayUtils.h"
#include "Game/Entity.h"
#include "Game/GameEngine.h"
#include "Math/GLMHelpers.h"
#include "Physics/Physics.h"
#include "Renderer.h"

using namespace physx;

template < typename T >
static float SampleHeightMap( const glm::vec2 vUV, const T* aData, const int iWidth, const int iHeight )
{
	static const float fNormalizeFactor = 1.f / ( float )( glm::pow( 2, 8 * sizeof( T ) ) - 1 );

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

	const T uValue00 = aData[ iY0 * iWidth + iX0 ];
	const T uValue10 = aData[ iY0 * iWidth + iX1 ];
	const T uValue01 = aData[ iY1 * iWidth + iX0 ];
	const T uValue11 = aData[ iY1 * iWidth + iX1 ];

	const float uValue0 = uValue00 * ( 1.f - fXRatio ) + uValue10 * fXRatio;
	const float uValue1 = uValue01 * ( 1.f - fXRatio ) + uValue11 * fXRatio;

	return fNormalizeFactor * ( uValue0 * ( 1 - fYRatio ) + uValue1 * fYRatio );
};

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

bool Terrain::OnLoading()
{
	return m_xTerrain->IsLoaded();
}

REGISTER_COMPONENT( TerrainComponent );

TerrainComponent::TerrainComponent( Entity* pEntity )
	: Component( pEntity )
	, m_xDiffuseMap( g_pResourceLoader->LoadTexture( "Liege-Diffuse-2048.png", true ) )
	, m_xBaseLayer( g_pResourceLoader->LoadTexture( "Liege-Heightmap-2048.png", false, true ) )
	, m_pTerrainNode( nullptr )
	, m_pRigidStatic( nullptr )
	, m_bUseTrench( false )
{
}

void TerrainComponent::Initialize()
{
	m_pRigidStatic = g_pPhysics->m_pPhysics->createRigidStatic( PxTransform( PxIdentity ) );

	const uint uChunkCount = m_uWidthChunks * m_uHeightChunks;
	m_aTerrainChunks.Resize( uChunkCount, nullptr );

	m_pTerrainNode = g_pRenderer->m_oVisualStructure.AddTerrain();
	m_pTerrainNode->m_aMeshes.Resize( uChunkCount );
	m_pTerrainNode->m_aEntitiesIDs.Resize( uChunkCount, UINT64_MAX );

	m_pTerrainNode->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTR();
}

bool TerrainComponent::IsInitialized() const
{
	return m_xDiffuseMap->IsLoading() == false && m_xBaseLayer->IsLoading() == false;
}

void TerrainComponent::Start()
{
	g_pPhysics->m_pScene->addActor( *m_pRigidStatic );

	Entity* pEntity = GetEntity();

	const glm::vec3 vPosition = pEntity->GetWorldPosition();
	const glm::quat qRotation = pEntity->GetRotation();

	m_pRigidStatic->setGlobalPose( PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) ) );

	m_pTerrainNode->m_oDiffuse = m_xDiffuseMap->GetTexture();
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

	m_pTerrainNode->m_mMatrix = GetEntity()->GetWorldTransform().GetMatrixTR();
}

void TerrainComponent::Stop()
{
	g_pPhysics->m_pScene->removeActor( *m_pRigidStatic );
}

void TerrainComponent::Dispose()
{
	g_pRenderer->m_oVisualStructure.RemoveTerrain( m_pTerrainNode );

	PX_RELEASE( m_pRigidStatic );

	m_xDiffuseMap = nullptr;
	m_xBaseLayer = nullptr;
}

void TerrainComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
	{
		for( const TerrainChunkComponent* pChunk : m_aTerrainChunks )
		{
			if( pChunk != nullptr )
				pChunk->DisplayWireMesh( glm::vec3( 0.f, 0.25f, 0.5f ) );
		}
	}
}

#ifdef EDITOR
bool TerrainComponent::DisplayInspector()
{
	if( ImGui::Button( "Update chunks" ) )
	{
		m_bUseTrench = false;
		for( TerrainChunkComponent* pChunk : m_aTerrainChunks )
		{
			if( pChunk != nullptr )
				pChunk->UpdateChunk();
		}
	}

	if( ImGui::Button( "Trench" ) )
	{
		m_bUseTrench = false;
		for( TerrainChunkComponent* pChunk : m_aTerrainChunks )
		{
			if( pChunk != nullptr )
				pChunk->UpdateChunk();
		}

		g_pEditor->m_oTrenchTool.Trench( this, m_oTrenchLayer );

		m_bUseTrench = true;
		for( TerrainChunkComponent* pChunk : m_aTerrainChunks )
		{
			if( pChunk != nullptr )
				pChunk->UpdateChunk();
		}
	}

	const bool bExistingChunks = AnyOf( m_aTerrainChunks, []( const TerrainChunkComponent* pChunk ) { return pChunk != nullptr; } );

	ImGui::Separator();

	static bool bAllowOverwriteExistingChunks = false;
	ImGui::Checkbox( "Allow overwrite existing chunks", &bAllowOverwriteExistingChunks );

	if( bExistingChunks == false || bAllowOverwriteExistingChunks )
	{
		ImGui::TextColored( ImVec4( 1.0f, 0.8f, 0.0f, 1.0f ), "Warning : this will erase all existing chunks !" );

		static int iChunks[] = { ( int )m_uWidthChunks, ( int )m_uHeightChunks };
		if( ImGui::InputInt2( "Chunks", iChunks ) )
		{
			m_uWidthChunks = iChunks[ 0 ];
			m_uHeightChunks = iChunks[ 1 ];
		}

		static int iChunksResolution[] = { 64, 64 };
		ImGui::InputInt2( "Chunks resolution", iChunksResolution );

		if( ImGui::Button( "Generate chunks" ) )
		{
			for( TerrainChunkComponent* pChunk : m_aTerrainChunks )
			{
				if( pChunk != nullptr )
					g_pGameWorld->RemoveEntity( pChunk->GetEntity() );
			}

			const uint uChunkCount = m_uWidthChunks * m_uHeightChunks;

			m_aTerrainChunks.Clear();
			m_aTerrainChunks.Resize( uChunkCount, nullptr );

			m_pTerrainNode->m_aMeshes.Resize( uChunkCount );
			m_pTerrainNode->m_aEntitiesIDs.Resize( uChunkCount, UINT64_MAX );

			for( uint uY = 0; uY < m_uHeightChunks; ++uY )
			{
				Entity* pRowEntity = g_pGameWorld->CreateEntity( std::format( "Row_{}", uY ), GetEntity() );

				for( uint uX = 0; uX < m_uWidthChunks; ++uX )
				{
					const uint uChunkIndex = uX + uY * m_uWidthChunks;

					Entity* pEntity = g_pGameWorld->CreateEntity( std::format( "Chunk_{}_{}", uX, uY ), pRowEntity );
					TerrainChunkComponent* pChunk = g_pComponentManager->CreateComponent< TerrainChunkComponent >( pEntity, ComponentManagement::NONE );
					pChunk->m_iChunkIndex = uChunkIndex;
					pChunk->m_uWidthResolution = iChunksResolution[ 0 ];
					pChunk->m_uHeightResolution = iChunksResolution[ 1 ];
					g_pComponentManager->InitializeComponent< TerrainChunkComponent >( pEntity, true );
				}
			}

			bAllowOverwriteExistingChunks = false;
		}
	}

	return false;
}
#endif

float TerrainComponent::GetWidth() const
{
	return m_fWidth;
}

float TerrainComponent::GetHeight() const
{
	return m_fHeight;
}

float TerrainComponent::GetIntensity() const
{
	return m_fIntensity;
}

const TerrainNode* TerrainComponent::GetTerrainNode() const
{
	return m_pTerrainNode;
}

void TerrainComponent::RegisterChunk( TerrainChunkComponent* pChunk )
{
	m_aTerrainChunks[ pChunk->m_iChunkIndex ] = pChunk;
}

void TerrainComponent::UnRegisterChunk( TerrainChunkComponent* pChunk )
{
	m_aTerrainChunks[ pChunk->m_iChunkIndex ] = nullptr;
}

REGISTER_COMPONENT( TerrainChunkComponent );
SET_COMPONENT_PRIORITY_AFTER( TerrainChunkComponent, TerrainComponent );

TerrainChunkComponent::TerrainChunkComponent( Entity* pEntity )
	: Component( pEntity )
	, m_pShape( nullptr )
{
}

void TerrainChunkComponent::Initialize()
{
	Entity* pCurrent = GetEntity();
	while( m_hTerrain.IsValid() == false && pCurrent != nullptr )
	{
		m_hTerrain = g_pComponentManager->GetComponent< TerrainComponent >( pCurrent );
		pCurrent = pCurrent->GetParent();
	}
	ASSERT( m_hTerrain.IsValid() );

	TerrainComponent* pTerrain = m_hTerrain;

	// TODO #eric not safe until we have a priority system
	pTerrain->RegisterChunk( this );
}

bool TerrainChunkComponent::IsInitialized() const
{
	return m_hTerrain->IsInitialized();
}

void TerrainChunkComponent::Start()
{
	TerrainComponent* pTerrain = m_hTerrain;

	GenerateTerrain();

	pTerrain->m_pTerrainNode->m_oDiffuse = pTerrain->m_xDiffuseMap->GetTexture();
	pTerrain->m_pTerrainNode->m_aMeshes[ m_iChunkIndex ] = m_oMesh;
	pTerrain->m_pTerrainNode->m_aEntitiesIDs[ m_iChunkIndex ] = GetEntityID();
}

void TerrainChunkComponent::Stop()
{
	TerrainComponent* pTerrain = m_hTerrain;

	if( pTerrain != nullptr )
	{
		if( m_pShape != nullptr )
			pTerrain->m_pRigidStatic->detachShape( *m_pShape );

		pTerrain->m_pTerrainNode->m_aMeshes[ m_iChunkIndex ] = Mesh();
		pTerrain->m_pTerrainNode->m_aEntitiesIDs[ m_iChunkIndex ] = UINT64_MAX;
	}
}

void TerrainChunkComponent::Dispose()
{
	TerrainComponent* pTerrain = m_hTerrain;

	if( pTerrain != nullptr )
		pTerrain->UnRegisterChunk( this );

	if( m_pShape != nullptr )
		PX_RELEASE( m_pShape );

	m_oMesh.Destroy();
}

void TerrainChunkComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
		DisplayWireMesh( glm::vec3( 1.f, 0.8f, 0.f ) );
}

#ifdef EDITOR
void TerrainChunkComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Width resolution" || sProperty == "Height resolution" || sProperty == "Has collisions" )
		UpdateChunk();
}
#endif

void TerrainChunkComponent::UpdateChunk()
{
	TerrainComponent* pTerrain = m_hTerrain;

	if( m_pShape != nullptr )
	{
		pTerrain->m_pRigidStatic->detachShape( *m_pShape );
		PX_RELEASE( m_pShape );
	}

	m_oMesh.Destroy();

	GenerateTerrain();

	pTerrain->m_pTerrainNode->m_aMeshes[ m_iChunkIndex ] = m_oMesh;
	pTerrain->m_pTerrainNode->m_aEntitiesIDs[ m_iChunkIndex ] = GetEntityID();
}

void TerrainChunkComponent::GenerateTerrain()
{
	TerrainComponent* pTerrain = m_hTerrain;

	const uint uChunkY = m_iChunkIndex / pTerrain->m_uWidthChunks;
	const uint uChunkX = m_iChunkIndex - uChunkY * pTerrain->m_uWidthChunks;

	const float fChunkWidth = pTerrain->m_fWidth / pTerrain->m_uWidthChunks;
	const float fChunkHeight = pTerrain->m_fHeight / pTerrain->m_uHeightChunks;
	const float fChunkStartX = uChunkX * fChunkWidth;
	const float fChunkStartY = uChunkY * fChunkHeight;
	const float fChunkWidthRatio = fChunkWidth / pTerrain->m_fWidth;
	const float fChunkHeightRatio = fChunkHeight / pTerrain->m_fHeight;
	const float fChunkStartXRatio = fChunkStartX / pTerrain->m_fWidth;
	const float fChunkStartYRatio = fChunkStartY / pTerrain->m_fHeight;

	Array< glm::vec2 > aUVs( ( m_uWidthResolution + 1 ) * ( m_uHeightResolution + 1 ) );
	for( uint uYQuad = 0; uYQuad <= m_uHeightResolution; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad <= m_uWidthResolution; ++uXQuad )
			aUVs[ uXQuad + ( m_uWidthResolution + 1 ) * uYQuad ] = glm::vec2( fChunkStartXRatio + ( fChunkWidthRatio * uXQuad ) / m_uWidthResolution, fChunkStartYRatio + ( fChunkHeightRatio * uYQuad ) / m_uHeightResolution );
	}

	const float fXOffset = fChunkWidth / m_uWidthResolution;
	const float fYOffset = fChunkHeight / m_uHeightResolution;
	const float fHalfWidth = pTerrain->m_fWidth / 2.f;
	const float fHalfHeight = pTerrain->m_fHeight / 2.f;

	const Texture& oBaseLayer = pTerrain->m_xBaseLayer->GetTexture();
	const Array< uint8, ArrayFlags::FAST_RESIZE > aRawBaseData = oBaseLayer.FetchData();
	const Array< uint8, ArrayFlags::FAST_RESIZE > aRawTrenchData = pTerrain->m_oTrenchLayer.FetchData();
	const uint16* aBaseData = ( uint16* )aRawBaseData.Data();
	const uint16* aTrenchData = ( uint16* )aRawTrenchData.Data();

	auto ComputeHeight = [ & ]( const glm::vec2& vUV )
	{
		const float fBaseHeight = pTerrain->m_fIntensity * SampleHeightMap( vUV, aBaseData, oBaseLayer.GetWidth(), oBaseLayer.GetHeight() );
		const float fDelta = pTerrain->m_bUseTrench ? pTerrain->m_fIntensity * ( 2.f * SampleHeightMap( vUV, aTrenchData, pTerrain->m_oTrenchLayer.GetWidth(), pTerrain->m_oTrenchLayer.GetHeight() ) - 1.f ) : 0.f;
		return fBaseHeight + fDelta;
	};

	Array< glm::vec3 > aVertices( ( m_uWidthResolution + 1 ) * ( m_uHeightResolution + 1 ) );
	for( uint uYQuad = 0; uYQuad <= m_uHeightResolution; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad <= m_uWidthResolution; ++uXQuad )
		{
			const uint uVertexIndex = uXQuad + ( m_uWidthResolution + 1 ) * uYQuad;
			aVertices[ uVertexIndex ] = glm::vec3( uXQuad * fXOffset - fHalfWidth + fChunkStartX, ComputeHeight( aUVs[ uVertexIndex ] ), uYQuad * fYOffset - fHalfHeight + fChunkStartY);
		}
	}

	auto GetTerrainChunk = [ & ]( const uint uChunkX, const uint uChunkY ) -> TerrainChunkComponent* {
		if( uChunkX < 0 || uChunkX >= pTerrain->m_uWidthChunks || uChunkY < 0 || uChunkY >= pTerrain->m_uHeightChunks )
			return nullptr;

		const uint uLeftChunkIndex = uChunkX + uChunkY * pTerrain->m_uWidthChunks;
		return pTerrain->m_aTerrainChunks[ uLeftChunkIndex ];
	};

	auto SmoothBorder = [ & ]( const TerrainChunkComponent* pNeighbourChunk, const Border eBorder, const uint uResolution, const uint uNeighbourResolution, std::function< uint( uint ) > GetVertexIndex ) {
		if( uResolution > uNeighbourResolution )
		{
			const Array< float > aBorderHeights = pNeighbourChunk->FetchBorder( eBorder );

			const float fResolutionRatio = ( float )uNeighbourResolution / ( float )uResolution;
			for( uint uQuad = 1; uQuad < uResolution; ++uQuad )
			{
				const float fLeftYQuad = uQuad * fResolutionRatio;
				const uint uLeftYQuad = ( uint )fLeftYQuad;
				const float fLeftYQuadRatio = fLeftYQuad - uLeftYQuad;

				const uint uVertexIndex = GetVertexIndex( uQuad );
				aVertices[ uVertexIndex ].y = glm::lerp( aBorderHeights[ uLeftYQuad ], aBorderHeights[ uLeftYQuad + 1 ], fLeftYQuadRatio );
			}
		}
	};

	const TerrainChunkComponent* pLeftChunk = GetTerrainChunk( uChunkX - 1, uChunkY );
	if( pLeftChunk != nullptr )
		SmoothBorder( pLeftChunk, Border::RIGHT, m_uHeightResolution, pLeftChunk->m_uHeightResolution, [ & ]( const uint uYQuad ) { return ( m_uWidthResolution + 1 ) * uYQuad; } );

	const TerrainChunkComponent* pRightChunk = GetTerrainChunk( uChunkX + 1, uChunkY );
	if( pRightChunk != nullptr )
		SmoothBorder( pRightChunk, Border::LEFT, m_uHeightResolution, pRightChunk->m_uHeightResolution, [ & ]( const uint uYQuad ) { return m_uWidthResolution + ( m_uWidthResolution + 1 ) * uYQuad; } );

	const TerrainChunkComponent* pUpChunk = GetTerrainChunk( uChunkX, uChunkY - 1 );
	if( pUpChunk != nullptr )
		SmoothBorder( pUpChunk, Border::DOWN, m_uWidthResolution, pUpChunk->m_uWidthResolution, [ & ]( const uint uXQuad ) { return uXQuad; } );

	const TerrainChunkComponent* pDownChunk = GetTerrainChunk( uChunkX, uChunkY + 1 );
	if( pDownChunk != nullptr )
		SmoothBorder( pDownChunk, Border::UP, m_uWidthResolution, pDownChunk->m_uWidthResolution, [ & ]( const uint uXQuad ) { return uXQuad + ( m_uWidthResolution + 1 ) * m_uHeightResolution; } );

	Array< uint > aIndices( 6 * m_uWidthResolution * m_uHeightResolution );
	for( uint uYQuad = 0; uYQuad < m_uHeightResolution; ++uYQuad )
	{
		for( uint uXQuad = 0; uXQuad < m_uWidthResolution; ++uXQuad )
		{
			const uint uQuadIndex = uXQuad + m_uWidthResolution * uYQuad;
			aIndices[ 6 * uQuadIndex ] = uQuadIndex + uYQuad;
			aIndices[ 6 * uQuadIndex + 1 ] = m_uWidthResolution + uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 2 ] = uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 3 ] = uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 4 ] = m_uWidthResolution + uQuadIndex + uYQuad + 1;
			aIndices[ 6 * uQuadIndex + 5 ] = m_uWidthResolution + uQuadIndex + uYQuad + 2;
		}
	}

	if( m_bHasCollisions )
		GenerateShape( aVertices, aIndices );

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();
}

void TerrainChunkComponent::GenerateShape( const Array< glm::vec3 >& aVertices, const Array< uint >& aIndices )
{
	TerrainComponent* pTerrain = m_hTerrain;

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
	pTerrain->m_pRigidStatic->attachShape( *m_pShape );
}

Array< float > TerrainChunkComponent::FetchBorder( const Border eBorder ) const
{
	const TerrainComponent* pTerrain = m_hTerrain;

	const uint uChunkY = m_iChunkIndex / pTerrain->m_uWidthChunks;
	const uint uChunkX = m_iChunkIndex - uChunkY * pTerrain->m_uWidthChunks;

	const float fChunkWidth = pTerrain->m_fWidth / pTerrain->m_uWidthChunks;
	const float fChunkHeight = pTerrain->m_fHeight / pTerrain->m_uHeightChunks;
	const float fChunkStartX = uChunkX * fChunkWidth;
	const float fChunkStartY = uChunkY * fChunkHeight;
	const float fChunkWidthRatio = fChunkWidth / pTerrain->m_fWidth;
	const float fChunkHeightRatio = fChunkHeight / pTerrain->m_fHeight;
	const float fChunkStartXRatio = fChunkStartX / pTerrain->m_fWidth;
	const float fChunkStartYRatio = fChunkStartY / pTerrain->m_fHeight;

	const Texture& oBaseLayer = pTerrain->m_xBaseLayer->GetTexture();
	const Array< uint8, ArrayFlags::FAST_RESIZE > aRawBaseData = oBaseLayer.FetchData();
	const Array< uint8, ArrayFlags::FAST_RESIZE > aRawTrenchData = pTerrain->m_oTrenchLayer.FetchData();
	const uint16* aBaseData = ( uint16* )aRawBaseData.Data();
	const uint16* aTrenchData = ( uint16* )aRawTrenchData.Data();

	Array< float > aHeights;

	auto ComputeHeight = [ & ]( const glm::vec2& vUV )
	{
		const float fBaseHeight = pTerrain->m_fIntensity * SampleHeightMap( vUV, aBaseData, oBaseLayer.GetWidth(), oBaseLayer.GetHeight() );
		const float fDelta = pTerrain->m_bUseTrench ? pTerrain->m_fIntensity * ( 2.f * SampleHeightMap( vUV, aTrenchData, pTerrain->m_oTrenchLayer.GetWidth(), pTerrain->m_oTrenchLayer.GetHeight() ) - 1.f ) : 0.f;
		return fBaseHeight + fDelta;
	};

	switch( eBorder )
	{
	case Border::LEFT:
		aHeights.Reserve( m_uHeightResolution );
		for( uint uYQuad = 0; uYQuad <= m_uHeightResolution; ++uYQuad )
		{
			const glm::vec2 vUV = glm::vec2( fChunkStartXRatio, fChunkStartYRatio + ( fChunkHeightRatio * uYQuad ) / m_uHeightResolution );
			aHeights.PushBack( ComputeHeight( vUV ) );
		}
		break;
	case Border::RIGHT:
		aHeights.Reserve( m_uHeightResolution );
		for( uint uYQuad = 0; uYQuad <= m_uHeightResolution; ++uYQuad )
		{
			const glm::vec2 vUV = glm::vec2( fChunkStartXRatio + fChunkWidthRatio, fChunkStartYRatio + ( fChunkHeightRatio * uYQuad ) / m_uHeightResolution );
			aHeights.PushBack( ComputeHeight( vUV ) );
		}
		break;
	case Border::UP:
		aHeights.Reserve( m_uWidthResolution );
		for( uint uXQuad = 0; uXQuad <= m_uWidthResolution; ++uXQuad )
		{
			const glm::vec2 vUV = glm::vec2( fChunkStartXRatio + ( fChunkWidthRatio * uXQuad ) / m_uWidthResolution, fChunkStartYRatio );
			aHeights.PushBack( ComputeHeight( vUV ) );
		}
		break;
	case Border::DOWN:
		aHeights.Reserve( m_uWidthResolution );
		for( uint uXQuad = 0; uXQuad <= m_uWidthResolution; ++uXQuad )
		{
			const glm::vec2 vUV = glm::vec2( fChunkStartXRatio + ( fChunkWidthRatio * uXQuad ) / m_uWidthResolution, fChunkStartYRatio + fChunkHeightRatio );
			aHeights.PushBack( ComputeHeight( vUV ) );
		}
		break;
	}

	return aHeights;
}

void TerrainChunkComponent::DisplayWireMesh( const glm::vec3& vColor ) const
{
	const TerrainComponent* pTerrain = m_hTerrain;

	g_pDebugDisplay->DisplayWireMesh( m_oMesh, pTerrain->GetEntity()->GetWorldTransform().GetMatrixTR(), vColor, false );
}
