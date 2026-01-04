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
