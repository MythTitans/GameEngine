#include "FreeCamera.h"

#include <glm/gtc/constants.hpp>

#include "GameEngine.h"
#include "Graphics/Renderer.h"
#include "InputHandler.h"

FreeCamera::FreeCamera()
	: m_vPosition( 10.f, 10.f, -10.f )
	, m_fHorizontalAngle( glm::radians( 45.f ) )
	, m_fVerticalAngle( glm::radians( -37.5f ) )
	, m_fSpeed( 10.f )
	, m_fHorizontalAngleSpeed( glm::radians( 90.f ) )
	, m_fVerticalAngleSpeed( glm::radians( 90.f ) )
{
}

void FreeCamera::Update( const float fDeltaTime )
{
	const float fMoveRight = -g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_MOVE_LEFT ) + g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_MOVE_RIGHT );
	const float fMoveForward = -g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_MOVE_BACKWARD ) + g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_MOVE_FORWARD );
	const float fLookRight = -g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_LOOK_LEFT ) + g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_LOOK_RIGHT );
	const float fLookUp = -g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_LOOK_DOWN ) + g_pInputHandler->GetAxisActionValue( AxisActionID::ACTION_LOOK_UP );

	m_fHorizontalAngle += fLookRight * m_fHorizontalAngleSpeed * fDeltaTime;
	m_fVerticalAngle += fLookUp * m_fVerticalAngleSpeed * fDeltaTime;

	glm::vec3 vUp( 0.f, 1.f, 0.f );
	glm::vec3 vForward = -glm::vec3( glm::cos( m_fHorizontalAngle - glm::half_pi< float >() ), 0.f, glm::sin( m_fHorizontalAngle - glm::half_pi< float >() ) );
	glm::vec3 vRight = glm::cross( vForward, glm::vec3( 0.f, 1.f, 0.f ) );
	vForward = vForward * glm::cos( m_fVerticalAngle ) + vUp * glm::sin( m_fVerticalAngle );
	vUp = glm::cross( vRight, vForward );

	m_vPosition += ( vRight * fMoveRight + vForward * fMoveForward ) *m_fSpeed * fDeltaTime;

	Camera& oCamera = g_pRenderer->GetCamera();
	oCamera.SetPosition( m_vPosition );
	oCamera.SetTarget( m_vPosition + vForward );
	oCamera.SetUp( vUp );

	g_pGameEngine->m_oDebugDisplay.DisplayText( std::format( "Camera position ({:.3f}, {:.3f}, {:.3f})", m_vPosition.x, m_vPosition.y, m_vPosition.z ) );
}
