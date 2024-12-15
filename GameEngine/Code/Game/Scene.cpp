#include "Scene.h"

#include <glm/gtc/quaternion.hpp>

#include "Component.h"
#include "GameEngine.h"

Mat3x4::Mat3x4()
	: m_mMatrix( 1.f )
	, m_vPosition( 0.f )
{
}

Mat3x4::Mat3x4( const glm::mat4& mMatrix )
{
	m_mMatrix[ 0 ] = glm::vec3( mMatrix[ 0 ] );
	m_mMatrix[ 1 ] = glm::vec3( mMatrix[ 1 ] );
	m_mMatrix[ 2 ] = glm::vec3( mMatrix[ 2 ] );
	m_vPosition = glm::vec3( mMatrix[ 3 ] );
}

glm::vec3& Mat3x4::GetI()
{
	return m_mMatrix[ 0 ];
}

const glm::vec3& Mat3x4::GetI() const
{
	return m_mMatrix[ 0 ];
}

glm::vec3& Mat3x4::GetJ()
{
	return m_mMatrix[ 1 ];
}

const glm::vec3& Mat3x4::GetJ() const
{
	return m_mMatrix[ 1 ];
}

glm::vec3& Mat3x4::GetK()
{
	return m_mMatrix[ 2 ];
}

const glm::vec3& Mat3x4::GetK() const
{
	return m_mMatrix[ 2 ];
}

glm::vec3& Mat3x4::GetO()
{
	return m_vPosition;
}

const glm::vec3& Mat3x4::GetO() const
{
	return m_vPosition;
}

glm::mat4 Mat3x4::GetMatrix() const
{
	glm::mat4 mResult;
	mResult[ 0 ] = glm::vec4( m_mMatrix[ 0 ], 0.f );
	mResult[ 1 ] = glm::vec4( m_mMatrix[ 1 ], 0.f );
	mResult[ 2 ] = glm::vec4( m_mMatrix[ 2 ], 0.f );
	mResult[ 3 ] = glm::vec4( m_vPosition, 1.f );

	return mResult;
}

Entity::Entity( const uint64 uID, const char* sName )
	: m_uID( uID )
	, m_sName( sName )
{
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
	m_mMatrix = Mat3x4( mMatrix );
}

void Entity::SetMatrix( const Mat3x4& mMatrix )
{
	m_mMatrix = mMatrix;
}

Mat3x4& Entity::GetMatrix()
{
	return m_mMatrix;
}

const Mat3x4& Entity::GetMatrix() const
{
	return m_mMatrix;
}

glm::vec3 Entity::GetPosition() const
{
	return m_mMatrix.m_vPosition;
}

void Entity::SetPosition( const glm::vec3& vPosition )
{
	m_mMatrix.m_vPosition = vPosition;
}

void Entity::SetPosition( const float fX, const float fY, const float fZ )
{
	SetPosition( glm::vec3( fX, fY, fZ ) );
}

glm::vec3 Entity::GetScale() const
{
	return glm::vec3( glm::length( m_mMatrix.m_mMatrix[ 0 ] ), glm::length( m_mMatrix.m_mMatrix[ 1 ] ), glm::length( m_mMatrix.m_mMatrix[ 2 ] ) );
}

void Entity::SetScale( const glm::vec3& vScale )
{
	m_mMatrix.m_mMatrix[ 0 ] = glm::normalize( m_mMatrix.m_mMatrix[ 0 ] ) * vScale.x;
	m_mMatrix.m_mMatrix[ 1 ] = glm::normalize( m_mMatrix.m_mMatrix[ 1 ] ) * vScale.y;
	m_mMatrix.m_mMatrix[ 2 ] = glm::normalize( m_mMatrix.m_mMatrix[ 2 ] ) * vScale.z;
}

void Entity::SetScale( const float fX, const float fY, const float fZ )
{
	SetScale( glm::vec3( fX, fY, fZ ) );
}

glm::quat Entity::GetRotation() const
{
	return glm::quat_cast( m_mMatrix.m_mMatrix );
}

void Entity::SetRotation( const glm::quat& qRotation )
{
	const glm::vec3 vScale = GetScale();
	m_mMatrix.m_mMatrix = glm::mat3_cast( qRotation );
	m_mMatrix.m_mMatrix[ 0 ] = glm::normalize( m_mMatrix.m_mMatrix[ 0 ] ) * vScale.x;
	m_mMatrix.m_mMatrix[ 1 ] = glm::normalize( m_mMatrix.m_mMatrix[ 1 ] ) * vScale.y;
	m_mMatrix.m_mMatrix[ 2 ] = glm::normalize( m_mMatrix.m_mMatrix[ 2 ] ) * vScale.z;
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

Scene::Scene()
	: m_xCube( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/cube.obj" ) ) )
	, m_xSphere( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/sphere.obj" ) ) )
	, m_xGolem( g_pResourceLoader->LoadModel( std::filesystem::path( "Data/Earth_Golem_OBJ.obj" ) ) )
{
	m_aEntities.PushBack( Entity( 0, "Entity 1" ) );
	m_aEntities.Back().SetPosition( -5.f, 0.f, -5.f );
	m_aEntities.Back().SetRotationY( glm::radians( 45.f ) );
	g_pComponentManager->CreateComponent< MyFirstComponent >( m_aEntities.Back() );
	g_pComponentManager->CreateComponent<VisualComponent >( m_aEntities.Back() ).Setup( m_xCube );
	m_aEntities.PushBack( Entity( 1, "Entity 2" ) );
	m_aEntities.Back().SetPosition( 5.f, 0.f, -5.f );
	m_aEntities.Back().SetRotationY( glm::radians( 90.f ) );
	g_pComponentManager->CreateComponent<VisualComponent >( m_aEntities.Back() ).Setup( m_xSphere );
	m_aEntities.PushBack( Entity( 2, "Entity 3" ) );
	m_aEntities.Back().SetPosition( -5.f, 0.f, 5.f );
	m_aEntities.Back().SetRotationY( glm::radians( 135.f ) );
	g_pComponentManager->CreateComponent<VisualComponent >( m_aEntities.Back() ).Setup( m_xGolem );
	m_aEntities.PushBack( Entity( 3, "Entity 4" ) );
	m_aEntities.Back().SetPosition( 5.f, 0.f, 5.f );
	m_aEntities.Back().SetRotationY( glm::radians( 180.f ) );
	g_pComponentManager->CreateComponent<VisualComponent >( m_aEntities.Back() ).Setup( m_xCube );
	m_aEntities.PushBack( Entity( 4, "Entity 5" ) );
	m_aEntities.Back().SetPosition( 0.f, 0.f, 0.f );
	m_aEntities.Back().SetScale( 1.5f, 1.5f, 1.5f );
	g_pComponentManager->CreateComponent< MyFirstComponent >( m_aEntities.Back() );
	g_pComponentManager->CreateComponent<VisualComponent >( m_aEntities.Back() ).Setup( m_xGolem );
}

bool Scene::OnLoading()
{
	return m_xCube->IsLoaded() && m_xSphere->IsLoaded() && m_xGolem->IsLoaded();
}
