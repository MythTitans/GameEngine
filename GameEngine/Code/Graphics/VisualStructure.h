#pragma once

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Mesh.h"
#include "Texture.h"

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

struct Sky
{
	CubeMap m_oCubeMap;
};

struct TerrainNode
{
	Texture			m_oDiffuse;
	Array< Mesh >	m_aMeshes;
	glm::mat4x3		m_mMatrix;
	Array< uint64 >	m_aEntitiesIDs;
};

struct VisualNode
{
	explicit VisualNode( const uint64 uEntityID );
	VisualNode( const uint64 uEntityID, const glm::mat4x3& mMatrix, const Array< Mesh >& aMeshes, const Array< glm::mat4x3 >& aBoneMatrices );

	uint64					m_uEntityID;
	glm::mat4x3				m_mMatrix;
	Array< Mesh >			m_aMeshes;
	Array< glm::mat4x3 >	m_aBoneMatrices;
};

class VisualStructure
{
public:
	friend class Renderer;

	VisualStructure();

	VisualNode*				AddNode( const Entity* pEntity, Technique& oTechnique );
	void					AddTemporaryNode( const Entity* pEntity, const glm::mat4x3& mMatrix, const Array< Mesh >& aMeshes, Technique& oTechnique );

	void					RemoveNode( VisualNode*& pNode );

	Array< VisualNode* >	FindNodes( const Entity* pEntity );
	Array< VisualNode* >	FindNodes( const uint64 uEntityID );

	DirectionalLight*		AddDirectionalLight();
	PointLight*				AddPointLight();
	SpotLight*				AddSpotLight();

	void					RemoveDirectionalLight( DirectionalLight*& pDirectionalLight );
	void					RemovePointLight( PointLight*& pPointLight );
	void					RemoveSpotLight( SpotLight*& pSpotLight );

	Sky*					AddSky();
	void					RemoveSky( Sky*& pSky );
	void					SetActiveSky( const Sky* pSky );
	const Sky*				GetActiveSky() const;

	TerrainNode*			AddTerrain();
	void					RemoveTerrain( TerrainNode*& pTerrain );
	TerrainNode*			GetTerrain() const;

private:
	void					Clear();

	Array< Array< VisualNode* > >	m_aVisualNodes;
	Array< Technique* >				m_aTechniques;

	Array< Array< VisualNode > >	m_aTemporaryVisualNodes;
	Array< Technique* >				m_aTemporaryTechniques;

	Array< DirectionalLight* >		m_aDirectionalLights;
	Array< PointLight* >			m_aPointLights;
	Array< SpotLight* >				m_aSpotLights;

	Array< Sky* >					m_aSkies;
	int								m_iActiveSkyIndex;

	TerrainNode*					m_pTerrain; // TODO #eric temporary
};
