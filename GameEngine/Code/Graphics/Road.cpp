#include "Road.h"

#include "Game/ResourceLoader.h"
#include "Graphics/Renderer.h"
#include "MaterialManager.h"
#include "Math/GLMHelpers.h"

Road::Road()
	: m_xRoad( g_pResourceLoader->LoadTechnique( "Shader/unlit.tech" ) )
{
}

void Road::Render( const Array<RoadNode*>& aRoads, const RenderContext& oRenderContext )
{
	Technique& oTechnique = m_xRoad->GetTechnique();
	g_pRenderer->SetTechnique( oTechnique );

	oTechnique.GetParameter( "materialID" ).SetValue( MaterialManager::GetRoadMaterialID() );
	oTechnique.GetParameter( "diffuseMap" ).SetValue( 0 );

	TextureSlot oDiffuseSlot;

	for( const RoadNode* pRoad : aRoads )
	{
		oTechnique.GetParameter( "modelViewProjection" ).SetValue( g_pRenderer->m_oCamera.GetViewProjectionMatrix() * ToMat4( pRoad->m_mMatrix ) );

		oDiffuseSlot.SetSlot( pRoad->m_oDiffuse, 0 );

		g_pRenderer->DrawMesh( pRoad->m_oMesh );
	}
}

bool Road::OnLoading()
{
	return m_xRoad->IsLoaded();
}
