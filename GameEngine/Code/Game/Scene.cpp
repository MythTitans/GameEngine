#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "GameEngine.h"

Scene::Scene()
{
	m_mEntities[ 0 ] = new Entity( 0, "Entity 1" );
	m_mEntities[ 1 ] = new Entity( 1, "Entity 2" );
	m_mEntities[ 2 ] = new Entity( 2, "Entity 3" );
	m_mEntities[ 3 ] = new Entity( 3, "Entity 4" );
	m_mEntities[ 4 ] = new Entity( 4, "Light 1" );
	m_mEntities[ 5 ] = new Entity( 5, "Light 2" );
	m_mEntities[ 6 ] = new Entity( 6, "Light 3" );
	m_mEntities[ 7 ] = new Entity( 7, "Light 4" );

	GizmoComponent& oTranslateGizmoX = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 0 ].GetPtr() );
	oTranslateGizmoX.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::X );

	GizmoComponent& oTranslateGizmoY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 1 ].GetPtr() );
	oTranslateGizmoY.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::Y );

	GizmoComponent& oTranslateGizmoZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 2 ].GetPtr() );
	oTranslateGizmoZ.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::Z );

	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 3 ].GetPtr() ).Setup( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) );

	m_mEntities[ 4 ]->SetPosition( -1.f, 10.f, 2.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 4 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight1 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 4 ].GetPtr() );
	oPointLight1.m_fIntensity = 10.f;
	oPointLight1.m_fFalloffFactor = 0.2f;

	m_mEntities[ 5 ]->SetPosition( 7.f, 5.f, 5.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 5 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight2 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 5 ].GetPtr() );
	oPointLight2.m_fIntensity = 10.f;
	oPointLight2.m_fFalloffFactor = 0.2f;

	g_pComponentManager->CreateComponent< DirectionalLightComponent >( m_mEntities[ 6 ].GetPtr() );
	g_pComponentManager->CreateComponent< SpotLightComponent >( m_mEntities[ 7 ].GetPtr() );
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
