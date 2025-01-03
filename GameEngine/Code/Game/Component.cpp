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
	, m_fRotation( 0.f )
{
}

void MyFirstComponent::Update( const float fDeltaTime )
{
	m_fRotation += 10.f * fDeltaTime;
	GetEntity()->SetRotationY( glm::radians( m_fRotation ) );
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
	: VisualComponent( pEntity )
	, m_eGizmoType( GizmoType::TRANSLATE )
	, m_eGizmoAxis( GizmoAxis::X )
	, m_bEditing( false )
{
}

void GizmoComponent::Setup( const GizmoType eGizmoType, const GizmoAxis eGizmoAxis )
{
	m_eGizmoType = eGizmoType;
	m_eGizmoAxis = eGizmoAxis;

	switch( m_eGizmoType )
	{
	case GizmoType::TRANSLATE:
		VisualComponent::Setup( std::filesystem::path( "Data/Translate_gizmo.obj" ) );
		break;
	case GizmoType::ROTATE:
		break;
	case GizmoType::SCALE:
		break;
	}

	Entity* pEntity = GetEntity();

	switch( m_eGizmoAxis )
	{
	case GizmoAxis::X:
		pEntity->SetRotationY( glm::radians( 90.f ) );
		break;
	case GizmoAxis::Y:
		pEntity->SetRotationX( glm::radians( -90.f ) );
		break;
	case GizmoAxis::Z:
		break;
	case GizmoAxis::XY:
		break;
	case GizmoAxis::XZ:
		break;
	case GizmoAxis::YZ:
		break;
	case GizmoAxis::XYZ:
		break;
	}

	m_qAxisRotation = pEntity->GetRotation();
}

void GizmoComponent::Update( const float fDeltaTime )
{
	if( m_xAnchor != nullptr )
	{
		Entity* pEntity = GetEntity();
		pEntity->SetPosition( m_xAnchor->GetPosition() );
		pEntity->SetRotation( m_xAnchor->GetRotation() * m_qAxisRotation );

		const float fDistance = glm::length( g_pRenderer->m_oCamera.GetPosition() - pEntity->GetPosition() );
		const float fScale = 0.06f * fDistance;
		pEntity->SetScale( fScale, fScale, fScale );
	}

	VisualComponent::Update( fDeltaTime );
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
		break;
	case GizmoAxis::XZ:
		break;
	case GizmoAxis::YZ:
		break;
	case GizmoAxis::XYZ:
		break;
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
