#include "Renderer.h"

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Game/GameEngine.h"
#include "ImGui/imgui.h"

RenderRect::RenderRect()
	: m_uX( 0 )
	, m_uY( 0 )
	, m_uWidth( 0 )
	, m_uHeight( 0 )
{
}

void RenderContext::OnFrameBufferResized( int iWidth, int iHeight )
{
	if( iWidth <= 0 )
	{
		LOG_ERROR( "Framebuffer resized with width=[{}]", iWidth );
		iWidth = 0;
	}

	if( iHeight <= 0 )
	{
		LOG_ERROR( "Framebuffer resized with height=[{}]", iHeight );
		iHeight = 0;
	}

	m_oRenderRect.m_uWidth = ( uint )iWidth;
	m_oRenderRect.m_uHeight = ( uint )iHeight;

	ASSERT( m_oRenderRect.m_uWidth >= 0 );
	ASSERT( m_oRenderRect.m_uHeight >= 0 );
}

float RenderContext::ComputeAspectRatio() const
{
	return ( float )m_oRenderRect.m_uWidth / ( float )m_oRenderRect.m_uHeight;
}

Renderer* g_pRenderer = nullptr;

Renderer::Renderer()
	: m_xRenderTechnique( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/basic" ) ) )
{
	glEnable( GL_CULL_FACE );

	glClearColor( 0.f, 0.f, 0.f, 1.f );

	g_pRenderer = this;
}

Renderer::~Renderer()
{
	g_pRenderer = nullptr;
}

void Renderer::Render( const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Renderer" );

	const RenderRect& oRenderRect = oRenderContext.m_oRenderRect;
	glViewport( oRenderRect.m_uX, oRenderRect.m_uY, oRenderRect.m_uWidth, oRenderRect.m_uHeight );
	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );

	glEnable( GL_DEPTH_TEST );

	const Technique& oTechnique = m_xRenderTechnique->GetTechnique();

	glUseProgram( oTechnique.m_uProgramID );

	m_oBasicTechniqueDefinition.SetView( m_oCamera.GetViewMatrix() );
	m_oBasicTechniqueDefinition.SetProjection( m_oCamera.GetProjectionMatrix() );

	if( g_pGameEngine->GetScene().m_xCube->IsLoaded() )
	{
		for( const Mesh& oMesh : g_pGameEngine->GetScene().m_xCube->GetMeshes() )
		{
			glBindVertexArray( oMesh.m_uVertexArrayID );
			glDrawElements( GL_TRIANGLES, oMesh.m_iIndexCount, GL_UNSIGNED_INT, nullptr );
			glBindVertexArray( 0 );
		}
	}

	glUseProgram( 0 );
}

Camera& Renderer::GetCamera()
{
	return m_oCamera;
}

const Camera& Renderer::GetCamera() const
{
	return m_oCamera;
}

TextRenderer& Renderer::GetTextRenderer()
{
	return m_oTextRenderer;
}

const TextRenderer& Renderer::GetTextRenderer() const
{
	return m_oTextRenderer;
}

bool Renderer::OnLoading()
{
	if( m_xRenderTechnique->IsLoaded() && m_oBasicTechniqueDefinition.IsValid() == false )
		m_oBasicTechniqueDefinition.Create( m_xRenderTechnique->GetTechnique() );

	return m_xRenderTechnique->IsLoaded() && m_oTextRenderer.OnLoading();
}
