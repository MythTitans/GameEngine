#include "GizmoRenderer.h"

#include "Renderer.h"

static constexpr uint ARROW_SEGMENT_COUNT = 32;
static constexpr uint ARROW_VERTEX_COUNT = 4 * ( 2 * ARROW_SEGMENT_COUNT + 2 ) + 3 * 2;

GizmoRenderer::GizmoRenderer()
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

GizmoRenderer::~GizmoRenderer()
{
	glDeleteBuffers( 1, &m_uVertexBufferID );
	glDeleteVertexArrays( 1, &m_uVertexArrayID );
}

void GizmoRenderer::RenderGizmo( const GizmoComponent::GizmoType eGizmoType, const GizmoComponent::GizmoAxis eGizmoAxis, const RenderContext& oRenderContext )
{
	switch( eGizmoType )
	{
	case GizmoComponent::GizmoType::TRANSLATE:
		RenderTranslationGizmo( eGizmoAxis, oRenderContext );
		break;
	case GizmoComponent::GizmoType::ROTATE:
		break;
	case GizmoComponent::GizmoType::SCALE:
		break;
	}
}

void GizmoRenderer::RenderTranslationGizmo( const GizmoComponent::GizmoAxis eGizmoAxis, const RenderContext& oRenderContext )
{
	switch( eGizmoAxis )
	{
	case GizmoComponent::GizmoAxis::X:
	case GizmoComponent::GizmoAxis::Y:
	case GizmoComponent::GizmoAxis::Z:
	{
		Array< GLfloat > aVertices = GenerateArrow( eGizmoAxis );
		glBindVertexArray( m_uVertexArrayID );
		glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
		glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

		glDrawArrays( GL_TRIANGLE_STRIP, 0, ARROW_VERTEX_COUNT );

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );
		break;
	}
	case GizmoComponent::GizmoAxis::XY:
	case GizmoComponent::GizmoAxis::XZ:
	case GizmoComponent::GizmoAxis::YZ:
	{
		glDisable( GL_CULL_FACE );

		Array< GLfloat > aVertices = GenerateQuad( eGizmoAxis );

		glBindVertexArray( m_uVertexArrayID );
		glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
		glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

		glDrawArrays( GL_TRIANGLE_STRIP, 0, 4 );

		glBindBuffer( GL_ARRAY_BUFFER, 0 );
		glBindVertexArray( 0 );

		glEnable( GL_CULL_FACE );
		break;
	}
	}
}

Array< GLfloat > GizmoRenderer::GenerateQuad( const GizmoComponent::GizmoAxis eGizmoAxis )
{
	glm::vec3 vPositions[ 4 ];
	glm::vec3 vOffset;

	const float fSize = 2.f;
	const float fOffset = 0.4f;

	switch( eGizmoAxis )
	{
	case GizmoComponent::GizmoAxis::XY:
		vPositions[ 0 ] = glm::vec3( 0.f, fSize, 0.f );
		vPositions[ 1 ] = glm::vec3( 0.f, 0.f, 0.f );
		vPositions[ 2 ] = glm::vec3( fSize, fSize, 0.f );
		vPositions[ 3 ] = glm::vec3( fSize, 0.f, 0.f );
		vOffset = glm::vec3( fOffset, fOffset, 0.f );
		break;
	case GizmoComponent::GizmoAxis::XZ:
		vPositions[ 0 ] = glm::vec3( 0.f, 0.f, fSize );
		vPositions[ 1 ] = glm::vec3( 0.f, 0.f, 0.f );
		vPositions[ 2 ] = glm::vec3( fSize, 0.f, fSize );
		vPositions[ 3 ] = glm::vec3( fSize, 0.f, 0.f );
		vOffset = glm::vec3( fOffset, 0.f, fOffset );
		break;
	case GizmoComponent::GizmoAxis::YZ:
		vPositions[ 0 ] = glm::vec3( 0.f, 0.f, fSize );
		vPositions[ 1 ] = glm::vec3( 0.f, 0.f, 0.f );
		vPositions[ 2 ] = glm::vec3( 0.f, fSize, fSize );
		vPositions[ 3 ] = glm::vec3( 0.f, fSize, 0.f );
		vOffset = glm::vec3( 0.f, fOffset, fOffset );
		break;
	}

	for( glm::vec3& vPosition : vPositions )
		vPosition += vOffset;

	Array< GLfloat > aVertices;
	aVertices.Reserve( 3 * 4 );

	for( const glm::vec3& vPosition : vPositions )
	{
		aVertices.PushBack( vPosition.x );
		aVertices.PushBack( vPosition.y );
		aVertices.PushBack( vPosition.z );
	}

	return aVertices;
}

