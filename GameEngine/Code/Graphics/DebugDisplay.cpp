#include "DebugDisplay.h"

#include "Core/Profiler.h"

#include "Graphics/Renderer.h"

DebugDisplay* g_pDebugDisplay = nullptr;

DebugDisplay::DebugDisplay()
{
	g_pDebugDisplay = this;
}

DebugDisplay::~DebugDisplay()
{
	g_pDebugDisplay = nullptr;
}

void DebugDisplay::NewFrame()
{
	m_aTexts.Clear();
	m_aLines.Clear();
	m_aSpheres.Clear();
	m_aWireSpheres.Clear();
	m_aWireCylinders.Clear();
	m_aWireCones.Clear();
	m_aWireBoxes.Clear();
	m_aWireMeshes.Clear();
}

void DebugDisplay::Display( const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "DebugDisplay" );
	GPUBlock oGPUBlock( "DebugDisplay" );

	glEnable( GL_DEPTH_TEST );
	g_pRenderer->m_oDebugRenderer.RenderLines( m_aLines, oRenderContext );
	g_pRenderer->m_oDebugRenderer.RenderSpheres( m_aSpheres, oRenderContext );
	g_pRenderer->m_oDebugRenderer.RenderWireSpheres( m_aWireSpheres, oRenderContext );
	g_pRenderer->m_oDebugRenderer.RenderWireCylinders( m_aWireCylinders, oRenderContext );
	g_pRenderer->m_oDebugRenderer.RenderWireCones( m_aWireCones, oRenderContext );
	g_pRenderer->m_oDebugRenderer.RenderWireBoxes( m_aWireBoxes, oRenderContext );
	g_pRenderer->m_oDebugRenderer.RenderWireMeshes( m_aWireMeshes, oRenderContext );
}

void DebugDisplay::DisplayOverlay( const float fDeltaTime, const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "DebugDisplayOverlay" );
	GPUBlock oGPUBlock( "DebugDisplayOverlay" );

	glDisable( GL_DEPTH_TEST );

	for( int i = m_aTimedTexts.Count() - 1; i >= 0; --i )
	{
		m_aTimedTextsRemaining[ i ] -= fDeltaTime;

		if( m_aTimedTextsRemaining[ i ] <= 0.f )
		{
			m_aTimedTexts.Remove( i );
			m_aTimedTextsRemaining.Remove( i );
		}
	}

	for( uint u = 0; u < m_aTimedTexts.Count(); ++u )
		m_aTimedTexts[ u ].m_vPosition = glm::vec2( 10.f, 10.f + ( m_aTexts.Count() + u ) * 20.f );

	g_pRenderer->m_oTextRenderer.RenderText( m_aTexts, oRenderContext );
	g_pRenderer->m_oTextRenderer.RenderText( m_aTimedTexts, oRenderContext );
}

void DebugDisplay::DisplayText( const std::string& sText, const glm::vec4& vColor /*= glm::vec4( 1.f, 1.f, 1.f, 1.f )*/ )
{
	m_aTexts.PushBack( Text( sText, glm::vec2( 10.f, 10.f + m_aTexts.Count() * 30.f ), vColor ) );
}

void DebugDisplay::DisplayText( const std::string& sText, const float fTime, const glm::vec4& vColor /*= glm::vec4( 1.f, 1.f, 1.f, 1.f )*/ )
{
	m_aTimedTexts.PushBack( Text( sText, glm::vec2( 10.f, 10.f ), vColor ) );
	m_aTimedTextsRemaining.PushBack( fTime );
}

void DebugDisplay::DisplayLine( const glm::vec3& vFrom, const glm::vec3& vTo, const glm::vec3& vColor )
{
	m_aLines.PushBack( Line( vFrom, vTo, vColor ) );
}

void DebugDisplay::DisplaySphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor )
{
	m_aSpheres.PushBack( Sphere( vPosition, fRadius, vColor ) );
}

void DebugDisplay::DisplayWireSphere( const glm::vec3& vPosition, const float fRadius, const glm::vec3& vColor )
{
	m_aWireSpheres.PushBack( Sphere( vPosition, fRadius, vColor ) );
}

void DebugDisplay::DisplayWireCylinder( const glm::vec3& vFrom, const glm::vec3& vTo, const float fRadius, const glm::vec3& vColor )
{
	m_aWireCylinders.PushBack( Cylinder( vFrom, vTo, fRadius, vColor ) );
}

void DebugDisplay::DisplayWireCone( const glm::vec3& vFrom, const glm::vec3& vTo, const float fRadius, const glm::vec3& vColor )
{
	m_aWireCones.PushBack( Cylinder( vFrom, vTo, fRadius, vColor ) );
}

void DebugDisplay::DisplayWireBox( const glm::vec3& vCenter, const glm::vec3& vHalfSize, const glm::mat3& mAxes, const glm::vec3& vColor )
{
	m_aWireBoxes.PushBack( Box( vCenter, vHalfSize, mAxes, vColor ) );
}

void DebugDisplay::DisplayWireMesh( const Mesh& oMesh, const glm::mat4x3& mMatrix, const glm::vec3& vColor )
{
	m_aWireMeshes.PushBack( WireMesh( oMesh, mMatrix, vColor ) );
}
