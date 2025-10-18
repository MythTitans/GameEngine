#pragma once

#include "Component.h"

class Camera;

class CameraComponentBase : public Component
{
public:
	CameraComponentBase( Entity* pEntity );

	virtual void ApplyCamera( Camera& oCamera ) = 0;
};

class CameraComponent : public CameraComponentBase
{
public:
	explicit CameraComponent( Entity* pEntity );

	void Start() override;

	void ApplyCamera( Camera& oCamera ) override;

private:
	PROPERTIES( CameraComponent );
	PROPERTY_DEFAULT( "Fov", m_fFov, float, 60.f );
	PROPERTY( "Target", m_oTarget, EntityHolder );
};
