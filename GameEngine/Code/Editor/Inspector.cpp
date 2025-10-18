#include "Inspector.h"

#ifdef EDITOR

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>

#include "Game/Entity.h"
#include "Game/GameWorld.h"
#include "Game/ResourceLoader.h"
#include "Graphics/Texture.h"

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

#endif