Array< GLfloat > GizmoRenderer::GenerateArrow( const GizmoComponent::GizmoAxis eGizmoAxis )
{
	const float fCylinderLength = 3.f;
	const float fCylinderRadius = 0.2f;
	const float fConeLength = 0.8f;
	const float fConeRadius = 0.4f;

	Array< glm::vec2 > aCircle;
	aCircle.Reserve( ARROW_SEGMENT_COUNT );

	const float fAngleStep = 360.f / ARROW_SEGMENT_COUNT;

	for( uint u = 0; u < ARROW_SEGMENT_COUNT; ++u )
	{
		const float fAngle = glm::radians( u * fAngleStep );
		aCircle.PushBack( glm::vec2( glm::cos( fAngle ), glm::sin( fAngle ) ) );
	}

	Array< glm::vec3 > vPositions;
	vPositions.Reserve( ARROW_VERTEX_COUNT );

	// Exterior
	for( uint u = 0; u < ARROW_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( aCircle[ u ].x * fCylinderRadius, 0.f, aCircle[ u ].y * fCylinderRadius ) );
		vPositions.PushBack( glm::vec3( aCircle[ u ].x * fCylinderRadius, fCylinderLength, aCircle[ u ].y * fCylinderRadius ) );
	}
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, 0.f, aCircle[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, fCylinderLength, aCircle[ 0 ].y * fCylinderRadius ) );

	// Separator
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, fCylinderLength, aCircle[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );

	// Bottom
	for( uint u = 0; u < ARROW_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );
		vPositions.PushBack( glm::vec3( aCircle[ u ].x * fCylinderRadius, 0.f, aCircle[ u ].y * fCylinderRadius ) );
	}
	vPositions.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, 0.f, aCircle[ 0 ].y * fCylinderRadius ) );

	// Separator
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, 0.f, aCircle[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, fCylinderLength, aCircle[ 0 ].y * fCylinderRadius ) );

	// Arrow bottom
	for( uint u = 0; u < ARROW_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( aCircle[ u ].x * fCylinderRadius, fCylinderLength, aCircle[ u ].y * fCylinderRadius ) );
		vPositions.PushBack( glm::vec3( aCircle[ u ].x * fConeRadius, fCylinderLength, aCircle[ u ].y * fConeRadius ) );
	}
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fCylinderRadius, fCylinderLength, aCircle[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fConeRadius, fCylinderLength, aCircle[ 0 ].y * fConeRadius ) );

	// Separator
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fConeRadius, fCylinderLength, aCircle[ 0 ].y * fConeRadius ) );
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fConeRadius, fCylinderLength, aCircle[ 0 ].y * fConeRadius ) );

	// Arrow top
	for( uint u = 0; u < ARROW_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( aCircle[ u ].x * fConeRadius, fCylinderLength, aCircle[ u ].y * fConeRadius ) );
		vPositions.PushBack( glm::vec3( 0.f, fCylinderLength + fConeLength, 0.f ) );
	}
	vPositions.PushBack( glm::vec3( aCircle[ 0 ].x * fConeRadius, fCylinderLength, aCircle[ 00 ].y * fConeRadius ) );
	vPositions.PushBack( glm::vec3( 0.f, fCylinderLength + fConeLength, 0.f ) );

	const float fOffset = 0.4f;

	switch( eGizmoAxis )
	{
	case GizmoComponent::GizmoAxis::X:
		for( glm::vec3& vPosition : vPositions )
			vPosition = glm::vec3( vPosition.y, -vPosition.x, vPosition.z ) + glm::vec3( fOffset, 0.f, 0.f );
		break;
	case GizmoComponent::GizmoAxis::Y:
		for( glm::vec3& vPosition : vPositions )
			vPosition = vPosition + glm::vec3( 0.f, fOffset, 0.f );
		break;
	case GizmoComponent::GizmoAxis::Z:
		for( glm::vec3& vPosition : vPositions )
			vPosition = glm::vec3( vPosition.x, -vPosition.z, vPosition.y ) + glm::vec3( 0.f, 0.f, fOffset );
		break;
	}

	Array< GLfloat > aVertices;
	aVertices.Reserve( 3 * vPositions.Count() );

	for( const glm::vec3& vPosition : vPositions )
	{
		aVertices.PushBack( vPosition.x );
		aVertices.PushBack( vPosition.y );
		aVertices.PushBack( vPosition.z );
	}

	return aVertices;
}
