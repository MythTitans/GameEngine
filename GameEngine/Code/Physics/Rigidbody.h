#pragma once

#include "Game/Component.h"
#include "PxPhysicsAPI.h"

class RigidbodyComponent : public Component
{
public:
	explicit RigidbodyComponent( Entity* pEntity );

	void						Initialize() override;
	void						Start() override;
	void						AfterPhysics() override;
	void						Update( const GameContext& oGameContext ) override;
	void						Dispose() override;

	physx::PxRigidBody*			GetRigidBody();
	const physx::PxRigidBody*	GetRigidBody() const;

private:
	physx::PxRigidBody* m_pRigidbody;

	physx::PxTransform	m_oLastTransform;
	physx::PxTransform	m_oTransform;

	float				m_fTime;
};

class SphereShapeComponent : public Component
{
public:
	explicit SphereShapeComponent( Entity* pEntity );

	void Update( const GameContext& oGameContext ) override;

	void DisplayGizmos( const bool bSelected ) override;
	void OnPropertyChanged( const std::string& sProperty ) override;

private:
	PROPERTIES( SphereShapeComponent );
	PROPERTY_DEFAULT( "Radius", m_fRadius, float, 1.f );

	using RigidBodyHandle = ComponentHandle< RigidbodyComponent >;
	RigidBodyHandle m_hRigidBody;
};

class BoxShapeComponent : public Component
{
public:
	explicit BoxShapeComponent( Entity* pEntity );

	void Update( const GameContext& oGameContext ) override;

	void DisplayGizmos( const bool bSelected ) override;
	void OnPropertyChanged( const std::string& sProperty ) override;

private:
	PROPERTIES( BoxShapeComponent );
	PROPERTY_DEFAULT( "HalfWidth", m_fHalfWidth, float, 1.f );
	PROPERTY_DEFAULT( "HalfHeight", m_fHalfHeight, float, 1.f );
	PROPERTY_DEFAULT( "HalfDepth", m_fHalfDepth, float, 1.f );

	using RigidBodyHandle = ComponentHandle< RigidbodyComponent >;
	RigidBodyHandle m_hRigidBody;
};
