#include "Editor.h"

#define GLM_ENABLE_EXPERIMENTAL
#include "glm/gtc/color_space.hpp"
#include "glm/gtx/norm.hpp"

#include "Core/Profiler.h"
#include "Game/Entity.h"
#include "Game/GameEngine.h"
#include "Game/InputHandler.h"
#include "Game/Light.h"
#include "Game/Scene.h"
#include "Game/Visual.h"
#include "Graphics/Renderer.h"

static glm::vec3 EditableVector3( const char* sName, glm::vec3 vVector )
{
	ImGui::BeginGroup();

	const float fWidth = ( ImGui::GetContentRegionAvail().x - 100.f ) / 3.f;
	const float fSpacing = 2.f;

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.6f, 0.f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.7f, 0.f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.8f, 0.f, 0.f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}X", sName ).c_str(), &vVector.x, 0.1f);
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0, fSpacing );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.f, 0.6f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.f, 0.7f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.f, 0.8f, 0.f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}Y", sName ).c_str(), &vVector.y, 0.1f );
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0, fSpacing );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.f, 0.f, 0.6f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.f, 0.f, 0.7f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.f, 0.f, 0.8f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}Z", sName ).c_str(), &vVector.z, 0.1f);
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0 );

	ImGui::Text( sName );

	ImGui::EndGroup();

	return vVector;
}

Editor* g_pEditor = nullptr;

Editor::Editor()
	: m_uSelectedEntityID( UINT64_MAX )
	, m_uGizmoEntityID( UINT64_MAX )
	, m_vInitialEntityPosition( 0.f )
	, m_vMoveStartPosition( 0.f )
	, m_bDisplayEditor( false )
{
	g_pEditor = this;
}

Editor::~Editor()
{
	g_pEditor = nullptr;
}

