#pragma once

#include <glm/glm.hpp>

#include "Component.h"

class DirectionalLightComponent : public Component
{
public:
	explicit DirectionalLightComponent( Entity* pEntity );

	void Update( const float fDeltaTime ) override;

	void DisplayGizmos( const bool bSelected ) override;

	glm::vec3	m_vColor;
	float		m_fIntensity;
};

class PointLightComponent : public Component
{
public:
	explicit PointLightComponent( Entity* pEntity );

	void Update( const float fDeltaTime ) override;

	void DisplayGizmos( const bool bSelected ) override;

public:
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fFalloffMinDistance;
	float		m_fFalloffMaxDistance;
};

class SpotLightComponent : public Component
{
public:
	explicit SpotLightComponent( Entity* pEntity );

	void Update( const float fDeltaTime ) override;

	void DisplayGizmos( const bool bSelected ) override;

public:
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fInnerAngle;
	float		m_fOuterAngle;
	float		m_fFalloffMinDistance;
	float		m_fFalloffMaxDistance;
};
