#pragma once

#include <GL/glew.h>

#include "Core/Array.h"

class Shader;

class Technique
{
public:
	friend class Renderer;

	Technique();

	void	Create( const Array< const Shader* > aShaders );
	void	Destroy();

	GLuint	GetParameterID( const char* sUniform ) const;

private:
	GLuint m_uProgramID;
};