#include "Shader.h"

#include "Core/Logger.h"

Shader::Shader()
	: m_uShaderID( 0 )
{
}

void Shader::Create( const std::string& sShaderCode, const ShaderType eShaderType )
{
	m_uShaderID = glCreateShader( GetGLShaderType( eShaderType ) );

	const std::string sVersion = "#version 330 core\n";
	const std::string sContent = ( "#line 1\n" + sShaderCode );

	const uint uShaderPartCount = 2;

	const GLchar* aShaderCode[ uShaderPartCount ];
	aShaderCode[ 0 ] = sVersion.c_str();
	aShaderCode[ 1 ] = sContent.c_str();

	GLint aShaderCodeLength[ uShaderPartCount ];
	aShaderCodeLength[ 0 ] = ( GLint )sVersion.length();
	aShaderCodeLength[ 1 ] = ( GLint )sContent.length();

	GLint iCompileResult;
	glShaderSource( m_uShaderID, uShaderPartCount, aShaderCode, aShaderCodeLength );
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
		ASSERT( false );
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
