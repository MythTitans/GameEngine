#pragma once

#include <glm/glm.hpp>

#include "Component.h"

class DirectionalLightComponent : public Component
{
public:
	explicit DirectionalLightComponent( Entity* pEntity );

	void Update( const float fDeltaTime ) override;

	void DisplayGizmos( const bool bSelected ) override;

private:
	PROPERTIES( DirectionalLightComponent );
	PROPERTY_DEFAULT( "Color", m_vColor, glm::vec3, glm::vec3( 1.f ) );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 1.f );
};

class PointLightComponent : public Component
{
public:
	explicit PointLightComponent( Entity* pEntity );

	void Update( const float fDeltaTime ) override;

	void DisplayGizmos( const bool bSelected ) override;

private:
	PROPERTIES( PointLightComponent );
	PROPERTY_DEFAULT( "Color", m_vColor, glm::vec3, glm::vec3( 1.f ) );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 1.f );
	PROPERTY_DEFAULT( "FalloffMinDistance", m_fFalloffMinDistance, float, 1.f );
	PROPERTY_DEFAULT( "FalloffMaxDistance", m_fFalloffMaxDistance, float, 10.f );
};

class SpotLightComponent : public Component
{
public:
	explicit SpotLightComponent( Entity* pEntity );

	void Update( const float fDeltaTime ) override;

	void DisplayGizmos( const bool bSelected ) override;

private:
	PROPERTIES( SpotLightComponent );
	PROPERTY_DEFAULT( "Color", m_vColor, glm::vec3, glm::vec3( 1.f ) );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 1.f );
	PROPERTY_DEFAULT( "InnerAngle", m_fInnerAngle, float, 30.f );
	PROPERTY_DEFAULT( "OuterAngle", m_fOuterAngle, float, 60.f );
	PROPERTY_DEFAULT( "FalloffMinDistance", m_fFalloffMinDistance, float, 1.f );
	PROPERTY_DEFAULT( "FalloffMaxDistance", m_fFalloffMaxDistance, float, 10.f );
};
