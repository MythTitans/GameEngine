#include "Mesh.h"

#include "Core/Types.h"

SkinData::SkinData()
{
	m_aBones[ 0 ] = 0;
	m_aWeights[ 0 ] = 1.f;

	for( uint u = 1; u < MAX_VERTEX_BONE_COUNT; ++u )
	{
		m_aBones[ u ] = 0;
		m_aWeights[ u ] = 0.f;
	}
}

Mesh::Mesh()
	: m_uVertexArrayID( GL_INVALID_VALUE )
	, m_uVertexBufferID( GL_INVALID_VALUE )
	, m_uIndexBufferID( GL_INVALID_VALUE )
	, m_iIndexCount( 0 )
{
}

void Mesh::Create( const Array< glm::vec3 >& aVertices, const Array< glm::vec2 >& aUVs, const Array< glm::vec3 >& aNormals, const Array< glm::vec3 >& aTangents, const Array< SkinData >& aSkinData, const Array< GLuint >& aIndices, const MaterialReference& oMaterial )
{
	ASSERT( aVertices.Empty() == false && aIndices.Empty() == false );
	ASSERT( aUVs.Empty() || aUVs.Count() == aVertices.Count() );
	ASSERT( aNormals.Empty() || aNormals.Count() == aVertices.Count() );
	ASSERT( aTangents.Empty() || aTangents.Count() == aVertices.Count() );
	ASSERT( aSkinData.Empty() || aSkinData.Count() == aVertices.Count() );

	if( aVertices.Empty() || aIndices.Empty() )
		return;

	const uint uVerticesSize = sizeof( aVertices[ 0 ] ) / sizeof( GLfloat );
	const uint uUVsSize = aUVs.Empty() ? 0 : sizeof( aUVs[ 0 ] ) / sizeof( GLfloat );
	const uint uNormalsSize = aNormals.Empty() ? 0 : sizeof( aNormals[ 0 ] ) / sizeof( GLfloat );
	const uint uTangentsSize = aTangents.Empty() ? 0 : sizeof( aTangents[ 0 ] ) / sizeof( GLfloat );
	const uint uBonesSize = aSkinData.Empty() ? 0 : sizeof( aSkinData[ 0 ].m_aBones ) / sizeof( GLfloat );
	const uint uWeightsSize = aSkinData.Empty() ? 0 : sizeof( aSkinData[ 0 ].m_aWeights ) / sizeof( GLfloat );

	const uint uUVsOffset = uVerticesSize;
	const uint uNormalsOffset = uUVsOffset + uUVsSize;
	const uint uTangentsOffset = uNormalsOffset + uNormalsSize;
	const uint uBonesOffset = uTangentsOffset + uTangentsSize;
	const uint uWeightsOffset = uBonesOffset + uBonesSize;

	const uint uVertexSize = uWeightsOffset + uWeightsSize;

	Array< GLfloat > aPackedVertices;
	aPackedVertices.Resize( uVertexSize * aVertices.Count() );

	for( uint u = 0; u < aVertices.Count(); ++u )
	{
		aPackedVertices[ u * uVertexSize ] = aVertices[ u ].x;
		aPackedVertices[ u * uVertexSize + 1 ] = aVertices[ u ].y;
		aPackedVertices[ u * uVertexSize + 2 ] = aVertices[ u ].z;

		if( uUVsSize != 0 )
		{
			aPackedVertices[ u * uVertexSize + uUVsOffset ] = aUVs[ u ].x;
			aPackedVertices[ u * uVertexSize + uUVsOffset + 1] = aUVs[ u ].y;
		}

		if( uNormalsSize != 0 )
		{
			aPackedVertices[ u * uVertexSize + uNormalsOffset ] = aNormals[ u ].x;
			aPackedVertices[ u * uVertexSize + uNormalsOffset + 1 ] = aNormals[ u ].y;
			aPackedVertices[ u * uVertexSize + uNormalsOffset + 2 ] = aNormals[ u ].z;
		}
		
		if( uTangentsSize != 0 )
		{
			aPackedVertices[ u * uVertexSize + uTangentsOffset ] = aTangents[ u ].x;
			aPackedVertices[ u * uVertexSize + uTangentsOffset + 1 ] = aTangents[ u ].y;
			aPackedVertices[ u * uVertexSize + uTangentsOffset + 2 ] = aTangents[ u ].z;
		}

		if( uBonesSize != 0 )
		{
			// TODO #eric should use a struct instead of GLfloats and having to do things like this
			aPackedVertices[ u * uVertexSize + uBonesOffset ] = *reinterpret_cast< const GLfloat* >( &aSkinData[ u ].m_aBones[ 0 ] );
			aPackedVertices[ u * uVertexSize + uBonesOffset + 1 ] = *reinterpret_cast< const GLfloat* >( &aSkinData[ u ].m_aBones[ 1 ] );
			aPackedVertices[ u * uVertexSize + uBonesOffset + 2 ] = *reinterpret_cast< const GLfloat* >( &aSkinData[ u ].m_aBones[ 2 ] );
			aPackedVertices[ u * uVertexSize + uBonesOffset + 3 ] = *reinterpret_cast< const GLfloat* >( &aSkinData[ u ].m_aBones[ 3 ] );
		}

		if( uWeightsSize != 0 )
		{
			aPackedVertices[ u * uVertexSize + uWeightsOffset ] = aSkinData[ u ].m_aWeights[ 0 ];
			aPackedVertices[ u * uVertexSize + uWeightsOffset + 1 ] = aSkinData[ u ].m_aWeights[ 1 ];
			aPackedVertices[ u * uVertexSize + uWeightsOffset + 2 ] = aSkinData[ u ].m_aWeights[ 2 ];
			aPackedVertices[ u * uVertexSize + uWeightsOffset + 3 ] = aSkinData[ u ].m_aWeights[ 3 ];
		}
	}

	m_iIndexCount = ( int )aIndices.Count();

	glGenVertexArrays( 1, &m_uVertexArrayID );
	glBindVertexArray( m_uVertexArrayID );

	glGenBuffers( 1, &m_uVertexBufferID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aPackedVertices[ 0 ] ) * aPackedVertices.Count(), aPackedVertices.Data(), GL_STATIC_DRAW );

	uint uAttributeIndex = 0;

	glVertexAttribPointer( uAttributeIndex, uVerticesSize, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * uVertexSize, nullptr );
	glEnableVertexAttribArray( uAttributeIndex );
	++uAttributeIndex;

	if( uUVsSize != 0 )
	{
		glVertexAttribPointer( uAttributeIndex, uUVsSize, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * uVertexSize, ( void* )( sizeof( aPackedVertices[ 0 ] ) * uUVsOffset ) );
		glEnableVertexAttribArray( uAttributeIndex );
		++uAttributeIndex;
	}
	
	if( uNormalsSize != 0 )
	{
		glVertexAttribPointer( uAttributeIndex, uNormalsSize, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * uVertexSize, ( void* )( sizeof( aPackedVertices[ 0 ] ) * uNormalsOffset ) );
		glEnableVertexAttribArray( uAttributeIndex );
		++uAttributeIndex;
	}
	
	if( uTangentsSize != 0 )
	{
		glVertexAttribPointer( uAttributeIndex, uTangentsSize, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * uVertexSize, ( void* )( sizeof( aPackedVertices[ 0 ] ) * uTangentsOffset ) );
		glEnableVertexAttribArray( uAttributeIndex );
		++uAttributeIndex;
	}

	if( uBonesSize != 0 )
	{
		glVertexAttribIPointer( uAttributeIndex, uBonesSize, GL_UNSIGNED_INT, sizeof( aPackedVertices[ 0 ] ) * uVertexSize, ( void* )( sizeof( aPackedVertices[ 0 ] ) * uBonesOffset ) );
		glEnableVertexAttribArray( uAttributeIndex );
		++uAttributeIndex;
	}

	if( uWeightsSize != 0 )
	{
		glVertexAttribPointer( uAttributeIndex, uWeightsSize, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * uVertexSize, ( void* )( sizeof( aPackedVertices[ 0 ] ) * uWeightsOffset ) );
		glEnableVertexAttribArray( uAttributeIndex );
		++uAttributeIndex;
	}

	// TODO #eric could generate both buffers at once
	glGenBuffers( 1, &m_uIndexBufferID );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_uIndexBufferID );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( aIndices[ 0 ] ) * m_iIndexCount, aIndices.Data(), GL_STATIC_DRAW );

	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );

	m_oMaterial = oMaterial;
}

