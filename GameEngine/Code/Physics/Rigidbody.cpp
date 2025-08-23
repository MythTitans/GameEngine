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
	, m_pRigidActor( nullptr )
	, m_fTime( 0.f )
{
}

void RigidbodyComponent::Initialize()
{
	if( m_bStatic )
		m_pRigidActor = g_pPhysics->m_pPhysics->createRigidStatic( PxTransform( PxIdentity ) );
	else
		m_pRigidActor = g_pPhysics->m_pPhysics->createRigidDynamic( PxTransform( PxIdentity ) );
}

void RigidbodyComponent::Start()
{
	g_pPhysics->m_pScene->addActor( *m_pRigidActor );

	Entity* pEntity = GetEntity();

	const glm::vec3 vPosition = pEntity->GetWorldPosition();
	const glm::quat qRotation = pEntity->GetRotation();

	m_oLastTransform = PxTransform( PxVec3( vPosition.x, vPosition.y, vPosition.z ), PxQuat( qRotation.x, qRotation.y, qRotation.z, qRotation.w ) );
	m_oTransform = m_oLastTransform;

	m_fTime = 0.f;

	m_pRigidActor->setGlobalPose( m_oTransform );
}

void RigidbodyComponent::Stop()
{
	g_pPhysics->m_pScene->removeActor( *m_pRigidActor );
}

void RigidbodyComponent::AfterPhysics()
{
	m_oLastTransform = m_oTransform;
	m_oTransform = m_pRigidActor->getGlobalPose();

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

		m_pRigidActor->setGlobalPose( m_oTransform );
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
	PX_RELEASE( m_pRigidActor );
}

void RigidbodyComponent::OnPropertyChanged( const std::string& sProperty )
{
	if( sProperty == "Static" )
	{
		g_pPhysics->m_pScene->removeActor( *m_pRigidActor );
		PX_RELEASE( m_pRigidActor );

		if( m_bStatic )
			m_pRigidActor = g_pPhysics->m_pPhysics->createRigidStatic( PxTransform( PxIdentity ) );
		else
			m_pRigidActor = g_pPhysics->m_pPhysics->createRigidDynamic( PxTransform( PxIdentity ) );

		g_pPhysics->m_pScene->addActor( *m_pRigidActor );

		m_pRigidActor->setGlobalPose( m_oTransform );
	}
}

physx::PxRigidActor* RigidbodyComponent::GetRigidActor()
{
	return m_pRigidActor;
}

const physx::PxRigidActor* RigidbodyComponent::GetRigidActor() const
{
	return m_pRigidActor;
}

ShapeComponentBase::ShapeComponentBase( Entity* pEntity )
	: Component( pEntity )
	, m_pShape( nullptr )
{
}

void ShapeComponentBase::Initialize()
{
	m_hRigidBody = GetComponent< RigidbodyComponent >();
	m_pShape = CreateShape();
}

void ShapeComponentBase::Start()
{
	PxRigidActor* pRigidActor = m_hRigidBody->GetRigidActor();
	pRigidActor->attachShape( *m_pShape );

	PxRigidBody* pRigidBody = pRigidActor->is< PxRigidBody >();
	if( pRigidBody != nullptr )
		PxRigidBodyExt::updateMassAndInertia( *pRigidBody, 1.f );
}

void ShapeComponentBase::Update( const GameContext& oGameContext )
{
	if( oGameContext.m_bEditing )
	{
		PxRigidActor* pRigidActor = m_hRigidBody->GetRigidActor();
		Array< PxShape* > aShapes( pRigidActor->getNbShapes() );
		pRigidActor->getShapes( aShapes.Data(), aShapes.Count() );

		bool bFound = false;
		for( const PxShape* pShape : aShapes )
		{
			if( pShape == m_pShape )
			{
				bFound = true;
				break;
			}
		}

		if( bFound == false )
			pRigidActor->attachShape( *m_pShape );
	}
}

void ShapeComponentBase::Stop()
{
	PxRigidActor* pRigidActor = m_hRigidBody->GetRigidActor();
	pRigidActor->detachShape( *m_pShape );
}

void ShapeComponentBase::Dispose()
{
	PX_RELEASE( m_pShape );
}

void ShapeComponentBase::UpdateShape()
{
	PxRigidActor* pRigidActor = m_hRigidBody->GetRigidActor();

	pRigidActor->detachShape( *m_pShape );
	PX_RELEASE( m_pShape );

	m_pShape = CreateShape();

	pRigidActor->attachShape( *m_pShape );

	PxRigidBody* pRigidBody = pRigidActor->is< PxRigidBody >();
	if( pRigidBody != nullptr )
		PxRigidBodyExt::updateMassAndInertia( *pRigidBody, 1.f );
}

REGISTER_COMPONENT( SphereShapeComponent, RigidbodyComponent );

SphereShapeComponent::SphereShapeComponent( Entity* pEntity )
	: ShapeComponentBase( pEntity )
{
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
		UpdateShape();
}

physx::PxShape* SphereShapeComponent::CreateShape()
{
	return g_pPhysics->m_pPhysics->createShape( PxSphereGeometry( m_fRadius ), *g_pPhysics->m_pMaterial );
}

REGISTER_COMPONENT( BoxShapeComponent, RigidbodyComponent );

BoxShapeComponent::BoxShapeComponent( Entity* pEntity )
	: ShapeComponentBase( pEntity )
{
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
		UpdateShape();
}

physx::PxShape* BoxShapeComponent::CreateShape()
{
	return g_pPhysics->m_pPhysics->createShape( PxBoxGeometry( m_fHalfWidth, m_fHalfHeight, m_fHalfDepth ), *g_pPhysics->m_pMaterial );
}
