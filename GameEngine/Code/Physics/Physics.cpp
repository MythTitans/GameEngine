#include "Physics.h"

using namespace physx;

Physics* g_pPhysics = nullptr;

Physics::Physics()
	: m_pFoundation( PxCreateFoundation( PX_PHYSICS_VERSION, m_oAllocator, m_oErrorCallback ) )
	, m_pPvd( PxCreatePvd( *m_pFoundation ) )
	, m_pPhysics( PxCreatePhysics( PX_PHYSICS_VERSION, *m_pFoundation, PxTolerancesScale(), true, m_pPvd ) )
	, m_pCPUDispatcher( PxDefaultCpuDispatcherCreate( 2 ) )
	, m_pScene( nullptr )
	, m_pMaterial( nullptr )
{
	PxPvdTransport* pTransport = PxDefaultPvdSocketTransportCreate( "127.0.0.1", 5425, 10 );
	m_pPvd->connect( *pTransport, PxPvdInstrumentationFlag::eALL );

	PxSceneDesc oSceneDesc( m_pPhysics->getTolerancesScale() );
	oSceneDesc.gravity = PxVec3( 0.f, -9.81f, 0.f );
	oSceneDesc.cpuDispatcher = m_pCPUDispatcher;
	oSceneDesc.filterShader = PxDefaultSimulationFilterShader;
	m_pScene = m_pPhysics->createScene( oSceneDesc );

	m_pMaterial = m_pPhysics->createMaterial( 0.5f, 0.5f, 0.75f );

	PxPvdSceneClient* pPvdClient = m_pScene->getScenePvdClient();
	pPvdClient->setScenePvdFlag( PxPvdSceneFlag::eTRANSMIT_CONSTRAINTS, true );
	pPvdClient->setScenePvdFlag( PxPvdSceneFlag::eTRANSMIT_CONTACTS, true );
	pPvdClient->setScenePvdFlag( PxPvdSceneFlag::eTRANSMIT_SCENEQUERIES, true );

	PxRigidStatic* pGround = PxCreatePlane( *m_pPhysics, PxPlane( 0, 1, 0, 0 ), *m_pMaterial );
	m_pScene->addActor( *pGround );

	g_pPhysics = this;
}

Physics::~Physics()
{
	PX_RELEASE( m_pScene );
	PX_RELEASE( m_pCPUDispatcher );
	PX_RELEASE( m_pPhysics );
	PxPvdTransport* pTransport = m_pPvd->getTransport();
	PX_RELEASE( pTransport );
	PX_RELEASE( m_pPvd );
	PX_RELEASE( m_pFoundation );

	g_pPhysics = nullptr;
}

void Physics::Tick()
{
	m_pScene->simulate( TICK_STEP );
	m_pScene->fetchResults( true );
}
