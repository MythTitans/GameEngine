#pragma once

#include <glm/glm.hpp>
#include <string>

#include "Core/Types.h"
#include "ImGui/imgui.h"
#include "ImGui/imgui_stdlib.h"

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

template <>
inline bool DisplayInspector( const char* sName, glm::vec3& vVector )
{
	return ColorEdit( sName, vVector );
}
