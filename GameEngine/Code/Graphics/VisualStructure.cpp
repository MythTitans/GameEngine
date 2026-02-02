#include "VisualStructure.h"

#include "Core/ArrayUtils.h"
#include "Game/Entity.h"

RoadNode::RoadNode( const uint64 uEntityID, const glm::mat4x3& mMatrix, const Texture& oDiffuse, const Mesh& oMesh )
	: m_uEntityID( uEntityID )
	, m_mMatrix( mMatrix )
	, m_oDiffuse( oDiffuse )
	, m_oMesh( oMesh )
{
}

VisualNode::VisualNode( const uint64 uEntityID )
	: m_uEntityID( uEntityID )
	, m_uBoneStorageIndex( 0 )
	, m_uBoneCount( 0 )
	, m_bVisible( true )
{
}

VisualNode::VisualNode( const uint64 uEntityID, const glm::mat4x3& mMatrix, const Array< Mesh >& aMeshes )
	: m_uEntityID( uEntityID )
	, m_mMatrix( mMatrix )
	, m_aMeshes( aMeshes )
	, m_uBoneStorageIndex( 0 )
	, m_uBoneCount( 0 )
	, m_bVisible( true )
{
}

VisualStructure::VisualStructure()
	: m_iActiveSkyIndex( -1 )
	, m_pTerrain( nullptr )
{
}

VisualNode* VisualStructure::AddVisual( const Entity* pEntity, Technique& oTechnique )
{
	int iIndex = Find( m_aTechniques, &oTechnique );
	if( iIndex == -1 )
	{
		iIndex = m_aTechniques.Count();
		m_aTechniques.PushBack( &oTechnique );

		if( iIndex >= ( int )m_aVisuals.Count() )
			m_aVisuals.PushBack( Array< VisualNode* >() );
	}

	m_aVisuals[ iIndex ].PushBack( new VisualNode( pEntity->GetID() ) );

	return m_aVisuals[ iIndex ].Back();
}

void VisualStructure::AddTemporaryVisual( const Entity* pEntity, const glm::mat4x3& mMatrix, const Array< Mesh >& aMeshes, Technique& oTechnique )
{
	ASSERT( aMeshes.Empty() == false );

	int iIndex = Find( m_aTemporaryTechniques, &oTechnique );
	if( iIndex == -1 )
	{
		iIndex = m_aTemporaryTechniques.Count();
		m_aTemporaryTechniques.PushBack( &oTechnique );

		if( iIndex >= ( int )m_aTemporaryVisuals.Count() )
			m_aTemporaryVisuals.PushBack( Array< VisualNode >() );
	}

	m_aTemporaryVisuals[ iIndex ].PushBack( VisualNode( pEntity->GetID(), mMatrix, aMeshes ) );
}

void VisualStructure::RemoveVisual( VisualNode*& pNode )
{
	for( uint uGroupIndex = 0; uGroupIndex < m_aVisuals.Count(); ++uGroupIndex )
	{
		Array< VisualNode* >& aNodes = m_aVisuals[ uGroupIndex ];

		const int iIndex = Find( aNodes, pNode );
		if( iIndex != -1 )
		{
			aNodes.Remove( iIndex );
			delete pNode;
			pNode = nullptr;

			if( aNodes.Empty() )
			{
				m_aVisuals.Remove( uGroupIndex );
				m_aTechniques.Remove( uGroupIndex );
			}

			return;
		}
	}
}

Array< VisualNode* > VisualStructure::FindVisuals( const Entity* pEntity )
{
	return FindVisuals( pEntity->GetID() );
}

Array< VisualNode* > VisualStructure::FindVisuals( const uint64 uEntityID )
{
	Array< VisualNode* > aFoundVisualNodes;
	for( Array< VisualNode* >& aVisualNodes : m_aVisuals )
	{
		for( VisualNode* pVisualNode : aVisualNodes )
		{
			if( pVisualNode->m_uEntityID == uEntityID )
			{
				aFoundVisualNodes.PushBack( pVisualNode );
				break;
			}
		}
	}

	for( Array< VisualNode >& aVisualNodes : m_aTemporaryVisuals )
	{
		for( VisualNode& oVisualNode : aVisualNodes )
		{
			if( oVisualNode.m_uEntityID == uEntityID )
			{
				aFoundVisualNodes.PushBack( &oVisualNode );
				break;
			}
		}
	}

	return aFoundVisualNodes;
}

DirectionalLightNode* VisualStructure::AddDirectionalLight()
{
	m_aDirectionalLights.PushBack( new DirectionalLightNode );

	return m_aDirectionalLights.Back();
}

PointLightNode* VisualStructure::AddPointLight()
{
	m_aPointLights.PushBack( new PointLightNode );

	return m_aPointLights.Back();
}

