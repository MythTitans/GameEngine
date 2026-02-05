#include "Color.h"

Color::Color()
	: Color( 1.f, 1.f, 1.f )
{
}

Color::Color( const glm::vec3& vColor )
	: m_vColor( vColor )
{
}

Color::Color( const float fR, const float fG, const float fB )
	: m_vColor( fR, fG, fB )
{
}

Color Color::White()
{
	return Color( 1.f, 1.f, 1.f );
}

Color Color::Black()
{
	return Color( 0.f, 0.f, 0.f );
}

Color Color::Red()
{
	return Color( 1.f, 0.f, 0.f );
}

Color Color::Green()
{
	return Color( 0.f, 1.f, 0.f );
}

Color Color::Blue()
{
	return Color( 0.f, 0.f, 1.f );
}

Color Color::Magenta()
{
	return Color( 1.f, 0.f, 1.f );
}

Color Color::Yellow()
{
	return Color( 1.f, 1.f, 0.f );
}

Color Color::Cyan()
{
	return Color( 0.f, 1.f, 1.f );
}
