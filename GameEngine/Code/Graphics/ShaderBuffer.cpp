#include "ShaderBuffer.h"

ShaderBufferDesc::ShaderBufferDesc()
	: m_bStatic( true )
	, m_bAllowUpdate( false )
{
}

ShaderBufferDesc& ShaderBufferDesc::Static( const bool bAllowUpdate /*= false */ )
{
	m_bStatic = true;
	m_bAllowUpdate = bAllowUpdate;

	return *this;
}

ShaderBufferDesc& ShaderBufferDesc::Dynamic()
{
	m_bStatic = true;
	m_bAllowUpdate = true;

	return *this;
}

ShaderBufferBase::ShaderBufferBase()
	: m_uShaderBufferId( GL_INVALID_VALUE )
	, m_bAllowUpdate( false )
{
}

void ShaderBufferBase::Destroy()
{
	glDeleteBuffers( 1, &m_uShaderBufferId );

	m_uShaderBufferId = GL_INVALID_VALUE;
	m_bAllowUpdate = false;
}

GLuint ShaderBufferBase::GetID() const
{
	return m_uShaderBufferId;
}
