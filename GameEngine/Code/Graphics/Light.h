#pragma once

#include <glm/glm.hpp>

#include "Game/Component.h"
#include "Graphics/VisualStructure.h"

class DirectionalLightComponent : public Component
{
public:
	explicit DirectionalLightComponent( Entity* pEntity );

	void Start() override;
	void Stop() override;
	void Update( const GameContext& oGameContext ) override;

	void DisplayGizmos( const bool bSelected ) override;

private:
	DirectionalLight* m_pDirectionalLight;

	PROPERTIES( DirectionalLightComponent );
	PROPERTY_DEFAULT( "Color", m_vColor, glm::vec3, glm::vec3( 1.f ) );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 1.f );
};

class PointLightComponent : public Component
{
public:
	explicit PointLightComponent( Entity* pEntity );

	void Start() override;
	void Stop() override;
	void Update( const GameContext& oGameContext ) override;

	void DisplayGizmos( const bool bSelected ) override;

private:
	PointLight* m_pPointLight;

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

	void Start() override;
	void Stop() override;
	void Update( const GameContext& oGameContext ) override;

	void DisplayGizmos( const bool bSelected ) override;

private:
	SpotLight* m_pSpotLight;

	PROPERTIES( SpotLightComponent );
	PROPERTY_DEFAULT( "Color", m_vColor, glm::vec3, glm::vec3( 1.f ) );
	PROPERTY_DEFAULT( "Intensity", m_fIntensity, float, 1.f );
	PROPERTY_DEFAULT( "InnerAngle", m_fInnerAngle, float, 30.f );
	PROPERTY_DEFAULT( "OuterAngle", m_fOuterAngle, float, 60.f );
	PROPERTY_DEFAULT( "FalloffMinDistance", m_fFalloffMinDistance, float, 1.f );
	PROPERTY_DEFAULT( "FalloffMaxDistance", m_fFalloffMaxDistance, float, 10.f );
};