void Mesh::Destroy()
{
	// TODO #eric could destroy both buffers at once
	glDeleteBuffers( 1, &m_uIndexBufferID );
	glDeleteBuffers( 1, &m_uVertexBufferID );
	glDeleteVertexArrays( 1, &m_uVertexArrayID );

	m_uVertexArrayID = GL_INVALID_VALUE;
	m_uVertexBufferID = GL_INVALID_VALUE;
	m_uIndexBufferID = GL_INVALID_VALUE;
	m_iIndexCount = 0;
}

void Mesh::SetMaterial( const MaterialReference& oMaterial )
{
	m_oMaterial = oMaterial;
}

const MaterialReference& Mesh::GetMaterial() const
{
	return m_oMaterial;
}

MeshBuilder::MeshBuilder( Array< glm::vec3 >&& aVertices, Array< GLuint >&& aIndices )
	: m_aVertices( aVertices )
	, m_aIndices( aIndices )
{
}

MeshBuilder& MeshBuilder::WithUVs()
{
	m_aUVs.Resize( m_aVertices.Count() );
	return *this;
}

MeshBuilder& MeshBuilder::WithUVs( Array< glm::vec2 >&& aUVs )
{
	m_aUVs = aUVs;
	return *this;
}

MeshBuilder& MeshBuilder::WithNormals()
{
	m_aNormals.Resize( m_aVertices.Count() );
	return *this;
}

MeshBuilder& MeshBuilder::WithNormals( Array< glm::vec3 >&& aNormals )
{
	m_aNormals = aNormals;
	return *this;
}

MeshBuilder& MeshBuilder::WithTangents()
{
	m_aTangents.Resize( m_aVertices.Count() );
	return *this;
}

MeshBuilder& MeshBuilder::WithTangents( Array< glm::vec3 >&& aTangents )
{
	m_aTangents = aTangents;
	return *this;
}

MeshBuilder& MeshBuilder::WithSkinData()
{
	m_aSkinData.Resize( m_aVertices.Count() );
	return *this;
}

MeshBuilder& MeshBuilder::WithSkinData( Array< SkinData >&& aSkinData )
{
	m_aSkinData = aSkinData;
	return *this;
}

MeshBuilder& MeshBuilder::WithMaterial( const MaterialReference& oMaterial )
{
	m_oMaterial = oMaterial;
	return *this;
}

Mesh MeshBuilder::Build()
{
	Mesh oMesh;
	oMesh.Create( m_aVertices, m_aUVs, m_aNormals, m_aTangents, m_aSkinData, m_aIndices, m_oMaterial );

	return oMesh;
}
