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
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->InitializeComponents();
}

void ComponentManager::InitializeComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->InitializeComponent( pEntity );
}

bool ComponentManager::AreComponentsInitialized() const
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		if( pHolder->AreComponentsInitialized() == false )
			return false;
	}

	return true;
}

void ComponentManager::StartPendingComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->StartPendingComponents();
}

void ComponentManager::StartComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->StartComponents();
}

void ComponentManager::StartComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->StartComponent( pEntity );
}

void ComponentManager::StopComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->StopComponents();
}

void ComponentManager::StopComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->StopComponent( pEntity );
}

void ComponentManager::DisposeComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->DisposeComponent( pEntity );
}

void ComponentManager::TickComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->TickComponents();
}

void ComponentManager::NotifyBeforePhysicsOnComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->NotifyBeforePhysicsOnComponents();
}

void ComponentManager::NotifyAfterPhysicsOnComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->NotifyAfterPhysicsOnComponents();
}

void ComponentManager::UpdateComponents( const GameContext& oGameContext )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->UpdateComponents( oGameContext );
}

Array< nlohmann::json > ComponentManager::SerializeComponents( const Entity* pEntity )
{
	Array< nlohmann::json > aSerializedComponents;
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		const nlohmann::json oSerializedComponent = pHolder->SerializeComponent( pEntity );
		if( oSerializedComponent.is_null() == false )
			aSerializedComponents.PushBack( oSerializedComponent );
	}

	return aSerializedComponents;
}

void ComponentManager::DeserializeComponent( const std::string& sComponentName, const nlohmann::json& oJsonContent, Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->DeserializeComponent( sComponentName, oJsonContent, pEntity );
}

bool ComponentManager::DisplayInspector( Entity* pEntity )
{
	bool bModified = false;

	int iImGuiID = 0;
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ImGui::PushID( iImGuiID++ );
		bModified |= pHolder->DisplayInspector( pEntity );
		ImGui::PopID();
	}

	return bModified;
}

void ComponentManager::DisplayGizmos( const uint64 uSelectedEntityID )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
		pHolder->DisplayGizmos( uSelectedEntityID );
}
