#include "Gizmo.h"

#include "Game/Entity.h"
#include "Graphics/Renderer.h"

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

void GizmoComponent::Update( const GameContext& oGameContext )
{
	if( m_xAnchor != nullptr )
	{
		const Transform oTransform = m_xAnchor->GetWorldTransform();

		const float fDistance = glm::length( g_pRenderer->m_oCamera.GetPosition() - oTransform.GetPosition() );
		float fBaseScale = 0.025f;

		Entity* pEntity = GetEntity();
		pEntity->SetPosition( oTransform.GetPosition() );
		pEntity->SetRotation( oTransform.GetRotation() );

		const float fScale = fBaseScale * fDistance;
		pEntity->SetScale( fScale, fScale, fScale );
	}
}

const glm::vec3 GizmoComponent::GetColor() const
{
	if( m_bEditing )
		return glm::vec3( 1.f, 1.f, 0.f );

	switch( m_eGizmoType )
	{
	case GizmoType::TRANSLATE:
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
		break;
	case GizmoType::ROTATE:
		switch( m_eGizmoAxis )
		{
		case GizmoAxis::XY:
			return glm::vec3( 0.f, 0.f, 1.f );
		case GizmoAxis::XZ:
			return glm::vec3( 0.f, 1.f, 0.f );
		case GizmoAxis::YZ:
			return glm::vec3( 1.f, 0.f, 0.f );
		}
		break;
	case GizmoType::SCALE:
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

GizmoType GizmoComponent::GetType() const
{
	return m_eGizmoType;
}

GizmoAxis GizmoComponent::GetAxis() const
{
	return m_eGizmoAxis;
}

glm::mat4x3 GizmoComponent::GetWorldMatrix() const
{
	return GetEntity()->GetWorldTransform().GetMatrixTRS();
}