#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Game/MaterialManager.h"

class Mesh
{
public:
	friend class Renderer;
	friend class TextRenderer;

	template < typename Technique >
	friend void DrawMeshes( Technique& oMeshesDefinition );

	Mesh();

	void						Create( const Array< glm::vec3 >& aVertices, const Array< glm::vec2 >& aUVs, const Array< glm::vec3 >& aNormals, const Array< glm::vec3 >& aTangents, const Array< GLuint >& aIndices, const MaterialReference& oMaterial );
	void						Destroy();

	const MaterialReference&	GetMaterial() const;

private:
	GLuint				m_uVertexArrayID;
	GLuint				m_uVertexBufferID;
	GLuint				m_uIndexBufferID;
	GLsizei				m_iIndexCount;

	MaterialReference	m_oMaterial;
};

class MeshBuilder
{
public:
	MeshBuilder( Array< glm::vec3 >&& aVertices, Array< GLuint >&& aIndices );

	MeshBuilder&	WithUVs();
	MeshBuilder&	WithUVs( Array< glm::vec2 >&& aUVs );
	MeshBuilder&	WithNormals();
	MeshBuilder&	WithNormals( Array< glm::vec3 >&& aNormals );
	MeshBuilder&	WithTangents();
	MeshBuilder&	WithTangents( Array< glm::vec3 >&& aTangents );
	MeshBuilder&	WithMaterial( const MaterialReference& oMaterial );

	Mesh			Build();

private:
	Array< glm::vec3 >	m_aVertices;
	Array< GLuint >		m_aIndices;
	Array< glm::vec2 >	m_aUVs;
	Array< glm::vec3 >	m_aNormals;
	Array< glm::vec3 >	m_aTangents;

	MaterialReference	m_oMaterial;
};