#pragma once

#include <string>

#include <GL/glew.h>

template < typename T >
class Array;

enum class ShaderType
{
	UNDEFINED,
	VERTEX_SHADER,
	PIXEL_SHADER
};

class Shader
{
public:
	friend class Renderer;
	friend class Technique;

	Shader();

	void Create( const std::string& sShaderCode, const ShaderType eShaderType, const Array< std::string >& aFlags );
	void Destroy();

private:
	static GLenum GetGLShaderType( const ShaderType eShaderType );

	GLuint m_uShaderID;
};