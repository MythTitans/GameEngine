#pragma once

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Mesh.h"

class Entity;

struct DirectionalLight
{
	DirectionalLight( const glm::vec3& vDirection, const glm::vec3& vColor, const float fIntensity );

	glm::vec3	m_vDirection;
	glm::vec3	m_vColor;
	float		m_fIntensity;
};

struct PointLight
{
	PointLight( const glm::vec3& vPosition, const glm::vec3& vColor, const float fIntensity, const float fFalloffMinDistance, const float fFalloffMaxDistance );

	glm::vec3	m_vPosition;
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fFalloffMinDistance;
	float		m_fFalloffMaxDistance;
};

struct SpotLight
{
	SpotLight( const glm::vec3& vPosition, const glm::vec3& vDirection, const glm::vec3& vColor, const float fIntensity, const float fInnerAngle, const float fOuterAngle, const float fFalloffMinDistance, const float fFalloffMaxDistance );

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
	VisualNode( const uint64 uEntityID, const glm::mat4& mMatrix, const Array< Mesh >* pMeshes, const Array< glm::mat4 >* pBoneMatrices );

	uint64						m_uEntityID;
	glm::mat4					m_mMatrix;
	const Array< Mesh >*		m_pMeshes;
	const Array< glm::mat4 >*	m_pBoneMatrices;
};

class VisualStructure
{
public:
	friend class Renderer;

	void						AddNode( const Entity* pEntity, const glm::mat4& mMatrix, const Array< Mesh >* pMeshes, const Array< glm::mat4 >* pBoneMatrices, Technique& oTechnique );
	Array< const VisualNode* >	FindNodes( const Entity* pEntity ) const;
	Array< const VisualNode* >	FindNodes( const uint64 uEntityID ) const;

	void						AddDirectionalLight( const Entity* pEntity, const glm::vec3& vColor, const float fIntensity );
	void						AddPointLight( const Entity* pEntity, const glm::vec3& vColor, const float fIntensity, const float fFalloffMinDistance, const float fFalloffMaxDistance );
	void						AddSpotLight( const Entity* pEntity, const glm::vec3& vColor, const float fIntensity, const float fInnerAngle, const float fOuterAngle, const float fFalloffMinDistance, const float fFalloffMaxDistance );

private:
	void						Clear();

	Array< Array< VisualNode > >	m_aVisualNodes;
	Array< Technique* >				m_aTechniques;

	Array< DirectionalLight >		m_aDirectionalLights;
	Array< PointLight >				m_aPointLights;
	Array< SpotLight >				m_aSpotLights;
};
