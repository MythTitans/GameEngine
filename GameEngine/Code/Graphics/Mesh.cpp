#include "Mesh.h"

#include "Core/Types.h"

static const uint VERTEX_COMPONENT_COUNT = 14;

Mesh::Mesh()
	: m_uVertexArrayID( GL_INVALID_VALUE )
	, m_uVertexBufferID( GL_INVALID_VALUE )
	, m_uIndexBufferID( GL_INVALID_VALUE )
	, m_iIndexCount( 0 )
{
}

void Mesh::Create( const Array< Float3 >& aVertices, const Array< Float2 >& aUVs, const Array< Float3 >& aNormals, const Array< Float3 >& aTangents, const Array< Float3 >& aBiTangents, const Array< GLuint >& aIndices )
{
	Array< GLfloat > aPackedVertices;
	aPackedVertices.Resize( VERTEX_COMPONENT_COUNT * aVertices.Count() );

	for( uint u = 0; u < aVertices.Count(); ++u )
	{
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT ] = aVertices[ u ].m_fX;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 1 ] = aVertices[ u ].m_fY;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 2 ] = aVertices[ u ].m_fZ;

		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 3 ] = aUVs[ u ].m_fX;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 4 ] = aUVs[ u ].m_fY;

		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 5 ] = aNormals[ u ].m_fX;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 6 ] = aNormals[ u ].m_fY;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 7 ] = aNormals[ u ].m_fZ;

		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 8 ] = aTangents[ u ].m_fX;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 9 ] = aTangents[ u ].m_fY;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 10 ] = aTangents[ u ].m_fZ;

		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 11 ] = aBiTangents[ u ].m_fX;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 12 ] = aBiTangents[ u ].m_fY;
		aPackedVertices[ u * VERTEX_COMPONENT_COUNT + 13 ] = aBiTangents[ u ].m_fZ;
	}

	m_iIndexCount = ( int )aIndices.Count();

	glGenVertexArrays( 1, &m_uVertexArrayID );
	glBindVertexArray( m_uVertexArrayID );

	glGenBuffers( 1, &m_uVertexBufferID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aPackedVertices[ 0 ] ) * aPackedVertices.Count(), aPackedVertices.Data(), GL_STATIC_DRAW );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * VERTEX_COMPONENT_COUNT, nullptr );
	glEnableVertexAttribArray( 0 );

	glVertexAttribPointer( 1, 2, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * VERTEX_COMPONENT_COUNT, ( void* )( sizeof( aPackedVertices[ 0 ] ) * 3 ) );
	glEnableVertexAttribArray( 1 );

	glVertexAttribPointer( 2, 3, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * VERTEX_COMPONENT_COUNT, ( void* )( sizeof( aPackedVertices[ 0 ] ) * 5 ) );
	glEnableVertexAttribArray( 2 );

	glVertexAttribPointer( 3, 3, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * VERTEX_COMPONENT_COUNT, ( void* )( sizeof( aPackedVertices[ 0 ] ) * 8 ) );
	glEnableVertexAttribArray( 3 );

	glVertexAttribPointer( 4, 3, GL_FLOAT, GL_FALSE, sizeof( aPackedVertices[ 0 ] ) * VERTEX_COMPONENT_COUNT, ( void* )( sizeof( aPackedVertices[ 0 ] ) * 11 ) );
	glEnableVertexAttribArray( 4 );

	// TODO #eric could generate both buffers at once
	glGenBuffers( 1, &m_uIndexBufferID );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, m_uIndexBufferID );
	glBufferData( GL_ELEMENT_ARRAY_BUFFER, sizeof( aIndices[ 0 ] ) * m_iIndexCount, aIndices.Data(), GL_STATIC_DRAW );

	glBindVertexArray( 0 );
	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindBuffer( GL_ELEMENT_ARRAY_BUFFER, 0 );
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
