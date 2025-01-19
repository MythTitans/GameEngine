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

GizmoComponent::GizmoComponent( Entity* pEntity )
	: Component( pEntity )
	, m_eGizmoType( GizmoType::TRANSLATE )
	, m_eGizmoAxis( GizmoAxis::X )
	, m_bEditing( false )
{
}

void GizmoComponent::Setup( const GizmoType eGizmoType, const GizmoAxis eGizmoAxis )
{
	m_eGizmoType = eGizmoType;
	m_eGizmoAxis = eGizmoAxis;
}

void GizmoComponent::Update( const float fDeltaTime )
{
	if( m_xAnchor != nullptr )
	{
		const float fDistance = glm::length( g_pRenderer->m_oCamera.GetPosition() - m_xAnchor->GetPosition() );
		float fBaseScale = 0.025f;

 		Entity* pEntity = GetEntity();
		pEntity->SetPosition( m_xAnchor->GetPosition() );
		pEntity->SetRotation( m_xAnchor->GetRotation() );

		const float fScale = fBaseScale * fDistance;
		pEntity->SetScale( fScale, fScale, fScale );
	}
}

const glm::vec3 GizmoComponent::GetColor() const
{
	if( m_bEditing )
		return glm::vec3( 1.f, 1.f, 0.f );

	switch( m_eGizmoAxis )
	{
	case GizmoAxis::X:
		return glm::vec3( 1.f, 0.f, 0.f );
	case GizmoAxis::Y:
		return glm::vec3( 0.f, 1.f, 0.f );
	case GizmoAxis::Z:
		return glm::vec3( 0.f, 0.f, 1.f );
	case GizmoAxis::XY:
		return glm::vec3( 0.5f, 0.5f, 1.f );
	case GizmoAxis::XZ:
		return glm::vec3( 0.5f, 1.f, 0.5f );
	case GizmoAxis::YZ:
		return glm::vec3( 1.f, 0.5f, 0.5f );
	}

	return glm::vec3( 1.f, 1.f, 1.f );
}

void GizmoComponent::SetAnchor( Entity* pEntity )
{
	m_xAnchor = pEntity;
}

void GizmoComponent::SetEditing( const bool bEditing )
{
	m_bEditing = bEditing;
}

GizmoComponent::GizmoType GizmoComponent::GetType() const
{
	return m_eGizmoType;
}

GizmoComponent::GizmoAxis GizmoComponent::GetAxis() const
{
	return m_eGizmoAxis;
}

glm::mat4 GizmoComponent::GetWorldMatrix() const
{
	return GetEntity()->GetMatrix();
}

DirectionalLightComponent::DirectionalLightComponent( Entity* pEntity )
	: Component( pEntity )
	, m_vDirection( 0.f, -1.f, 0.f )
	, m_vColor( 1.f )
	, m_fIntensity( 1.f )
{
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
	m_vPosition = GetEntity()->GetPosition();
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
	m_vPosition = GetEntity()->GetPosition();
}

const glm::vec3& SpotLightComponent::GetPosition() const
{
	return m_vPosition;
}
