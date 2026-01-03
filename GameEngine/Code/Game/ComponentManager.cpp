#include "ComponentManager.h"

#include <nlohmann/json.hpp>

#include "Core/Profiler.h"

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

	for( const auto& oPair : GetComponentsFactory() )
		oPair.second.m_pSetup();
}

ComponentManager::~ComponentManager()
{
	g_pComponentManager = nullptr;
}

void ComponentManager::InitializeComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->InitializeComponents();
	}
}

void ComponentManager::InitializeComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->InitializeComponent( pEntity );
	}
}

bool ComponentManager::AreComponentsInitialized() const
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		if( pHolder->AreComponentsInitialized() == false )
			return false;
	}

	return true;
}

void ComponentManager::StartPendingComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->StartPendingComponents();
	}
}

void ComponentManager::StartComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->StartComponents();
	}
}

void ComponentManager::StartComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->StartComponent( pEntity );
	}
}

void ComponentManager::StopComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->StopComponents();
	}
}

void ComponentManager::StopComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->StopComponent( pEntity );
	}
}

void ComponentManager::DisposeComponents( Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->DisposeComponent( pEntity );
	}
}

void ComponentManager::TickComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->TickComponents();
	}
}

void ComponentManager::NotifyBeforePhysicsOnComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->NotifyBeforePhysicsOnComponents();
	}
}

void ComponentManager::NotifyAfterPhysicsOnComponents()
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->NotifyAfterPhysicsOnComponents();
	}
}

void ComponentManager::UpdateComponents( const GameContext& oGameContext )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->UpdateComponents( oGameContext );
	}
}

Array< nlohmann::json > ComponentManager::SerializeComponents( const Entity* pEntity )
{
	Array< nlohmann::json > aSerializedComponents;
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		nlohmann::json oSerializedComponent;
		Array< nlohmann::json > aSerializedProperties;
		pHolder->SerializeComponent( oSerializedComponent, aSerializedProperties, pEntity );
		if( oSerializedComponent.is_null() == false )
			aSerializedComponents.PushBack( oSerializedComponent );
	}

	return aSerializedComponents;
}

void ComponentManager::DeserializeComponent( const std::string& sComponentName, const nlohmann::json& oJsonContent, Entity* pEntity )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->DeserializeComponent( sComponentName, oJsonContent, pEntity );
	}
}

#ifdef EDITOR
bool ComponentManager::DisplayInspector( Entity* pEntity )
{
	bool bModified = false;

	int iImGuiID = 0;
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		ImGui::PushID( iImGuiID++ );
		bModified |= pHolder->DisplayInspector( pEntity );
		ImGui::PopID();
	}

	return bModified;
}
#endif

void ComponentManager::DisplayGizmos( const uint64 uSelectedEntityID )
{
	for( ComponentsHolderBase* pHolder : m_aPriorityComponentsHolder )
	{
		ProfilerBlock oBlock( pHolder->GetConcreteComponentName().c_str() );

		pHolder->DisplayGizmos( uSelectedEntityID );
	}
}
