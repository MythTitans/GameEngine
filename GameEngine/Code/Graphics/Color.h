#pragma once

#include <glm/glm.hpp>

struct Color
{
	Color();
	explicit Color( const glm::vec3& vColor );
	Color( const float fR, const float fG, const float fB );

	glm::vec3 m_vColor;

	static Color White();
	static Color Black();
	static Color Red();
	static Color Green();
	static Color Blue();
	static Color Magenta();
	static Color Yellow();
	static Color Cyan();
};
