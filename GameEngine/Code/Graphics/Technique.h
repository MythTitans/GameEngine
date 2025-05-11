#pragma once

#include <unordered_map>

#include <GL/glew.h>
#include <glm/fwd.hpp>

#include "Core/Array.h"
#include "Core/Types.h"
#include "Texture.h"

class Shader;
struct VisualNode;

class Technique
{
public:
	friend class DebugRenderer;
	friend class Renderer;
	friend class TextRenderer;
	friend class GizmoRenderer;

	template < typename Technique >
	friend void DrawMeshes( const Array< VisualNode* >& aVisualNodes, Technique& oTechnique );

	Technique();

	void			Create( const Array< const Shader* > aShaders, const Array< std::string >& aParameters, const Array< std::pair< std::string, uint > > aArrayParameters );
	void			Destroy();

	bool			IsValid() const;

	void			SetParameter( const std::string& sParameter, const int iValue );
	void			SetParameter( const std::string& sParameter, const uint uValue );
	void			SetParameter( const std::string& sParameter, const float fValue );
	void			SetParameter( const std::string& sParameter, const glm::vec2& vValue );
	void			SetParameter( const std::string& sParameter, const glm::vec3& vValue );
	void			SetParameter( const std::string& sParameter, const glm::vec4& vValue );
	void			SetParameter( const std::string& sParameter, const glm::uvec2& vValue );
	void			SetParameter( const std::string& sParameter, const glm::uvec3& vValue );
	void			SetParameter( const std::string& sParameter, const glm::uvec4& vValue );
	void			SetParameter( const std::string& sParameter, const glm::mat3& mValue );
	void			SetParameter( const std::string& sParameter, const glm::mat4& mValue );
	void			SetParameter( const std::string& sParameter, const Texture* pTexture );

	void			SetArrayParameter( const std::string& sParameter, const int iValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const uint uValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const float fValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::vec2& vValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::vec3& vValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::vec4& vValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::uvec2& vValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::uvec3& vValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::uvec4& vValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::mat3& mValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const glm::mat4& mValue, const uint uIndex );
	void			SetArrayParameter( const std::string& sParameter, const Texture* pTexture, const uint uIndex );

	bool			HasParameter( const std::string& sParameter ) const;
	bool			HasArrayParameter( const std::string& sParameter ) const;

private:
	GLint			GetParameterID( const char* sUniform ) const;
	Array< GLint >	GetParameterIDArray( const char* sUniform, const uint uCount ) const;

	GLint			FindParameter( const std::string& sParameter );
	GLint			FindArrayParameter( const std::string& sParameter, const uint uIndex );

	GLuint					m_uProgramID;

	using ParametersMap = std::unordered_map< std::string, GLint >;
	ParametersMap			m_mParameters;

	using ArrayParametersMap = std::unordered_map< std::string, Array< GLint > >;
	ArrayParametersMap		m_mArrayParameters;

	Array< const Texture* >	m_aTextures;
};