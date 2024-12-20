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

Entity* Component::GetEntity()
{
	return m_pEntity;
}

const Entity* Component::GetEntity() const
{
	return m_pEntity;
}

MyFirstComponent::MyFirstComponent( Entity* pEntity )
	: Component( pEntity )
	, m_fScale( 1.f )
{
}

void MyFirstComponent::Update( const float /*fDeltaTime*/ )
{
	GetEntity()->SetScale( 1.f, m_fScale, 1.f );
}

void MyFirstComponent::SetScale( const float fScale )
{
	m_fScale = fScale;
}

VisualComponent::VisualComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void VisualComponent::Setup( const std::filesystem::path& oModelFile )
{
	m_oModelFile = oModelFile;
}

void VisualComponent::Initialize()
{
	m_xModel = g_pResourceLoader->LoadModel( m_oModelFile );
}

bool VisualComponent::IsInitialized()
{
	return m_xModel->IsLoaded();
}

void VisualComponent::Update( const float fDeltaTime )
{
	GetEntity()->SetScale( 1.f, 2.f, 1.f );

	m_mWorldMatrix = GetEntity()->GetMatrix();
}

const ModelResPtr& VisualComponent::GetResource() const
{
	return m_xModel;
}

const glm::mat4& VisualComponent::GetWorldMatrix() const
{
	return m_mWorldMatrix;
}
