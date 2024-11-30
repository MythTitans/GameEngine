#include "Technique.h"

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

GLuint Technique::GetUniformLocation( const char* sUniform ) const
{
	return glGetUniformLocation( m_uProgramID, sUniform );
}
