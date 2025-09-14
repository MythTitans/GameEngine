#include "Editor.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/color_space.hpp>
#include <glm/gtx/norm.hpp>
#include <nlohmann/json.hpp>

#include "Core/ArrayUtils.h"
#include "Core/FileUtils.h"
#include "Core/Logger.h"
#include "Core/Profiler.h"
#include "Core/Serialization.h"
#include "Game/Entity.h"
#include "Game/GameEngine.h"
#include "Game/InputHandler.h"
#include "Game/Scene.h"
#include "Graphics/Renderer.h"

static bool EditableVector3( const char* sName, glm::vec3& vVector )
{
	bool bModified = false;

	ImGui::BeginGroup();

	const float fWidth = ( ImGui::GetContentRegionAvail().x - 100.f ) / 3.f;
	const float fSpacing = 2.f;

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.6f, 0.f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.7f, 0.f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.8f, 0.f, 0.f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}X", sName ).c_str(), &vVector.x, 0.1f);
	bModified |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0, fSpacing );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.f, 0.6f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.f, 0.7f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.f, 0.8f, 0.f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}Y", sName ).c_str(), &vVector.y, 0.1f );
	bModified |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0, fSpacing );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.f, 0.f, 0.6f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.f, 0.f, 0.7f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.f, 0.f, 0.8f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}Z", sName ).c_str(), &vVector.z, 0.1f);
	bModified |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0 );

	ImGui::Text( sName );

	ImGui::EndGroup();

	return bModified;
}

SnapshotStore::SnapshotStore( const uint uCapacity )
	: m_aSnapshots( uCapacity )
	, m_uStart( 0 )
	, m_uEnd( 0 )
	, m_uCurrentEnd( 0 )
{
}

void SnapshotStore::Push( const bool bResetForward /*= true*/ )
{
	if( bResetForward == false )
	{
		if( m_uCurrentEnd == m_uEnd )
			m_uEnd = ( m_uEnd + 1 ) % m_aSnapshots.Capacity();
	}

	m_uCurrentEnd = ( m_uCurrentEnd + 1 ) % m_aSnapshots.Capacity();

	if( bResetForward )
		m_uEnd = m_uCurrentEnd;

	if( m_uEnd == m_uStart )
		m_uStart = ( m_uStart + 1 ) % m_aSnapshots.Capacity();
}

void SnapshotStore::Pop()
{
	ASSERT( m_uStart != m_uCurrentEnd );

	m_uCurrentEnd = ( m_uCurrentEnd - 1 + m_aSnapshots.Capacity() ) % m_aSnapshots.Capacity();
}

nlohmann::json& SnapshotStore::Back()
{
	ASSERT( m_uStart != m_uCurrentEnd );

	const uint uLastIndex = ( m_uCurrentEnd + m_aSnapshots.Capacity() - 1 ) % m_aSnapshots.Capacity();
	return m_aSnapshots[ uLastIndex ];
}

uint SnapshotStore::BackwardCount() const
{
	return ( m_uCurrentEnd + m_aSnapshots.Capacity() - m_uStart - 1) % m_aSnapshots.Capacity();
}

uint SnapshotStore::ForwardCount() const
{
	return ( m_uEnd + m_aSnapshots.Capacity() - m_uCurrentEnd ) % m_aSnapshots.Capacity();
}

Editor* g_pEditor = nullptr;

Editor::Editor()
	: m_uSelectedEntityID( UINT64_MAX )
	, m_uGizmoEntityID( UINT64_MAX )
	, m_vInitialEntityPosition( 0.f )
	, m_vMoveStartPosition( 0.f )
	, m_oSnapshotStore( 256 )
	, m_bDisplayEditor( false )
	, m_bStoreSnapshot( true )
{
	g_pEditor = this;
}

Editor::~Editor()
{
	g_pEditor = nullptr;
}

void Editor::OnSceneLoaded()
{
	if( m_bStoreSnapshot )
		StoreSnapshot();
	else
		m_bStoreSnapshot = true;
}

