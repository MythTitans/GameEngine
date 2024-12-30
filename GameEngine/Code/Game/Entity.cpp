#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Game/ComponentManager.h"

Transform::Transform()
	: m_mMatrix( 1.f )
	, m_vPosition( 0.f )
	, m_vScale( 1.f )
{
}

Transform::Transform( const glm::mat4& mMatrix )
{
	m_mMatrix[ 0 ] = glm::normalize( glm::vec3( mMatrix[ 0 ] ) );
	m_mMatrix[ 1 ] = glm::normalize( glm::vec3( mMatrix[ 1 ] ) );
	m_mMatrix[ 2 ] = glm::normalize( glm::vec3( mMatrix[ 2 ] ) );
	m_vPosition = glm::vec3( mMatrix[ 3 ] );
	m_vScale = glm::vec3( glm::length( mMatrix[ 0 ] ), glm::length( mMatrix[ 1 ] ), glm::length( mMatrix[ 2 ] ) );
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
	return m_vScale;
}

void Transform::SetScale( const glm::vec3& vScale )
{
	m_vScale = vScale;
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
	const glm::mat4 mMatrix = glm::mat4_cast( qRotation );
	glm::extractEulerAngleYXZ( mMatrix, m_vRotationEuler.y, m_vRotationEuler.x, m_vRotationEuler.z );

	m_mMatrix[ 0 ] = glm::normalize( glm::vec3( mMatrix[ 0 ] ) );
	m_mMatrix[ 1 ] = glm::normalize( glm::vec3( mMatrix[ 1 ] ) );
	m_mMatrix[ 2 ] = glm::normalize( glm::vec3( mMatrix[ 2 ] ) );
}

void Transform::SetRotation( const glm::vec3& vAxis, const float fAngle )
{
	SetRotation( glm::rotate( glm::quat( 1.f, 0.f, 0.f, 0.f ), fAngle, vAxis ) );
}

void Transform::SetRotationEuler( const glm::vec3& vEuler )
{
	m_vRotationEuler = vEuler;

	glm::mat4 mMat = glm::eulerAngleYXZ( vEuler.y, vEuler.x, vEuler.z );
	m_mMatrix[ 0 ] = glm::normalize( glm::vec3( mMat[ 0 ] ) );
	m_mMatrix[ 1 ] = glm::normalize( glm::vec3( mMat[ 1 ] ) );
	m_mMatrix[ 2 ] = glm::normalize( glm::vec3( mMat[ 2 ] ) );
}

void Transform::SetRotationEuler( const float fX, const float fY, const float fZ )
{
	SetRotationEuler( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 Transform::GetRotationEuler() const
{
	return m_vRotationEuler;
}

glm::mat4 Transform::GetMatrix() const
{
	glm::mat4 mResult;
	mResult[ 0 ] = glm::vec4( m_mMatrix[ 0 ] * m_vScale.x, 0.f );
	mResult[ 1 ] = glm::vec4( m_mMatrix[ 1 ] * m_vScale.y, 0.f );
	mResult[ 2 ] = glm::vec4( m_mMatrix[ 2 ] * m_vScale.z, 0.f );
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
	return m_hTransformComponent->m_oTransform.GetPosition();
}

void Entity::SetPosition( const glm::vec3& vPosition )
{
	m_hTransformComponent->m_oTransform.SetPosition( vPosition );
}

void Entity::SetPosition( const float fX, const float fY, const float fZ )
{
	SetPosition( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 Entity::GetScale() const
{
	return m_hTransformComponent->m_oTransform.GetScale();
}

void Entity::SetScale( const glm::vec3& vScale )
{
	m_hTransformComponent->m_oTransform.SetScale( vScale );
}

void Entity::SetScale( const float fX, const float fY, const float fZ )
{
	SetScale( glm::vec3( fX, fY, fZ ) );
}

glm::quat Entity::GetRotation() const
{
	return m_hTransformComponent->m_oTransform.GetRotation();
}

void Entity::SetRotation( const glm::quat& qRotation )
{
	m_hTransformComponent->m_oTransform.SetRotation( qRotation );
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
