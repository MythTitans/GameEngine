#include "Rigidbody.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>
#include <glm/gtx/compatibility.hpp>

#include "Game/ComponentManager.h"
#include "Game/Entity.h"
#include "Game/GameEngine.h"
#include "Physics/Physics.h"

using namespace physx;

REGISTER_COMPONENT( RigidbodyComponent );

RigidbodyComponent::RigidbodyComponent( Entity* pEntity )
	: Component( pEntity )
	, m_pRigidbody( nullptr )
	, m_fTime( 0.f )
{
}

void RigidbodyComponent::Initialize()
{
	m_pRigidbody = g_pPhysics->m_pPhysics->createRigidDynamic( PxTransform( PxIdentity ) );
	g_pPhysics->m_pScene->addActor( *m_pRigidbody );
}

void RigidbodyComponent::Start()
{
	Entity* pEntity = GetEntity();

	const glm::vec3 vPosition = pEntity->GetWorldPosition();
	const glm::quat qRotation = pEntity->GetRotation();

	m_oLastTransform = PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) );
	m_oTransform = m_oLastTransform;

	m_fTime = 0.f;

	m_pRigidbody->setGlobalPose( m_oTransform );

	//m_pRigidbody->is< PxRigidDynamic >()->setAngularVelocity( PxVec3( 0.5f, 0.1f, 0.2f ) );
	//m_pRigidbody->is< PxRigidDynamic >()->setLinearVelocity( PxVec3( 1.f, 0.5f, 0.2f ) );
}

void RigidbodyComponent::AfterPhysics()
{
	m_oLastTransform = m_oTransform;
	m_oTransform = m_pRigidbody->getGlobalPose();

	m_fTime -= Physics::TICK_STEP;
}

void RigidbodyComponent::Update( const GameContext& oGameContext )
{
	if( oGameContext.m_bEditing )
	{
		Entity* pEntity = GetEntity();

		const glm::vec3 vPosition = pEntity->GetWorldPosition();
		const glm::quat qRotation = pEntity->GetRotation();

		m_oLastTransform = PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) );
		m_oTransform = m_oLastTransform;

		m_pRigidbody->setGlobalPose( m_oTransform );
	}

	m_fTime += oGameContext.m_fLastDeltaTime;

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

void RigidbodyComponent::Dispose()
{
	g_pPhysics->m_pScene->removeActor( *m_pRigidbody );
	PX_RELEASE( m_pRigidbody );
}

physx::PxRigidBody* RigidbodyComponent::GetRigidBody()
{
	return m_pRigidbody;
}

const physx::PxRigidBody* RigidbodyComponent::GetRigidBody() const
{
	return m_pRigidbody;
}

REGISTER_COMPONENT( SphereShapeComponent );

SphereShapeComponent::SphereShapeComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void SphereShapeComponent::Update( const GameContext& oGameContext )
{
	// TODO #eric this should only be done in editing, but it would require an initialization priority
	// TODO #eric handle shape removal
	if( m_hRigidBody.IsValid() == false )
	{
		m_hRigidBody = GetComponent< RigidbodyComponent >();

		if( m_hRigidBody.IsValid() )
		{
			PxRigidBody* pRigidBody = m_hRigidBody->GetRigidBody();
			PxRigidActorExt::createExclusiveShape( *pRigidBody, PxSphereGeometry( m_fRadius ), *g_pPhysics->m_pMaterial );
			PxRigidBodyExt::updateMassAndInertia( *pRigidBody, 1.f );
		}
	}
}

void SphereShapeComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
	{
		const Transform& oTransform = GetEntity()->GetWorldTransform();
		const glm::vec3 vPosition = oTransform.GetO();

		g_pDebugDisplay->DisplayWireSphere( vPosition, m_fRadius, glm::vec3( 0.f, 1.f, 0.f ) );
	}
}

void SphereShapeComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Radius" )
	{
		if( m_hRigidBody.IsValid() )
		{
			PxRigidBody* pRigidBody = m_hRigidBody->GetRigidBody();

			PxShape* pShape = nullptr;
			pRigidBody->getShapes( &pShape, 1 );

			PxSphereGeometry oSphere = PxGeometryHolder( pShape->getGeometry() ).sphere();
			oSphere.radius = m_fRadius;
			pShape->setGeometry( oSphere );
			
			PxRigidBodyExt::updateMassAndInertia( *pRigidBody, 1.f );

			pRigidBody->is< PxRigidDynamic >()->wakeUp();
		}
	}
}

REGISTER_COMPONENT( BoxShapeComponent );

BoxShapeComponent::BoxShapeComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void BoxShapeComponent::Update( const GameContext& oGameContext )
{
	// TODO #eric this should only be done in editing, but it would require an initialization priority
	// TODO #eric handle shape removal
	if( m_hRigidBody.IsValid() == false )
	{
		m_hRigidBody = GetComponent< RigidbodyComponent >();

		if( m_hRigidBody.IsValid() )
		{
			PxRigidBody* pRigidBody = m_hRigidBody->GetRigidBody();
			PxRigidActorExt::createExclusiveShape( *pRigidBody, PxBoxGeometry( m_fHalfWidth, m_fHalfHeight, m_fHalfDepth ), *g_pPhysics->m_pMaterial );
			PxRigidBodyExt::updateMassAndInertia( *pRigidBody, 1.f );
		}
	}
}

void BoxShapeComponent::DisplayGizmos( const bool bSelected )
{
	if( bSelected )
	{
		const Transform& oTransform = GetEntity()->GetWorldTransform();
		const glm::vec3 vPosition = oTransform.GetO();

		g_pDebugDisplay->DisplayWireBox( vPosition, glm::vec3( m_fHalfWidth, m_fHalfHeight, m_fHalfDepth ), oTransform.GetMatrixTR(), glm::vec3( 0.f, 1.f, 0.f ) );
	}
}

void BoxShapeComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "HalfWidth" || sProperty == "HalfHeight" || sProperty == "HalfDepth" )
	{
		if( m_hRigidBody.IsValid() )
		{
			PxRigidBody* pRigidBody = m_hRigidBody->GetRigidBody();

			PxShape* pShape = nullptr;
			pRigidBody->getShapes( &pShape, 1 );

			PxBoxGeometry oBox = PxGeometryHolder( pShape->getGeometry() ).box();
			oBox.halfExtents.x = m_fHalfWidth;
			oBox.halfExtents.y = m_fHalfHeight;
			oBox.halfExtents.z = m_fHalfDepth;
			pShape->setGeometry( oBox );

			PxRigidBodyExt::updateMassAndInertia( *pRigidBody, 1.f );

			pRigidBody->is< PxRigidDynamic >()->wakeUp();
		}
	}
}