void Editor::Update( const InputContext& oInputContext, const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Editor" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_EDITOR ) )
		m_bDisplayEditor = !m_bDisplayEditor;

	if( m_bDisplayEditor == false )
		return;

	Entity* pSelectedEntity = g_pGameWorld->m_oScene.FindEntity( m_uSelectedEntityID );
	if( pSelectedEntity == nullptr )
		m_uSelectedEntityID = UINT64_MAX;

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

				Entity* pGizmoEntity = g_pGameWorld->m_oScene.FindEntity( uGizmoEntityID );
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

			Entity* pSelectedEntity = g_pGameWorld->m_oScene.FindEntity( m_uSelectedEntityID );
			Entity* pGizmoEntity = g_pGameWorld->m_oScene.FindEntity( m_uGizmoEntityID );
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

	bool bModified = false;

	// TODO #eric handle multiple selection
	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_MOUSE_LEFT_RELEASE ) && ImGui::GetIO().WantCaptureMouse == false )
	{
		if( m_uGizmoEntityID != UINT64_MAX )
		{
			Entity* pEntity = g_pGameWorld->m_oScene.FindEntity( m_uGizmoEntityID );
			GizmoComponent* pGizmoComponent = g_pComponentManager->GetComponent< GizmoComponent >( pEntity );

			pGizmoComponent->SetEditing( false );
			bModified = true;
			m_uGizmoEntityID = UINT64_MAX;
		}
		else
		{
			m_uSelectedEntityID = g_pRenderer->RenderPicking( oRenderContext, oInputContext.GetCursorX(), oInputContext.GetCursorY(), false );

			Array< GizmoComponent* > aGizmoComponents = g_pComponentManager->GetComponents< GizmoComponent >();

			if( m_uSelectedEntityID != UINT64_MAX )
			{
				Entity* pEntity = g_pGameWorld->m_oScene.FindEntity( m_uSelectedEntityID );

				for( GizmoComponent* pGizmoComponent : aGizmoComponents )
					pGizmoComponent->SetAnchor( pEntity );
			}
			else
			{
				for( GizmoComponent* pGizmoComponent : aGizmoComponents )
					pGizmoComponent->SetAnchor( nullptr );
			}
		}
	}

	ImGui::Begin( "Hierarchy" );

	ImGui::DragFloat( "Camera speed", &g_pGameWorld->m_oFreeCamera.m_fSpeed );
	ImGui::DragFloat( "Camera fast speed multiplier", &g_pGameWorld->m_oFreeCamera.m_fFastSpeedMultiplier );

	if( ImGui::Button( "Save scene" ) )
	{
		nlohmann::json oJsonContent;
		g_pGameWorld->m_oScene.Save( oJsonContent );
		WriteTextFile( oJsonContent.dump( 4 ), std::filesystem::path( "Data/Scene/test.scene" ) );
	}

	if( g_pGameEngine->m_eGameState == GameEngine::GameState::EDITING && ImGui::Button( "Start running " ) )
	{
		g_pGameEngine->m_eGameState = GameEngine::GameState::RUNNING;
		m_oSceneJson.clear();
		g_pGameWorld->m_oScene.Save( m_oSceneJson );
		g_pGameWorld->Run();
	}
	else if( g_pGameEngine->m_eGameState == GameEngine::GameState::RUNNING && ImGui::Button( "Stop running " ) )
	{
		g_pGameEngine->m_eGameState = GameEngine::GameState::EDITING;
		g_pGameWorld->m_oSceneJson = m_oSceneJson;
		g_pGameWorld->Reset();
	}

	if( ImGui::TreeNode( "Root" ) )
	{
		if( ImGui::BeginPopupContextItem( "ContextMenu" ) )
		{
			if( ImGui::MenuItem( "Create entity" ) )
				g_pGameWorld->m_oScene.CreateEntity( "NewEntity" );

			ImGui::EndPopup();
		}

		Array< uint64 > aIDs;
		aIDs.Reserve( ( uint )g_pGameWorld->m_oScene.m_mEntities.size() );
		for( auto& it : g_pGameWorld->m_oScene.m_mEntities )
		{
			const uint64 uID = it.first;
			if( uID >= ENTITIES_START_ID && it.second->GetParent() == nullptr )
				aIDs.PushBack( uID );
		}

		Sort( aIDs, []( const uint64 uA, const uint64 uB ) { return uA < uB; } );

		int iImGuiID = 0;
		for( const uint64 uID : aIDs )
		{
			auto it = g_pGameWorld->m_oScene.m_mEntities.find( uID );
			if( it != g_pGameWorld->m_oScene.m_mEntities.end() && it->second != nullptr )
				bModified |= DisplayHierarchy( it->second.GetPtr(), iImGuiID++ );
		}

		ImGui::TreePop();
	}

	ImGui::End();

	ImGui::Begin( "Inspector" );

	pSelectedEntity = g_pGameWorld->m_oScene.FindEntity( m_uSelectedEntityID );
	if( pSelectedEntity != nullptr )
	{
		std::string sID = std::format( "{}", pSelectedEntity->GetID() );
		ImGui::InputText( "ID", &sID, ImGuiInputTextFlags_ReadOnly );

		std::string sName = pSelectedEntity->GetName();
		if( ImGui::InputText( "Name", &sName, ImGuiInputTextFlags_EnterReturnsTrue ) )
		{
			pSelectedEntity->SetName( sName );
			bModified |= ImGui::IsItemDeactivatedAfterEdit();
		}

		glm::vec3 vPosition = pSelectedEntity->GetPosition();
		bModified |= EditableVector3( "Position", vPosition );
		pSelectedEntity->SetPosition( vPosition );

		EulerComponent* pEuler = g_pComponentManager->GetComponent< EulerComponent >( pSelectedEntity );
		glm::vec3 vEuler = pEuler->GetRotationEuler();
		vEuler = glm::vec3( glm::degrees( vEuler.x ), glm::degrees( vEuler.y ), glm::degrees( vEuler.z ) );
		bModified |= EditableVector3( "Rotation", vEuler );
		vEuler = glm::vec3( glm::radians( vEuler.x ), glm::radians( vEuler.y ), glm::radians( vEuler.z ) );
		pEuler->SetRotationEuler( vEuler );

		glm::vec3 vScale = pSelectedEntity->GetScale();
		bModified |= EditableVector3( "Scale", vScale );
		pSelectedEntity->SetScale( vScale );

		bModified |= g_pComponentManager->DisplayInspector( pSelectedEntity );

		if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_DUPLICATE ) )
		{
			DuplicateEntity( pSelectedEntity );
			bModified = true;
		}

		if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_DELETE ) )
		{
			g_pGameWorld->m_oScene.RemoveEntity( pSelectedEntity );
			bModified = true;
		}
	}

	if( bModified )
		StoreSnapshot();
	else if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_REDO ) )
		RestoreSnapshotForward();
	else if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_UNDO ) )
		RestoreSnapshotBackward();

	ImGui::End();
}

