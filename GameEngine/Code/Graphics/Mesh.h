#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Utils.h"

class Mesh
{
public:
	Mesh();

	void Create( const Array< Float3 >& aVertices, const Array< Float2 >& aUVs, const Array< Float3 >& aNormals, const Array< Float3 >& aTangents, const Array< Float3 >& aBiTangents, const Array< GLuint >& aIndices );
	void Destroy();

private:
	GLuint	m_uVertexArrayID;
	GLuint	m_uVertexBufferID;
	GLuint	m_uIndexBufferID;
	GLsizei m_iIndexCount;
};