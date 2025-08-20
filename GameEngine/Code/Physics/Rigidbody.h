#pragma once

#include "Game/Component.h"
#include "PxPhysicsAPI.h"

class RigidbodyComponent : public Component
{
public:
	explicit RigidbodyComponent( Entity* pEntity );

	void						Initialize() override;
	void						Start() override;
	void						Stop() override;
	void						AfterPhysics() override;
	void						Update( const GameContext& oGameContext ) override;
	void						Dispose() override;

	void						OnPropertyChanged( const std::string& sProperty ) override;

	physx::PxRigidActor*		GetRigidActor();
	const physx::PxRigidActor*	GetRigidActor() const;

private:
	/*
	* TODO #eric : we have two problems :
	* - changing from static to dynamic does not notify the shapes, so they don't get reattached (and maybe the existing are not even destroyed ? not sure)
	* - current implementation can handle adding the rigidbody component after the shape, but this is a hack and should be done better
	* 
	* we should probably :
	* - create a dependency mechanism : if a component requires another, it should be created if not already and removal should be disabled
	* - dependent components should be notified when a property they depend on is changed (m_bStatic for example) so they can react properly
	*/
	PROPERTIES( RigidbodyComponent );
	PROPERTY_DEFAULT( "Static", m_bStatic, bool, true );

	physx::PxRigidActor* m_pRigidActor;

	physx::PxTransform	m_oLastTransform;
	physx::PxTransform	m_oTransform;

	float				m_fTime;
};

class SphereShapeComponent : public Component
{
public:
	explicit SphereShapeComponent( Entity* pEntity );

	void Initialize() override;
	void Start() override;

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

	void Initialize() override;
	void Start() override;

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
