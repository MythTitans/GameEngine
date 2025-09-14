#include "Scene.h"

#include "Core/ArrayUtils.h"
#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Entity.h"
#include "GameEngine.h"

Scene::Scene()
	: m_uNextInternalID( 0 )
	, m_uNextID( ENTITIES_START_ID )
{
}

void Scene::Load( const nlohmann::json& oJsonContent )
{
	CreateInternalEntities();

	for( const auto& oEntityIt : oJsonContent[ "scene" ].items() )
	{
		const nlohmann::json& oEntity = oEntityIt.value();

		const uint64 uEntityID = oEntity[ "id" ];
		const std::string& sName = oEntity[ "name" ];

		Entity* pEntity = CreateEntity( sName, uEntityID );

		pEntity->SetPosition( oEntity[ "position" ] );
		pEntity->SetRotation( oEntity[ "rotation" ] );
		pEntity->SetScale( oEntity[ "scale" ] );
	}

	for( const auto& oEntityIt : oJsonContent[ "scene" ].items() )
	{
		const nlohmann::json& oEntity = oEntityIt.value();

		const uint64 uEntityID = oEntity[ "id" ];
		Entity* pEntity = m_mEntities[ uEntityID ].GetPtr();

		if( oEntity.contains( "parentId" ) )
		{
			const uint64 uParentID = oEntity[ "parentId" ];
			Entity* pParent = m_mEntities[ uParentID ].GetPtr();
			AttachToParent( pEntity, pParent );
		}

		if( oEntityIt.value().contains( "components" ) )
		{
			for( const auto& oComponentIt : oEntityIt.value()[ "components" ].items() )
			{
				const nlohmann::json& oComponent = oComponentIt.value();

				const std::string& sComponentName = oComponent[ "name" ];
				ComponentManager::GetComponentsFactory()[ sComponentName ].m_pCreate( pEntity, ComponentManagement::NONE );

				if( oComponent.contains( "properties" ) )
					g_pComponentManager->DeserializeComponent( sComponentName, oComponent[ "properties" ], pEntity );
			}
		}
	}
}

void Scene::Save( nlohmann::json& oJsonContent )
{
	Array< nlohmann::json > aSerializedEntities;
	aSerializedEntities.Reserve( ( uint )m_mEntities.size() );

	for( const auto& it : m_mEntities )
	{
		if( it.first >= ENTITIES_START_ID )
			aSerializedEntities.PushBack( *it.second.GetPtr() );
	}

	oJsonContent[ "scene" ] = aSerializedEntities;
}

Entity* Scene::CreateEntity( const std::string& sName )
{
	return CreateEntity( sName, GenerateID() );
}

Entity* Scene::CreateEntity( const std::string& sName, const uint64 uID )
{
	const bool bIDAlreadyExist = m_mEntities.find( uID ) != m_mEntities.cend();
	ASSERT( bIDAlreadyExist == false );

	if( bIDAlreadyExist )
	{
		LOG_WARN( "Cannot create entity {} (id : {}), ID already exists", sName, uID );
		return nullptr;
	}

	LOG_INFO( "Create entity {} (id : {})", sName, uID );

	UpdateID( uID );

	StrongPtr< Entity >& xEntity = m_mEntities[ uID ];
	xEntity = new Entity( uID, sName );

	return xEntity.GetPtr();
}

void Scene::RemoveEntity( const uint64 uEntityID )
{
	Entity* pEntity = FindEntity( uEntityID );
	if( pEntity != nullptr )
		RemoveEntity( pEntity );
}

void Scene::RemoveEntity( Entity* pEntity )
{
	LOG_INFO( "Remove entity {} (id : {})", pEntity->GetName(), pEntity->GetID() );

	for( int i = pEntity->m_aChildren.Count() - 1; i >= 0; --i )
	{
		Entity* pChild = pEntity->m_aChildren[ i ];
		RemoveEntity( pChild );
	}

	pEntity->m_aChildren.Clear();

	m_mEntities.erase( pEntity->GetID() );
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
	ASSERT( pParent != nullptr );
	if( pParent == nullptr )
		return;

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
		const int iIndex = Find( pParent->m_aChildren, pChild );
		if( iIndex != -1 )
			pParent->m_aChildren.Remove( iIndex );
	}

	pChild->m_pParent = nullptr;
}

void Scene::Clear()
{
	m_mEntities.clear();
}

void Scene::CreateInternalEntities()
{
	Entity* pEntity = CreateInternalEntity( "X" );
	GizmoComponent* oTranslateGizmoX = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oTranslateGizmoX->Setup( GizmoType::TRANSLATE, GizmoAxis::X );

	pEntity = CreateInternalEntity( "Y" );
	GizmoComponent* oTranslateGizmoY = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oTranslateGizmoY->Setup( GizmoType::TRANSLATE, GizmoAxis::Y );

	pEntity = CreateInternalEntity( "Z" );
	GizmoComponent* oTranslateGizmoZ = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oTranslateGizmoZ->Setup( GizmoType::TRANSLATE, GizmoAxis::Z );

	pEntity = CreateInternalEntity( "XY" );
	GizmoComponent* oTranslateGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oTranslateGizmoXY->Setup( GizmoType::TRANSLATE, GizmoAxis::XY );

	pEntity = CreateInternalEntity( "YZ" );
	GizmoComponent* oTranslateGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oTranslateGizmoYZ->Setup( GizmoType::TRANSLATE, GizmoAxis::YZ );

	pEntity = CreateInternalEntity( "XZ" );
	GizmoComponent* oTranslateGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oTranslateGizmoXZ->Setup( GizmoType::TRANSLATE, GizmoAxis::XZ );

	pEntity = CreateInternalEntity( "GiroXY" );
	GizmoComponent* oGiroGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oGiroGizmoXY->Setup( GizmoType::ROTATE, GizmoAxis::XY );

	pEntity = CreateInternalEntity( "GiroYZ" );
	GizmoComponent* oGiroGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oGiroGizmoYZ->Setup( GizmoType::ROTATE, GizmoAxis::YZ );

	pEntity = CreateInternalEntity( "GiroXZ" );
	GizmoComponent* oGiroGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( pEntity, ComponentManagement::NONE );
	oGiroGizmoXZ->Setup( GizmoType::ROTATE, GizmoAxis::XZ );
}

Entity* Scene::CreateInternalEntity( const std::string& sName )
{
	return CreateEntity( sName, GenerateInternalID() );
}

uint64 Scene::GenerateInternalID()
{
	return m_uNextInternalID++;
}

uint64 Scene::GenerateID()
{
	return m_uNextID++;
}

void Scene::UpdateID( const uint64 uID )
{
	if( uID < ENTITIES_START_ID )
	{
		if( m_uNextInternalID <= uID )
		{
			m_uNextInternalID = uID;
			m_uNextInternalID++;
		}
	}
	else
	{
		if( m_uNextID <= uID )
		{
			m_uNextID = uID;
			m_uNextID++;
		}
	}
}
