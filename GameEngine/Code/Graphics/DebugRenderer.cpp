#include "DebugRenderer.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/epsilon.hpp>
#include <glm/gtx/vector_query.hpp>

#include "Core/Array.h"
#include "Renderer.h"

static const std::string PARAM_VIEW_PROJECTION( "viewProjection" );
static const std::string PARAM_COLOR( "color" );
static const std::string PARAM_POSITION( "position" );
static const std::string PARAM_RADIUS( "radius" );

static constexpr uint SPHERE_SEGMENT_COUNT = 32;
static constexpr uint SPHERE_RING_COUNT = 16;
static constexpr uint SPHERE_VERTEX_COUNT = ( SPHERE_RING_COUNT - 1 ) * ( 4 + 2 * ( SPHERE_SEGMENT_COUNT - 1 ) ) + 2 * ( SPHERE_RING_COUNT - 2 );

static constexpr uint WIRE_SPHERE_SEGMENT_COUNT = 32;
static constexpr uint WIRE_SPHERE_EQUATOR_VERTEX_COUNT = WIRE_SPHERE_SEGMENT_COUNT + 1;
static constexpr uint WIRE_SPHERE_MERIDIANS_VERTEX_COUNT = 2 * ( WIRE_SPHERE_SEGMENT_COUNT + 1 );

static constexpr uint CIRCLE_SEGMENT_COUNT = 32;
static constexpr uint CIRCLE_SEGMENT_VERTEX_COUNT = CIRCLE_SEGMENT_COUNT + 1;

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

static const Array< GLfloat > SPHERE_VERTICES = []() {

	Array< GLfloat > aResult( 3 * SPHERE_VERTEX_COUNT );

	const float fSegmentStep = 360.f / SPHERE_SEGMENT_COUNT;
	const float fRingStep = 180.f / ( SPHERE_RING_COUNT - 1 );

	uint uIndex = 0;

	const Array < glm::vec3 > aSegments = [ fSegmentStep ]() {
		Array< glm::vec3 > aSegments( SPHERE_SEGMENT_COUNT + 1 );

		for( uint u = 0; u <= SPHERE_SEGMENT_COUNT; ++u )
		{
			const float fSegmentAngle = u * fSegmentStep;
			const float fSegmentCos = glm::cos( glm::radians( fSegmentAngle ) );
			const float fSegmentSin = glm::sin( glm::radians( fSegmentAngle ) );

			aSegments[ u ] = glm::vec3( fSegmentCos, 0.f, fSegmentSin );
		}

		return aSegments;
	}();

	const Array< glm::vec2 > aRings = [ fRingStep ]() {
		Array< glm::vec2 > aRings( SPHERE_RING_COUNT );

		for( uint u = 0; u < SPHERE_RING_COUNT; ++u )
		{
			const float fRingAngle = -90.f + u * fRingStep;
			const float fRingCos = glm::cos( glm::radians( fRingAngle ) );
			const float fRingSin = glm::sin( glm::radians( fRingAngle ) );

			aRings[ u ] = glm::vec2( fRingCos, fRingSin );
		}

		return aRings;
	}();

	auto ComputePosition = [ &aSegments, &aRings ]( const uint uSegmentIndex, const uint uRingIndex ) {
		glm::vec3 vPosition = aSegments[ uSegmentIndex ] * aRings[ uRingIndex ].x;
		vPosition.y = aRings[ uRingIndex ].y;

		return vPosition;
	};

	auto StorePosition = [ &aResult, &uIndex ]( const glm::vec3& vPosition ) {
		aResult[ 3 * uIndex ] = vPosition.x;
		aResult[ 3 * uIndex + 1 ] = vPosition.y;
		aResult[ 3 * uIndex + 2 ] = vPosition.z;

		++uIndex;
	};

	for( uint u = 0; u < SPHERE_RING_COUNT - 1; ++u )
	{
		if( u != 0 && u != SPHERE_RING_COUNT - 1 )
			StorePosition( ComputePosition( 0, u ) );

		for( uint v = 0; v <= SPHERE_SEGMENT_COUNT; ++v )
		{
			StorePosition( ComputePosition( v, u ) );
			StorePosition( ComputePosition( v, u + 1 ) );
		}

		if( u != SPHERE_RING_COUNT - 2 )
			StorePosition( ComputePosition( SPHERE_SEGMENT_COUNT, u + 1 ) );
	}

	return aResult;
}();

