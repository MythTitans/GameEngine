#include "Utils.h"

Float2::Float2()
	: Float2( 0.f, 0.f )
{
}

Float2::Float2( const GLfloat fX, const GLfloat fY )
	: m_fX( fX )
	, m_fY( fY )
{
}

Float3::Float3()
	: Float3( 0.f, 0.f, 0.f )
{
}

Float3::Float3( const GLfloat fX, const GLfloat fY, const GLfloat fZ )
	: m_fX( fX )
	, m_fY( fY )
	, m_fZ( fZ )
{
}

Float4::Float4()
	: Float4( 0.f, 0.f, 0.f, 0.f )
{
}

Float4::Float4( const GLfloat fX, const GLfloat fY, const GLfloat fZ, const GLfloat fW )
	: m_fX( fX )
	, m_fY( fY )
	, m_fZ( fZ )
	, m_fW( fW )
{
}
