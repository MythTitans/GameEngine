#include "EditorHelpers.h"

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>

#include "Game/ResourceLoader.h"
#include "Graphics/Texture.h"
#include "ImGui/imgui.h"

void ColorEdit( const char* sLabel, glm::vec3& vColor )
{
	glm::vec3 vSRGBColor = glm::convertLinearToSRGB( vColor );
	if( ImGui::ColorEdit3( sLabel, &vSRGBColor.x ) )
		vColor = glm::convertSRGBToLinear( vSRGBColor );
}

void TexturePreview( const char* sLabel, const TextureResource* pTexture )
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