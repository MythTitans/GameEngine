#pragma once

#include "Game/Component.h"
#include "PxPhysicsAPI.h"

class ShapeCreator
{
public:
	virtual void CreateShape( physx::PxRigidBody* pRigidbody, physx::PxMaterial* pMaterial ) const = 0;
};

class SphereShapeComponent : public Component, public ShapeCreator
{
public:
	explicit SphereShapeComponent( Entity* pEntity );

	void CreateShape( physx::PxRigidBody* pRigidbody, physx::PxMaterial* pMaterial ) const override;

private:
	PROPERTIES( SphereShapeComponent );
	PROPERTY_DEFAULT( "Radius", m_fRadius, float, 1.f );
};

class BoxShapeComponent : public Component, public ShapeCreator
{
public:
	explicit BoxShapeComponent( Entity* pEntity );

	void CreateShape( physx::PxRigidBody* pRigidbody, physx::PxMaterial* pMaterial ) const override;

private:
	PROPERTIES( BoxShapeComponent );
	PROPERTY_DEFAULT( "HalfWidth", m_fHalfWidth, float, 1.f );
	PROPERTY_DEFAULT( "HalfHeight", m_fHalfHeight, float, 1.f );
	PROPERTY_DEFAULT( "HalfDepth", m_fHalfDepth, float, 1.f );
};

class RigidbodyComponent : public Component
{
public:
	explicit RigidbodyComponent( Entity* pEntity );

	void Initialize() override;
	void Start() override;
	void AfterPhysics() override;
	void Update( const GameContext& oGameContext ) override;

private:
	const ShapeCreator* FindShapeCreator() const;

	physx::PxRigidBody* m_pRigidbody;

	physx::PxTransform	m_oLastTransform;
	physx::PxTransform	m_oTransform;

	float				m_fTime;
};
