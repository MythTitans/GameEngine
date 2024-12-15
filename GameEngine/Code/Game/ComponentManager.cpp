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

void ComponentManager::Start()
{
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->Start();
}

void ComponentManager::Stop()
{
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->Stop();
}

void ComponentManager::Update( const float fDeltaTime )
{
	for( auto& oPair : m_mComponentsHolders )
		oPair.second->Update( fDeltaTime );
}