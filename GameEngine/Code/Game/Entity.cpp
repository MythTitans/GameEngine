#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Game/ComponentManager.h"

static bool IsUniformScale( const glm::vec3& vScale )
{
	const float fEpsilon = 0.001f;
	return glm::epsilonEqual( vScale.x, vScale.y, fEpsilon ) && glm::epsilonEqual( vScale.x, vScale.z, fEpsilon );
}

Transform::Transform()
	: m_mMatrix( 1.f )
	, m_vPosition( 0.f )
	, m_vScale( 1.f )
	, m_bDirtyRotation( true )
	, m_bUniformScale( true )
{
}

Transform::Transform( const glm::mat4& mMatrix )	
	: m_bDirtyRotation( true )
{
	m_mMatrix[ 0 ] = glm::normalize( glm::vec3( mMatrix[ 0 ] ) );
	m_mMatrix[ 1 ] = glm::normalize( glm::vec3( mMatrix[ 1 ] ) );
	m_mMatrix[ 2 ] = glm::normalize( glm::vec3( mMatrix[ 2 ] ) );
	m_vPosition = glm::vec3( mMatrix[ 3 ] );
	m_vScale = glm::vec3( glm::length( mMatrix[ 0 ] ), glm::length( mMatrix[ 1 ] ), glm::length( mMatrix[ 2 ] ) );

	m_bUniformScale = IsUniformScale( m_vScale );
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
	m_bUniformScale = IsUniformScale( m_vScale );
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

	m_mMatrix[ 0 ] = glm::vec3( mMatrix[ 0 ] );
	m_mMatrix[ 1 ] = glm::vec3( mMatrix[ 1 ] );
	m_mMatrix[ 2 ] = glm::vec3( mMatrix[ 2 ] );

	m_bDirtyRotation = true;
}

void Transform::SetRotation( const glm::vec3& vAxis, const float fAngle )
{
	SetRotation( glm::rotate( glm::quat( 1.f, 0.f, 0.f, 0.f ), fAngle, vAxis ) );
}

glm::mat4 Transform::GetMatrixTR() const
{
	glm::mat4 mResult;
	mResult[ 0 ] = glm::vec4( m_mMatrix[ 0 ], 0.f );
	mResult[ 1 ] = glm::vec4( m_mMatrix[ 1 ], 0.f );
	mResult[ 2 ] = glm::vec4( m_mMatrix[ 2 ], 0.f );
	mResult[ 3 ] = glm::vec4( m_vPosition, 1.f );

	return mResult;
}

glm::mat4 Transform::GetMatrixTRS() const
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

EulerComponent::EulerComponent( Entity* pEntity )
	: Component( pEntity )
	, m_vRotationEuler( 0.f, 0.f, 0.f )
{
}

void EulerComponent::Start()
{
	m_hTransformComponent = GetComponent< TransformComponent >();
	ASSERT( m_hTransformComponent.IsValid() );
}

void EulerComponent::Update( const float fDeltaTime )
{
	Transform& oTransform = m_hTransformComponent->m_oTransform;

	if( oTransform.m_bDirtyRotation )
	{
		const glm::mat4 mMatrix( oTransform.m_mMatrix );
		glm::extractEulerAngleYXZ( mMatrix, m_vRotationEuler.y, m_vRotationEuler.x, m_vRotationEuler.z );
		oTransform.m_bDirtyRotation = false;
	}
}

void EulerComponent::SetRotationEuler( const glm::vec3& vEuler )
{
	Transform& oTransform = m_hTransformComponent->m_oTransform;

	m_vRotationEuler = vEuler;

	const glm::mat4 mMat = glm::eulerAngleYXZ( vEuler.y, vEuler.x, vEuler.z );
	oTransform.m_mMatrix[ 0 ] = glm::vec3( mMat[ 0 ] );
	oTransform.m_mMatrix[ 1 ] = glm::vec3( mMat[ 1 ] );
	oTransform.m_mMatrix[ 2 ] = glm::vec3( mMat[ 2 ] );
}

void EulerComponent::SetRotationEuler( const float fX, const float fY, const float fZ )
{
	SetRotationEuler( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 EulerComponent::GetRotationEuler() const
{
	const Transform& oTransform = m_hTransformComponent->m_oTransform;

	if( oTransform.m_bDirtyRotation )
	{
		glm::vec3 vRotationEuler;
		const glm::mat4 mMatrix( oTransform.m_mMatrix );
		glm::extractEulerAngleYXZ( mMatrix, vRotationEuler.y, vRotationEuler.x, vRotationEuler.z );
		return vRotationEuler;
	}

	return m_vRotationEuler;
}

Entity::Entity()
	: m_uID( UINT64_MAX )
	, m_sName( "" )
	, m_pParent( nullptr )
{
}

Entity::Entity( const uint64 uID, const std::string& sName )
	: m_uID( uID )
	, m_sName( sName )
	, m_pParent( nullptr )
{
	m_hTransformComponent = &g_pComponentManager->CreateComponent< TransformComponent >( this );
	g_pComponentManager->CreateComponent< EulerComponent >( this );
}

uint64 Entity::GetID() const
{
	return m_uID;
}

const std::string& Entity::GetName() const
{
	return m_sName;
}

Entity* Entity::GetParent()
{
	return m_pParent;
}

const Entity* Entity::GetParent() const
{
	return m_pParent;
}

void Entity::SetWorldTransform( const Transform& oTransform )
{
	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();

	m_hTransformComponent->m_oTransform = Transform( glm::inverse( oParentTransform.GetMatrixTR() ) * oTransform.GetMatrixTR() );
	m_hTransformComponent->m_oTransform.SetScale( oTransform.GetScale() );
}

Transform Entity::GetWorldTransform() const
{
	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();

	Transform oTransform( oParentTransform.GetMatrixTR() * m_hTransformComponent->m_oTransform.GetMatrixTR() );
	oTransform.SetScale( m_hTransformComponent->m_oTransform.GetScale() );

	return oTransform;
}

void Entity::SetWorldPosition( const glm::vec3& vPosition )
{
	Transform oTransform;
	oTransform.SetPosition( vPosition );

	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();
	m_hTransformComponent->m_oTransform.SetPosition( Transform( glm::inverse( oParentTransform.GetMatrixTR() ) * oTransform.GetMatrixTR() ).GetPosition() );
}

void Entity::SetWorldPosition( const float fX, const float fY, const float fZ )
{
	SetWorldPosition( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 Entity::GetWorldPosition() const
{
	return GetWorldTransform().GetPosition();
}

void Entity::SetRotation( const glm::quat& qRotation )
{
	Transform oTransform;
	oTransform.SetRotation( qRotation );

	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();
	m_hTransformComponent->m_oTransform.SetRotation( Transform( glm::inverse( oParentTransform.GetMatrixTR() ) * oTransform.GetMatrixTR() ).GetRotation() );
}

void Entity::SetRotation( const glm::vec3& vAxis, const float fAngle )
{
	Transform oTransform;
	oTransform.SetRotation( vAxis, fAngle );

	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();
	m_hTransformComponent->m_oTransform.SetRotation( Transform( glm::inverse( oParentTransform.GetMatrixTR() ) * oTransform.GetMatrixTR() ).GetRotation() );
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

glm::quat Entity::GetRotation() const
{
	return GetWorldTransform().GetRotation();
}

void Entity::SetTransform( const Transform& oTransform )
{
	m_hTransformComponent->m_oTransform = oTransform;
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