SpotLightNode* VisualStructure::AddSpotLight()
{
	m_aSpotLights.PushBack( new SpotLightNode );

	return m_aSpotLights.Back();
}

void VisualStructure::RemoveDirectionalLight( DirectionalLightNode*& pDirectionalLight )
{
	const int iIndex = Find( m_aDirectionalLights, pDirectionalLight );
	if( iIndex != -1 )
	{
		m_aDirectionalLights.Remove( iIndex );
		delete pDirectionalLight;
		pDirectionalLight = nullptr;
	}
}

void VisualStructure::RemovePointLight( PointLightNode*& pPointLight )
{
	const int iIndex = Find( m_aPointLights, pPointLight );
	if( iIndex != -1 )
	{
		m_aPointLights.Remove( iIndex );
		delete pPointLight;
		pPointLight = nullptr;
	}
}

void VisualStructure::RemoveSpotLight( SpotLightNode*& pSpotLight )
{
	const int iIndex = Find( m_aSpotLights, pSpotLight );
	if( iIndex != -1 )
	{
		m_aSpotLights.Remove( iIndex );
		delete pSpotLight;
		pSpotLight = nullptr;
	}
}

SkyNode* VisualStructure::AddSky()
{
	m_aSkies.PushBack( new SkyNode );

	return m_aSkies.Back();
}

void VisualStructure::RemoveSky( SkyNode*& pSky )
{
	const int iIndex = Find( m_aSkies, pSky );
	if( iIndex != -1 )
	{
		m_aSkies.Remove( iIndex );
		delete pSky;
		pSky = nullptr;

		if( iIndex == m_iActiveSkyIndex )
			m_iActiveSkyIndex = -1;
		else if( iIndex < m_iActiveSkyIndex )
			--m_iActiveSkyIndex;
	}
}

void VisualStructure::SetActiveSky( const SkyNode* pSky )
{
	m_iActiveSkyIndex = -1;

	for( uint u = 0; u < m_aSkies.Count(); ++u )
	{
		if( m_aSkies[ u ] == pSky )
		{
			m_iActiveSkyIndex = ( int )u;
			break;
		}
	}
}

const SkyNode* VisualStructure::GetActiveSky() const
{
	if( m_iActiveSkyIndex == -1 )
		return nullptr;

	return m_aSkies[ m_iActiveSkyIndex ];
}

TerrainNode* VisualStructure::AddTerrain()
{
	m_pTerrain = new TerrainNode;
	return m_pTerrain;
}

void VisualStructure::RemoveTerrain( TerrainNode*& pTerrain )
{
	delete m_pTerrain;
	m_pTerrain = nullptr;
	pTerrain = nullptr;
}

TerrainNode* VisualStructure::GetTerrain() const
{
	return m_pTerrain;
}

RoadNode* VisualStructure::AddRoad( const Entity* pEntity, const Texture& oTexture, const Mesh& oMesh )
{
	m_aRoads.PushBack( new RoadNode( pEntity->GetID(), pEntity->GetWorldTransform().GetMatrixTR(), oTexture, oMesh ) );
	return m_aRoads.Back();
}

void VisualStructure::RemoveRoad( RoadNode*& pRoad )
{
	const int iIndex = Find( m_aRoads, pRoad );
	if( iIndex != -1 )
	{
		m_aRoads.Remove( iIndex );
		delete pRoad;
		pRoad = nullptr;
	}
}

void VisualStructure::GetVisualNodes( Array< VisualNode* >& aNodes, Array< VisualNode* >& aTemporaryNodes )
{
	for( const Array< VisualNode* >& aGroupedNodes : m_aVisuals )
	{
		aNodes.Reserve( aNodes.Count() + aGroupedNodes.Count() );

		for( VisualNode* pNode : aGroupedNodes )
			aNodes.PushBack( pNode );
	}

	for( Array< VisualNode >& aGroupedNodes : m_aTemporaryVisuals )
	{
		aTemporaryNodes.Reserve( aTemporaryNodes.Count() + aGroupedNodes.Count() );

		for( VisualNode& oNode : aGroupedNodes )
			aTemporaryNodes.PushBack( &oNode );
	}
}

void VisualStructure::GetLights( Array< DirectionalLightNode* >& aDirectionalLights, Array< PointLightNode* >& aPointLights, Array< SpotLightNode* >& aSpotLights )
{
	aDirectionalLights = m_aDirectionalLights;
	aPointLights = m_aPointLights;
	aSpotLights = m_aSpotLights;
}

void VisualStructure::GetRoads( Array<RoadNode*>& aRoads )
{
	aRoads = m_aRoads;
}

void VisualStructure::Clear()
{
	for( Array< VisualNode >& aNodes : m_aTemporaryVisuals )
		aNodes.Clear();

	m_aTemporaryTechniques.Clear();
}
