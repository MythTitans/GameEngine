#include "Light.h"

#include <glm/glm.hpp>

#include "DebugDisplay.h"
#include "Editor/Inspector.h"
#include "Entity.h"
#include "Graphics/Mesh.h"
#include "Graphics/Renderer.h"

static void DisplayLightVisual( const Entity* pEntity, const glm::vec3& vColor )
{
	static Array< Mesh > s_aLightVisuals;
	static ModelResPtr s_xLightModel;
	static TechniqueResPtr s_xUnlitTechnique;

	if( s_aLightVisuals.Empty() )
	{
		if( s_xLightModel == nullptr )
			s_xLightModel = g_pResourceLoader->LoadModel( "sphere.obj" );

		if( s_xUnlitTechnique == nullptr )
			s_xUnlitTechnique = g_pResourceLoader->LoadTechnique( "Shader/unlit.tech" );

		if( s_xLightModel->IsLoaded() && s_xUnlitTechnique->IsLoaded() )
		{
			UnlitMaterialData oMaterialData;
			oMaterialData.m_vDiffuseColor = vColor;
			MaterialReference oMaterial = g_pMaterialManager->CreateMaterial( oMaterialData );

			s_aLightVisuals = s_xLightModel->GetMeshes();
			for( Mesh& oMesh : s_aLightVisuals )
				oMesh.SetMaterial( oMaterial );
		}
	}
	else
	{
		Transform oTransform = pEntity->GetWorldTransform();
		oTransform.SetScale( 0.25f, 0.25f, 0.25f );
		g_pRenderer->m_oVisualStructure.AddNode( pEntity, oTransform.GetMatrixTRS(), &s_aLightVisuals, nullptr, s_xUnlitTechnique->GetTechnique() );
	}
}

REGISTER_COMPONENT( DirectionalLightComponent );

DirectionalLightComponent::DirectionalLightComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void DirectionalLightComponent::Update( const float fDeltaTime )
{
	g_pRenderer->m_oVisualStructure.AddDirectionalLight( GetEntity(), m_vColor, m_fIntensity );
}

void DirectionalLightComponent::DisplayGizmos( const bool bSelected )
{
	DisplayLightVisual( GetEntity(), m_vColor );

	if( bSelected )
	{
		const Transform& oTransform = GetEntity()->GetWorldTransform();
		const glm::vec3 vPosition = oTransform.GetO();
		const glm::vec3 vDirection = oTransform.GetK();

		g_pDebugDisplay->DisplayWireCylinder( vPosition, vPosition + 2.f * vDirection, 0.25f, glm::vec3( 1.f, 1.f, 0.f ) );
	}
}

REGISTER_COMPONENT( PointLightComponent );

PointLightComponent::PointLightComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void PointLightComponent::Update( const float fDeltaTime )
{
	g_pRenderer->m_oVisualStructure.AddPointLight( GetEntity(), m_vColor, m_fIntensity, m_fFalloffMinDistance, m_fFalloffMaxDistance );
}

void PointLightComponent::DisplayGizmos( const bool bSelected )
{
	DisplayLightVisual( GetEntity(), m_vColor );

	if( bSelected )
	{
		const Transform& oTransform = GetEntity()->GetWorldTransform();
		const glm::vec3 vPosition = oTransform.GetO();

		g_pDebugDisplay->DisplayWireSphere( vPosition, m_fFalloffMinDistance, glm::vec3( 0.f, 0.5f, 1.f ) );
		g_pDebugDisplay->DisplayWireSphere( vPosition, m_fFalloffMaxDistance, glm::vec3( 1.f, 0.5f, 0.f ) );
	}
}

REGISTER_COMPONENT( SpotLightComponent );

SpotLightComponent::SpotLightComponent( Entity* pEntity )
	: Component( pEntity )
{
}

void SpotLightComponent::Update( const float fDeltaTime )
{
	g_pRenderer->m_oVisualStructure.AddSpotLight( GetEntity(), m_vColor, m_fIntensity, m_fInnerAngle, m_fOuterAngle, m_fFalloffMinDistance, m_fFalloffMaxDistance );
}

void SpotLightComponent::DisplayGizmos( const bool bSelected )
{
	DisplayLightVisual( GetEntity(), m_vColor );

	if( bSelected )
	{
		const Transform& oTransform = GetEntity()->GetWorldTransform();
		const glm::vec3 vPosition = oTransform.GetO();
		const glm::vec3 vDirection = oTransform.GetK();

		const float fInnerRadius1 = glm::tan( glm::radians( m_fInnerAngle ) ) * m_fFalloffMinDistance;
		const float fInnerRadius2 = glm::tan( glm::radians( m_fInnerAngle ) ) * m_fFalloffMaxDistance;
		const float fOuterRadius = glm::tan( glm::radians( m_fOuterAngle ) ) * m_fFalloffMaxDistance;

		g_pDebugDisplay->DisplayWireCone( vPosition, vPosition + m_fFalloffMinDistance * vDirection, fInnerRadius1, glm::vec3( 0.f, 0.5f, 1.f ) );
		g_pDebugDisplay->DisplayWireCone( vPosition, vPosition + m_fFalloffMaxDistance * vDirection, fInnerRadius2, glm::vec3( 1.f, 0.5f, 1.f ) );
		g_pDebugDisplay->DisplayWireCone( vPosition, vPosition + m_fFalloffMaxDistance * vDirection, fOuterRadius, glm::vec3( 1.f, 0.5f, 0.f ) );
	}
}

// This is kept as a reference for ranges, for the day I want properties to handle that
// void SpotLightComponent::DisplayInspector()
// {
// 	if( ImGui::CollapsingHeader( "Spot light" ) )
// 	{
// 		ImGui::DragFloat( "Intensity", &m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
// 		ColorEdit( "Color", m_vColor );
// 		ImGui::DragFloat( "Inner angle", &m_fInnerAngle, 1.f, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
// 		ImGui::DragFloat( "Outer angle", &m_fOuterAngle, 1.f, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
// 		ImGui::DragFloat( "Falloff min distance", &m_fFalloffMinDistance, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
// 		ImGui::DragFloat( "Falloff max distance", &m_fFalloffMaxDistance, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
// 	}
// }
