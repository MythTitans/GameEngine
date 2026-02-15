#include "Terrain.h"

#include "Game/ResourceLoader.h"
#include "Math/GLMHelpers.h"
#include "Renderer.h"

Terrain::Terrain()
	: m_xTerrain( g_pResourceLoader->LoadTechnique( "Shader/terrain.tech" ) )
{
}

void Terrain::Render( const TerrainNode* pTerrain, const RenderContext& oRenderContext )
{
	Technique& oTechnique = m_xTerrain->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	oTechnique.GetParameter( "modelViewProjection" ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pTerrain->m_mMatrix ) );
	oTechnique.GetParameter( "diffuseColor" ).SetValue( glm::vec3( 1.f, 1.f, 1.f ) );

	const TextureSlot oDiffuseSlot( pTerrain->m_oDiffuse, 0 );
	oTechnique.GetParameter( "diffuseMap" ).SetValue( 0 );

	for( const Mesh& oMesh : pTerrain->m_aMeshes )
		g_pRenderer->DrawMesh( oMesh );
}

bool Terrain::OnLoading()
{
	return m_xTerrain->IsLoaded();
}
