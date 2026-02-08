#pragma once

#include <GL/glew.h>

#include <glm/glm.hpp>

#include "Core/Array.h"
#include "Graphics/Material.h"

struct VisualNode;

inline constexpr uint MAX_VERTEX_BONE_COUNT = 4;

struct SkinData
{
	SkinData();

	uint	m_aBones[ MAX_VERTEX_BONE_COUNT ];
	float	m_aWeights[ MAX_VERTEX_BONE_COUNT ];
};

class Mesh
{
public:
	friend class Renderer;
	friend class TextRenderer;

	template < bool bApplyMaterials >
	friend void DrawNodes( const Array< VisualNode* >& aVisualNodes, Technique& oTechnique, const glm::mat4& mViewProjectionMatrix );

	Mesh();

	void						Create( const Array< glm::vec3 >& aVertices, const Array< glm::vec2 >& aUVs, const Array< glm::vec3 >& aNormals, const Array< glm::vec3 >& aTangents, const Array< SkinData >& aSkinData, const Array< GLuint >& aIndices, const MaterialReference& oMaterial );
	void						Destroy();

	void						SetMaterial( const MaterialReference& oMaterial );
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
	MeshBuilder&	WithSkinData();
	MeshBuilder&	WithSkinData( Array< SkinData >&& aSkinData );
	MeshBuilder&	WithMaterial( const MaterialReference& oMaterial );

	Mesh			Build();

private:
	Array< glm::vec3 >	m_aVertices;
	Array< GLuint >		m_aIndices;
	Array< glm::vec2 >	m_aUVs;
	Array< glm::vec3 >	m_aNormals;
	Array< glm::vec3 >	m_aTangents;
	Array< SkinData >	m_aSkinData;

	MaterialReference	m_oMaterial;
};