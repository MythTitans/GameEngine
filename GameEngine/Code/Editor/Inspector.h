#pragma once

#ifdef EDITOR

#include <glm/glm.hpp>
#include <format>
#include <string>

#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

#include "Core/Array.h"
#include "Game/EntityHolder.h"

class Spline;
class TextureResource;

bool ColorEdit( const char* sLabel, glm::vec3& vColor );
void TexturePreview( const char* sLabel, const TextureResource* pTexture );

template < typename Type >
inline bool DisplayInspector( const char* sName, Type& oValue ) = delete;

template <>
inline bool DisplayInspector( const char* sName, bool& bValue )
{
	ImGui::Checkbox( sName, &bValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
inline bool DisplayInspector( const char* sName, int& iValue )
{
	ImGui::DragInt( sName, &iValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
inline bool DisplayInspector( const char* sName, uint& uValue )
{
	int iTempValue = ( int )uValue;
	ImGui::DragInt( sName, &iTempValue );
	uValue = glm::max( iTempValue, 0 );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
inline bool DisplayInspector( const char* sName, float& fValue )
{
	ImGui::DragFloat( sName, &fValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template <>
inline bool DisplayInspector( const char* sName, std::string& sValue )
{
	ImGui::InputText( sName, &sValue );

	return ImGui::IsItemDeactivatedAfterEdit();
}

template<>
inline bool DisplayInspector( const char* sName, glm::bvec3& vVector )
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
inline bool DisplayInspector( const char* sName, glm::vec3& vVector )
{
	return ColorEdit( sName, vVector );
}

template <>
inline bool DisplayInspector( const char* sName, EntityHolder& oEntityHolder )
{
	return oEntityHolder.DisplayInspector( sName );
}

template <>
inline bool DisplayInspector( const char* sName, Array< float >& aVectors )
{
	bool bModified = false;
	for( uint u = 0; u < aVectors.Count(); ++u )
		bModified |= DisplayInspector( std::format( "{}[{}]", sName, u ).c_str(), aVectors[ u ] );

	return bModified;
}

template <>
inline bool DisplayInspector( const char* sName, Array< glm::vec3 >& aVectors )
{
	bool bModified = false;
	for( uint u = 0; u < aVectors.Count(); ++u )
		bModified |= DisplayInspector( std::format( "{}[{}]", sName, u ).c_str(), aVectors[ u ] );

	return bModified;
}

template <>
inline bool DisplayInspector( const char* sName, Spline& oSpline )
{
	return false;
}

#endif
