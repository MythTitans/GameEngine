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
	m_mEntities[ 6 ] = new Entity( 6, "GiroXY" );
	m_mEntities[ 7 ] = new Entity( 7, "GiroYZ" );
	m_mEntities[ 8 ] = new Entity( 8, "GiroXZ" );
	m_mEntities[ 9 ] = new Entity( 9, "Golem" );
	m_mEntities[ 10 ] = new Entity( 10, "Light 1" );
	m_mEntities[ 11 ] = new Entity( 11, "Light 2" );
	m_mEntities[ 12 ] = new Entity( 12, "Light 3" );
	m_mEntities[ 13 ] = new Entity( 13, "Light 4" );

	GizmoComponent& oTranslateGizmoX = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 0 ].GetPtr() );
	oTranslateGizmoX.Setup( GizmoType::TRANSLATE, GizmoAxis::X );

	GizmoComponent& oTranslateGizmoY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 1 ].GetPtr() );
	oTranslateGizmoY.Setup( GizmoType::TRANSLATE, GizmoAxis::Y );

	GizmoComponent& oTranslateGizmoZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 2 ].GetPtr() );
	oTranslateGizmoZ.Setup( GizmoType::TRANSLATE, GizmoAxis::Z );

	GizmoComponent& oTranslateGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 3 ].GetPtr() );
	oTranslateGizmoXY.Setup( GizmoType::TRANSLATE, GizmoAxis::XY );

	GizmoComponent& oTranslateGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 4 ].GetPtr() );
	oTranslateGizmoYZ.Setup( GizmoType::TRANSLATE, GizmoAxis::YZ );

	GizmoComponent& oTranslateGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 5 ].GetPtr() );
	oTranslateGizmoXZ.Setup( GizmoType::TRANSLATE, GizmoAxis::XZ );

	GizmoComponent& oGiroGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 6 ].GetPtr() );
	oGiroGizmoXY.Setup( GizmoType::ROTATE, GizmoAxis::XY );

	GizmoComponent& oGiroGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 7 ].GetPtr() );
	oGiroGizmoYZ.Setup( GizmoType::ROTATE, GizmoAxis::YZ );

	GizmoComponent& oGiroGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ 8 ].GetPtr() );
	oGiroGizmoXZ.Setup( GizmoType::ROTATE, GizmoAxis::XZ );

	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 9 ].GetPtr() ).Setup( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) );

	m_mEntities[ 10 ]->SetPosition( -1.f, 10.f, 2.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 10 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight1 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 10 ].GetPtr() );
	oPointLight1.m_fIntensity = 10.f;
	oPointLight1.m_fFalloffFactor = 0.2f;

	m_mEntities[ 11 ]->SetPosition( 7.f, 5.f, 5.f );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 11 ].GetPtr() ).Setup( std::filesystem::path( "Data/sphere.obj" ) );
	PointLightComponent& oPointLight2 = g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 11 ].GetPtr() );
	oPointLight2.m_fIntensity = 10.f;
	oPointLight2.m_fFalloffFactor = 0.2f;

	g_pComponentManager->CreateComponent< DirectionalLightComponent >( m_mEntities[ 12 ].GetPtr() );
	g_pComponentManager->CreateComponent< SpotLightComponent >( m_mEntities[ 13 ].GetPtr() );
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