void Editor::Update( const InputContext& oInputContext, const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Editor" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_EDITOR ) )
		m_bDisplayEditor = !m_bDisplayEditor;

	if( m_bDisplayEditor == false )
		return;

// 	g_pDebugDisplay->DisplayLine( glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 20.f, 0.f, 0.f ), glm::vec3( 1.f, 0.f, 0.f ) );
// 	g_pDebugDisplay->DisplayLine( glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 0.f, 20.f, 0.f ), glm::vec3( 0.f, 1.f, 0.f ) );
// 	g_pDebugDisplay->DisplayLine( glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 0.f, 0.f, 20.f ), glm::vec3( 0.f, 0.f, 1.f ) );
// 
// 	g_pDebugDisplay->DisplaySphere( glm::vec3( 20.f, 0.f, 0.f ), 0.3f, glm::vec3( 1.f, 0.f, 0.f ) );
// 	g_pDebugDisplay->DisplaySphere( glm::vec3( 0.f, 20.f, 0.f ), 0.3f, glm::vec3( 0.f, 1.f, 0.f ) );
// 	g_pDebugDisplay->DisplaySphere( glm::vec3( 0.f, 0.f, 20.f ), 0.3f, glm::vec3( 0.f, 0.f, 1.f ) );

	g_pComponentManager->DisplayGizmos( m_uSelectedEntityID );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_MOUSE_LEFT_PRESS ) && ImGui::GetIO().WantCaptureMouse == false )
	{
		if( m_uSelectedEntityID != UINT64_MAX )
		{
			const uint64 uGizmoEntityID = g_pRenderer->RenderPicking( oRenderContext, oInputContext.GetCursorX(), oInputContext.GetCursorY(), true );

			if( uGizmoEntityID != UINT64_MAX )
			{
				const Ray oRay = ComputeCursorViewRay( oInputContext, oRenderContext );

				Entity* pSelectedEntity = g_pGameEngine->GetScene().FindEntity( m_uSelectedEntityID );
				Entity* pGizmoEntity = g_pGameEngine->GetScene().FindEntity( uGizmoEntityID );
				GizmoComponent* pGizmoComponent = g_pComponentManager->GetComponent< GizmoComponent >( pGizmoEntity );
				if( pGizmoComponent != nullptr )
				{
					pGizmoComponent->SetEditing( true );
					m_uGizmoEntityID = uGizmoEntityID;

					m_vInitialEntityPosition = pSelectedEntity->GetWorldPosition();
					m_qInitialEntityRotation = pSelectedEntity->GetRotation();
					m_vMoveStartPosition = ProjectOnGizmo( oRay, *pGizmoComponent );
					m_vRotationAxis = [ pGizmoEntity ]( const GizmoAxis eAxis ) {
						glm::vec3 vAxis( 0.f );
						switch( eAxis )
						{
						case GizmoAxis::XY:
							vAxis = pGizmoEntity->GetWorldTransform().GetK();
							break;
						case GizmoAxis::YZ:
							vAxis = pGizmoEntity->GetWorldTransform().GetI();
							break;
						case GizmoAxis::XZ:
							vAxis = pGizmoEntity->GetWorldTransform().GetJ();
							break;
						}

						return vAxis;
					}( pGizmoComponent->GetAxis() );
				}
			}
		}
	}

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_MOUSE_LEFT_PRESSING ) && ImGui::GetIO().WantCaptureMouse == false )
	{
		if( m_uGizmoEntityID != UINT64_MAX )
		{
			const Ray oRay = ComputeCursorViewRay( oInputContext, oRenderContext );

			Entity* pSelectedEntity = g_pGameEngine->GetScene().FindEntity( m_uSelectedEntityID );
			Entity* pGizmoEntity = g_pGameEngine->GetScene().FindEntity( m_uGizmoEntityID );
			GizmoComponent* pGizmoComponent = g_pComponentManager->GetComponent< GizmoComponent >( pGizmoEntity );

			switch( pGizmoComponent->GetType() )
			{
			case GizmoType::TRANSLATE:
			{
				const glm::vec3 vNewPosition = m_vInitialEntityPosition + ProjectOnGizmo( oRay, *pGizmoComponent ) - m_vMoveStartPosition;
				pSelectedEntity->SetWorldPosition( vNewPosition );

				g_pDebugDisplay->DisplayLine( m_vInitialEntityPosition, vNewPosition, glm::vec3( 1.f, 1.f, 0.f ) );
				break;
			}
			case GizmoType::ROTATE:
			{
				const glm::vec3 vProjectedPosition = ProjectOnGizmo( oRay, *pGizmoComponent );
				const glm::vec3 vCenterToStart = glm::normalize( m_vMoveStartPosition - m_vInitialEntityPosition );
				const glm::vec3 vCenterToPoint = glm::normalize( vProjectedPosition - m_vInitialEntityPosition );

				const float fAngle = glm::acos( glm::dot( vCenterToStart, vCenterToPoint ) );

				if( fAngle > 0.f )
				{
					const glm::vec3 vAxis = glm::cross( vCenterToStart, vCenterToPoint );
					const float fSign = glm::sign( glm::dot( vAxis, m_vRotationAxis ) );

					const glm::quat qRotation = glm::rotate( glm::quat( 1.f, 0.f, 0.f, 0.f ), fSign * fAngle, m_vRotationAxis );
					pSelectedEntity->SetRotation( qRotation * m_qInitialEntityRotation );
				}

				g_pDebugDisplay->DisplayLine( m_vInitialEntityPosition, vProjectedPosition, glm::vec3( 1.f, 1.f, 0.f ) );
				break;
			}
			case GizmoType::SCALE:
				break;
			}
		}	
	}

	// TODO #eric handle multiple selection
	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_MOUSE_LEFT_RELEASE ) && ImGui::GetIO().WantCaptureMouse == false )
	{
		if( m_uGizmoEntityID != UINT64_MAX )
		{
			Entity* pEntity = g_pGameEngine->GetScene().FindEntity( m_uGizmoEntityID );
			GizmoComponent* pGizmoComponent = g_pComponentManager->GetComponent< GizmoComponent >( pEntity );

			pGizmoComponent->SetEditing( false );
			m_uGizmoEntityID = UINT64_MAX;
		}
		else
		{
			m_uSelectedEntityID = g_pRenderer->RenderPicking( oRenderContext, oInputContext.GetCursorX(), oInputContext.GetCursorY(), false );

			ArrayView< GizmoComponent > aGizmoComponents = g_pComponentManager->GetComponents< GizmoComponent >();

			if( m_uSelectedEntityID != UINT64_MAX )
			{
				Entity* pEntity = g_pGameEngine->GetScene().FindEntity( m_uSelectedEntityID );

				for( GizmoComponent& oGizmoComponent : aGizmoComponents )
					oGizmoComponent.SetAnchor( pEntity );
			}
			else
			{
				for( GizmoComponent& oGizmoComponent : aGizmoComponents )
					oGizmoComponent.SetAnchor( nullptr );
			}
		}
	}

	ImGui::Begin( "Editor" );

	if( ImGui::TreeNode( "Root" ) )
	{
		for( auto& it : g_pGameEngine->GetScene().m_mEntities )
		{
			if( ImGui::TreeNode( it.second->GetName() ) )
			{
				Entity* pEntity = it.second.GetPtr();
				EulerComponent* pEuler = g_pComponentManager->GetComponent< EulerComponent >( pEntity );

				pEntity->SetPosition( EditableVector3( "Position", pEntity->GetPosition() ) );

				glm::vec3 vEuler = pEuler->GetRotationEuler();
				vEuler = glm::vec3( glm::degrees( vEuler.x ), glm::degrees( vEuler.y ), glm::degrees( vEuler.z ) );
				vEuler = EditableVector3( "Rotation", vEuler );
				vEuler = glm::vec3( glm::radians( vEuler.x ), glm::radians( vEuler.y ), glm::radians( vEuler.z ) );
				pEuler->SetRotationEuler( vEuler );

				pEntity->SetScale( EditableVector3( "Scale", pEntity->GetScale() ) );

				DirectionalLightComponent* pDirectionalLightComponent = g_pComponentManager->GetComponent< DirectionalLightComponent >( it.second.GetPtr() );
				if( pDirectionalLightComponent != nullptr && ImGui::CollapsingHeader( "Directional light" ) )
				{
					ImGui::DragFloat( "Intensity", &pDirectionalLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ColorEdit( "Color", pDirectionalLightComponent->m_vColor );
				}

				PointLightComponent* pPointLightComponent = g_pComponentManager->GetComponent< PointLightComponent >( it.second.GetPtr() );
				if( pPointLightComponent != nullptr && ImGui::CollapsingHeader( "Point light" ) )
				{
					ImGui::DragFloat( "Intensity", &pPointLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ColorEdit( "Color", pPointLightComponent->m_vColor );
					ImGui::DragFloat( "Falloff min distance", &pPointLightComponent->m_fFalloffMinDistance, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::DragFloat( "Falloff max distance", &pPointLightComponent->m_fFalloffMaxDistance, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
				}

				SpotLightComponent* pSpotLightComponent = g_pComponentManager->GetComponent< SpotLightComponent >( it.second.GetPtr() );
				if( pSpotLightComponent != nullptr && ImGui::CollapsingHeader( "Spot light" ) )
				{
					ImGui::DragFloat( "Intensity", &pSpotLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ColorEdit( "Color", pSpotLightComponent->m_vColor );
					ImGui::DragFloat( "Inner angle", &pSpotLightComponent->m_fInnerAngle, 1.f, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::DragFloat( "Outer angle", &pSpotLightComponent->m_fOuterAngle, 1.f, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::DragFloat( "Falloff min distance", &pSpotLightComponent->m_fFalloffMinDistance, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::DragFloat( "Falloff max distance", &pSpotLightComponent->m_fFalloffMaxDistance, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
				}

				VisualComponent* pVisualComponent = g_pComponentManager->GetComponent< VisualComponent >( it.second.GetPtr() );
				if( pVisualComponent != nullptr && ImGui::CollapsingHeader( "Material" ) )
				{
					for( const Mesh& oMesh : pVisualComponent->GetMeshes() )
					{
						if( g_pMaterialManager->IsMaterialType< LitMaterialData >( oMesh.GetMaterial() ) )
						{
							LitMaterialData oMaterialData = g_pMaterialManager->GetMaterial< LitMaterialData >( oMesh.GetMaterial() );
							ColorEdit( "Diffuse color", oMaterialData.m_vDiffuseColor );
							ColorEdit( "Specular color", oMaterialData.m_vSpecularColor );
							ColorEdit( "Emissive color", oMaterialData.m_vEmissiveColor );
							ImGui::DragFloat( "Shininess", &oMaterialData.m_fShininess );
							TexturePreview( "Diffuse map", oMaterialData.m_xDiffuseTextureResource.GetPtr() );
							TexturePreview( "Normal map", oMaterialData.m_xNormalTextureResource.GetPtr() );
							TexturePreview( "Specular map", oMaterialData.m_xSpecularTextureResource.GetPtr() );
							TexturePreview( "Emissive map", oMaterialData.m_xEmissiveTextureResource.GetPtr() );
							g_pMaterialManager->UpdateMaterial( oMesh.GetMaterial(), oMaterialData );
						}
					}
				}

				ImGui::TreePop();
			}
		}

		ImGui::TreePop();
	}

	ImGui::End();
}

void Editor::Render( const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Editor" );

	if( m_bDisplayEditor )
	{
		if( m_uSelectedEntityID != UINT64_MAX )
		{
			const Array< const VisualNode* > aVisualNodes = g_pRenderer->m_oVisualStructure.FindNodes( m_uSelectedEntityID );
			for( const VisualNode* pVisualNode : aVisualNodes )
			{
				if( pVisualNode != nullptr )
					g_pRenderer->RenderOutline( oRenderContext, *pVisualNode );
			}

			g_pRenderer->RenderGizmos( oRenderContext );
		}
	}
}

Ray Editor::ComputeCursorViewRay( const InputContext& oInputContext, const RenderContext& oRenderContext ) const
{
	const float fWidth = ( float )oRenderContext.GetRenderRect().m_uWidth;
	const float fHeight = ( float )oRenderContext.GetRenderRect().m_uHeight;

	const glm::vec3 vEye = g_pRenderer->m_oCamera.GetPosition();
	const glm::vec2 vCursor( 2.f * oInputContext.GetCursorX() / fWidth - 1.f, -( 2.f * oInputContext.GetCursorY() / fHeight - 1.f ) );

	const glm::vec4 vCursorWorldH = g_pRenderer->m_oCamera.GetInverseViewProjectionMatrix() * glm::vec4( vCursor, -1.f, 1.f );
	const glm::vec3 vCursorWorld = vCursorWorldH / vCursorWorldH.w;

	return Ray( vEye, vCursorWorld - vEye );
}

glm::vec3 Editor::ProjectOnGizmo( const Ray& oRay, const GizmoComponent& oGizmo ) const
{
	const Transform& oTransform = oGizmo.GetEntity()->GetWorldTransform();

	auto ComputeClosestPlaneIntersection = [ &oTransform ]( const Ray& oRay, const Plane& oPlaneA, const Plane& oPlaneB ) {
		glm::vec3 vIntersectionA;
		glm::vec3 vIntersectionB;
		const bool bIntersectA = Intersect( oRay, oPlaneA, vIntersectionA );
		const bool bIntersectB = Intersect( oRay, oPlaneB, vIntersectionB );

		if( bIntersectA && bIntersectB )
		{
			const float fSqrDistanceA = glm::length2( oRay.m_vOrigin - vIntersectionA );
			const float fSqrDistanceB = glm::length2( oRay.m_vOrigin - vIntersectionB );

			return fSqrDistanceA <= fSqrDistanceB ? vIntersectionA : vIntersectionB;
		}

		if( bIntersectA )
			return vIntersectionA;

		if( bIntersectB )
			return vIntersectionB;

		return oTransform.GetO();
	};

	auto ComputePlaneIntersection = [ &oTransform ]( const Ray& oRay, const Plane& oPlane ) {
		glm::vec3 vIntersection;
		if( Intersect( oRay, oPlane, vIntersection ) )
			return vIntersection;

		return oTransform.GetO();
	};

	switch( oGizmo.GetAxis() )
	{
	case GizmoAxis::X:
	{
		const Plane oXYPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetJ() ) );
		const Plane oXZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetK() ) );
		const Segment oSegment( oTransform.GetO(), ComputeClosestPlaneIntersection( oRay, oXYPlane, oXZPlane ) );

		return Project( oSegment, oTransform.GetI() );
	}
	case GizmoAxis::Y:
	{
		const Plane oXYPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetJ() ) );
		const Plane oYZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetJ(), oTransform.GetK() ) );
		const Segment oSegment( oTransform.GetO(), ComputeClosestPlaneIntersection( oRay, oXYPlane, oYZPlane ) );

		return Project( oSegment, oTransform.GetJ() );
	}
	case GizmoAxis::Z:
	{
		const Plane oXZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetK() ) );
		const Plane oYZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetJ(), oTransform.GetK() ) );
		const Segment oSegment( oTransform.GetO(), ComputeClosestPlaneIntersection( oRay, oXZPlane, oYZPlane ) );

		return Project( oSegment, oTransform.GetK() );
	}
	case GizmoAxis::XY:
		return ComputePlaneIntersection( oRay, Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetJ() ) ) );
	case GizmoAxis::XZ:
		return ComputePlaneIntersection( oRay, Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetK() ) ) );
	case GizmoAxis::YZ:
		return ComputePlaneIntersection( oRay, Plane( oTransform.GetO(), glm::cross( oTransform.GetJ(), oTransform.GetK() ) ) );
	}

	return oTransform.GetO();
}

