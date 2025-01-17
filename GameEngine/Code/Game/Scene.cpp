#include "Scene.h"

#include "Component.h"
#include "Entity.h"
#include "GameEngine.h"

Scene::Scene()
{
	m_mEntities[ 0 ] = new Entity( 0, "X" );
	m_mEntities[ 1 ] = new Entity( 1, "Y" );
	m_mEntities[ 2 ] = new Entity( 2, "Z" );
	m_mEntities[ 3 ] = new Entity( 3, "XY" );
	m_mEntities[ 4 ] = new Entity( 4, "YZ" );
	m_mEntities[ 5 ] = new Entity( 5, "XZ" );
	m_mEntities[ 6 ] = new Entity( 6, "Golem" );
	m_mEntities[ 7 ] = new Entity( 7, "Light 1" );
	m_mEntities[ 8 ] = new Entity( 8, "Light 2" );
	m_mEntities[ 9 ] = new Entity( 9, "Light 3" );
	m_mEntities[ 10 ] = new Entity( 10, "Light 4" );

	GizmoComponent& oTranslateGizmoX = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 0 ].GetPtr() );
	oTranslateGizmoX.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::X );

	GizmoComponent& oTranslateGizmoY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 1 ].GetPtr() );
	oTranslateGizmoY.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::Y );

	GizmoComponent& oTranslateGizmoZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 2 ].GetPtr() );
	oTranslateGizmoZ.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::Z );

	GizmoComponent& oTranslateGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 3 ].GetPtr() );
	oTranslateGizmoXY.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::XY );

	GizmoComponent& oTranslateGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 4 ].GetPtr() );
	oTranslateGizmoYZ.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::YZ );

	GizmoComponent& oTranslateGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 5 ].GetPtr() );
	oTranslateGizmoXZ.Setup( GizmoComponent::GizmoType::TRANSLATE, GizmoComponent::GizmoAxis::XZ );

	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 6 ].GetPtr() ).Setup( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) );

	m_mEntities[ 7 ]->SetPosition( -1.f, 10.f, 2.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 7 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight1 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 7 ].GetPtr() );
	oPointLight1.m_fIntensity = 10.f;
	oPointLight1.m_fFalloffFactor = 0.2f;

	m_mEntities[ 8 ]->SetPosition( 7.f, 5.f, 5.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 8 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight2 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 8 ].GetPtr() );
	oPointLight2.m_fIntensity = 10.f;
	oPointLight2.m_fFalloffFactor = 0.2f;

	g_pComponentManager->CreateComponent< DirectionalLightComponent >( m_mEntities[ 9 ].GetPtr() );
	g_pComponentManager->CreateComponent< SpotLightComponent >( m_mEntities[ 10 ].GetPtr() );
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
