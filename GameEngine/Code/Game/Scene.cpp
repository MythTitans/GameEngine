#include "Scene.h"

#include "Core/ArrayUtils.h"
#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Entity.h"
#include "GameEngine.h"

inline constexpr uint64 ENTITIES_START_ID = 1024;

Scene::Scene()
	: m_uNextInternalID( 0 )
	, m_uNextID( ENTITIES_START_ID )
{
}

void Scene::Load( const std::string& sFilePath )
{
	CreateInternalEntities();

	const nlohmann::json oJsonContent = nlohmann::json::parse( ReadTextFile( std::filesystem::path( sFilePath ) ) );

	for( const auto& oEntityIt : oJsonContent[ "scene" ].items() )
	{
		const nlohmann::json& oEntity = oEntityIt.value();

		const uint64 uEntityID = oEntity[ "id" ];
		const std::string& sName = oEntity[ "name" ];
		LOG_INFO( "Create entity {} (id : {})", sName, uEntityID );
		m_mEntities[ uEntityID ] = new Entity( uEntityID, sName );
		Entity* pEntity = m_mEntities[ uEntityID ].GetPtr();

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

		for( const auto& oComponentIt : oEntityIt.value()[ "components" ].items() )
		{
			const nlohmann::json& oComponent = oComponentIt.value();

			const std::string& sComponentName = oComponent[ "name" ];
			const std::function< void( Entity* ) >& pCreateComponentFunction = ComponentManager::GetComponentsFactory()[ sComponentName ];
			pCreateComponentFunction( pEntity );

			g_pComponentManager->DeserializeComponents( oComponent[ "properties" ], pEntity );
		}
	}
}

void Scene::Save( const std::string& sFilePath )
{
	Array< nlohmann::json > aSerializedEntities;
	aSerializedEntities.Reserve( ( uint )m_mEntities.size() );

	for( const auto& it : m_mEntities )
	{
		if( it.first >= ENTITIES_START_ID )
			aSerializedEntities.PushBack( *it.second.GetPtr() );
	}

	nlohmann::json oJsonContent;
	oJsonContent[ "scene" ] = aSerializedEntities;
	WriteTextFile( oJsonContent.dump( 4 ), std::filesystem::path( sFilePath ) );
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

void Scene::CreateInternalEntities()
{
	uint64 uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "X" );
	GizmoComponent* oTranslateGizmoX = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oTranslateGizmoX->Setup( GizmoType::TRANSLATE, GizmoAxis::X );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "Y" );
	GizmoComponent* oTranslateGizmoY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oTranslateGizmoY->Setup( GizmoType::TRANSLATE, GizmoAxis::Y );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "Z" );
	GizmoComponent* oTranslateGizmoZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oTranslateGizmoZ->Setup( GizmoType::TRANSLATE, GizmoAxis::Z );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "XY" );
	GizmoComponent* oTranslateGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oTranslateGizmoXY->Setup( GizmoType::TRANSLATE, GizmoAxis::XY );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "YZ" );
	GizmoComponent* oTranslateGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oTranslateGizmoYZ->Setup( GizmoType::TRANSLATE, GizmoAxis::YZ );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "XZ" );
	GizmoComponent* oTranslateGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oTranslateGizmoXZ->Setup( GizmoType::TRANSLATE, GizmoAxis::XZ );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "GiroXY" );
	GizmoComponent* oGiroGizmoXY = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oGiroGizmoXY->Setup( GizmoType::ROTATE, GizmoAxis::XY );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "GiroYZ" );
	GizmoComponent* oGiroGizmoYZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oGiroGizmoYZ->Setup( GizmoType::ROTATE, GizmoAxis::YZ );

	uEntityID = GenerateInternalID();
	m_mEntities[ uEntityID ] = new Entity( uEntityID, "GiroXZ" );
	GizmoComponent* oGiroGizmoXZ = g_pComponentManager->CreateComponent< GizmoComponent >( m_mEntities[ uEntityID ].GetPtr() );
	oGiroGizmoXZ->Setup( GizmoType::ROTATE, GizmoAxis::XZ );
}

uint64 Scene::GenerateInternalID()
{
	return m_uNextInternalID++;
}

uint64 Scene::GenerateID()
{
	return m_uNextID++;
}
