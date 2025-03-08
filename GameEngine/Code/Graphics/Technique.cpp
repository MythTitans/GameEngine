#include "Technique.h"

#include <glm/gtc/type_ptr.hpp>

#include "Core/Logger.h"
#include "Shader.h"

Technique::Technique()
	: m_uProgramID( 0 )
{
}

void Technique::Create( const Array< const Shader* > aShaders, const Array< std::string >& aParameters, const Array< std::pair< std::string, uint > > aArrayParameters )
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

	for( const std::string& sParameter : aParameters )
		m_mParameters[ sParameter ] = GetParameterID( sParameter.c_str() );

	for( const std::pair< std::string, uint >& oArrayParameter : aArrayParameters )
		m_mArrayParameters[ oArrayParameter.first ] = GetParameterIDArray( oArrayParameter.first.c_str(), oArrayParameter.second );
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

void Technique::SetParameter( const std::string& sParameter, const int iValue )
{
	glUniform1i( FindParameter( sParameter ), iValue );
}

void Technique::SetParameter( const std::string& sParameter, const uint uValue )
{
	glUniform1ui( FindParameter( sParameter ), uValue );
}

void Technique::SetParameter( const std::string& sParameter, const float fValue )
{
	glUniform1f( FindParameter( sParameter ), fValue );
}

void Technique::SetParameter( const std::string& sParameter, const glm::vec2& vValue )
{
	glUniform2fv( FindParameter( sParameter ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::vec3& vValue )
{
	glUniform3fv( FindParameter( sParameter ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::vec4& vValue )
{
	glUniform4fv( FindParameter( sParameter ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::mat3& mValue )
{
	glUniformMatrix3fv( FindParameter( sParameter ), 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::mat4& mValue )
{
	glUniformMatrix4fv( FindParameter( sParameter ), 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::uvec2& vValue )
{
	glUniform2uiv( FindParameter( sParameter ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::uvec3& vValue )
{
	glUniform3uiv( FindParameter( sParameter ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const glm::uvec4& vValue )
{
	glUniform4uiv( FindParameter( sParameter ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetParameter( const std::string& sParameter, const Texture* pTexture )
{
	glUniform1i( FindParameter( sParameter ), m_aTextures.Count() );
	m_aTextures.PushBack( pTexture );
}

void Technique::SetArrayParameter( const std::string& sParameter, const int iValue, const uint uIndex )
{
	glUniform1i( FindArrayParameter( sParameter, uIndex ), iValue );
}

void Technique::SetArrayParameter( const std::string& sParameter, const uint uValue, const uint uIndex )
{
	glUniform1ui( FindArrayParameter( sParameter, uIndex ), uValue );
}

void Technique::SetArrayParameter( const std::string& sParameter, const float fValue, const uint uIndex )
{
	glUniform1f( FindArrayParameter( sParameter, uIndex ), fValue );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::vec2& vValue, const uint uIndex )
{
	glUniform2fv( FindArrayParameter( sParameter, uIndex ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::vec3& vValue, const uint uIndex )
{
	glUniform3fv( FindArrayParameter( sParameter, uIndex ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::vec4& vValue, const uint uIndex )
{
	glUniform4fv( FindArrayParameter( sParameter, uIndex ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::mat3& mValue, const uint uIndex )
{
	glUniformMatrix3fv( FindArrayParameter( sParameter, uIndex ), 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::mat4& mValue, const uint uIndex )
{
	glUniformMatrix4fv( FindArrayParameter( sParameter, uIndex ), 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::uvec2& vValue, const uint uIndex )
{
	glUniform2uiv( FindArrayParameter( sParameter, uIndex ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::uvec3& vValue, const uint uIndex )
{
	glUniform3uiv( FindArrayParameter( sParameter, uIndex ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const glm::uvec4& vValue, const uint uIndex )
{
	glUniform4uiv( FindArrayParameter( sParameter, uIndex ), 1, glm::value_ptr( vValue ) );
}

void Technique::SetArrayParameter( const std::string& sParameter, const Texture* pTexture, const uint uIndex )
{
	glUniform1i( FindArrayParameter( sParameter, uIndex ), m_aTextures.Count() );
	m_aTextures.PushBack( pTexture );
}

bool Technique::HasParameter( const std::string& sParameter ) const
{
	return m_mParameters.find( sParameter ) != std::end( m_mParameters );
}

bool Technique::HasArrayParameter( const std::string& sParameter ) const
{
	return m_mArrayParameters.find( sParameter ) != std::end( m_mArrayParameters );
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

GLint Technique::FindParameter( const std::string& sParameter )
{
	const auto oIterator = m_mParameters.find( sParameter );
	if( oIterator == std::end( m_mParameters ) )
	{
		ASSERT( false );
		return -1;
	}

	return oIterator->second;
}

GLint Technique::FindArrayParameter( const std::string& sParameter, const uint uIndex )
{
	const auto oIterator = m_mArrayParameters.find( sParameter );
	if( oIterator == std::end( m_mArrayParameters ) )
	{
		ASSERT( false );
		return -1;
	}

	return oIterator->second[ uIndex ];
}
