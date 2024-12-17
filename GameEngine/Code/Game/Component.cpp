#include "Component.h"

#include "GameEngine.h"
#include "Scene.h"

Component::Component( const Entity& oEntity )
	: m_uEntityID( oEntity.GetID() )
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

Entity& Component::GetEntity() const
{
	Entity* pEntity = g_pGameEngine->GetScene().FindEntity( m_uEntityID );
	ASSERT( pEntity != nullptr );

	return *pEntity;
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

void VisualComponent::Start()
{
	m_hTest = GetComponent< MyFirstComponent >();
}

void VisualComponent::Update( const float fDeltaTime )
{
	if( m_hTest.IsValid() )
		m_hTest->SetScale( 2.f );

	m_mWorldMatrix = GetEntity().GetMatrix().GetMatrix();
}

const ModelResPtr& VisualComponent::GetResource() const
{
	return m_xModel;
}

const glm::mat4& VisualComponent::GetWorldMatrix() const
{
	return m_mWorldMatrix;
}
