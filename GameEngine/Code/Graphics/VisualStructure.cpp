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

VisualNode::VisualNode( const uint64 uEntityID, const glm::mat4& mMatrix, const Array< Mesh >& aMeshes, const Array< glm::mat4 >& aBoneMatrices )
	: m_uEntityID( uEntityID )
	, m_mMatrix( mMatrix )
	, m_aMeshes( aMeshes )
	, m_aBoneMatrices( aBoneMatrices )
{
}

void VisualStructure::AddNode( const Entity* pEntity, const glm::mat4& mMatrix, const Array< Mesh >& aMeshes, const Array< glm::mat4 >& aBoneMatrices, Technique& oTechnique )
{
	ASSERT( aMeshes.Empty() == false );

	int iIndex = Find( m_aTechniques, &oTechnique );
	if( iIndex == -1 )
	{
		iIndex = m_aTechniques.Count();
		m_aTechniques.PushBack( &oTechnique );

		if( iIndex >= ( int )m_aVisualNodes.Count() )
			m_aVisualNodes.PushBack( Array< VisualNode >() );
	}

	m_aVisualNodes[ iIndex ].PushBack( VisualNode( pEntity->GetID(), mMatrix, aMeshes, aBoneMatrices ) );
}

Array< const VisualNode* > VisualStructure::FindNodes( const Entity* pEntity ) const
{
	return FindNodes( pEntity->GetID() );
}

Array< const VisualNode* > VisualStructure::FindNodes( const uint64 uEntityID ) const
{
	Array< const VisualNode* > aFoundVisualNodes;
	for( const Array< VisualNode >& aVisualNodes : m_aVisualNodes )
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
	for( Array< VisualNode >& aNodes : m_aVisualNodes )
		aNodes.Clear();
	m_aTechniques.Clear();

	m_aDirectionalLights.Clear();
	m_aPointLights.Clear();
	m_aSpotLights.Clear();
}
