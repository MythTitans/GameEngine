#include "VisualStructure.h"

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

void VisualStructure::AddNode( const Entity* pEntity, const Array< Mesh >* pMeshes )
{
	ASSERT( pMeshes != nullptr );
	ASSERT( pMeshes->Empty() == false );

	m_aNodes.PushBack( VisualNode( pEntity->GetID(), pEntity->GetWorldTransform().GetMatrixTRS(), pMeshes ) );
}

void VisualStructure::AddNode( const Entity* pEntity, glm::mat4 mMatrix, const Array< Mesh >* pMeshes )
{
	ASSERT( pMeshes != nullptr );
	ASSERT( pMeshes->Empty() == false );

	m_aNodes.PushBack( VisualNode( pEntity->GetID(), mMatrix, pMeshes ) );
}

const VisualNode* VisualStructure::FindNode( const Entity* pEntity )
{
	return FindNode( pEntity->GetID() );
}

const VisualNode* VisualStructure::FindNode( const uint64 uEntityID )
{
	for( const VisualNode& oVisualNode : m_aNodes )
	{
		if( oVisualNode.m_uEntityID == uEntityID )
			return &oVisualNode;
	}

	return nullptr;
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
	m_aNodes.Clear();

	m_aDirectionalLights.Clear();
	m_aPointLights.Clear();
	m_aSpotLights.Clear();
}

const VisualNode* VisualStructure::begin() const
{
	return m_aNodes.begin();
}

const VisualNode* VisualStructure::end() const
{
	return m_aNodes.end();
}
