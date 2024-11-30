#pragma once

#include <string>

#include <GL/glew.h>

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

	void Create( const std::string& sShaderCode, const ShaderType eShaderType );
	void Destroy();

private:
	static GLenum GetGLShaderType( const ShaderType eShaderType );

	GLuint m_uShaderID;
};