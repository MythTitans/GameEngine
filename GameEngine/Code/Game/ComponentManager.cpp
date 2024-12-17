#include "ComponentManager.h"

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
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->InitializeComponents();
}

bool ComponentManager::AreComponentsInitialized() const
{
	for( auto& oPair : m_mComponentsHolders )
	{
		if( oPair.second->AreComponentsInitialized() == false )
			return false;
	}

	return true;
}

void ComponentManager::StartComponents()
{
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->StartComponents();
}

void ComponentManager::StopComponents()
{
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->StopComponents();
}

void ComponentManager::UpdateComponents( const float fDeltaTime )
{
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->UpdateComponents( fDeltaTime );
}