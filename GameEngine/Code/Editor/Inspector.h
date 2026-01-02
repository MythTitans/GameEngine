#pragma once

#ifdef EDITOR

#include <glm/fwd.hpp>
#include <string>

#include "Core/Types.h"

template < typename T >
class Array;
class EntityHolder;
class Spline;
class TextureResource;

bool ColorEdit( const char* sLabel, glm::vec3& vColor );
void TexturePreview( const char* sLabel, const TextureResource* pTexture );

template < typename Type >
bool DisplayInspector( const char* sName, Type& oValue ) = delete;

template <>
bool DisplayInspector( const char* sName, bool& bValue );

template <>
bool DisplayInspector( const char* sName, int& iValue );

template <>
bool DisplayInspector( const char* sName, uint& uValue );

template <>
bool DisplayInspector( const char* sName, float& fValue );

template <>
bool DisplayInspector( const char* sName, std::string& sValue );

template<>
bool DisplayInspector( const char* sName, glm::bvec3& vVector );

template <>
bool DisplayInspector( const char* sName, glm::vec3& vVector );

template <>
bool DisplayInspector( const char* sName, EntityHolder& oEntityHolder );

template <>
bool DisplayInspector( const char* sName, Array< float >& aVectors );

template <>
bool DisplayInspector( const char* sName, Array< glm::vec3 >& aVectors );

template <>
bool DisplayInspector( const char* sName, Spline& oSpline );

#endif
