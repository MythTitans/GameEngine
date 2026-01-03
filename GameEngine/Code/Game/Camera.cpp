#include "Camera.h"

#include "Entity.h"
#include "CameraManager.h"
#include "Graphics/Camera.h"

CameraComponentBase::CameraComponentBase( Entity* pEntity )
	: Component( pEntity )
{
}

REGISTER_COMPONENT( CameraComponent );

CameraComponent::CameraComponent( Entity* pEntity )
	: CameraComponentBase( pEntity )
{
}

void CameraComponent::Start()
{
	g_pCameraManager->SetActiveCamera< CameraComponent >( this );
}

void CameraComponent::ApplyCamera( Camera& oCamera )
{
	const Transform& oTransform = GetEntity()->GetTransform();

	oCamera.SetPosition( oTransform.GetO() );
	oCamera.SetFov( glm::radians( m_fFov ) );

	Entity* pTarget = m_oTarget.GetEntity();
	if( pTarget != nullptr )
	{
		const glm::vec3 vForward = glm::normalize( pTarget->GetWorldPosition() - oTransform.GetO() );
		const glm::vec3 vWorldUp( 0.f, 1.f, 0.f );
		const glm::vec3 vLeft = glm::normalize( glm::cross( vWorldUp, vForward ) );
		const glm::vec3 vUp = glm::normalize( glm::cross( vForward, vLeft ) );
		oCamera.SetTarget( pTarget->GetWorldPosition() );
		oCamera.SetUp( vUp );
	}
	else
	{
		oCamera.SetTarget( oTransform.GetO() + oTransform.GetK() );
		oCamera.SetUp( oTransform.GetJ() );
	}
}
