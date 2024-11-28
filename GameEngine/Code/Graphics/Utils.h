#pragma once

#include <GL/glew.h>

struct Float2
{
	Float2();
	Float2( const GLfloat fX, const GLfloat fY );

	GLfloat m_fX;
	GLfloat m_fY;
};

struct Float3
{
	Float3();
	Float3( const GLfloat fX, const GLfloat fY, const GLfloat fZ );

	GLfloat m_fX;
	GLfloat m_fY;
	GLfloat m_fZ;
};

struct Float4
{
	Float4();
	Float4( const GLfloat fX, const GLfloat fY, const GLfloat fZ, const GLfloat fW );

	GLfloat m_fX;
	GLfloat m_fY;
	GLfloat m_fZ;
	GLfloat m_fW;
};