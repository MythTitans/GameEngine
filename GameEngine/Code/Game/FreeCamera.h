#pragma once

#include <glm/glm.hpp>

class FreeCamera
{
public:
	FreeCamera();

	void Update( const float fDeltaTime );

	float		m_fSpeed;
	float		m_fFastSpeedMultiplier;
	float		m_fHorizontalAngleSpeed;
	float		m_fVerticalAngleSpeed;

private:
	glm::vec3	m_vPosition;
	float		m_fHorizontalAngle;
	float		m_fVerticalAngle;
};