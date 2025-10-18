#include "Serialization.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include "Game/ComponentManager.h"
#include "Game/Entity.h"
#include "Game/EntityHolder.h"
#include "Editor/Inspector.h"

namespace glm
{
	void to_json( nlohmann::json& oJsonContent, const bvec3& vVector )
	{
		oJsonContent[ "x" ] = vVector.x;
		oJsonContent[ "y" ] = vVector.y;
		oJsonContent[ "z" ] = vVector.z;
	}

	void from_json( const nlohmann::json& oJsonContent, bvec3& vVector )
	{
		vVector.x = oJsonContent[ "x" ];
		vVector.y = oJsonContent[ "y" ];
		vVector.z = oJsonContent[ "z" ];
	}

	void to_json( nlohmann::json& oJsonContent, const glm::vec3& vVector )
	{
		oJsonContent[ "x" ] = vVector.x;
		oJsonContent[ "y" ] = vVector.y;
		oJsonContent[ "z" ] = vVector.z;
	}

	void from_json( const nlohmann::json& oJsonContent, vec3& vVector )
	{
		vVector.x = oJsonContent[ "x" ];
		vVector.y = oJsonContent[ "y" ];
		vVector.z = oJsonContent[ "z" ];
	}

	void to_json( nlohmann::json& oJsonContent, const quat& qQuaternion )
	{
		oJsonContent[ "x" ] = qQuaternion.x;
		oJsonContent[ "y" ] = qQuaternion.y;
		oJsonContent[ "z" ] = qQuaternion.z;
		oJsonContent[ "w" ] = qQuaternion.w;
	}

	void from_json( const nlohmann::json& oJsonContent, quat& qQuaternion )
	{
		qQuaternion.x = oJsonContent[ "x" ];
		qQuaternion.y = oJsonContent[ "y" ];
		qQuaternion.z = oJsonContent[ "z" ];
		qQuaternion.w = oJsonContent[ "w" ];
	}
}

void to_json( nlohmann::json& oJsonContent, const Entity& oEntity )
{
	oJsonContent[ "id" ] = oEntity.GetID();
	oJsonContent[ "name" ] = oEntity.GetName();
	oJsonContent[ "position" ] = oEntity.GetPosition();
	oJsonContent[ "rotation" ] = oEntity.GetRotation();
	oJsonContent[ "scale" ] = oEntity.GetScale();

	if( oEntity.GetParent() != nullptr )
		oJsonContent[ "parentId" ] = oEntity.GetParent()->GetID();

	const Array< nlohmann::json > aSerializedComponents = g_pComponentManager->SerializeComponents( &oEntity );
	if( aSerializedComponents.Empty() == false )
		oJsonContent[ "components" ] = aSerializedComponents;
}

void to_json( nlohmann::json& oJsonContent, const EntityHolder& oEntityHolder )
{
	oJsonContent[ "entityID" ] = oEntityHolder.GetEntityID();
}

void from_json( const nlohmann::json& oJsonContent, EntityHolder& oEntityHolder )
{
	oEntityHolder.SetEntity( oJsonContent[ "entityID" ] );
}
