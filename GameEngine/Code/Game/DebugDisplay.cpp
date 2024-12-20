#include "DebugDisplay.h"

#include "Core/Profiler.h"

#include "Graphics/Renderer.h"

void DebugDisplay::NewFrame()
{
	m_aTexts.Clear();
}

void DebugDisplay::Display( const float fDeltaTime, const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "DebugDisplay" );

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

	glDisable( GL_DEPTH_TEST );
	g_pRenderer->GetTextRenderer().RenderText( m_aTexts, oRenderContext );
	g_pRenderer->GetTextRenderer().RenderText( m_aTimedTexts, oRenderContext );
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
