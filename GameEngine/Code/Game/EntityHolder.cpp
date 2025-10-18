#include "EntityHolder.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

#include "GameWorld.h"
#include "Entity.h"

EntityHolder::EntityHolder()
	: m_uEntityID( UINT64_MAX )
{
}

bool EntityHolder::DisplayInspector( const char* sName )
{
	std::string sSelected = m_xEntity != nullptr ? m_xEntity->GetName() : "";

	ImGui::InputText( sName, &sSelected, ImGuiInputTextFlags_ReadOnly );

	if( ImGui::BeginDragDropTarget() )
	{
		if( const ImGuiPayload* pPayload = ImGui::AcceptDragDropPayload( "ENTITY" ) )
		{
			m_uEntityID = *( const uint64* )pPayload->Data;
			m_xEntity = g_pGameWorld->FindEntity( m_uEntityID );
		}

		ImGui::EndDragDropTarget();
	}

	bool bModified = ImGui::IsItemDeactivatedAfterEdit();

	ImGui::SameLine();

	if( ImGui::Button( "Clear" ) )
	{
		m_uEntityID = UINT64_MAX;
		m_xEntity = nullptr;

		bModified = true;
	}

	return ImGui::IsItemDeactivatedAfterEdit();
}

void EntityHolder::SetEntity( const uint uEntityID )
{
	m_uEntityID = uEntityID;
	m_xEntity = g_pGameWorld->FindEntity( m_uEntityID );
}

Entity* EntityHolder::GetEntity()
{
	return m_xEntity.GetPtr();
}

uint64 EntityHolder::GetEntityID() const
{
	return m_uEntityID;
}