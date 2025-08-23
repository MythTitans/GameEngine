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
	PROPERTIES( RigidbodyComponent );
	PROPERTY_DEFAULT( "Static", m_bStatic, bool, true );

	physx::PxRigidActor* m_pRigidActor;

	physx::PxTransform	m_oLastTransform;
	physx::PxTransform	m_oTransform;

	float				m_fTime;
};

class ShapeComponentBase : public Component
{
public:
	explicit ShapeComponentBase( Entity* pEntity );

	void					Initialize() override;
	void					Start() override;
	void					Update( const GameContext& oGameContext ) override;
	void					Stop() override;
	void					Dispose() override;

	void					UpdateShape();

private:
	virtual physx::PxShape* CreateShape() = 0;

	using RigidBodyHandle = ComponentHandle< RigidbodyComponent >;
	RigidBodyHandle m_hRigidBody;

	physx::PxShape* m_pShape;
};

class SphereShapeComponent : public ShapeComponentBase
{
public:
	explicit SphereShapeComponent( Entity* pEntity );

	void			DisplayGizmos( const bool bSelected ) override;
	void			OnPropertyChanged( const std::string& sProperty ) override;

private:
	physx::PxShape* CreateShape() override;

	PROPERTIES( SphereShapeComponent );
	PROPERTY_DEFAULT( "Radius", m_fRadius, float, 1.f );
};

class BoxShapeComponent : public ShapeComponentBase
{
public:
	explicit BoxShapeComponent( Entity* pEntity );

	void			DisplayGizmos( const bool bSelected ) override;
	void			OnPropertyChanged( const std::string& sProperty ) override;

private:
	physx::PxShape* CreateShape() override;

	PROPERTIES( BoxShapeComponent );
	PROPERTY_DEFAULT( "HalfWidth", m_fHalfWidth, float, 1.f );
	PROPERTY_DEFAULT( "HalfHeight", m_fHalfHeight, float, 1.f );
	PROPERTY_DEFAULT( "HalfDepth", m_fHalfDepth, float, 1.f );
};
