#pragma once

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Mesh.h"
#include "Technique.h"
#include "Texture.h"

class Entity;

struct DirectionalLightNode
{
	glm::vec3	m_vDirection;
	glm::vec3	m_vColor;
	float		m_fIntensity;
};

struct PointLightNode
{
	glm::vec3	m_vPosition;
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fFalloffMinDistance;
	float		m_fFalloffMaxDistance;
};

struct SpotLightNode
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

struct SkyNode
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

struct RoadNode
{
	RoadNode( const uint64 uEntityID, const glm::mat4x3& mMatrix, const Texture& oDiffuse, const Mesh& oMesh );

	uint64		m_uEntityID;
	glm::mat4x3	m_mMatrix;
	Texture		m_oDiffuse;
	Mesh		m_oMesh;
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

	VisualNode*				AddVisual( const Entity* pEntity, Technique& oTechnique );
	void					AddTemporaryVisual( const Entity* pEntity, const glm::mat4x3& mMatrix, const Array< Mesh >& aMeshes, Technique& oTechnique );
	void					RemoveVisual( VisualNode*& pNode );

	Array< VisualNode* >	FindVisuals( const Entity* pEntity );
	Array< VisualNode* >	FindVisuals( const uint64 uEntityID );

	DirectionalLightNode*	AddDirectionalLight();
	PointLightNode*			AddPointLight();
	SpotLightNode*			AddSpotLight();
	void					RemoveDirectionalLight( DirectionalLightNode*& pDirectionalLight );
	void					RemovePointLight( PointLightNode*& pPointLight );
	void					RemoveSpotLight( SpotLightNode*& pSpotLight );

	SkyNode*				AddSky();
	void					RemoveSky( SkyNode*& pSky );
	void					SetActiveSky( const SkyNode* pSky );
	const SkyNode*			GetActiveSky() const;

	TerrainNode*			AddTerrain();
	void					RemoveTerrain( TerrainNode*& pTerrain );
	TerrainNode*			GetTerrain() const;

	RoadNode*				AddRoad( const Entity* pEntity, const Texture& oTexture, const Mesh& oMesh );
	void					RemoveRoad( RoadNode*& pRoad );

	void					GetVisualNodes( Array< VisualNode* >& aNodes, Array< VisualNode* >& aTemporaryNodes );
	void					GetLights( Array< DirectionalLightNode* >& aDirectionalLights, Array< PointLightNode* >& aPointLights, Array< SpotLightNode* >& aSpotLights );
	void					GetRoads( Array<RoadNode*>& aRoads );

private:
	void					Clear();

	Array< Array< VisualNode* > >	m_aVisuals;
	Array< Technique* >				m_aTechniques;

	Array< Array< VisualNode > >	m_aTemporaryVisuals;
	Array< Technique* >				m_aTemporaryTechniques;

	Array< DirectionalLightNode* >	m_aDirectionalLights;
	Array< PointLightNode* >		m_aPointLights;
	Array< SpotLightNode* >			m_aSpotLights;

	Array< SkyNode* >				m_aSkies;
	int								m_iActiveSkyIndex;

	TerrainNode*					m_pTerrain; // TODO #eric temporary

	Array< RoadNode* >				m_aRoads;
};
