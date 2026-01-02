#pragma once

#include <glm/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

#include "Core/Array.h"

class Entity;
class EntityHolder;
class Spline;

namespace glm
{
	void to_json( nlohmann::json& oJsonContent, const glm::bvec3& vVector );
	void from_json( const nlohmann::json& oJsonContent, glm::bvec3& vVector );

	void to_json( nlohmann::json& oJsonContent, const glm::vec3& vVector );
	void from_json( const nlohmann::json& oJsonContent, glm::vec3& vVector );

	void to_json( nlohmann::json& oJsonContent, const glm::quat& vVector );
	void from_json( const nlohmann::json& oJsonContent, glm::quat& qQuaternion );
}

void to_json( nlohmann::json& oJsonContent, const Entity& oEntity );

void to_json( nlohmann::json& oJsonContent, const EntityHolder& oEntityHolder );
void from_json( const nlohmann::json& oJsonContent, EntityHolder& oEntityHolder );

void to_json( nlohmann::json& oJsonContent, const Array< float >& aVectors );
void from_json( const nlohmann::json& oJsonContent, Array< float >& aVectors );

void to_json( nlohmann::json& oJsonContent, const Array< glm::vec3 >& aVectors );
void from_json( const nlohmann::json& oJsonContent, Array< glm::vec3 >& aVectors );

void to_json( nlohmann::json& oJsonContent, const Spline& oSpline );
void from_json( const nlohmann::json& oJsonContent, Spline& oSpline );

void SerializeComponent( nlohmann::json& oJsonContent, const std::string& sComponentName, const Array< nlohmann::json >& aSerializedProperties );

template < typename T >
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< T >& aProperties ) = delete;

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< bool >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< bool >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< int >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< uint >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< float >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< std::string >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< glm::bvec3 >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< glm::vec3 >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< EntityHolder >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< Array< glm::vec3 > >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< Array< float > >& aProperties );

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< Spline >& aProperties );

template < typename T >
void DeserializeProperties( Array< T >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent ) = delete;

template <>
void DeserializeProperties( Array< bool >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< int >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< uint >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< float >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< std::string >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< glm::bvec3 >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< glm::vec3 >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< EntityHolder >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< Array< float > >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< Array< glm::vec3 > >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );

template <>
void DeserializeProperties( Array< Spline >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent );
