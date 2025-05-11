#include "VisualStructure.h"

#include "Core/ArrayUtils.h"
#include "Game/Entity.h"

VisualNode::VisualNode( const uint64 uEntityID )
	: m_uEntityID( uEntityID )
{
}

VisualNode::VisualNode( const uint64 uEntityID, const glm::mat4& mMatrix, const Array< Mesh >& aMeshes, const Array< glm::mat4 >& aBoneMatrices )
	: m_uEntityID( uEntityID )
	, m_mMatrix( mMatrix )
	, m_aMeshes( aMeshes )
	, m_aBoneMatrices( aBoneMatrices )
{
}

VisualNode* VisualStructure::AddNode( const Entity* pEntity, Technique& oTechnique )
{
	int iIndex = Find( m_aTechniques, &oTechnique );
	if( iIndex == -1 )
	{
		iIndex = m_aTechniques.Count();
		m_aTechniques.PushBack( &oTechnique );

		if( iIndex >= ( int )m_aVisualNodes.Count() )
			m_aVisualNodes.PushBack( Array< VisualNode* >() );
	}

	m_aVisualNodes[ iIndex ].PushBack( new VisualNode( pEntity->GetID() ) );

	return m_aVisualNodes[ iIndex ].Back();
}

void VisualStructure::AddTemporaryNode( const Entity* pEntity, const glm::mat4& mMatrix, const Array< Mesh >& aMeshes, Technique& oTechnique )
{
	ASSERT( aMeshes.Empty() == false );

	int iIndex = Find( m_aTemporaryTechniques, &oTechnique );
	if( iIndex == -1 )
	{
		iIndex = m_aTemporaryTechniques.Count();
		m_aTemporaryTechniques.PushBack( &oTechnique );

		if( iIndex >= ( int )m_aTemporaryVisualNodes.Count() )
			m_aTemporaryVisualNodes.PushBack( Array< VisualNode >() );
	}

	m_aTemporaryVisualNodes[ iIndex ].PushBack( VisualNode( pEntity->GetID(), mMatrix, aMeshes, Array< glm::mat4 >() ) );
}

void VisualStructure::RemoveNode( VisualNode*& pNode )
{
	for( Array< VisualNode* >& aNodes : m_aVisualNodes )
	{
		const int iIndex = Find( aNodes, pNode );
		if( iIndex != -1 )
		{
			aNodes.Remove( iIndex );
			delete pNode;
			pNode = nullptr;
			return;
		}
	}
}

Array< VisualNode* > VisualStructure::FindNodes( const Entity* pEntity )
{
	return FindNodes( pEntity->GetID() );
}

Array< VisualNode* > VisualStructure::FindNodes( const uint64 uEntityID )
{
	Array< VisualNode* > aFoundVisualNodes;
	for( Array< VisualNode* >& aVisualNodes : m_aVisualNodes )
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

	for( Array< VisualNode >& aVisualNodes : m_aTemporaryVisualNodes )
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

DirectionalLight* VisualStructure::AddDirectionalLight()
{
	m_aDirectionalLights.PushBack( new DirectionalLight );

	return m_aDirectionalLights.Back();
}

PointLight* VisualStructure::AddPointLight()
{
	m_aPointLights.PushBack( new PointLight );

	return m_aPointLights.Back();
}

SpotLight* VisualStructure::AddSpotLight()
{
	m_aSpotLights.PushBack( new SpotLight );

	return m_aSpotLights.Back();
}

void VisualStructure::RemoveDirectionalLight( DirectionalLight*& pDirectionalLight )
{
	const int iIndex = Find( m_aDirectionalLights, pDirectionalLight );
	if( iIndex != -1 )
	{
		m_aDirectionalLights.Remove( iIndex );
		delete pDirectionalLight;
		pDirectionalLight = nullptr;
	}
}

void VisualStructure::RemovePointLight( PointLight*& pPointLight )
{
	const int iIndex = Find( m_aPointLights, pPointLight );
	if( iIndex != -1 )
	{
		m_aPointLights.Remove( iIndex );
		delete pPointLight;
		pPointLight = nullptr;
	}
}

void VisualStructure::RemoveSpotLight( SpotLight*& pSpotLight )
{
	const int iIndex = Find( m_aSpotLights, pSpotLight );
	if( iIndex != -1 )
	{
		m_aSpotLights.Remove( iIndex );
		delete pSpotLight;
		pSpotLight = nullptr;
	}
}

void VisualStructure::Clear()
{
	for( Array< VisualNode >& aNodes : m_aTemporaryVisualNodes )
		aNodes.Clear();

	m_aTemporaryTechniques.Clear();
}
