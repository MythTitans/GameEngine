#include "ProceduralGridGenerator.h"

#include <random>

#include "Entity.h"
#include "GameWorld.h"
#include "Graphics/Visual.h"

REGISTER_COMPONENT( ProceduralGridGenerator );

ProceduralGridGenerator::ProceduralGridGenerator( Entity* pEntity )
	: Component( pEntity )
{
}

void ProceduralGridGenerator::Start()
{
	Generate();
}

#ifdef EDITOR
bool ProceduralGridGenerator::DisplayInspector()
{
	if( ImGui::Button( "Generate" ) )
	{
		Clear();
		Generate();
	}

	if( ImGui::Button( "Clear" ) )
		Clear();

	return false;
}
#endif

void ProceduralGridGenerator::Generate()
{
	std::random_device oRandomDevice;
	std::mt19937 oRandomGenerator( oRandomDevice() );

	std::uniform_real_distribution oSignVariation( -1.f, 1.f );
	std::uniform_real_distribution oXVariation( m_vMinVariationXOffset, m_vMaxVariationXOffset );
	std::uniform_real_distribution oYVariation( m_vMinVariationYOffset, m_vMaxVariationYOffset );
	std::uniform_real_distribution oZVariation( m_vMinVariationZOffset, m_vMaxVariationZOffset );

	const glm::vec3 vBaseOffset = glm::vec3( 0.5f * m_uWidth * m_fCellWidth - 0.5f * m_fCellWidth, 0.f, 0.5f * m_uHeight * m_fCellHeight - 0.5f * m_fCellHeight );

	for( uint uX = 0; uX < m_uWidth; ++uX )
	{
		for( uint uY = 0; uY < m_uHeight; ++uY )
		{
			const glm::vec3 vLocalOffset( glm::sign( oSignVariation( oRandomGenerator ) ) * oXVariation( oRandomGenerator ), glm::sign( oSignVariation( oRandomGenerator ) ) * oYVariation( oRandomGenerator ), glm::sign( oSignVariation( oRandomGenerator ) ) * oZVariation( oRandomGenerator ) );

			Entity* pEntity = g_pGameWorld->CreateEntity( std::format( "{}/{}", uX, uY ), GetEntity() );
			pEntity->SetPosition( glm::vec3( uX * m_fCellWidth, 0.f, uY * m_fCellHeight ) - vBaseOffset + vLocalOffset );

			VisualComponent* pVisual = g_pComponentManager->CreateComponent< VisualComponent >( pEntity, ComponentManagement::NONE );
			pVisual->Setup( "sphere.obj" );
			g_pComponentManager->InitializeComponent< VisualComponent >( pEntity, true );
		}
	}
}

void ProceduralGridGenerator::Clear()
{
	const Array< Entity* >& aChildren = GetEntity()->GetChildren();
	for( int i = GetEntity()->GetChildren().Count() - 1; i >= 0; --i )
		g_pGameWorld->RemoveEntity( aChildren[ i ] );
}
