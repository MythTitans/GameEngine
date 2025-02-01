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

VisualComponent::VisualComponent( Entity* pEntity )
	: Component( pEntity )
	, m_mWorldMatrix( 1.f )
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
	m_mWorldMatrix = GetEntity()->GetWorldTransform().GetMatrixTRS();
}

const ModelResPtr& VisualComponent::GetResource() const
{
	return m_xModel;
}

const glm::mat4& VisualComponent::GetWorldMatrix() const
{
	return m_mWorldMatrix;
}

DirectionalLightComponent::DirectionalLightComponent( Entity* pEntity )
	: Component( pEntity )
	, m_vDirection( 0.f, -1.f, 0.f )
	, m_vColor( 1.f )
	, m_fIntensity( 1.f )
{
}

void DirectionalLightComponent::Update( const float fDeltaTime )
{
	const glm::vec3 vPosition = GetEntity()->GetWorldPosition();
	g_pDebugDisplay->DisplayWireSphere( vPosition, 0.25f, glm::vec3( 1.f, 1.f, 0.f ) );
	g_pDebugDisplay->DisplayLine( vPosition, vPosition + m_vDirection, glm::vec3( 1.f, 1.f, 0.f ) );
}

PointLightComponent::PointLightComponent( Entity* pEntity )
	: Component( pEntity )
	, m_vPosition( 0.f )
	, m_vColor( 1.f )
	, m_fIntensity( 1.f )
	, m_fFalloffFactor( 1.f )
{
}

void PointLightComponent::Update( const float fDeltaTime )
{
	m_vPosition = GetEntity()->GetWorldPosition();

	g_pDebugDisplay->DisplayWireSphere( m_vPosition, 1.25f, glm::vec3( 1.f, 1.f, 0.f ) );
}

const glm::vec3& PointLightComponent::GetPosition() const
{
	return m_vPosition;
}

SpotLightComponent::SpotLightComponent( Entity* pEntity )
	: Component( pEntity )
	, m_vPosition( 0.f )
	, m_vDirection( 0.f, -1.f, 0.f )
	, m_vColor( 1.f )
	, m_fIntensity( 1.f )
	, m_fInnerAngle( 30.f )
	, m_fOuterAngle( 60.f )
	, m_fFalloffFactor( 1.f )
{
}

void SpotLightComponent::Update( const float fDeltaTime )
{
	m_vPosition = GetEntity()->GetWorldPosition();
}

const glm::vec3& SpotLightComponent::GetPosition() const
{
	return m_vPosition;
}
