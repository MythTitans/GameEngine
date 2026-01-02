#include "Skybox.h"

#include "Game/ResourceLoader.h"
#include "Math/GLMHelpers.h"
#include "Renderer.h"

Skybox::Skybox()
	: m_xSkybox( g_pResourceLoader->LoadTechnique( "Shader/skybox.tech" ) )
{
	Array< glm::vec3 > aVertices( 8 );
	aVertices[ 0 ] = glm::vec3( 1.f, 1.f, 1.f );
	aVertices[ 1 ] = glm::vec3( -1.f, 1.f, 1.f );
	aVertices[ 2 ] = glm::vec3( -1.f, 1.f, -1.f );
	aVertices[ 3 ] = glm::vec3( 1.f, 1.f, -1.f );
	aVertices[ 4 ] = glm::vec3( 1.f, -1.f, 1.f );
	aVertices[ 5 ] = glm::vec3( -1.f, -1.f, 1.f );
	aVertices[ 6 ] = glm::vec3( -1.f, -1.f, -1.f );
	aVertices[ 7 ] = glm::vec3( 1.f, -1.f, -1.f );

	Array< GLuint > aIndices( 36 );
	aIndices[ 0 ] = 0; aIndices[ 1 ] = 1; aIndices[ 2 ] = 2;
	aIndices[ 3 ] = 0; aIndices[ 4 ] = 2; aIndices[ 5 ] = 3;
	aIndices[ 6 ] = 6; aIndices[ 7 ] = 5; aIndices[ 8 ] = 4;
	aIndices[ 9 ] = 7; aIndices[ 10 ] = 6; aIndices[ 11 ] = 4;
	aIndices[ 12 ] = 3; aIndices[ 13 ] = 2; aIndices[ 14 ] = 6;
	aIndices[ 15 ] = 3; aIndices[ 16 ] = 6; aIndices[ 17 ] = 7;
	aIndices[ 18 ] = 5; aIndices[ 19 ] = 1; aIndices[ 20 ] = 0;
	aIndices[ 21 ] = 4; aIndices[ 22 ] = 5; aIndices[ 23 ] = 0;
	aIndices[ 24 ] = 6; aIndices[ 25 ] = 2; aIndices[ 26 ] = 1;
	aIndices[ 27 ] = 5; aIndices[ 28 ] = 6; aIndices[ 29 ] = 1;
	aIndices[ 30 ] = 0; aIndices[ 31 ] = 3; aIndices[ 32 ] = 7;
	aIndices[ 33 ] = 0; aIndices[ 34 ] = 7; aIndices[ 35 ] = 4;

	m_oMesh = MeshBuilder( std::move( aVertices ), std::move( aIndices ) ).Build();
}

void Skybox::Render( const Sky* pSky, const RenderContext& /*oRenderContext*/ )
{
	glDepthMask( GL_FALSE );

	Technique& oTechnique = m_xSkybox->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	const glm::mat4 mViewProjection = g_pRenderer->m_oCamera.GetProjectionMatrix() * ToMat4( glm::mat3( g_pRenderer->m_oCamera.GetViewMatrix() ) );
	m_oSkyboxSheet.GetParameter( SkyboxParam::VIEW_PROJECTION ).SetValue( mViewProjection );

	g_pRenderer->SetCubeMapSlot( pSky->m_oCubeMap, 0 );
	m_oSkyboxSheet.GetParameter( SkyboxParam::CUBE_MAP ).SetValue( 0 );

	g_pRenderer->DrawMesh( m_oMesh );

	g_pRenderer->ClearCubeMapSlot( 0 );

	glDepthMask( GL_TRUE );
}

bool Skybox::OnLoading()
{
	return m_xSkybox->IsLoaded();
}

void Skybox::OnLoaded()
{
	m_oSkyboxSheet.Init( m_xSkybox->GetTechnique() );
	m_oSkyboxSheet.BindParameter( SkyboxParam::VIEW_PROJECTION, "viewProjection" );
	m_oSkyboxSheet.BindParameter( SkyboxParam::CUBE_MAP, "cubeMap" );
}
