#pragma once

#include <glm/fwd.hpp>
#include <nlohmann/json_fwd.hpp>

class Entity;

namespace glm
{
	void to_json( nlohmann::json& oJsonContent, const glm::vec3& qQuaternion );
	void from_json( const nlohmann::json& oJsonContent, glm::vec3& vVector );

	void to_json( nlohmann::json& oJsonContent, const glm::quat& qQuaternion );
	void from_json( const nlohmann::json& oJsonContent, glm::quat& qQuaternion );
}

void to_json( nlohmann::json& oJsonContent, const Entity& oEntity );
