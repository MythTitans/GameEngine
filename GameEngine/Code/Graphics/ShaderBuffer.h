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
		glCreateBuffers( 1, &m_uShaderBufferId );
		glNamedBufferData( m_uShaderBufferId, sizeof( ShaderData ), nullptr, oDesc.m_bStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );

		m_bAllowUpdate = oDesc.m_bAllowUpdate;
	}

	void Create( const ShaderBufferDesc& oDesc, const ShaderData& oShaderData )
	{
		glCreateBuffers( 1, &m_uShaderBufferId );
		glNamedBufferData( m_uShaderBufferId, sizeof( ShaderData ), &oShaderData, oDesc.m_bStatic ? GL_STATIC_DRAW : GL_DYNAMIC_DRAW );

		m_bAllowUpdate = oDesc.m_bAllowUpdate;
	}

	void Update( const ShaderData& oShaderData )
	{
		ASSERT( m_bAllowUpdate );
		if( m_bAllowUpdate )
			glNamedBufferSubData( m_uShaderBufferId, 0, sizeof( ShaderData ), &oShaderData );
	}
};