void Editor::ColorEdit( const char* sLabel, glm::vec3& vColor )
{
	glm::vec3 vSRGBColor = glm::convertLinearToSRGB( vColor );
	if( ImGui::ColorEdit3( sLabel, &vSRGBColor.x ) )
		vColor = glm::convertSRGBToLinear( vSRGBColor );
}

void Editor::TexturePreview( const char* sLabel, const TextureResource* pTexture )
{
	if( pTexture == nullptr )
		return;

	const Texture& oTexture = pTexture->GetTexture();

	ImGui::Text( "%s :", sLabel );

	float fWidth = min( oTexture.GetWidth(), 128.f );
	float fHeight = min( oTexture.GetHeight(), 128.f );

	const ImVec2 vFrom = ImGui::GetCursorScreenPos();
	ImGui::Image( oTexture.GetID(), ImVec2( fWidth, fHeight ) );
	const ImVec2 vTo = ImVec2( vFrom.x + fWidth, vFrom.y + fHeight );
	
	if( ImGui::IsMouseHoveringRect( vFrom, vTo ) )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "%d x %d", oTexture.GetWidth(), oTexture.GetHeight() );
		fWidth = min( oTexture.GetWidth(), 512.f );
		fHeight = min( oTexture.GetHeight(), 512.f );
		ImGui::Image( oTexture.GetID(), ImVec2( fWidth, fHeight ) );
		ImGui::EndTooltip();
	}
}
