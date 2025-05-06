#include "Rigidbody.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "ComponentManager.h"
#include "Entity.h"
#include "Physics/Physics.h"

using namespace physx;

REGISTER_COMPONENT( SphereShapeComponent );

SphereShapeComponent::SphereShapeComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void SphereShapeComponent::CreateShape( physx::PxRigidBody* pRigidbody, physx::PxMaterial* pMaterial ) const
{
	PxRigidActorExt::createExclusiveShape( *pRigidbody, PxSphereGeometry( m_fRadius ), *pMaterial );
}

REGISTER_COMPONENT( BoxShapeComponent );

BoxShapeComponent::BoxShapeComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void BoxShapeComponent::CreateShape( physx::PxRigidBody* pRigidbody, physx::PxMaterial* pMaterial ) const
{
	PxRigidActorExt::createExclusiveShape( *pRigidbody, PxBoxGeometry( m_fHalfWidth, m_fHalfHeight, m_fHalfDepth ), *pMaterial );
}

REGISTER_COMPONENT( RigidbodyComponent );

RigidbodyComponent::RigidbodyComponent( Entity* pEntity )
	: Component( pEntity )
	, m_pRigidbody( nullptr )
	, m_fTime( 0.f )
{
}

void RigidbodyComponent::Start()
{
	Entity* pEntity = GetEntity();

	const glm::vec3 vPosition = pEntity->GetWorldPosition();
	const glm::quat qRotation = pEntity->GetRotation();

	m_oLastTransform = PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) );
	m_oTransform = PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ) );

	m_fTime = 0.f;

	m_pRigidbody = g_pPhysics->m_pPhysics->createRigidDynamic( m_oTransform );

	const ShapeCreator* pColliderCreator = FindShapeCreator();
	if( pColliderCreator != nullptr )
		pColliderCreator->CreateShape( m_pRigidbody, g_pPhysics->m_pMaterial );
	PxRigidBodyExt::updateMassAndInertia( *m_pRigidbody, 1.f );

	g_pPhysics->m_pScene->addActor( *m_pRigidbody );

	//m_pRigidbody->is< PxRigidDynamic >()->setAngularVelocity( PxVec3( 0.5f, 0.1f, 0.2f ) );
	//m_pRigidbody->is< PxRigidDynamic >()->setLinearVelocity( PxVec3( 1.f, 0.5f, 0.2f ) );
}

void RigidbodyComponent::AfterPhysics()
{
	m_oLastTransform = m_oTransform;
	m_oTransform = m_pRigidbody->getGlobalPose();

	m_fTime -= Physics::TICK_STEP;
}

void RigidbodyComponent::Update( const float fDeltaTime )
{
	m_fTime += fDeltaTime;

	Entity* pEntity = GetEntity();

	const float fInterpolationRatio = ( m_fTime + Physics::TICK_STEP ) / Physics::TICK_STEP;
	const glm::vec3 vLastPosition = glm::vec3( m_oLastTransform.p.x, m_oLastTransform.p.y, m_oLastTransform.p.z );
	const glm::vec3 vPosition = glm::vec3( m_oTransform.p.x, m_oTransform.p.y, m_oTransform.p.z );
	const glm::quat qLastRotation = glm::quat( m_oLastTransform.q.w, m_oLastTransform.q.x, m_oLastTransform.q.y, m_oLastTransform.q.z );
	const glm::quat qRotation = glm::quat( m_oTransform.q.w, m_oTransform.q.x, m_oTransform.q.y, m_oTransform.q.z );

	Transform oTransfom;
	oTransfom.SetPosition( glm::lerp( vLastPosition, vPosition, fInterpolationRatio ) );
	oTransfom.SetRotation( glm::slerp( qLastRotation, qRotation, fInterpolationRatio ) );
	oTransfom.SetScale( pEntity->GetScale() );

	pEntity->SetWorldTransform( oTransfom );
}

const ShapeCreator* RigidbodyComponent::FindShapeCreator() const
{
	// TODO #eric improve this code
	const ShapeCreator* pShapeCreator = static_cast< const ShapeCreator* >( GetComponent< SphereShapeComponent >() );
	if( pShapeCreator != nullptr )
		return pShapeCreator;

	pShapeCreator = static_cast< const ShapeCreator* >( &*GetComponent< BoxShapeComponent >() );
	return pShapeCreator;
}
