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

bool Component::IsInitialized()
{
	return true;
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

void Component::DisplayGizmos( const bool /*bSelected*/ )
{
}

void Component::DisplayInspector()
{
}

Entity* Component::GetEntity()
{
	return m_pEntity;
}

const Entity* Component::GetEntity() const
{
	return m_pEntity;
}
