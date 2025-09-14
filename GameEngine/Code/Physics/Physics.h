#pragma once

#include "PxPhysicsAPI.h"

class Physics
{
public:
	friend class RigidbodyComponent;
	// TODO #eric this is temporary
	friend class SphereShapeComponent;
	friend class BoxShapeComponent;
	friend class TerrainComponent;
	friend class TerrainChunkComponent;

	Physics();
	~Physics();

	void Tick();

	static constexpr float TICK_STEP = 1.f / 60.f;

private:
	physx::PxDefaultAllocator		m_oAllocator;
	physx::PxDefaultErrorCallback	m_oErrorCallback;
	physx::PxFoundation*			m_pFoundation;
	physx::PxPvd*					m_pPvd;
	physx::PxPhysics*				m_pPhysics;
	physx::PxDefaultCpuDispatcher*	m_pCPUDispatcher;
	physx::PxScene*					m_pScene;
	physx::PxMaterial*				m_pMaterial;
};

extern Physics* g_pPhysics;