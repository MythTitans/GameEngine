#include "VisualStructure.h"

#include "Core/ArrayUtils.h"
#include "Game/Entity.h"

DirectionalLight::DirectionalLight( const glm::vec3& vDirection, const glm::vec3& vColor, const float fIntensity )
	: m_vDirection( vDirection )
	, m_vColor( vColor )
	, m_fIntensity( fIntensity )
{
}

PointLight::PointLight( const glm::vec3& vPosition, const glm::vec3& vColor, const float fIntensity, const float fFalloffMinDistance, const float fFalloffMaxDistance )
	: m_vPosition( vPosition )
	, m_vColor( vColor )
	, m_fIntensity( fIntensity )
	, m_fFalloffMinDistance( fFalloffMinDistance )
	, m_fFalloffMaxDistance( fFalloffMaxDistance )
{
}

SpotLight::SpotLight( const glm::vec3& vPosition, const glm::vec3& vDirection, const glm::vec3& vColor, const float fIntensity, const float fInnerAngle, const float fOuterAngle, const float fFalloffMinDistance, const float fFalloffMaxDistance )
	: m_vPosition( vPosition )
	, m_vDirection( vDirection )
	, m_vColor( vColor )
	, m_fIntensity( fIntensity )
	, m_fInnerAngle( fInnerAngle )
	, m_fOuterAngle( fOuterAngle )
	, m_fFalloffMinDistance( fFalloffMinDistance )
	, m_fFalloffMaxDistance( fFalloffMaxDistance )
{
}

VisualNode::VisualNode( const uint64 uEntityID, const glm::mat4& mMatrix, const Array< Mesh >* pMeshes )
	: m_uEntityID( uEntityID )
	, m_mMatrix( mMatrix )
	, m_pMeshes( pMeshes )
{
}

void VisualStructure::AddNode( const Entity* pEntity, const Array< Mesh >* pMeshes, Technique& oTechnique )
{
	AddNode( pEntity, pEntity->GetWorldTransform().GetMatrixTRS(), pMeshes, oTechnique );
}

void VisualStructure::AddNode( const Entity* pEntity, glm::mat4 mMatrix, const Array< Mesh >* pMeshes, Technique& oTechnique )
{
	ASSERT( pMeshes != nullptr );
	ASSERT( pMeshes->Empty() == false );

	int iIndex = Find( m_aTechniques, &oTechnique );
	if( iIndex == -1 )
	{
		iIndex = m_aTechniques.Count();
		m_aTechniques.PushBack( &oTechnique );

		if( iIndex >= ( int )m_aImprovedNodes.Count() )
			m_aImprovedNodes.PushBack( Array< VisualNode >() );
	}

	m_aImprovedNodes[ iIndex ].PushBack( VisualNode( pEntity->GetID(), mMatrix, pMeshes ) );
}

Array< const VisualNode* > VisualStructure::FindNodes( const Entity* pEntity ) const
{
	return FindNodes( pEntity->GetID() );
}

Array< const VisualNode* > VisualStructure::FindNodes( const uint64 uEntityID ) const
{
	Array< const VisualNode* > aFoundVisualNodes;
	for( const Array< VisualNode >& aVisualNodes : m_aImprovedNodes )
	{
		for( const VisualNode& oVisualNode : aVisualNodes )
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

void VisualStructure::AddDirectionalLight( const Entity* pEntity, const glm::vec3& vColor, const float fIntensity )
{
	const Transform& oTransform = pEntity->GetWorldTransform();
	m_aDirectionalLights.PushBack( DirectionalLight( oTransform.GetK(), vColor, fIntensity ) );
}

void VisualStructure::AddPointLight( const Entity* pEntity, const glm::vec3& vColor, const float fIntensity, const float fFalloffMinDistance, const float fFalloffMaxDistance )
{
	const Transform& oTransform = pEntity->GetWorldTransform();
	m_aPointLights.PushBack( PointLight( oTransform.GetO(), vColor, fIntensity, fFalloffMinDistance, fFalloffMaxDistance ) );
}

void VisualStructure::AddSpotLight( const Entity* pEntity, const glm::vec3& vColor, const float fIntensity, const float fInnerAngle, const float fOuterAngle, const float fFalloffMinDistance, const float fFalloffMaxDistance )
{
	const Transform& oTransform = pEntity->GetWorldTransform();
	m_aSpotLights.PushBack( SpotLight( oTransform.GetO(), oTransform.GetK(), vColor, fIntensity, fInnerAngle, fOuterAngle, fFalloffMinDistance, fFalloffMaxDistance ) );
}

void VisualStructure::Clear()
{
	for( Array< VisualNode >& aNodes : m_aImprovedNodes )
		aNodes.Clear();
	m_aTechniques.Clear();

	m_aDirectionalLights.Clear();
	m_aPointLights.Clear();
	m_aSpotLights.Clear();
}
