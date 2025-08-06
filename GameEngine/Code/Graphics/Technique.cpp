#include "Technique.h"

#include <glm/gtc/type_ptr.hpp>

#include "Core/Logger.h"
#include "Shader.h"

TechniqueParameter::TechniqueParameter()
	: m_iParameterID( -1 )
{
}

TechniqueParameter::TechniqueParameter( const GLint iParameterID )
	: m_iParameterID( iParameterID )
{
}

bool TechniqueParameter::IsValid() const
{
	return m_iParameterID != -1;
}

void TechniqueParameter::SetValue( const int iValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform1i( m_iParameterID, iValue );
}

void TechniqueParameter::SetValue( const uint uValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform1ui( m_iParameterID, uValue );
}

void TechniqueParameter::SetValue( const float fValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform1f( m_iParameterID, fValue );
}

void TechniqueParameter::SetValue( const glm::vec2& vValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform2fv( m_iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueParameter::SetValue( const glm::vec3& vValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform3fv( m_iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueParameter::SetValue( const glm::vec4& vValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform4fv( m_iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueParameter::SetValue( const glm::mat3& mValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniformMatrix3fv( m_iParameterID, 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void TechniqueParameter::SetValue( const glm::mat4& mValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniformMatrix4fv( m_iParameterID, 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void TechniqueParameter::SetValue( const glm::uvec2& vValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform2uiv( m_iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueParameter::SetValue( const glm::uvec3& vValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform3uiv( m_iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueParameter::SetValue( const glm::uvec4& vValue )
{
	ASSERT( m_iParameterID != -1 );
	glUniform4uiv( m_iParameterID, 1, glm::value_ptr( vValue ) );
}

void TechniqueParameter::SetValue( const Texture* pTexture, Technique& oTechnique )
{
	ASSERT( m_iParameterID != -1 );
	glUniform1i( m_iParameterID, oTechnique.m_aTextures.Count() );
	oTechnique.m_aTextures.PushBack( pTexture );
}

TechniqueArrayParameter::TechniqueArrayParameter()
	: m_bValid( false )
{
}

TechniqueArrayParameter::TechniqueArrayParameter( const Array< GLint >& aParametersIDs )
	: m_aParametersIDs( aParametersIDs )
	, m_bValid( m_aParametersIDs.Empty() == false )
{
	for( const GLint iParameterID : m_aParametersIDs )
	{
		if( iParameterID == -1 )
		{
			m_bValid = false;
			break;
		}
	}
}

bool TechniqueArrayParameter::IsValid() const
{
	return m_bValid;
}

void TechniqueArrayParameter::SetValue( const int iValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform1i( m_aParametersIDs[ uIndex ], iValue );
}

void TechniqueArrayParameter::SetValue( const uint uValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform1ui( m_aParametersIDs[ uIndex ], uValue );
}

void TechniqueArrayParameter::SetValue( const float fValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform1f( m_aParametersIDs[ uIndex ], fValue );
}

void TechniqueArrayParameter::SetValue( const glm::vec2& vValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform2fv( m_aParametersIDs[ uIndex ], 1, glm::value_ptr( vValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::vec3& vValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform3fv( m_aParametersIDs[ uIndex ], 1, glm::value_ptr( vValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::vec4& vValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform4fv( m_aParametersIDs[ uIndex ], 1, glm::value_ptr( vValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::mat3& mValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniformMatrix3fv( m_aParametersIDs[ uIndex ], 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::mat4& mValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniformMatrix4fv( m_aParametersIDs[ uIndex ], 1, GL_FALSE, glm::value_ptr( mValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::uvec2& vValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform2uiv( m_aParametersIDs[ uIndex ], 1, glm::value_ptr( vValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::uvec3& vValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform3uiv( m_aParametersIDs[ uIndex ], 1, glm::value_ptr( vValue ) );
}

void TechniqueArrayParameter::SetValue( const glm::uvec4& vValue, const uint uIndex )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform4uiv( m_aParametersIDs[ uIndex ], 1, glm::value_ptr( vValue ) );
}

void TechniqueArrayParameter::SetValue( const Texture* pTexture, const uint uIndex, Technique& oTechnique )
{
	ASSERT( m_aParametersIDs[ uIndex ] != -1 );
	glUniform1i( m_aParametersIDs[ uIndex ], oTechnique.m_aTextures.Count() );
	oTechnique.m_aTextures.PushBack( pTexture );
}

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
		m_mParameters[ sParameter ] = TechniqueParameter( GetParameterID( sParameter.c_str() ) );

	for( const std::pair< std::string, uint >& oArrayParameter : aArrayParameters )
		m_mArrayParameters[ oArrayParameter.first ] = TechniqueArrayParameter( GetParameterIDArray( oArrayParameter.first.c_str(), oArrayParameter.second ) );
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

TechniqueParameter& Technique::GetParameter( const std::string& sParameter )
{
	const auto oIterator = m_mParameters.find( sParameter );
	if( oIterator == std::end( m_mParameters ) )
		return s_oMissingParameter;

	return oIterator->second;
}

TechniqueArrayParameter& Technique::GetArrayParameter( const std::string& sParameter )
{
	const auto oIterator = m_mArrayParameters.find( sParameter );
	if( oIterator == std::end( m_mArrayParameters ) )
		return s_oMissingArrayParameter;

	return oIterator->second;
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

TechniqueParameter Technique::s_oMissingParameter;
TechniqueArrayParameter Technique::s_oMissingArrayParameter;
