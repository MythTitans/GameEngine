#pragma once

#include <unordered_map>

#include <GL/glew.h>
#include <glm/fwd.hpp>

#include "Core/Array.h"
#include "Core/Types.h"
#include "Texture.h"

class Shader;
class Technique;
struct VisualNode;

class TechniqueParameter
{
public:
	TechniqueParameter();
	explicit TechniqueParameter( const GLint iParameterID );

	bool IsValid() const;

	void SetValue( const int iValue );
	void SetValue( const uint uValue );
	void SetValue( const float fValue );
	void SetValue( const glm::vec2& vValue );
	void SetValue( const glm::vec3& vValue );
	void SetValue( const glm::vec4& vValue );
	void SetValue( const glm::uvec2& vValue );
	void SetValue( const glm::uvec3& vValue );
	void SetValue( const glm::uvec4& vValue );
	void SetValue( const glm::mat3& mValue );
	void SetValue( const glm::mat4& mValue );
	void SetValue( const Texture* pTexture, Technique& oTechnique );

private:
	GLint m_iParameterID;
};

class TechniqueArrayParameter
{
public:
	TechniqueArrayParameter();
	explicit TechniqueArrayParameter( const Array< GLint >& aParametersIDs );

	bool IsValid() const;

	void SetValue( const int iValue, const uint uIndex );
	void SetValue( const uint uValue, const uint uIndex );
	void SetValue( const float fValue, const uint uIndex );
	void SetValue( const glm::vec2& vValue, const uint uIndex );
	void SetValue( const glm::vec3& vValue, const uint uIndex );
	void SetValue( const glm::vec4& vValue, const uint uIndex );
	void SetValue( const glm::uvec2& vValue, const uint uIndex );
	void SetValue( const glm::uvec3& vValue, const uint uIndex );
	void SetValue( const glm::uvec4& vValue, const uint uIndex );
	void SetValue( const glm::mat3& mValue, const uint uIndex );
	void SetValue( const glm::mat4& mValue, const uint uIndex );
	void SetValue( const Texture* pTexture, const uint uIndex, Technique& oTechnique );

private:
	Array< GLint >	m_aParametersIDs;
	bool			m_bValid;
};

class Technique
{
public:
	friend class DebugRenderer;
	friend class GizmoRenderer;
	friend class Renderer;
	friend class TechniqueArrayParameter;
	friend class TechniqueParameter;
	friend class TextRenderer;

	template < typename Binding, uint uCount >
	friend class TechniqueSheet;

	template < typename Technique >
	friend void DrawMeshes( const Array< VisualNode* >& aVisualNodes, Technique& oTechnique );

	Technique();

	void						Create( const Array< const Shader* > aShaders, const Array< std::string >& aParameters, const Array< std::pair< std::string, uint > > aArrayParameters );
	void						Destroy();

	bool						IsValid() const;

	template < typename T >
	void						SetParameter( const std::string& sParameter, T oValue )
	{
		GetParameter( sParameter ).SetValue( oValue );
	}

	template <>
	void						SetParameter( const std::string& sParameter, const Texture* pTexture )
	{
		GetParameter( sParameter ).SetValue( pTexture, *this );
	}

	TechniqueParameter&			GetParameter( const std::string& sParameter );
	TechniqueArrayParameter&	GetArrayParameter( const std::string& sParameter );

private:
	GLint						GetParameterID( const char* sUniform ) const;
	Array< GLint >				GetParameterIDArray( const char* sUniform, const uint uCount ) const;

	GLuint							m_uProgramID;

	using ParametersMap = std::unordered_map< std::string, TechniqueParameter >;
	ParametersMap					m_mParameters;

	using ArrayParametersMap = std::unordered_map< std::string, TechniqueArrayParameter >;
	ArrayParametersMap				m_mArrayParameters;

	Array< const Texture* >			m_aTextures;

	static TechniqueParameter		s_oMissingParameter;
	static TechniqueArrayParameter	s_oMissingArrayParameter;
};

template < typename Binding, uint uCount >
class TechniqueSheet
{
public:
	TechniqueSheet()
		: m_pTechnique( nullptr )
	{
		for( uint u = 0; u < uCount; ++u )
		{
			m_aParameters[ u ] = &Technique::s_oMissingParameter;
			m_aArrayParameters[ u ] = &Technique::s_oMissingArrayParameter;
		}
	}

	void Init( Technique& oTechnique )
	{
		m_pTechnique = &oTechnique;
	}

	void BindParameter( const Binding& oValue, const std::string& sParameter )
	{
		ASSERT( m_pTechnique != nullptr );
		m_aParameters[ ( uint )oValue ] = &m_pTechnique->GetParameter( sParameter );
	}

	void BindArrayParameter( const Binding& oValue, const std::string& sParameter )
	{
		ASSERT( m_pTechnique != nullptr );
		m_aArrayParameters[ ( uint )oValue ] = &m_pTechnique->GetArrayParameter( sParameter );
	}

	TechniqueParameter& GetParameter( const Binding& oValue )
	{
		return *m_aParameters[ ( uint )oValue ];
	}

	TechniqueArrayParameter& GetArrayParameter( const Binding& oValue )
	{
		return *m_aArrayParameters[ ( uint )oValue ];
	}

private:
	Technique*					m_pTechnique;
	TechniqueParameter*			m_aParameters[ uCount ];
	TechniqueArrayParameter*	m_aArrayParameters[ uCount ];
};

#define PARAM_SHEET( Binding ) TechniqueSheet< Binding, ( uint ) Binding::_COUNT >