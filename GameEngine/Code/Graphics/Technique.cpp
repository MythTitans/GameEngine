#include "Technique.h"

#include <glm/gtc/type_ptr.hpp>

#include "Core/Logger.h"
#include "Shader.h"

Technique::Technique()
	: m_uProgramID( 0 )
{
}

void Technique::Create( const Array< const Shader* > aShaders )
{
	m_uProgramID = glCreateProgram();

	for( const Shader* pShader : aShaders )
		glAttachShader( m_uProgramID, pShader->m_uShaderID );

	glLinkProgram( m_uProgramID );

	GLint iLinkResult;
	glGetProgramiv( m_uProgramID, GL_LINK_STATUS, &iLinkResult );

	if( iLinkResult == GL_FALSE )
	{
		GLint iLinkLogLength;
		glGetProgramiv( m_uProgramID, GL_INFO_LOG_LENGTH, &iLinkLogLength );

		std::string sLinkLog;
		sLinkLog.resize( iLinkLogLength );

		glGetProgramInfoLog( m_uProgramID, iLinkLogLength, &iLinkLogLength, &sLinkLog[ 0 ] );

		LOG_ERROR( "Technique link error : {}", sLinkLog );
	}
}

void Technique::Destroy()
{
	glDeleteProgram( m_uProgramID );

	m_uProgramID = 0;
}

bool Technique::IsValid() const
{
	return m_uProgramID != 0;
}

GLint Technique::GetParameterID( const char* sParameter ) const
{
	GLint iUniform = glGetUniformLocation( m_uProgramID, sParameter );
	ASSERT( iUniform != -1 );
	return iUniform;
}

Array< GLint > Technique::GetParameterIDArray( const char* sUniform, const uint uCount ) const
{
	Array< GLint > aUniforms( uCount, -1 );
	for( uint u = 0; u < uCount; ++u )
	{
		aUniforms[ u ] = GetParameterID( std::format( "{}[{}]", sUniform, u ).c_str() );
		ASSERT( aUniforms[ u ] != -1 );
	}

	return aUniforms;
}
