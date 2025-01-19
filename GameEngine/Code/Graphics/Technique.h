#pragma once

#include <GL/glew.h>

#include "Core/Array.h"

class Shader;

class Technique
{
public:
	friend class DebugRenderer;
	friend class Renderer;
	friend class TextRenderer;
	friend class GizmoRenderer;

	Technique();

	void			Create( const Array< const Shader* > aShaders );
	void			Destroy();

	bool			IsValid() const;

	GLint			GetParameterID( const char* sUniform ) const;
	Array< GLint >	GetParameterIDArray( const char* sUniform, const uint uCount ) const;

private:
	GLuint m_uProgramID;
};