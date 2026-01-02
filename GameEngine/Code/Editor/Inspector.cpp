#include "Inspector.h"

#ifdef EDITOR

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <format>

#include "Core/Array.h"
#include "Game/EntityHolder.h"
#include "Game/ResourceTypes.h"
#include "Graphics/Texture.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

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

template <>
bool DisplayInspector( const char* sName, bool& bValue )
{
	ImGui::Checkbox( sName, &bValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
bool DisplayInspector( const char* sName, int& iValue )
{
	ImGui::DragInt( sName, &iValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
bool DisplayInspector( const char* sName, uint& uValue )
{
	int iTempValue = ( int )uValue;
	ImGui::DragInt( sName, &iTempValue );
	uValue = glm::max( iTempValue, 0 );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
bool DisplayInspector( const char* sName, float& fValue )
{
	ImGui::DragFloat( sName, &fValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
bool DisplayInspector( const char* sName, std::string& sValue )
{
	ImGui::InputText( sName, &sValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template<>
bool DisplayInspector( const char* sName, glm::bvec3& vVector )
{
	bool bModified = false;

	ImGui::Text( sName );
	ImGui::Checkbox( std::format( "X##{}", sName ).c_str(), &vVector[ 0 ] );

	bModified |= ImGui::IsItemDeactivatedAfterEdit();

	ImGui::SameLine();
	ImGui::Checkbox( std::format( "Y##{}", sName ).c_str(), &vVector[ 1 ] );

	bModified |= ImGui::IsItemDeactivatedAfterEdit();

	ImGui::SameLine();
	ImGui::Checkbox( std::format( "Z##{}", sName ).c_str(), &vVector[ 2 ] );

	bModified |= ImGui::IsItemDeactivatedAfterEdit();

	return bModified;
}

template <>
bool DisplayInspector( const char* sName, glm::vec3& vVector )
{
	return ColorEdit( sName, vVector );
}

template <>
bool DisplayInspector( const char* sName, EntityHolder& oEntityHolder )
{
	return oEntityHolder.DisplayInspector( sName );
}

template <>
bool DisplayInspector( const char* sName, Array< float >& aVectors )
{
	bool bModified = false;
	for( uint u = 0; u < aVectors.Count(); ++u )
		bModified |= DisplayInspector( std::format( "{}[{}]", sName, u ).c_str(), aVectors[ u ] );

	return bModified;
}

template <>
bool DisplayInspector( const char* sName, Array< glm::vec3 >& aVectors )
{
	bool bModified = false;
	for( uint u = 0; u < aVectors.Count(); ++u )
		bModified |= DisplayInspector( std::format( "{}[{}]", sName, u ).c_str(), aVectors[ u ] );

	return bModified;
}

template <>
bool DisplayInspector( const char* sName, Spline& oSpline )
{
	return false;
}

#endif
