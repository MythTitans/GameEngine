#include "Entity.h"

#include <glm/gtc/quaternion.hpp>

#include "Game/ComponentManager.h"

Transform::Transform()
	: m_mMatrix( 1.f )
	, m_vPosition( 0.f )
{
}

Transform::Transform( const glm::mat4& mMatrix )
{
	m_mMatrix[ 0 ] = glm::vec3( mMatrix[ 0 ] );
	m_mMatrix[ 1 ] = glm::vec3( mMatrix[ 1 ] );
	m_mMatrix[ 2 ] = glm::vec3( mMatrix[ 2 ] );
	m_vPosition = glm::vec3( mMatrix[ 3 ] );
}

glm::vec3& Transform::GetI()
{
	return m_mMatrix[ 0 ];
}

const glm::vec3& Transform::GetI() const
{
	return m_mMatrix[ 0 ];
}

glm::vec3& Transform::GetJ()
{
	return m_mMatrix[ 1 ];
}

const glm::vec3& Transform::GetJ() const
{
	return m_mMatrix[ 1 ];
}

glm::vec3& Transform::GetK()
{
	return m_mMatrix[ 2 ];
}

const glm::vec3& Transform::GetK() const
{
	return m_mMatrix[ 2 ];
}

glm::vec3& Transform::GetO()
{
	return m_vPosition;
}

const glm::vec3& Transform::GetO() const
{
	return m_vPosition;
}

glm::vec3 Transform::GetPosition() const
{
	return m_vPosition;
}

void Transform::SetPosition( const glm::vec3& vPosition )
{
	m_vPosition = vPosition;
}

