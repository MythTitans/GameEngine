#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Core/Array.h"

struct Material;

class Mesh
{
public:
	friend class Renderer;
	friend class TextRenderer;

	Mesh();

	void Create( const Array< glm::vec3 >& aVertices, const Array< glm::vec2 >& aUVs, const Array< glm::vec3 >& aNormals, const Array< glm::vec3 >& aTangents, const Array< GLuint >& aIndices, const Material* pMaterial );
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
	MeshBuilder( Array< glm::vec3 >&& aVertices, Array< GLuint >&& aIndices );

	MeshBuilder& WithUVs();
	MeshBuilder& WithUVs( Array< glm::vec2 >&& aUVs );
	MeshBuilder& WithNormals();
	MeshBuilder& WithNormals( Array< glm::vec3 >&& aNormals );
	MeshBuilder& WithTangents();
	MeshBuilder& WithTangents( Array< glm::vec3 >&& aTangents );
	MeshBuilder& WithMaterial( const Material* pMaterial );

	Mesh			Build();

private:
	Array< glm::vec3 >	m_aVertices;
	Array< GLuint >		m_aIndices;
	Array< glm::vec2 >	m_aUVs;
	Array< glm::vec3 >	m_aNormals;
	Array< glm::vec3 >	m_aTangents;

	const Material* m_pMaterial;
};