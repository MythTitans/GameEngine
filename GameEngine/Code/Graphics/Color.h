#pragma once

#include <glm/fwd.hpp>

struct Color
{
	Color();
	explicit Color( const glm::vec3& vColor );
	Color( const float fR, const float fG, const float fB );

	explicit operator glm::vec3() const;

	float m_fR;
	float m_fG;
	float m_fB;

	static Color White();
	static Color Black();
	static Color Red();
	static Color Green();
	static Color Blue();
	static Color Magenta();
	static Color Yellow();
	static Color Cyan();
};
