#include "Scene.h"

#include "Core/ArrayUtils.h"
#include "Entity.h"
#include "GameEngine.h"
#include "Light.h"
#include "Visual.h"

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
	m_mEntities[ 14 ] = new Entity( 14, "Env" );
	m_mEntities[ 15 ] = new Entity( 15, "Sphere" );

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

	//g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 9 ].GetPtr() ).Setup( "Earth_Golem_OBJ.obj" );
	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 9 ].GetPtr() ).Setup( "TestAnim.fbx" );
	m_mEntities[ 9 ]->SetScale( 0.05f, 0.05f, 0.05f );

	m_mEntities[ 10 ]->SetPosition( -1.f, 10.f, 2.f );
	g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 10 ].GetPtr() );

	m_mEntities[ 11 ]->SetPosition( 7.f, 5.f, 5.f );
	g_pComponentManager->CreateComponent< PointLightComponent >( m_mEntities[ 11 ].GetPtr() );

	m_mEntities[ 12 ]->SetRotationX( glm::radians( 90.f ) );
	g_pComponentManager->CreateComponent< DirectionalLightComponent >( m_mEntities[ 12 ].GetPtr() );

	m_mEntities[ 13 ]->SetRotationX( glm::radians( 90.f ) );
	g_pComponentManager->CreateComponent< SpotLightComponent >( m_mEntities[ 13 ].GetPtr() );

	AttachToParent( m_mEntities[ 10 ].GetPtr(), m_mEntities[ 9 ].GetPtr() );

	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 14 ].GetPtr() ).Setup( "plane.obj" );
//  	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 14 ].GetPtr() ).Setup( "sponza.obj" );
//  	m_mEntities[ 14 ]->SetScale( 0.05f, 0.05f, 0.05f );

	g_pComponentManager->CreateComponent< VisualComponent >( m_mEntities[ 15 ].GetPtr() ).Setup( "sphere.obj" );
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

void Scene::AttachToParent( Entity* pChild, Entity* pParent )
{
	ASSERT( pChild != nullptr );
	if( pChild == nullptr )
		return;

	ASSERT( pChild != pParent );
	if( pChild == pParent )
		return;

	DetachFromParent( pChild );
	pChild->m_pParent = pParent;

	const bool bAlreadyChild = Contains( pParent->m_aChildren, pChild );
	if( pParent != nullptr && bAlreadyChild == false )
		pParent->m_aChildren.PushBack( pChild );
}

void Scene::DetachFromParent( Entity* pChild )
{
	ASSERT( pChild != nullptr );
	if( pChild == nullptr )
		return;

	Entity* pParent = pChild->m_pParent;
	if( pParent != nullptr )
	{
		for( uint u = 0; u < pParent->m_aChildren.Count(); ++u )
		{
			if( pChild == pParent->m_aChildren[ u ] )
				pParent->m_aChildren.Remove( u );
		}
	}

	pChild->m_pParent = nullptr;
}
