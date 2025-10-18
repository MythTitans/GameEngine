#include "Inspector.h"

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>

#include "Game/Entity.h"
#include "Game/GameWorld.h"
#include "Game/ResourceLoader.h"
#include "Graphics/Texture.h"

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

bool ColorEdit( const char* sLabel, glm::vec3& vColor )
{
	bool bEdit = false;
	glm::vec3 vSRGBColor = glm::convertLinearToSRGB( vColor );
	bEdit = ImGui::ColorEdit3( sLabel, &vSRGBColor.x );
	if( bEdit )
		vColor = glm::convertSRGBToLinear( vSRGBColor );

	return ImGui::IsItemDeactivatedAfterEdit();
}

void TexturePreview( const char* sLabel, const TextureResource* pTexture )
{
	if( pTexture == nullptr )
		return;

	const Texture& oTexture = pTexture->GetTexture();

	ImGui::Text( "%s :", sLabel );

	float fWidth = glm::min( ( float )oTexture.GetWidth(), 128.f );
	float fHeight = glm::min( ( float )oTexture.GetHeight(), 128.f );

	const ImVec2 vFrom = ImGui::GetCursorScreenPos();
	ImGui::Image( oTexture.GetID(), ImVec2( fWidth, fHeight ) );
	const ImVec2 vTo = ImVec2( vFrom.x + fWidth, vFrom.y + fHeight );

	if( ImGui::IsMouseHoveringRect( vFrom, vTo ) )
	{
		ImGui::BeginTooltip();
		ImGui::Text( "%d x %d", oTexture.GetWidth(), oTexture.GetHeight() );
		fWidth = glm::min( ( float )oTexture.GetWidth(), 512.f );
		fHeight = glm::min( ( float )oTexture.GetHeight(), 512.f );
		ImGui::Image( oTexture.GetID(), ImVec2( fWidth, fHeight ) );
		ImGui::EndTooltip();
	}
}