void Transform::SetPosition( const float fX, const float fY, const float fZ )
{
	SetPosition( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 Transform::GetScale() const
{
	return glm::vec3( glm::length( m_mMatrix[ 0 ] ), glm::length( m_mMatrix[ 1 ] ), glm::length( m_mMatrix[ 2 ] ) );
}

void Transform::SetScale( const glm::vec3& vScale )
{
	m_mMatrix[ 0 ] = glm::normalize( m_mMatrix[ 0 ] ) * vScale.x;
	m_mMatrix[ 1 ] = glm::normalize( m_mMatrix[ 1 ] ) * vScale.y;
	m_mMatrix[ 2 ] = glm::normalize( m_mMatrix[ 2 ] ) * vScale.z;
}

void Transform::SetScale( const float fX, const float fY, const float fZ )
{
	SetScale( glm::vec3( fX, fY, fZ ) );
}

glm::quat Transform::GetRotation() const
{
	return glm::quat_cast( m_mMatrix );
}

void Transform::SetRotation( const glm::quat& qRotation )
{
	const glm::vec3 vScale = GetScale();
	m_mMatrix = glm::mat3_cast( qRotation );
	m_mMatrix[ 0 ] = glm::normalize( m_mMatrix[ 0 ] ) * vScale.x;
	m_mMatrix[ 1 ] = glm::normalize( m_mMatrix[ 1 ] ) * vScale.y;
	m_mMatrix[ 2 ] = glm::normalize( m_mMatrix[ 2 ] ) * vScale.z;
}

void Transform::SetRotation( const glm::vec3& vAxis, const float fAngle )
{
	SetRotation( glm::rotate( glm::quat( 1.f, 0.f, 0.f, 0.f ), fAngle, vAxis ) );
}

void Transform::SetRotationX( const float fAngle )
{
	SetRotation( glm::vec3( 1.f, 0.f, 0.f ), fAngle );
}

void Transform::SetRotationY( const float fAngle )
{
	SetRotation( glm::vec3( 0.f, 1.f, 0.f ), fAngle );
}

void Transform::SetRotationZ( const float fAngle )
{
	SetRotation( glm::vec3( 0.f, 0.f, 1.f ), fAngle );
}

glm::mat4 Transform::GetMatrix() const
{
	glm::mat4 mResult;
	mResult[ 0 ] = glm::vec4( m_mMatrix[ 0 ], 0.f );
	mResult[ 1 ] = glm::vec4( m_mMatrix[ 1 ], 0.f );
	mResult[ 2 ] = glm::vec4( m_mMatrix[ 2 ], 0.f );
	mResult[ 3 ] = glm::vec4( m_vPosition, 1.f );

	return mResult;
}

TransformComponent::TransformComponent( Entity* pEntity )
	: Component( pEntity )
{
}

Entity::Entity()
	: m_uID( UINT64_MAX )
	, m_sName( "" )
{
}

Entity::Entity( const uint64 uID, const char* sName )
	: m_uID( uID )
	, m_sName( sName )
{
	m_hTransformComponent = &g_pComponentManager->CreateComponent< TransformComponent >( this );
}

uint64 Entity::GetID() const
{
	return m_uID;
}

const char* Entity::GetName() const
{
	return m_sName;
}

void Entity::SetMatrix( const glm::mat4& mMatrix )
{
	m_hTransformComponent->m_oTransform = Transform( mMatrix );
}

glm::mat4 Entity::GetMatrix() const
{
	return m_hTransformComponent->m_oTransform.GetMatrix();
}

void Entity::SetTransform( const Transform& mMatrix )
{
	m_hTransformComponent->m_oTransform = mMatrix;
}

Transform& Entity::GetTransform()
{
	return m_hTransformComponent->m_oTransform;
}

const Transform& Entity::GetTransform() const
{
	return m_hTransformComponent->m_oTransform;
}

glm::vec3 Entity::GetPosition() const
{
	return m_hTransformComponent->m_oTransform.m_vPosition;
}

void Entity::SetPosition( const glm::vec3& vPosition )
{
	m_hTransformComponent->m_oTransform.m_vPosition = vPosition;
}

void Entity::SetPosition( const float fX, const float fY, const float fZ )
{
	SetPosition( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 Entity::GetScale() const
{
	return glm::vec3( glm::length( m_hTransformComponent->m_oTransform.m_mMatrix[ 0 ] ), glm::length( m_hTransformComponent->m_oTransform.m_mMatrix[ 1 ] ), glm::length( m_hTransformComponent->m_oTransform.m_mMatrix[ 2 ] ) );
}

void Entity::SetScale( const glm::vec3& vScale )
{
	m_hTransformComponent->m_oTransform.m_mMatrix[ 0 ] = glm::normalize( m_hTransformComponent->m_oTransform.m_mMatrix[ 0 ] ) * vScale.x;
	m_hTransformComponent->m_oTransform.m_mMatrix[ 1 ] = glm::normalize( m_hTransformComponent->m_oTransform.m_mMatrix[ 1 ] ) * vScale.y;
	m_hTransformComponent->m_oTransform.m_mMatrix[ 2 ] = glm::normalize( m_hTransformComponent->m_oTransform.m_mMatrix[ 2 ] ) * vScale.z;
}

void Entity::SetScale( const float fX, const float fY, const float fZ )
{
	SetScale( glm::vec3( fX, fY, fZ ) );
}

glm::quat Entity::GetRotation() const
{
	return glm::quat_cast( m_hTransformComponent->m_oTransform.m_mMatrix );
}

void Entity::SetRotation( const glm::quat& qRotation )
{
	const glm::vec3 vScale = GetScale();
	m_hTransformComponent->m_oTransform.m_mMatrix = glm::mat3_cast( qRotation );
	m_hTransformComponent->m_oTransform.m_mMatrix[ 0 ] = glm::normalize( m_hTransformComponent->m_oTransform.m_mMatrix[ 0 ] ) * vScale.x;
	m_hTransformComponent->m_oTransform.m_mMatrix[ 1 ] = glm::normalize( m_hTransformComponent->m_oTransform.m_mMatrix[ 1 ] ) * vScale.y;
	m_hTransformComponent->m_oTransform.m_mMatrix[ 2 ] = glm::normalize( m_hTransformComponent->m_oTransform.m_mMatrix[ 2 ] ) * vScale.z;
}

void Entity::SetRotation( const glm::vec3& vAxis, const float fAngle )
{
	SetRotation( glm::rotate( glm::quat( 1.f, 0.f, 0.f, 0.f ), fAngle, vAxis ) );
}

void Entity::SetRotationX( const float fAngle )
{
	SetRotation( glm::vec3( 1.f, 0.f, 0.f ), fAngle );
}

void Entity::SetRotationY( const float fAngle )
{
	SetRotation( glm::vec3( 0.f, 1.f, 0.f ), fAngle );
}

void Entity::SetRotationZ( const float fAngle )
{
	SetRotation( glm::vec3( 0.f, 0.f, 1.f ), fAngle );
}
