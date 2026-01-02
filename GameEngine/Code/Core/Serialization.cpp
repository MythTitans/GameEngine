#include "Serialization.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <nlohmann/json.hpp>

#include "Game/ComponentManager.h"
#include "Game/Entity.h"
#include "Game/EntityHolder.h"
#include "Game/Spline.h"

template < typename T >
void TypedSerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< T >& aProperties )
{
	aSerializedProperties.Reserve( aNames.Count() );

	for( uint u = 0; u < aNames.Count(); ++u )
	{
		nlohmann::json oSerializedProperty;
		oSerializedProperty[ aNames[ u ] ] = aProperties[ u ];
		aSerializedProperties.PushBack( oSerializedProperty );
	}
}

template < typename T >
static void TypedDeserializeProperties( Array< T >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	aProperties.Resize( aNames.Count() );

	for( uint u = 0; u < aNames.Count(); ++u )
	{
		for( const auto& it : oJsonContent.items() )
		{
			const nlohmann::json& oProperty = it.value();
			if( oProperty.contains( aNames[ u ] ) )
				aProperties[ u ] = oProperty[ aNames[ u ] ];
		}
	}
}

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

void SerializeComponent( nlohmann::json& oJsonContent, const std::string& sComponentName, const Array< nlohmann::json >& aSerializedProperties )
{
	if( aSerializedProperties.Empty() == false )
		oJsonContent[ "properties" ] = aSerializedProperties;

	oJsonContent[ "name" ] = sComponentName;
}

void to_json( nlohmann::json& oJsonContent, const Array< float >& aFloats )
{
	Array< nlohmann::json > aJsonVectors( aFloats.Count() );
	for( uint u = 0; u < aJsonVectors.Count(); ++u )
		aJsonVectors[ u ] = aFloats[ u ];

	oJsonContent = aJsonVectors;
}

void from_json( const nlohmann::json& oJsonContent, Array< float >& aFloats )
{
	for( const auto& oVectorIt : oJsonContent.items() )
	{
		const nlohmann::json& oVector = oVectorIt.value();
		aFloats.PushBack( oVector );
	}
}

void to_json( nlohmann::json& oJsonContent, const Array< glm::vec3 >& aVectors )
{
	Array< nlohmann::json > aJsonVectors( aVectors.Count() );
	for( uint u = 0; u < aJsonVectors.Count(); ++u )
		aJsonVectors[ u ] = aVectors[ u ];

	oJsonContent = aJsonVectors;
}

void from_json( const nlohmann::json& oJsonContent, Array< glm::vec3 >& aVectors )
{
	for( const auto& oVectorIt : oJsonContent.items() )
	{
		const nlohmann::json& oVector = oVectorIt.value();
		aVectors.PushBack( oVector );
	}
}

void to_json( nlohmann::json& oJsonContent, const Spline& oSpline )
{
	oJsonContent[ "controlPoints" ] = oSpline.GetControlPoints();
	oJsonContent[ "tangents" ] = oSpline.GetTangents();
}

void from_json( const nlohmann::json& oJsonContent, Spline& oSpline )
{
	from_json( oJsonContent[ "controlPoints" ], oSpline.GetControlPoints() );
	from_json( oJsonContent[ "tangents" ], oSpline.GetTangents() );

	oSpline.RebuildDistances();
	//oSpline.RebuildCurvatures();
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< bool >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< int >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< uint >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< float >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< std::string >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< glm::bvec3 >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< glm::vec3 >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< EntityHolder >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< Array< glm::vec3 > >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< Array< float > >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void SerializeProperties( Array< nlohmann::json >& aSerializedProperties, const Array< std::string >& aNames, const Array< Spline >& aProperties )
{
	TypedSerializeProperties( aSerializedProperties, aNames, aProperties );
}

template <>
void DeserializeProperties( Array< bool >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< int >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< uint >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< float >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< std::string >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< glm::bvec3 >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< glm::vec3 >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< EntityHolder >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< Array< float > >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< Array< glm::vec3 > >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}

template <>
void DeserializeProperties( Array< Spline >& aProperties, const Array< std::string >& aNames, const nlohmann::json& oJsonContent )
{
	TypedDeserializeProperties( aProperties, aNames, oJsonContent );
}
