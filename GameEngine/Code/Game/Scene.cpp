#include "Scene.h"

#include "Component.h"
#include "GameEngine.h"

Scene::Scene()
{
	m_mEntities[ 0 ] = Entity( 0, "Entity 1" );
	m_mEntities[ 1 ] = Entity( 1, "Entity 2" );
	m_mEntities[ 2 ] = Entity( 2, "Entity 3" );
	m_mEntities[ 3 ] = Entity( 3, "Entity 4" );
	m_mEntities[ 4 ] = Entity( 4, "Entity 5" );

	m_mEntities[ 0 ].SetPosition( -5.f, 0.f, -5.f );
	m_mEntities[ 0 ].SetRotationY( glm::radians( 45.f ) );
	g_pComponentManager->CreateComponent< MyFirstComponent >( m_mEntities[ 0 ] );
	g_pComponentManager->CreateComponent<VisualComponent >( m_mEntities[ 0 ] ).Setup( std::filesystem::path( "Data/cube.obj" ) );

	m_mEntities[ 1 ].SetPosition( 5.f, 0.f, -5.f );
	m_mEntities[ 1 ].SetRotationY( glm::radians( 90.f ) );
	g_pComponentManager->CreateComponent<VisualComponent >( m_mEntities[ 1 ] ).Setup( std::filesystem::path( "Data/sphere.obj" ) );

	m_mEntities[ 2 ].SetPosition( -5.f, 0.f, 5.f );
	m_mEntities[ 2 ].SetRotationY( glm::radians( 135.f ) );
	g_pComponentManager->CreateComponent<VisualComponent >( m_mEntities[ 2 ] ).Setup( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) );

	m_mEntities[ 3 ].SetPosition( 5.f, 0.f, 5.f );
	m_mEntities[ 3 ].SetRotationY( glm::radians( 180.f ) );
	g_pComponentManager->CreateComponent<VisualComponent >( m_mEntities[ 3 ] ).Setup( std::filesystem::path( "Data/cube.obj" ) );

	m_mEntities[ 4 ].SetPosition( 0.f, 0.f, 0.f );
	m_mEntities[ 4 ].SetScale( 1.5f, 1.5f, 1.5f );
	g_pComponentManager->CreateComponent< MyFirstComponent >( m_mEntities[ 4 ] );
	g_pComponentManager->CreateComponent<VisualComponent >( m_mEntities[ 4 ] ).Setup( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) );
}

Entity* Scene::FindEntity( const uint64 uEntityID )
{
	auto it = m_mEntities.find( uEntityID );
	if( it == m_mEntities.end() )
		return nullptr;

	return &it->second;
}

const Entity* Scene::FindEntity( const uint64 uEntityID ) const
{
	auto it = m_mEntities.find( uEntityID );
	if( it == m_mEntities.end() )
		return nullptr;

	return &it->second;
}