static void PushVertex( Array< GLfloat >& aVertices, const glm::vec3& vVertex )
{
	aVertices.PushBack( vVertex.x );
	aVertices.PushBack( vVertex.y );
	aVertices.PushBack( vVertex.z );
}

DebugRenderer::DebugRenderer()
	: m_xLine( g_pResourceLoader->LoadTechnique( "Shader/line.tech" ) )
	, m_xSphere( g_pResourceLoader->LoadTechnique( "Shader/sphere.tech" ) )
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
	Technique& oTechnique = m_xLine->GetTechnique();
	glUseProgram( oTechnique.m_uProgramID );

	oTechnique.SetParameter( PARAM_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

	Array< GLfloat > aVertices;
	aVertices.Reserve( 3 * 2 * aLines.Count() );
	for( const Line& oLine : aLines )
	{
		PushVertex( aVertices, oLine.m_vFrom );
		PushVertex( aVertices, oLine.m_vTo );
	}

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

	for( uint u = 0; u < aLines.Count(); ++u )
	{
		oTechnique.SetParameter( PARAM_COLOR, aLines[ u ].m_vColor );
		glDrawArrays( GL_LINES, 2 * u, 2 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

void DebugRenderer::RenderSpheres( const Array< Sphere >& aSpheres, const RenderContext& oRenderContext )
{
	Technique& oTechnique = m_xSphere->GetTechnique();
	glUseProgram( oTechnique.m_uProgramID );

	oTechnique.SetParameter( PARAM_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( SPHERE_VERTICES[ 0 ] ) * SPHERE_VERTICES.Count(), SPHERE_VERTICES.Data(), GL_STATIC_DRAW );

	for( const Sphere& oSphere : aSpheres )
	{
		oTechnique.SetParameter( PARAM_POSITION, oSphere.m_vPosition );
		oTechnique.SetParameter( PARAM_RADIUS, oSphere.m_fRadius );
		oTechnique.SetParameter( PARAM_COLOR, oSphere.m_vColor );
		glDrawArrays( GL_TRIANGLE_STRIP, 0, SPHERE_VERTEX_COUNT );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

void DebugRenderer::RenderWireSpheres( const Array< Sphere >& aSpheres, const RenderContext& oRenderContext )
{
	const Array< GLfloat > aEquatorVertices = GenerateSphereEquator();
	const Array< GLfloat > aMeridiansVertices = GenerateSphereMeridians();

	Technique& oTechnique = m_xSphere->GetTechnique();
	glUseProgram( oTechnique.m_uProgramID );

	oTechnique.SetParameter( PARAM_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aEquatorVertices[ 0 ] ) * aEquatorVertices.Count(), aEquatorVertices.Data(), GL_STATIC_DRAW );

	for( const Sphere& oSphere : aSpheres )
	{
		oTechnique.SetParameter( PARAM_POSITION, oSphere.m_vPosition );
		oTechnique.SetParameter( PARAM_RADIUS, oSphere.m_fRadius );
		oTechnique.SetParameter( PARAM_COLOR, oSphere.m_vColor );
		glDrawArrays( GL_LINE_STRIP, 0, WIRE_SPHERE_EQUATOR_VERTEX_COUNT );
	}

	glBufferData( GL_ARRAY_BUFFER, sizeof( aMeridiansVertices[ 0 ] ) * aMeridiansVertices.Count(), aMeridiansVertices.Data(), GL_STATIC_DRAW );

	for( const Sphere& oSphere : aSpheres )
	{
		oTechnique.SetParameter( PARAM_POSITION, oSphere.m_vPosition );
		oTechnique.SetParameter( PARAM_RADIUS, oSphere.m_fRadius );
		oTechnique.SetParameter( PARAM_COLOR, oSphere.m_vColor );
		glDrawArrays( GL_LINE_STRIP, 0, WIRE_SPHERE_MERIDIANS_VERTEX_COUNT );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

void DebugRenderer::RenderWireCylinders( const Array< Cylinder >& aCylinders, const RenderContext& oRenderContext )
{
	Technique& oSphereTechnique = m_xSphere->GetTechnique();
	glUseProgram( oSphereTechnique.m_uProgramID );

	oSphereTechnique.SetParameter( PARAM_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );

	for( const Cylinder& oCylinder : aCylinders )
	{
		const Array< GLfloat > aCircleVertices = GenerateCylinderEquator( glm::normalize( oCylinder.m_vTo - oCylinder.m_vFrom ), oCylinder.m_fRadius );
		glBufferData( GL_ARRAY_BUFFER, sizeof( aCircleVertices[ 0 ] ) * aCircleVertices.Count(), aCircleVertices.Data(), GL_STATIC_DRAW );

		oSphereTechnique.SetParameter( PARAM_POSITION, oCylinder.m_vFrom );
		oSphereTechnique.SetParameter( PARAM_RADIUS, 1.f );
		oSphereTechnique.SetParameter( PARAM_COLOR, oCylinder.m_vColor );
		glDrawArrays( GL_LINE_STRIP, 0, CIRCLE_SEGMENT_VERTEX_COUNT );

		oSphereTechnique.SetParameter( PARAM_POSITION, 0.5f * ( oCylinder.m_vFrom + oCylinder.m_vTo ) );
		glDrawArrays( GL_LINE_STRIP, 0, CIRCLE_SEGMENT_VERTEX_COUNT );

		oSphereTechnique.SetParameter( PARAM_POSITION, oCylinder.m_vTo );
		glDrawArrays( GL_LINE_STRIP, 0, CIRCLE_SEGMENT_VERTEX_COUNT );

		Technique& oLineTechnique = m_xLine->GetTechnique();
		glUseProgram( oLineTechnique.m_uProgramID );

		oLineTechnique.SetParameter( PARAM_COLOR, oCylinder.m_vColor );

		Array< GLfloat > aVertices;
		aVertices.Reserve( 24 );
		for( uint u = 0; u < 4; ++u )
		{
			const uint uIndex = u * ( CIRCLE_SEGMENT_COUNT / 4 ) * 3;

			const glm::vec3 vCircleVertex( aCircleVertices[ uIndex ], aCircleVertices[ uIndex + 1 ] , aCircleVertices[ uIndex + 2 ] );
			PushVertex( aVertices, vCircleVertex + oCylinder.m_vFrom );
			PushVertex( aVertices, vCircleVertex + oCylinder.m_vTo );
		}

		glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

		glDrawArrays( GL_LINES, 0, 8 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

void DebugRenderer::RenderWireCones( const Array< Cylinder >& aCylinders, const RenderContext& oRenderContext )
{
	Technique& oSphereTechnique = m_xSphere->GetTechnique();
	glUseProgram( oSphereTechnique.m_uProgramID );

	oSphereTechnique.SetParameter( PARAM_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );

	for( const Cylinder& oCylinder : aCylinders )
	{
		glUseProgram( oSphereTechnique.m_uProgramID );

		oSphereTechnique.SetParameter( PARAM_RADIUS, 1.f );
		oSphereTechnique.SetParameter( PARAM_COLOR, oCylinder.m_vColor );

		Array< GLfloat > aCircleVertices = GenerateCylinderEquator( glm::normalize( oCylinder.m_vTo - oCylinder.m_vFrom ), 0.5f * oCylinder.m_fRadius );
		glBufferData( GL_ARRAY_BUFFER, sizeof( aCircleVertices[ 0 ] ) * aCircleVertices.Count(), aCircleVertices.Data(), GL_STATIC_DRAW );

		oSphereTechnique.SetParameter( PARAM_POSITION, 0.5f * ( oCylinder.m_vFrom + oCylinder.m_vTo ) );
		glDrawArrays( GL_LINE_STRIP, 0, CIRCLE_SEGMENT_VERTEX_COUNT );

		aCircleVertices = GenerateCylinderEquator( glm::normalize( oCylinder.m_vTo - oCylinder.m_vFrom ), oCylinder.m_fRadius );
		glBufferData( GL_ARRAY_BUFFER, sizeof( aCircleVertices[ 0 ] ) * aCircleVertices.Count(), aCircleVertices.Data(), GL_STATIC_DRAW );

		oSphereTechnique.SetParameter( PARAM_POSITION, oCylinder.m_vTo );
		glDrawArrays( GL_LINE_STRIP, 0, CIRCLE_SEGMENT_VERTEX_COUNT );

		Technique& oLineTechnique = m_xLine->GetTechnique();
		glUseProgram( oLineTechnique.m_uProgramID );

		oLineTechnique.SetParameter( PARAM_COLOR, oCylinder.m_vColor );

		Array< GLfloat > aVertices;
		aVertices.Reserve( 24 );
		for( uint u = 0; u < 4; ++u )
		{
			const uint uIndex = u * ( CIRCLE_SEGMENT_COUNT / 4 ) * 3;

			const glm::vec3 vCircleVertex( aCircleVertices[ uIndex ], aCircleVertices[ uIndex + 1 ], aCircleVertices[ uIndex + 2 ] );
			PushVertex( aVertices, oCylinder.m_vFrom );
			PushVertex( aVertices, vCircleVertex + oCylinder.m_vTo );
		}

		glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

		glDrawArrays( GL_LINES, 0, 8 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

void DebugRenderer::RenderWireBoxes( const Array< Box >& aBoxes, const RenderContext& oRenderContext )
{
	Technique& oTechnique = m_xLine->GetTechnique();
	glUseProgram( oTechnique.m_uProgramID );

	oTechnique.SetParameter( PARAM_VIEW_PROJECTION, g_pRenderer->m_oCamera.GetViewProjectionMatrix() );

	Array< GLfloat > aVertices;
	aVertices.Reserve( 3 * 16 * aBoxes.Count() );
	for( const Box& oBox : aBoxes )
	{
		const glm::vec3 vP1 = oBox.m_vCenter - oBox.m_mAxes * oBox.m_vHalfSize;
		const glm::vec3 vP2 = vP1 + 2.f * oBox.m_vHalfSize.x * oBox.m_mAxes[ 0 ];
		const glm::vec3 vP3 = vP1 + 2.f * oBox.m_vHalfSize.x * oBox.m_mAxes[ 0 ] + 2.f * oBox.m_vHalfSize.z * oBox.m_mAxes[ 2 ];
		const glm::vec3 vP4 = vP1 + 2.f * oBox.m_vHalfSize.z * oBox.m_mAxes[ 2 ];
		const glm::vec3 vVerticalOffset = 2.f * oBox.m_vHalfSize.y * oBox.m_mAxes[ 1 ];

		PushVertex( aVertices, vP1);
		PushVertex( aVertices, vP2);
		PushVertex( aVertices, vP3);
		PushVertex( aVertices, vP4);

		PushVertex( aVertices, vP1 + vVerticalOffset );
		PushVertex( aVertices, vP2 + vVerticalOffset );
		PushVertex( aVertices, vP3 + vVerticalOffset );
		PushVertex( aVertices, vP4 + vVerticalOffset );

		PushVertex( aVertices, vP1 );
		PushVertex( aVertices, vP1 + vVerticalOffset );
		PushVertex( aVertices, vP2 );
		PushVertex( aVertices, vP2 + vVerticalOffset );
		PushVertex( aVertices, vP3 );
		PushVertex( aVertices, vP3 + vVerticalOffset );
		PushVertex( aVertices, vP4 );
		PushVertex( aVertices, vP4 + vVerticalOffset );
	}

	glBindVertexArray( m_uVertexArrayID );
	glBindBuffer( GL_ARRAY_BUFFER, m_uVertexBufferID );
	glBufferData( GL_ARRAY_BUFFER, sizeof( aVertices[ 0 ] ) * aVertices.Count(), aVertices.Data(), GL_STATIC_DRAW );

	for( uint u = 0; u < aBoxes.Count(); ++u )
	{
		oTechnique.SetParameter( PARAM_COLOR, aBoxes[ u ].m_vColor );
		glDrawArrays( GL_LINE_LOOP, 4 * u, 4 );
 		glDrawArrays( GL_LINE_LOOP, 4 * u + 4, 4 );
 		glDrawArrays( GL_LINES, 4 * u + 8, 8 );
	}

	glBindBuffer( GL_ARRAY_BUFFER, 0 );
	glBindVertexArray( 0 );

	glUseProgram( 0 );
}

bool DebugRenderer::OnLoading()
{
	return m_xLine->IsLoaded() && m_xSphere->IsLoaded();
}

Array< GLfloat > DebugRenderer::GenerateSphereEquator()
{
	Array< glm::vec3 > vPositions;
	vPositions.Reserve( WIRE_SPHERE_SEGMENT_COUNT );

	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x, 0.f, CIRCLE_POINTS[ u ].y ) );

	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x, 0.f, CIRCLE_POINTS[ 0 ].y ) );

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

Array< GLfloat > DebugRenderer::GenerateSphereMeridians()
{
	Array< glm::vec3 > vPositions;
	vPositions.Reserve( WIRE_SPHERE_SEGMENT_COUNT );

	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].y, CIRCLE_POINTS[ u ].x, 0.f ) );

	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].y, CIRCLE_POINTS[ 0 ].x, 0.f ) );

	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
		vPositions.PushBack( glm::vec3( 0.f, CIRCLE_POINTS[ u ].x, CIRCLE_POINTS[ u ].y ) );

	vPositions.PushBack( glm::vec3( 0.f, CIRCLE_POINTS[ 0 ].x, CIRCLE_POINTS[ 0 ].y ) );

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

Array< GLfloat > DebugRenderer::GenerateCylinderEquator( const glm::vec3& vNormal, const float fRadius )
{
	ASSERT( glm::isNormalized( vNormal, 0.001f ) );

	glm::vec3 vX;
	if( glm::epsilonEqual( glm::abs( glm::dot( vNormal, glm::vec3( 0.f, 1.f, 0.f ) ) ), 1.f, 0.001f ) )
		vX = glm::normalize( glm::cross( vNormal, glm::vec3( 0.f, 0.f, 1.f ) ) );
	else
		vX = glm::normalize( glm::cross( vNormal, glm::vec3( 0.f, 1.f, 0.f ) ) );

	glm::vec3 vZ = glm::normalize( glm::cross( vNormal, vX ) );
	glm::vec3 vY = glm::normalize( glm::cross( vX, vZ ) );

	glm::mat3 mMatrix;
	mMatrix[ 0 ] = vX;
	mMatrix[ 1 ] = vY;
	mMatrix[ 2 ] = vZ;

	Array< glm::vec3 > vPositions;
	vPositions.Reserve( WIRE_SPHERE_SEGMENT_COUNT );

	for( uint u = 0; u < CIRCLE_SEGMENT_COUNT; ++u )
		vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ u ].x, 0.f, CIRCLE_POINTS[ u ].y ) * fRadius);

	vPositions.PushBack( glm::vec3( CIRCLE_POINTS[ 0 ].x, 0.f, CIRCLE_POINTS[ 0 ].y ) * fRadius );

	Array< GLfloat > aVertices;
	aVertices.Reserve( 3 * vPositions.Count() );

	for( glm::vec3& vPosition : vPositions )
	{
		vPosition = mMatrix * vPosition;

		aVertices.PushBack( vPosition.x );
		aVertices.PushBack( vPosition.y );
		aVertices.PushBack( vPosition.z );
	}

	return aVertices;
}
