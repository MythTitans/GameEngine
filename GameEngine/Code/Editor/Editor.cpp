#include "Editor.h"

#include "Game/GameEngine.h"
#include "Core/Profiler.h"
#include "Game/Entity.h"
#include "Game/InputHandler.h"
#include "Game/Scene.h"

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

Editor::Editor()
	: m_bDisplayEditor( false )
{
}

void Editor::Display()
{
	ProfilerBlock oBlock( "Editor" );

	if( g_pInputHandler->IsInputActionTriggered( InputActionID::ACTION_TOGGLE_EDITOR ) )
		m_bDisplayEditor = !m_bDisplayEditor;

	if( m_bDisplayEditor )
	{
		ImGui::Begin( "Editor" );

		if( ImGui::TreeNode( "Root" ) )
		{
			for( auto& it : g_pGameEngine->GetScene().m_mEntities )
			{
				if( ImGui::TreeNode( it.second->GetName() ) )
				{
					Transform& oTransform = it.second->GetTransform();

					oTransform.SetPosition( EditableVector3( "Position", oTransform.GetPosition() ) );
					//oTransform.SetPosition( EditableVector3( "Rotation", oTransform.GetRotationXYZ() ) );
					oTransform.SetScale( EditableVector3( "Scale", oTransform.GetScale() ) );

					PointLightComponent* pPointLightComponent = g_pComponentManager->GetComponent< PointLightComponent >( it.second.GetPtr() );
					if( pPointLightComponent != nullptr && ImGui::CollapsingHeader( "Point light" ) )
					{
						ImGui::DragFloat( "Intensity", &pPointLightComponent->m_fIntensity, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
						ImGui::ColorEdit3( "Color", &pPointLightComponent->m_vColor.x );
						ImGui::DragFloat( "Falloff factor", &pPointLightComponent->m_fFalloffFactor, 1.f, 0.f, 100.f, "%.3f", ImGuiSliderFlags_AlwaysClamp );
					}

					ImGui::TreePop();
				}
			}

			ImGui::TreePop();
		}

		ImGui::End();
	}
}
