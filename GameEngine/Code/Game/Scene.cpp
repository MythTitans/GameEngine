#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "GameEngine.h"

Scene::Scene()
{
	m_mEntities[ 0 ] = new Entity( 0, "Entity 1" );
	m_mEntities[ 1 ] = new Entity( 1, "Entity 2" );
	m_mEntities[ 2 ] = new Entity( 2, "Entity 3" );
	m_mEntities[ 3 ] = new Entity( 3, "Light 1" );
	m_mEntities[ 4 ] = new Entity( 4, "Light 2" );

// 	m_mEntities[ 0 ]->SetPosition( -5.f, 0.f, -5.f );
// 	//m_mEntities[ 0 ]->SetRotationY( glm::radians( 45.f ) );
// 	g_pComponentManager->CreateComponent< MyFirstComponent >( m_mEntities[ 0 ].GetPtr() );
// 	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 0 ].GetPtr() ).Setup( std::filesystem::path( "Data/cube.obj" ) );
// 
// 	m_mEntities[ 1 ]->SetPosition( 5.f, 0.f, -5.f );
// 	//m_mEntities[ 1 ]->SetRotationY( glm::radians( 90.f ) );
// 	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 1 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );

	m_mEntities[ 2 ]->SetPosition( 0.f, 0.f, 0.f );
	//m_mEntities[ 2 ]->SetRotationY( glm::radians( 135.f ) );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 2 ].GetPtr() ).Setup( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) );

	m_mEntities[ 3 ]->SetPosition( -1.f, 10.f, 2.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 3 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight1 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 3 ].GetPtr() );
	oPointLight1.m_fIntensity = 10.f;
	oPointLight1.m_fFalloffFactor = 0.2f;

	m_mEntities[ 4 ]->SetPosition( 7.f, 5.f, 5.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 4 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight2 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 4 ].GetPtr() );
	oPointLight2.m_fIntensity = 10.f;
	oPointLight2.m_fFalloffFactor = 0.2f;
}

Entity* Scene::FindEntity( const uint64 uEntityID )
{
	auto it = m_mEntities.find( uEntityID );
	if( it == m_mEntities.end() )
		return nullptr;

	return it->second.GetPtr();
}

const Entity* Scene::FindEntity( const uint64 uEntityID ) const
{
	auto it = m_mEntities.find( uEntityID );
	if( it == m_mEntities.end() )
		return nullptr;

	return it->second.GetPtr();
}
