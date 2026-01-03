#pragma once

#include <GL/glew.h>

#include "Core/Common.h"
#include "Core/Types.h"

struct ShaderBufferDesc
{
	ShaderBufferDesc();

	ShaderBufferDesc& Static( const bool bAllowUpdate = false );
	ShaderBufferDesc& Dynamic();

	bool m_bStatic;
	bool m_bAllowUpdate;
};

class ShaderBufferBase
{
public:
	ShaderBufferBase();

	void	Destroy();

	GLuint	GetID() const;

protected:
	GLuint	m_uShaderBufferId;
	bool	m_bAllowUpdate;
};

template < typename ShaderData >
class ShaderBuffer : public ShaderBufferBase
{
public:
	void Create( const ShaderBufferDesc& oDesc )
	{
		glGenBuffers( 1, &m_uShaderBufferId );
		glBindBuffer( GL_UNIFORM_BUFFER, m_uShaderBufferId );
		glBufferData( GL_UNIFORM_BUFFER, sizeof( ShaderData ), nullptr, oDesc.m_bStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );
		glBindBuffer( GL_UNIFORM_BUFFER, 0 );

		m_bAllowUpdate = oDesc.m_bAllowUpdate;
	}

	void Create( const ShaderBufferDesc& oDesc, const ShaderData& oShaderData )
	{
		glGenBuffers( 1, &m_uShaderBufferId );
		glBindBuffer( GL_UNIFORM_BUFFER, m_uShaderBufferId );
		glBufferData( GL_UNIFORM_BUFFER, sizeof( ShaderData ), &oShaderData, oDesc.m_bStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );
		glBindBuffer( GL_UNIFORM_BUFFER, 0 );

		m_bAllowUpdate = oDesc.m_bAllowUpdate;
	}

	void Update( const ShaderData& oShaderData )
	{
		ASSERT( m_bAllowUpdate );
		if( m_bAllowUpdate )
		{
			glBindBuffer( GL_UNIFORM_BUFFER, m_uShaderBufferId );
			glBufferSubData( GL_UNIFORM_BUFFER, 0, sizeof( ShaderData ), &oShaderData );
			glBindBuffer( GL_UNIFORM_BUFFER, 0 );
		}
	}
};
