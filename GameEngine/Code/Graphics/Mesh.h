#pragma once

#include <GL/glew.h>

#include "Core/Array.h"
#include "Utils.h"

struct Material;

class Mesh
{
public:
	friend class Renderer;
	friend class TextRenderer;

	Mesh();

	void Create( const Array< Float3 >& aVertices, const Array< Float2 >& aUVs, const Array< Float3 >& aNormals, const Array< Float3 >& aTangents, const Array< Float3 >& aBiTangents, const Array< GLuint >& aIndices, const Material* pMaterial );
	void Destroy();

private:
	GLuint			m_uVertexArrayID;
	GLuint			m_uVertexBufferID;
	GLuint			m_uIndexBufferID;
	GLsizei			m_iIndexCount;

	const Material*	m_pMaterial;
};

class MeshBuilder
{
public:
	MeshBuilder( Array< Float3 >&& aVertices, Array< GLuint >&& aIndices );

	MeshBuilder& WithUVs();
	MeshBuilder& WithUVs( Array< Float2 >&& aUVs );
	MeshBuilder& WithNormals();
	MeshBuilder& WithNormals( Array< Float3 >&& aNormals );
	MeshBuilder& WithTangents();
	MeshBuilder& WithTangents( Array< Float3 >&& aTangents );
	MeshBuilder& WithBiTangents();
	MeshBuilder& WithBiTangents( Array< Float3 >&& aBiTangents );
	MeshBuilder& WithMaterial( const Material* pMaterial );

	Mesh			Build();

private:
	Array< Float3 > m_aVertices;
	Array< GLuint > m_aIndices;
	Array< Float2 > m_aUVs;
	Array< Float3 > m_aNormals;
	Array< Float3 > m_aTangents;
	Array< Float3 > m_aBiTangents;

	const Material* m_pMaterial;
};