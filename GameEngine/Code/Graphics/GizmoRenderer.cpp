#include "GizmoRenderer.h"

#include "Renderer.h"

static constexpr uint CIRCLE_SEGMENT_COUNT = 32;
static constexpr uint ARROW_VERTEX_COUNT = 4 * ( 2 * CIRCLE_SEGMENT_COUNT + 2 ) + 3 * 2;
static constexpr uint GIRO_VERTEX_COUNT = 2 * CIRCLE_SEGMENT_COUNT + 2;

static const Array< glm::vec2 > CIRCLE_POINTS = []() {
	Array< glm::vec2 > aCircle;
	aCircle.Reserve( CIRCLE_SEGMENT_COUNT );

	const float fAngleStep = 360.f / CIRCLE_SEGMENT_COUNT;

	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
	{
		const float fAngle = glm::radians( u * fAngleStep );
		aCircle.PushBack( glm::vec2( glm::cos( fAngle ), glm::sin( fAngle ) ) );
	}

	return aCircle;
}();

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

void GizmoRenderer::RenderGizmo( const GizmoType eGizmoType, const GizmoAxis eGizmoAxis, const RenderContext& oRenderContext )
{
	switch( eGizmoType )
	{
	case GizmoType::TRANSLATE:
		RenderTranslationGizmo( eGizmoAxis, oRenderContext );
		break;
	case GizmoType::ROTATE:
		RenderRotationGizmo( eGizmoAxis, oRenderContext );
		break;
	case GizmoType::SCALE:
		break;
	}
}

