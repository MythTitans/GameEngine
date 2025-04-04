#include "Camera.h"

#include <glm/gtc/matrix_inverse.hpp>
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera()
	: m_vPosition( 0.f, 0.f, 0.f )
	, m_vTarget( 0.f, 0.f, -1.f )
	, m_vUp( 0.f, 1.f, 0.f )
	, m_fNear( 0.2f )
	, m_fFar( 200.f )
	, m_fFov( glm::radians( 60.f ) )
	, m_fAspectRatio( 16.f / 9.f )
	, m_bViewDirty( true )
	, m_bProjectionDirty( true )
	, m_bViewProjectionDirty( true )
	, m_bInverseViewProjectionDirty( true )
{
}

void Camera::SetPosition( const glm::vec3& vPosition )
{
	m_vPosition = vPosition;

	m_bViewDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

const glm::vec3& Camera::GetPosition() const
{
	return m_vPosition;
}

void Camera::SetTarget( const glm::vec3& vTarget )
{
	m_vTarget = vTarget;

	m_bViewDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

void Camera::SetUp( const glm::vec3& vUp )
{
	m_vUp = vUp;

	m_bViewDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

void Camera::SetNear( const float fNear )
{
	m_fNear = fNear;

	m_bProjectionDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

void Camera::SetFar( const float fFar )
{
	m_fFar = fFar;

	m_bProjectionDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

void Camera::SetFov( const float fFov )
{
	m_fFov = fFov;

	m_bProjectionDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

void Camera::SetAspectRatio( const float fAspectRatio )
{
	m_fAspectRatio = fAspectRatio;

	m_bProjectionDirty = true;
	m_bViewProjectionDirty = true;
	m_bInverseViewProjectionDirty = true;
}

const glm::mat4& Camera::GetViewMatrix() const
{
	if( m_bViewDirty )
	{
		m_mView = glm::lookAt( m_vPosition, m_vTarget, m_vUp );
		m_bViewDirty = false;
	}

	return m_mView;
}

const glm::mat4& Camera::GetProjectionMatrix() const
{
	if( m_bProjectionDirty )
	{
		m_mProjection = glm::perspective( m_fFov, m_fAspectRatio, m_fNear, m_fFar );
		m_bProjectionDirty = false;
	}

	return m_mProjection;
}

const glm::mat4& Camera::GetViewProjectionMatrix() const
{
	if( m_bViewProjectionDirty )
	{
		m_mViewProjection = GetProjectionMatrix() * GetViewMatrix();
		m_bViewProjectionDirty = false;
	}

	return m_mViewProjection;
}

const glm::mat4& Camera::GetInverseViewProjectionMatrix() const
{
	if( m_bInverseViewProjectionDirty )
	{
		m_mInverseViewProjection = glm::inverse( GetViewProjectionMatrix() );
		m_bInverseViewProjectionDirty = false;
	}

	return m_mInverseViewProjection;
}