void Editor::Render( const RenderContext& oRenderContext )
{
	ProfilerBlock oBlock( "Editor" );

	if( m_bDisplayEditor )
	{
		if( m_uSelectedEntityID != UINT64_MAX )
		{
			const Array< VisualNode* > aVisualNodes = g_pRenderer->m_oVisualStructure.FindNodes( m_uSelectedEntityID );
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

Entity* Editor::DuplicateEntity( const Entity* pEntity, const std::string& sNameSuffix /*= "_Duplicate"*/, Entity* pForcedParent /*= nullptr*/ )
{
	Scene& oScene = g_pGameWorld->m_oScene;

	Entity* pDuplicatedEntity = oScene.CreateEntity( pEntity->GetName() + sNameSuffix, oScene.GenerateID() );

	if( pForcedParent != nullptr )
		oScene.AttachToParent( pDuplicatedEntity, pForcedParent );
	else if( pEntity->GetParent() != nullptr )
		oScene.AttachToParent( pDuplicatedEntity, pEntity->GetParent() );

	nlohmann::json oJsonContent( *pEntity );

	pDuplicatedEntity->SetPosition( oJsonContent[ "position" ] );
	pDuplicatedEntity->SetRotation( oJsonContent[ "rotation" ] );
	pDuplicatedEntity->SetScale( oJsonContent[ "scale" ] );

	for( const auto& oComponentIt : oJsonContent[ "components" ].items() )
	{
		const nlohmann::json& oComponent = oComponentIt.value();

		const std::string& sComponentName = oComponent[ "name" ];
		ComponentManager::GetComponentsFactory()[ sComponentName ].m_pCreate( pDuplicatedEntity, ComponentManagement::NONE );

		if( oComponent.contains( "properties" ) )
			g_pComponentManager->DeserializeComponent( sComponentName, oComponent[ "properties" ], pDuplicatedEntity );

		g_pComponentManager->InitializeComponents( pDuplicatedEntity );
		g_pComponentManager->StartComponents( pDuplicatedEntity );
	}

	for( const Entity* pChild : pEntity->GetChildren() )
		DuplicateEntity( pChild, "", pDuplicatedEntity );

	return pDuplicatedEntity;
}

bool Editor::DisplayHierarchy( Entity* pEntity, int iImGuiID )
{
	ImGui::PushID( iImGuiID++ );

	StrongPtr< Entity > xEntity = pEntity;

	ImGuiTreeNodeFlags oFlags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
	if( pEntity->GetID() == m_uSelectedEntityID )
		oFlags |= ImGuiTreeNodeFlags_Selected;
	if( pEntity->GetChildren().Empty() )
		oFlags |= ImGuiTreeNodeFlags_Leaf;

	bool bOpened = ImGui::TreeNodeEx( pEntity->GetName().c_str(), oFlags );
	if( ImGui::IsItemClicked() && ImGui::IsItemToggledOpen() == false )
	{
		m_uSelectedEntityID = pEntity->GetID();

		Array< GizmoComponent* > aGizmoComponents = g_pComponentManager->GetComponents< GizmoComponent >();

		for( GizmoComponent* pGizmoComponent : aGizmoComponents )
			pGizmoComponent->SetAnchor( pEntity );
	}

	bool bModified = false;
	if( ImGui::BeginPopupContextItem( "ContextMenu" ) )
	{
		if( ImGui::MenuItem( "Create entity" ) )
		{
			Entity* pChild = g_pGameWorld->m_oScene.CreateEntity( "NewEntity" );
			g_pGameWorld->m_oScene.AttachToParent( pChild, pEntity );
			bModified = true;
		}

		if( ImGui::MenuItem( "Duplicate entity" ) )
		{
			DuplicateEntity( pEntity );
			bModified = true;
		}

		if( ImGui::MenuItem( "Remove entity" ) )
		{
			g_pGameWorld->m_oScene.RemoveEntity( pEntity );
			bModified = true;
		}

		const std::unordered_map< std::string, ComponentManager::ComponentFactory >& mComponentsFactory = g_pComponentManager->GetComponentsFactory();
		if( ImGui::BeginMenu( "Add component" ) )
		{
			for( const auto& it : mComponentsFactory )
			{
				if( ImGui::MenuItem( it.first.c_str() ) )
				{
					it.second.m_pCreate( pEntity, ComponentManagement::INITIALIZE_THEN_START );
					bModified = true;
				}
			}

			ImGui::EndMenu();
		}
		if( ImGui::BeginMenu( "Remove component" ) )
		{
			for( const auto& it : mComponentsFactory )
			{
				if( ImGui::MenuItem( it.first.c_str() ) )
				{
					it.second.m_pDispose( pEntity );
					bModified = true;
				}
			}

			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}

	if( bOpened )
	{
		Array< uint64 > aIDs;
		aIDs.Reserve( pEntity->GetChildren().Count() );
		for( Entity* pChild : pEntity->GetChildren() )
			aIDs.PushBack( pChild->GetID() );

		Sort( aIDs, []( const uint64 uA, const uint64 uB ) { return uA < uB; } );

		for( const uint64 uID : aIDs )
		{
			auto it = g_pGameWorld->m_oScene.m_mEntities.find( uID );
			if( it != g_pGameWorld->m_oScene.m_mEntities.end() && it->second != nullptr )
				bModified |= DisplayHierarchy( it->second.GetPtr(), iImGuiID++ );
		}

		ImGui::TreePop();
	}

	ImGui::PopID();

	return bModified;
}

void Editor::StoreSnapshot()
{
	m_oSnapshotStore.Push();
	g_pGameWorld->m_oScene.Save( m_oSnapshotStore.Back() );
}

void Editor::RestoreSnapshotBackward()
{
	if( m_oSnapshotStore.BackwardCount() > 0 )
	{
		m_oSnapshotStore.Pop();

		m_oSceneJson = m_oSnapshotStore.Back();
		g_pGameWorld->m_oSceneJson = m_oSceneJson;
		g_pGameWorld->Reset();

		m_bStoreSnapshot = false;
	}
}

void Editor::RestoreSnapshotForward()
{
	if( m_oSnapshotStore.ForwardCount() > 0 )
	{
		m_oSnapshotStore.Push( false );

		m_oSceneJson = m_oSnapshotStore.Back();
		g_pGameWorld->m_oSceneJson = m_oSceneJson;
		g_pGameWorld->Reset();

		m_bStoreSnapshot = false;
	}
}
