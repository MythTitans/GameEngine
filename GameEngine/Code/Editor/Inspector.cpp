#include "Inspector.h"

#ifdef EDITOR

#include <glm/glm.hpp>
#include <glm/gtc/color_space.hpp>
#include <format>

#include "Core/Array.h"
#include "Game/EntityHolder.h"
#include "Game/ResourceTypes.h"
#include "Graphics/Color.h"
#include "Graphics/Texture.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

bool Vector3Edit( const char* sLabel, glm::vec3& vVector )
{
	bool bModified = false;

	ImGui::BeginGroup();

	const float fWidth = ( ImGui::GetContentRegionAvail().x - 100.f ) / 3.f;
	const float fSpacing = 2.f;

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.6f, 0.f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.7f, 0.f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.8f, 0.f, 0.f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}X", sLabel ).c_str(), &vVector.x, 0.1f );
	bModified |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0, fSpacing );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.f, 0.6f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.f, 0.7f, 0.f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.f, 0.8f, 0.f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}Y", sLabel ).c_str(), &vVector.y, 0.1f );
	bModified |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0, fSpacing );

	ImGui::PushStyleColor( ImGuiCol_FrameBg, ImVec4( 0.f, 0.f, 0.6f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgHovered, ImVec4( 0.f, 0.f, 0.7f, 1.f ) );
	ImGui::PushStyleColor( ImGuiCol_FrameBgActive, ImVec4( 0.f, 0.f, 0.8f, 1.f ) );
	ImGui::PushItemWidth( fWidth );
	ImGui::DragFloat( std::format( "##{}Z", sLabel ).c_str(), &vVector.z, 0.1f );
	bModified |= ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopItemWidth();
	ImGui::PopStyleColor( 3 );

	ImGui::SameLine( 0 );

	ImGui::Text( sLabel );

	ImGui::EndGroup();

	return bModified;
}

bool ColorEdit( const char* sLabel, Color& oColor )
{
	bool bEdit = false;

	glm::vec3 vSRGBColor = glm::convertLinearToSRGB( oColor.m_vColor );
	bEdit = ImGui::ColorEdit3( sLabel, &vSRGBColor.x );
	if( bEdit )
		oColor.m_vColor = glm::convertSRGBToLinear( vSRGBColor );

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
	return Vector3Edit( sName, vVector );
}

template <>
bool DisplayInspector( const char* sName, Color& oColor )
{
	return ColorEdit( sName, oColor );
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
