#include "Component.h"

#include "GameEngine.h"
#include "Scene.h"

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

MyFirstComponent::MyFirstComponent( const Entity& oEntity )
	: Component( oEntity )
	, m_fScale( 1.f )
{
}

void MyFirstComponent::Update( const float /*fDeltaTime*/ )
{
	GetEntity().SetScale( 1.f, m_fScale, 1.f );
}

void MyFirstComponent::SetScale( const float fScale )
{
	m_fScale = fScale;
}

VisualComponent::VisualComponent( const Entity& oEntity )
	: Component( oEntity )
{
}

void VisualComponent::Setup( const ModelResPtr& xResource )
{
	m_xResource = xResource;
}

void VisualComponent::Start()
{
	m_hTest = GetComponent< MyFirstComponent >();
}

void VisualComponent::Update( const float fDeltaTime )
{
	if( m_hTest.IsValid() )
		m_hTest->SetScale( 2.f );
}

const ModelResPtr& VisualComponent::GetResource() const
{
	return m_xResource;
}