void GizmoRenderer::RenderTranslationGizmo( const GizmoAxis eGizmoAxis, const RenderContext& oRenderContext )
{
	switch( eGizmoAxis )
	{
	case GizmoAxis::X:
	case GizmoAxis::Y:
	case GizmoAxis::Z:
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
	case GizmoAxis::XY:
	case GizmoAxis::XZ:
	case GizmoAxis::YZ:
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

void GizmoRenderer::RenderRotationGizmo( const GizmoAxis eGizmoAxis, const RenderContext& oRenderContext )
{
	glDisable( GL_CULL_FACE );

	Array< GLfloat > aVertices = GenerateGiro( eGizmoAxis );

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

	glDrawArrays( GL_TRIANGLE_STRIP, 0, GIRO_VERTEX_COUNT );

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glEnable( GL_CULL_FACE );
}

Array< GLfloat > GizmoRenderer::GenerateQuad( const GizmoAxis eGizmoAxis )
{
	glm::vec3 vPositions[ 4 ];
	glm::vec3 vOffset;

	const float fSize = 1.5f;
	const float fOffset = 0.4f;

	switch( eGizmoAxis )
	{
	case GizmoAxis::XY:
		vPositions[ 0 ] = glm::vec3( 0.f, fSize, 0.f );
		vPositions[ 1 ] = glm::vec3( 0.f, 0.f, 0.f );
		vPositions[ 2 ] = glm::vec3( fSize, fSize, 0.f );
		vPositions[ 3 ] = glm::vec3( fSize, 0.f, 0.f );
		vOffset = glm::vec3( fOffset, fOffset, 0.f );
		break;
	case GizmoAxis::XZ:
		vPositions[ 0 ] = glm::vec3( 0.f, 0.f, fSize );
		vPositions[ 1 ] = glm::vec3( 0.f, 0.f, 0.f );
		vPositions[ 2 ] = glm::vec3( fSize, 0.f, fSize );
		vPositions[ 3 ] = glm::vec3( fSize, 0.f, 0.f );
		vOffset = glm::vec3( fOffset, 0.f, fOffset );
		break;
	case GizmoAxis::YZ:
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

Array< GLfloat > GizmoRenderer::GenerateArrow( const GizmoAxis eGizmoAxis )
{
	const float fCylinderLength = 3.f;
	const float fCylinderRadius = 0.2f;
	const float fConeLength = 0.8f;
	const float fConeRadius = 0.4f;

	Array< glm::vec3 > vPositions;
	vPositions.Reserve( ARROW_VERTEX_COUNT );

	// Exterior
	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fCylinderRadius, 0.f, CIRCLE_POINTS[ u ].y * fCylinderRadius ) );
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fCylinderRadius, fCylinderLength, CIRCLE_POINTS[ u ].y * fCylinderRadius ) );
	}
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x* fCylinderRadius, 0.f, CIRCLE_POINTS[ 0 ].y* fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x* fCylinderRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y* fCylinderRadius ) );

	// Separator
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x* fCylinderRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );

	// Bottom
	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fCylinderRadius, 0.f, CIRCLE_POINTS[ u ].y * fCylinderRadius ) );
	}
	vPositions.PushBack( glm::vec3( 0.f, 0.f, 0.f ) );
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x* fCylinderRadius, 0.f, CIRCLE_POINTS[ 0 ].y * fCylinderRadius ) );

	// Separator
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fCylinderRadius, 0.f, CIRCLE_POINTS[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fCylinderRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y * fCylinderRadius ) );

	// Arrow bottom
	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fCylinderRadius, fCylinderLength, CIRCLE_POINTS[ u ].y * fCylinderRadius ) );
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fConeRadius, fCylinderLength, CIRCLE_POINTS[ u ].y * fConeRadius ) );
	}
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fCylinderRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y * fCylinderRadius ) );
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fConeRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y * fConeRadius ) );

	// Separator
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fConeRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y * fConeRadius ) );
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fConeRadius, fCylinderLength, CIRCLE_POINTS[ 0 ].y * fConeRadius ) );

	// Arrow top
	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fConeRadius, fCylinderLength, CIRCLE_POINTS[ u ].y * fConeRadius ) );
		vPositions.PushBack( glm::vec3( 0.f, fCylinderLength + fConeLength, 0.f ) );
	}
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x* fConeRadius, fCylinderLength, CIRCLE_POINTS[ 00 ].y * fConeRadius ) );
	vPositions.PushBack( glm::vec3( 0.f, fCylinderLength + fConeLength, 0.f ) );

	const float fOffset = 0.6f;

	switch( eGizmoAxis )
	{
	case GizmoAxis::X:
		for( glm::vec3& vPosition : vPositions )
			vPosition = glm::vec3( vPosition.y, -vPosition.x, vPosition.z ) + glm::vec3( fOffset, 0.f, 0.f );
		break;
	case GizmoAxis::Y:
		for( glm::vec3& vPosition : vPositions )
			vPosition = vPosition + glm::vec3( 0.f, fOffset, 0.f );
		break;
	case GizmoAxis::Z:
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

Array< GLfloat > GizmoRenderer::GenerateGiro( const GizmoAxis eGizmoAxis )
{
	const float fInnerRadius = 3.f;
	const float fOuterRadius = 3.4f;

	Array< glm::vec3 > vPositions;
	vPositions.Reserve( GIRO_VERTEX_COUNT );

	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
	{
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fOuterRadius, 0.f, CIRCLE_POINTS[ u ].y * fOuterRadius ) );
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x * fInnerRadius, 0.f, CIRCLE_POINTS[ u ].y * fInnerRadius ) );
	}
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fOuterRadius, 0.f, CIRCLE_POINTS[ 0 ].y * fOuterRadius ) );
	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x * fInnerRadius, 0.f, CIRCLE_POINTS[ 0 ].y * fInnerRadius ) );

	switch( eGizmoAxis )
	{
	case GizmoAxis::XY:
		for( glm::vec3& vPosition : vPositions )
			vPosition = glm::vec3( vPosition.x, -vPosition.z, vPosition.y );
		break;
	case GizmoAxis::YZ:
		for( glm::vec3& vPosition : vPositions )
			vPosition = glm::vec3( vPosition.y, -vPosition.x, vPosition.z );
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
