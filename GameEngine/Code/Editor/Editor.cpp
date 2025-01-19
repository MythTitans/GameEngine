#include "Editor.h"

 #define GLM_ENABLE_EXPERIMENTAL
 #include "glm/gtx/norm.hpp"

#include "Core/Profiler.h"
#include "Game/GameEngine.h"
#include "Game/Entity.h"
#include "Game/InputHandler.h"
#include "Game/Scene.h"
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
	, m_vInitialEntityWorldPosition( 0.f )
	, m_vMoveStartWorldPosition( 0.f )
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

	g_pDebugDisplay->DisplayLine( glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 20.f, 0.f, 0.f ), glm::vec3( 1.f, 0.f, 0.f ) );
	g_pDebugDisplay->DisplayLine( glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 0.f, 20.f, 0.f ), glm::vec3( 0.f, 1.f, 0.f ) );
	g_pDebugDisplay->DisplayLine( glm::vec3( 0.f, 0.f, 0.f ), glm::vec3( 0.f, 0.f, 20.f ), glm::vec3( 0.f, 0.f, 1.f ) );

	g_pDebugDisplay->DisplaySphere( glm::vec3( 20.f, 0.f, 0.f ), 0.3f, glm::vec3( 1.f, 0.f, 0.f ) );
	g_pDebugDisplay->DisplaySphere( glm::vec3( 0.f, 20.f, 0.f ), 0.3f, glm::vec3( 0.f, 1.f, 0.f ) );
	g_pDebugDisplay->DisplaySphere( glm::vec3( 0.f, 0.f, 20.f ), 0.3f, glm::vec3( 0.f, 0.f, 1.f ) );

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

					m_vInitialEntityWorldPosition = pSelectedEntity->GetPosition();
					m_vMoveStartWorldPosition = ProjectOnGizmo( oRay, *pGizmoComponent );
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

			const glm::vec3 vNewPosition = m_vInitialEntityWorldPosition + ProjectOnGizmo( oRay, *pGizmoComponent ) - m_vMoveStartWorldPosition;
			pSelectedEntity->SetPosition( vNewPosition );

			g_pDebugDisplay->DisplayLine( m_vInitialEntityWorldPosition, vNewPosition, glm::vec3( 1.f, 1.f, 0.f ) );
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
				Transform& oTransform = it.second->GetTransform();

				oTransform.SetPosition( EditableVector3( "Position", oTransform.GetPosition() ) );

				glm::vec3 vEuler = oTransform.GetRotationEuler();
				vEuler = glm::vec3( glm::degrees( vEuler.x ), glm::degrees( vEuler.y ), glm::degrees( vEuler.z ) );
				vEuler = EditableVector3( "Rotation", vEuler );
				vEuler = glm::vec3( glm::radians( vEuler.x ), glm::radians( vEuler.y ), glm::radians( vEuler.z ) );
				oTransform.SetRotationEuler( vEuler );

				oTransform.SetScale( EditableVector3( "Scale", oTransform.GetScale() ) );

				DirectionalLightComponent* pDirectionalLightComponent = g_pComponentManager->GetComponent< DirectionalLightComponent >( it.second.GetPtr() );
				if( pDirectionalLightComponent != nullptr && ImGui::CollapsingHeader( "Directional light" ) )
				{
					pDirectionalLightComponent->m_vDirection = EditableVector3( "Direction", pDirectionalLightComponent->m_vDirection );
					ImGui::DragFloat( "Intensity", &pDirectionalLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::ColorEdit3( "Color", &pDirectionalLightComponent->m_vColor.x );
				}

				PointLightComponent* pPointLightComponent = g_pComponentManager->GetComponent< PointLightComponent >( it.second.GetPtr() );
				if( pPointLightComponent != nullptr && ImGui::CollapsingHeader( "Point light" ) )
				{
					ImGui::DragFloat( "Intensity", &pPointLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::ColorEdit3( "Color", &pPointLightComponent->m_vColor.x );
					ImGui::DragFloat( "Falloff factor", &pPointLightComponent->m_fFalloffFactor, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
				}

				SpotLightComponent* pSpotLightComponent = g_pComponentManager->GetComponent< SpotLightComponent >( it.second.GetPtr() );
				if( pSpotLightComponent != nullptr && ImGui::CollapsingHeader( "Spot light" ) )
				{
					pSpotLightComponent->m_vDirection = EditableVector3( "Direction", pSpotLightComponent->m_vDirection );
					ImGui::DragFloat( "Intensity", &pSpotLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::ColorEdit3( "Color", &pSpotLightComponent->m_vColor.x );
					ImGui::DragFloat( "Inner angle", &pSpotLightComponent->m_fInnerAngle, 1.f, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::DragFloat( "Outer angle", &pSpotLightComponent->m_fOuterAngle, 1.f, 0.f, 90.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					ImGui::DragFloat( "Falloff factor", &pSpotLightComponent->m_fFalloffFactor, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
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
			Entity* pEntity = g_pGameEngine->GetScene().FindEntity( m_uSelectedEntityID );
			const VisualComponent* pVisualComponent = g_pComponentManager->GetComponent< VisualComponent >( pEntity );
			if( pVisualComponent != nullptr )
				g_pRenderer->RenderOutline( oRenderContext, *pVisualComponent );

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
	const Transform& oTransform = oGizmo.GetEntity()->GetTransform();

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
	case GizmoComponent::GizmoAxis::X:
	{
		const Plane oXYPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetJ() ) );
		const Plane oXZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetK() ) );
		const Segment oSegment( oTransform.GetO(), ComputeClosestPlaneIntersection( oRay, oXYPlane, oXZPlane ) );

		return Project( oSegment, oTransform.GetI() );
	}
	case GizmoComponent::GizmoAxis::Y:
	{
		const Plane oXYPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetJ() ) );
		const Plane oYZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetJ(), oTransform.GetK() ) );
		const Segment oSegment( oTransform.GetO(), ComputeClosestPlaneIntersection( oRay, oXYPlane, oYZPlane ) );

		return Project( oSegment, oTransform.GetJ() );
	}
	case GizmoComponent::GizmoAxis::Z:
	{
		const Plane oXZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetK() ) );
		const Plane oYZPlane = Plane( oTransform.GetO(), glm::cross( oTransform.GetJ(), oTransform.GetK() ) );
		const Segment oSegment( oTransform.GetO(), ComputeClosestPlaneIntersection( oRay, oXZPlane, oYZPlane ) );

		return Project( oSegment, oTransform.GetK() );
	}
	case GizmoComponent::GizmoAxis::XY:
		return ComputePlaneIntersection( oRay, Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetJ() ) ) );
	case GizmoComponent::GizmoAxis::XZ:
		return ComputePlaneIntersection( oRay, Plane( oTransform.GetO(), glm::cross( oTransform.GetI(), oTransform.GetK() ) ) );
	case GizmoComponent::GizmoAxis::YZ:
		return ComputePlaneIntersection( oRay, Plane( oTransform.GetO(), glm::cross( oTransform.GetJ(), oTransform.GetK() ) ) );
	}

	return oTransform.GetO();
}
