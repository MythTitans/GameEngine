#include "Component.h"

#include "GameEngine.h"
#include "Scene.h"

ComponentsHolderBase::~ComponentsHolderBase()
{
}

Component::Component( const Entity& oEntity )
	: m_uEntityID( oEntity.GetID() )
{
}

void Component::Start()
{
}

void Component::Stop()
{
}

void Component::Update( const float /*fDeltaTime*/ )
{
}

Entity& Component::GetEntity() const
{
	static Entity sFakeEntity( 0, "" );

	for( Entity& oEntity : g_pGameEngine->GetScene().m_aEntities )
	{
		if( oEntity.GetID() == m_uEntityID )
			return oEntity;
	}

	ASSERT( false );
	return sFakeEntity; // TODO #eric this is impossible and crappy, change this
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
