#pragma once

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Mesh.h"

class Entity;

struct DirectionalLight
{
	glm::vec3	m_vDirection;
	glm::vec3	m_vColor;
	float		m_fIntensity;
};

struct PointLight
{
	glm::vec3	m_vPosition;
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fFalloffMinDistance;
	float		m_fFalloffMaxDistance;
};

struct SpotLight
{
	glm::vec3	m_vPosition;
	glm::vec3	m_vDirection;
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fInnerAngle;
	float		m_fOuterAngle;
	float		m_fFalloffMinDistance;
	float		m_fFalloffMaxDistance;
};

struct VisualNode
{
	explicit VisualNode( const uint64 uEntityID );
	VisualNode( const uint64 uEntityID, const glm::mat4& mMatrix, const Array< Mesh >& aMeshes, const Array< glm::mat4 >& aBoneMatrices );

	uint64				m_uEntityID;
	glm::mat4			m_mMatrix;
	Array< Mesh >		m_aMeshes;
	Array< glm::mat4 >	m_aBoneMatrices;
};

class VisualStructure
{
public:
	friend class Renderer;

	VisualNode*				AddNode( const Entity* pEntity, Technique& oTechnique );
	void					AddTemporaryNode( const Entity* pEntity, const glm::mat4& mMatrix, const Array< Mesh >& aMeshes, Technique& oTechnique );

	void					RemoveNode( VisualNode*& pNode );

	Array< VisualNode* >	FindNodes( const Entity* pEntity );
	Array< VisualNode* >	FindNodes( const uint64 uEntityID );

	DirectionalLight*		AddDirectionalLight();
	PointLight*				AddPointLight();
	SpotLight*				AddSpotLight();

	void					RemoveDirectionalLight( DirectionalLight*& pDirectionalLight );
	void					RemovePointLight( PointLight*& pPointLight );
	void					RemoveSpotLight( SpotLight*& pSpotLight );

private:
	void					Clear();

	Array< Array< VisualNode* > >	m_aVisualNodes;
	Array< Technique* >				m_aTechniques;

	Array< Array< VisualNode > >	m_aTemporaryVisualNodes;
	Array< Technique* >				m_aTemporaryTechniques;

	Array< DirectionalLight* >		m_aDirectionalLights;
	Array< PointLight* >			m_aPointLights;
	Array< SpotLight* >				m_aSpotLights;
};
