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

void Component::DisplayGizmos()
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
	, m_sModelFile( "" )
	, m_mWorldMatrix( 1.f )
{
}

void VisualComponent::Setup( const char* sModelFile )
{
	m_sModelFile = sModelFile;
}

void VisualComponent::Initialize()
{
	m_xModel = g_pResourceLoader->LoadModel( m_sModelFile.c_str() );
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
	m_vDirection = GetEntity()->GetWorldTransform().GetK();
}

void DirectionalLightComponent::DisplayGizmos()
{
	const glm::vec3 vPosition = GetEntity()->GetWorldPosition();
	g_pDebugDisplay->DisplaySphere( vPosition, 0.25f, glm::vec3( 1.f, 1.f, 0.f ) );
	g_pDebugDisplay->DisplayWireCylinder( vPosition, vPosition + 2.f * m_vDirection, 0.25f, glm::vec3( 1.f, 1.f, 0.f ) );
}

PointLightComponent::PointLightComponent( Entity* pEntity )
	: Component( pEntity )
	, m_vPosition( 0.f )
	, m_vColor( 1.f )
	, m_fIntensity( 1.f )
	, m_fFalloffMinDistance( 1.f )
	, m_fFalloffMaxDistance( 10.f )
{
}

void PointLightComponent::Update( const float fDeltaTime )
{
	m_vPosition = GetEntity()->GetWorldPosition();
}

void PointLightComponent::DisplayGizmos()
{
	g_pDebugDisplay->DisplaySphere( m_vPosition, 0.25f, glm::vec3( 1.f, 1.f, 0.f ) );
	g_pDebugDisplay->DisplayWireSphere( m_vPosition, m_fFalloffMinDistance, glm::vec3( 0.f, 0.5f, 1.f ) );
	g_pDebugDisplay->DisplayWireSphere( m_vPosition, m_fFalloffMaxDistance, glm::vec3( 1.f, 0.5f, 0.f ) );
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
	, m_fFalloffMinDistance( 1.f )
	, m_fFalloffMaxDistance( 10.f )
{
}

void SpotLightComponent::Update( const float fDeltaTime )
{
	const Transform& oTransform = GetEntity()->GetWorldTransform();
	m_vPosition = oTransform.GetO();
	m_vDirection = oTransform.GetK();
}

void SpotLightComponent::DisplayGizmos()
{
	const float fInnerRadius1 = glm::tan( glm::radians( m_fInnerAngle ) ) * m_fFalloffMinDistance;
	const float fInnerRadius2 = glm::tan( glm::radians( m_fInnerAngle ) ) * m_fFalloffMaxDistance;
	const float fOuterRadius = glm::tan( glm::radians( m_fOuterAngle ) ) * m_fFalloffMaxDistance;

	g_pDebugDisplay->DisplaySphere( m_vPosition, 0.25f, glm::vec3( 1.f, 1.f, 0.f ) );
	g_pDebugDisplay->DisplayWireCone( m_vPosition, m_vPosition + m_fFalloffMinDistance * m_vDirection, fInnerRadius1, glm::vec3( 0.f, 0.5f, 1.f ) );
	g_pDebugDisplay->DisplayWireCone( m_vPosition, m_vPosition + m_fFalloffMaxDistance * m_vDirection, fInnerRadius2, glm::vec3( 1.f, 0.5f, 1.f ) );
	g_pDebugDisplay->DisplayWireCone( m_vPosition, m_vPosition + m_fFalloffMaxDistance * m_vDirection, fOuterRadius, glm::vec3( 1.f, 0.5f, 0.f ) );
}

const glm::vec3& SpotLightComponent::GetPosition() const
{
	return m_vPosition;
}
