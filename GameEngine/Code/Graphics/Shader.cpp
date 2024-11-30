#include "Shader.h"

#include "Core/Logger.h"

Shader::Shader()
	: m_uShaderID( 0 )
{
}

void Shader::Create( const std::string& sShaderCode, const ShaderType eShaderType )
{
	m_uShaderID = glCreateShader( GetGLShaderType( eShaderType ) );

	const GLchar* aShaderCode[ 1 ];
	aShaderCode[ 0 ] = sShaderCode.c_str();

	GLint aShaderCodeLength[ 1 ];
	aShaderCodeLength[ 0 ] = ( GLint )sShaderCode.length();

	GLint iCompileResult;
	glShaderSource( m_uShaderID, 1, aShaderCode, aShaderCodeLength );
	glCompileShader( m_uShaderID );

	glGetShaderiv( m_uShaderID, GL_COMPILE_STATUS, &iCompileResult );

	if( iCompileResult == GL_FALSE )
	{
		GLint iCompileLogLength;
		glGetShaderiv( m_uShaderID, GL_INFO_LOG_LENGTH, &iCompileLogLength );

		std::string sCompileLog;
		sCompileLog.resize( iCompileLogLength );

		glGetShaderInfoLog( m_uShaderID, iCompileLogLength, &iCompileLogLength, &sCompileLog[ 0 ] );

		LOG_ERROR( "Shader compilation error : {}", sCompileLog );
	}
}

void Shader::Destroy()
{
	glDeleteShader( m_uShaderID );

	m_uShaderID = 0;
}

GLenum Shader::GetGLShaderType( const ShaderType eShaderType )
{
	switch( eShaderType )
	{
	case ShaderType::VERTEX_SHADER:
		return GL_VERTEX_SHADER;
	case ShaderType::PIXEL_SHADER:
		return GL_FRAGMENT_SHADER;
	case ShaderType::UNDEFINED:
		return GL_INVALID_VALUE;
	}

	return GL_INVALID_VALUE;
}
