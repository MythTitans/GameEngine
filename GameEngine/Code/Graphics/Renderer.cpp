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
	: m_xDeferredMaps( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/deferred_maps" ) ) )
	, m_xDeferredCompose( g_pResourceLoader->LoadTechnique( std::filesystem::path( "Data/deferred_compose" ) ) )
{
	glEnable( GL_CULL_FACE );

	glClearColor( 0.f, 0.f, 0.f, 1.f );

	Array< glm::vec3 > aVertices( 3 );
	aVertices[ 0 ] = glm::vec3( -3.f, -1.f, 0.f );
	aVertices[ 1 ] = glm::vec3( 1.f, -1.f, 0.f );
	aVertices[ 2 ] = glm::vec3( 1.f, 3.f, 0.f );

	Array< GLuint > aIndices( 3 );
	aIndices[ 0 ] = 0;
	aIndices[ 1 ] = 1;
	aIndices[ 2 ] = 2;

	Array< glm::vec2 > aUVs( 3 );
	aUVs[ 0 ] = glm::vec2( -1.f, 0.f );
	aUVs[ 1 ] = glm::vec2( 1.f, 0.f );
	aUVs[ 2 ] = glm::vec2( 1.f, 2.f );

	m_oRenderMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).WithUVs( std::move( aUVs ) ).Build();

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

	if( oRenderRect.m_uWidth != m_oRenderTarget.GetWidth() || oRenderRect.m_uHeight != m_oRenderTarget.GetHeight() )
	{
		m_oRenderTarget.Destroy();

		Array< TextureFormat > aFormats( 2 );
		aFormats[ 0 ] = TextureFormat::RGB;
		aFormats[ 1 ] = TextureFormat::NORMAL;
		m_oRenderTarget.Create( oRenderRect.m_uWidth, oRenderRect.m_uHeight, aFormats, true );

		m_oCamera.SetAspectRatio( oRenderContext.ComputeAspectRatio() );
	}

	SetRenderTarget( m_oRenderTarget );

	glClear( GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT );
	glEnable( GL_DEPTH_TEST );

	SetTechnique( m_xDeferredMaps->GetTechnique() );

	ArrayView< VisualComponent > aVisualComponents = g_pGameEngine->GetScene().m_oComponentManager.GetComponentsOfType< VisualComponent >();
	for( const VisualComponent& oVisualComponent : aVisualComponents )
	{
		m_oDeferredMaps.SetModelViewProjection( m_oCamera.GetViewProjectionMatrix() * oVisualComponent.GetEntity().GetMatrix().GetMatrix() );

		const Array< Mesh >& aMeshes = oVisualComponent.GetResource()->GetMeshes();
		for( const Mesh& oMesh : aMeshes )
		{
			if( oMesh.m_pMaterial != nullptr )
			{
				m_oDeferredMaps.SetDiffuseColor( oMesh.m_pMaterial->m_vDiffuseColor );

				if( oMesh.m_pMaterial->m_xDiffuseTextureResource != nullptr )
				{
					SetTextureSlot( oMesh.m_pMaterial->m_xDiffuseTextureResource->GetTexture(), 0 );
					m_oDeferredMaps.SetDiffuseTexture( 0 );
				}
				else
				{
					ClearTextureSlot( 0 );
				}
			}
			else
			{
				ClearTextureSlot( 0 );
			}

			DrawMesh( oMesh );
		}
	}

	ClearRenderTarget();

	glClear( GL_COLOR_BUFFER_BIT );
	glDisable( GL_DEPTH_TEST );

	SetTechnique( m_xDeferredCompose->GetTechnique() );

	SetTextureSlot( m_oRenderTarget.GetColorMap( 0 ), 0 );
	m_oDeferredCompose.SetColor( 0 );
	SetTextureSlot( m_oRenderTarget.GetColorMap( 1 ), 1 );
	m_oDeferredCompose.SetNormal( 1 );
	SetTextureSlot( m_oRenderTarget.GetDepthMap(), 2 );
	m_oDeferredCompose.SetDepth( 2 );
	m_oDeferredCompose.SetInverseViewProjection( m_oCamera.GetInverseViewProjectionMatrix() );
	DrawMesh( m_oRenderMesh );

	ClearTextureSlot( 0 );
	ClearTechnique();
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
	if( m_xDeferredMaps->IsLoaded() && m_oDeferredMaps.IsValid() == false )
		m_oDeferredMaps.Create( m_xDeferredMaps->GetTechnique() );

	if( m_xDeferredCompose->IsLoaded() && m_oDeferredCompose.IsValid() == false )
		m_oDeferredCompose.Create( m_xDeferredCompose->GetTechnique() );

	return m_xDeferredMaps->IsLoaded() && m_xDeferredCompose->IsLoaded() && m_oTextRenderer.OnLoading();
}

void Renderer::SetTechnique( const Technique& oTechnique )
{
	glUseProgram( oTechnique.m_uProgramID );
}

void Renderer::ClearTechnique()
{
	glUseProgram( 0 );
}

void Renderer::SetTextureSlot( const Texture& oTexture, const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_2D, oTexture.m_uTextureID );
}

void Renderer::ClearTextureSlot( const uint uTextureUnit )
{
	glActiveTexture( GL_TEXTURE0 + uTextureUnit );
	glBindTexture( GL_TEXTURE_2D, 0 );
}

void Renderer::SetRenderTarget( const RenderTarget& oRenderTarget )
{
	glBindFramebuffer( GL_FRAMEBUFFER, oRenderTarget.m_uFrameBufferID );
}

void Renderer::ClearRenderTarget()
{
	glBindFramebuffer( GL_FRAMEBUFFER, 0 );
}

void Renderer::DrawMesh( const Mesh& oMesh )
{
	glBindVertexArray( oMesh.m_uVertexArrayID );
	glDrawElements( GL_TRIANGLES, oMesh.m_iIndexCount, GL_UNSIGNED_INT, nullptr );
	glBindVertexArray( 0 );
}
