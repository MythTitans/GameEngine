#pragma once

#include <GL/glew.h>

#include "Core/Array.h"

class Shader;

class Technique
{
public:
	Technique();

	void Create( const Array< const Shader* > aShaders );
	void Destroy();

private:
	GLuint m_uProgramID;
};