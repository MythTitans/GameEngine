#include "Entity.h"

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/euler_angles.hpp>

#include "Game/ComponentManager.h"
#include "Game/GameWorld.h"
#include "Math/GLMHelpers.h"

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

Transform::Transform( const glm::mat4x3& mTRSMatrix )	
	: m_bDirtyRotation( true )
{
	m_mMatrix[ 0 ] = glm::normalize( mTRSMatrix[ 0 ] );
	m_mMatrix[ 1 ] = glm::normalize( mTRSMatrix[ 1 ] );
	m_mMatrix[ 2 ] = glm::normalize( mTRSMatrix[ 2 ] );

	m_vPosition = mTRSMatrix[ 3 ];

	// Get back original lengths without costly glm::length()
	const float fLength1 = glm::dot( m_mMatrix[ 0 ], mTRSMatrix[ 0 ] );
	const float fLength2 = glm::dot( m_mMatrix[ 1 ], mTRSMatrix[ 1 ] );
	const float fLength3 = glm::dot( m_mMatrix[ 2 ], mTRSMatrix[ 2 ] );
	m_vScale = glm::vec3( fLength1, fLength2, fLength3 );

	m_bUniformScale = IsUniformScale( m_vScale );
}

Transform::Transform( const glm::mat4x3& mTRMatrix, const glm::vec3& vScale )
	: m_bDirtyRotation( true )
{
	m_mMatrix[ 0 ] = mTRMatrix[ 0 ];
	m_mMatrix[ 1 ] = mTRMatrix[ 1 ];
	m_mMatrix[ 2 ] = mTRMatrix[ 2 ];

	m_vPosition = mTRMatrix[ 3 ];

	m_vScale = vScale;

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

glm::mat4x3 Transform::GetMatrixTR() const
{
	return glm::mat4x3( m_mMatrix[ 0 ], m_mMatrix[ 1 ], m_mMatrix[ 2 ], m_vPosition );
}

glm::mat4x3 Transform::GetMatrixTRS() const
{
	return glm::mat4x3( m_mMatrix[ 0 ] * m_vScale.x, m_mMatrix[ 1 ] * m_vScale.y, m_mMatrix[ 2 ] * m_vScale.z, m_vPosition );
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

void EulerComponent::Initialize()
{
	m_hTransformComponent = GetComponent< TransformComponent >();
	ASSERT( m_hTransformComponent.IsValid() );
}

void EulerComponent::Update( const GameContext& oGameContext )
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
	m_hTransformComponent = g_pComponentManager->CreateComponent< TransformComponent >( this, ComponentManagement::INITIALIZE_THEN_START );
	g_pComponentManager->CreateComponent< EulerComponent >( this, ComponentManagement::INITIALIZE_THEN_START );
}

Entity::~Entity()
{
	if( g_pGameWorld != nullptr )
	{
		for( int i = ( int )m_aChildren.Count() - 1; i >= 0; --i )
			g_pGameWorld->DetachFromParent( m_aChildren[ i ] );

		g_pGameWorld->DetachFromParent( this );
	}

	g_pComponentManager->StopComponents( this );
	g_pComponentManager->DisposeComponents( this );
}

uint64 Entity::GetSize() const
{
	return sizeof( Entity );
}

uint64 Entity::GetID() const
{
	return m_uID;
}

void Entity::SetName( const std::string& sName )
{
	m_sName = sName;
}

const std::string& Entity::GetName() const
{
	return m_sName;
}

Entity* Entity::GetParent() const
{
	return m_pParent;
}

const Array< Entity* >& Entity::GetChildren() const
{
	return m_aChildren;
}

void Entity::SetWorldTransform( const Transform& oTransform )
{
	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();
	m_hTransformComponent->m_oTransform = Transform( glm::inverse( oParentTransform.GetMatrixTR() ) * oTransform.GetMatrixTR(), oTransform.GetScale() );
}

Transform Entity::GetWorldTransform() const
{
	const Transform oParentTransform = m_pParent != nullptr ? m_pParent->GetWorldTransform() : Transform();
	return Transform( oParentTransform.GetMatrixTR() * m_hTransformComponent->m_oTransform.GetMatrixTR(), m_hTransformComponent->m_oTransform.GetScale() );
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
