#include "DebugRenderer.h"

#include "Renderer.h"

DebugRenderer::DebugRenderer()
	: m_xLine( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/Shader/line" ) ) )
{
	glGenVertexArrays( 1, &m_uVertexArrayID );
	glGenBuffers( 1, &m_uVertexBufferID );

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );

	glVertexAttribPointer( 0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), nullptr );
	glEnableVertexAttribArray( 0 );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );
}

DebugRenderer::~DebugRenderer()
{
	glDeleteBuffers( 1, &m_uVertexBufferID );
	glDeleteVertexArrays( 1, &m_uVertexArrayID );
}

void DebugRenderer::RenderLines( const Array< Line >& aLines, const RenderContext& oRenderContext )
{
	glUseProgram( m_xLine->GetTechnique().m_uProgramID );

	m_oLine.SetViewProjection( g_pRenderer->GetCamera().GetViewProjectionMatrix() );

	Array< GLfloat > aVertices;
	aVertices.Reserve( 3 * 2 * aLines.Count() );
	for( const Line& oLine : aLines )
	{
		aVertices.PushBack( oLine.m_vFrom.x );
		aVertices.PushBack( oLine.m_vFrom.y );
		aVertices.PushBack( oLine.m_vFrom.z );
		aVertices.PushBack( oLine.m_vTo.x );
		aVertices.PushBack( oLine.m_vTo.y );
		aVertices.PushBack( oLine.m_vTo.z );
	}

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

	for( uint u = 0; u < aLines.Count(); ++u )
	{
		m_oLine.SetColor( aLines[ u ].m_vColor );
		glDrawArrays( GL_LINES, 2 * u, 2 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

bool DebugRenderer::OnLoading()
{
	if( m_xLine->IsLoaded() && m_oLine.IsValid() == false )
		m_oLine.Create( m_xLine->GetTechnique() );

	return m_xLine->IsLoaded();
}
