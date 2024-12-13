#pragma once

#include <glm/glm.hpp>

class Camera
{
public:
	friend class Renderer;

	Camera();

	void				SetPosition( const glm::vec3& vPosition );
	void				SetTarget( const glm::vec3& vTarget );
	void				SetUp( const glm::vec3& vUp );

	void				SetNear( const float fNear );
	void				SetFar( const float fFar );
	void				SetFov( const float fFov );
	void				SetAspectRatio( const float fAspectRatio );

	const glm::mat4&	GetViewMatrix() const;
	const glm::mat4&	GetProjectionMatrix() const;
	const glm::mat4&	GetViewProjectionMatrix() const;
	const glm::mat4&	GetInverseViewProjectionMatrix() const;

private:
	glm::vec3			m_vPosition;
	glm::vec3			m_vTarget;
	glm::vec3			m_vUp;

	float				m_fNear;
	float				m_fFar;
	float				m_fFov;
	float				m_fAspectRatio;

	mutable	glm::mat4	m_mView;
	mutable	glm::mat4	m_mProjection;
	mutable	glm::mat4	m_mViewProjection;
	mutable	glm::mat4	m_mInverseViewProjection;

	mutable bool		m_bViewDirty;
	mutable bool		m_bProjectionDirty;
	mutable bool		m_bViewProjectionDirty;
	mutable bool		m_bInverseViewProjectionDirty;
};