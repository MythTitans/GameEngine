#include "ComponentManager.h"

PropertiesHolderBase::~PropertiesHolderBase()
{
}

ComponentsHolderBase::ComponentsHolderBase()
	: m_uVersion( 0 )
{
}

ComponentsHolderBase::~ComponentsHolderBase()
{
}

ComponentManager* g_pComponentManager = nullptr;

ComponentManager::ComponentManager()
{
	g_pComponentManager = this;
}

ComponentManager::~ComponentManager()
{
	g_pComponentManager = nullptr;
}

void ComponentManager::InitializeComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->InitializeComponents();
}

bool ComponentManager::AreComponentsInitialized() const
{
	for( const auto& oPair : m_mComponentsHolders )
	{
		if( oPair.second->AreComponentsInitialized() == false )
			return false;
	}

	return true;
}

void ComponentManager::StartPendingComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->StartPendingComponents();
}

void ComponentManager::StartComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->StartComponents();
}

void ComponentManager::StopComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->StopComponents();
}

void ComponentManager::TickComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->TickComponents();
}

void ComponentManager::NotifyBeforePhysicsOnComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->NotifyBeforePhysicsOnComponents();
}

void ComponentManager::NotifyAfterPhysicsOnComponents()
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->NotifyAfterPhysicsOnComponents();
}

void ComponentManager::UpdateComponents( const GameContext& oGameContext )
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->UpdateComponents( oGameContext );
}

Array< nlohmann::json > ComponentManager::SerializeComponents( const Entity* pEntity )
{
	Array< nlohmann::json > aSerializedComponents;
	for( const auto& oPair : m_mComponentsHolders )
	{
		const nlohmann::json oSerializedComponent = oPair.second->SerializeComponent( pEntity );
		if( oSerializedComponent.is_null() == false )
			aSerializedComponents.PushBack( oSerializedComponent );
	}

	return aSerializedComponents;
}

void ComponentManager::DeserializeComponents( const nlohmann::json& oJsonContent, Entity* pEntity )
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->DeserializeComponent( oJsonContent, pEntity );
}

void ComponentManager::DisplayInspector( Entity* pEntity )
{
	int iImGuiID = 0;
	for( const auto& oPair : m_mComponentsHolders )
	{
		ImGui::PushID( iImGuiID++ );
		oPair.second->DisplayInspector( pEntity );
		ImGui::PopID();
	}
}

void ComponentManager::DisplayGizmos( const uint64 uSelectedEntityID )
{
	for( const auto& oPair : m_mComponentsHolders )
		oPair.second->DisplayGizmos( uSelectedEntityID );
}
