#include "Component.h"

#include "GameEngine.h"
#include "Entity.h"
#include "Scene.h"

Component::Component( Entity* pEntity )
	: m_pEntity( pEntity )
{
}

void Component::Initialize()
{
}

bool Component::IsInitialized() const
{
	return true;
}

void Component::Start()
{
}

void Component::Stop()
{
}

void Component::Tick()
{
}

void Component::BeforePhysics()
{
}

void Component::AfterPhysics()
{
}

void Component::Update( const GameContext& oGameContext )
{
}

void Component::Dispose()
{
}

void Component::DisplayGizmos( const bool bSelected )
{
}

#ifdef EDITOR
void Component::DisplayInspector()
{
}

void Component::OnPropertyChanged( const std::string& sProperty )
{
}
#endif

Entity* Component::GetEntity()
{
	return m_pEntity;
}

const Entity* Component::GetEntity() const
{
	return m_pEntity;
}

uint64 Component::GetEntityID() const
{
	return m_pEntity->GetID();
}
