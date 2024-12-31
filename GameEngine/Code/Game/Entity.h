#pragma once

#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

#include "Core/Intrusive.h"
#include "Core/Types.h"
#include "Game/Component.h"

struct Transform
{
	Transform();
	explicit Transform( const glm::mat4& mMatrix );

	glm::vec3&			GetI();
	const glm::vec3&	GetI() const;
	glm::vec3&			GetJ();
	const glm::vec3&	GetJ() const;
	glm::vec3&			GetK();
	const glm::vec3&	GetK() const;
	glm::vec3&			GetO();
	const glm::vec3&	GetO() const;

	glm::vec3			GetPosition() const;
	void				SetPosition( const glm::vec3& vPosition );
	void				SetPosition( const float fX, const float fY, const float fZ );

	glm::vec3			GetScale() const;
	void				SetScale( const glm::vec3& vScale );
	void				SetScale( const float fX, const float fY, const float fZ );

	glm::quat			GetRotation() const;
	void				SetRotation( const glm::quat& qRotation );
	void				SetRotation( const glm::vec3& vAxis, const float fAngle );
	void				SetRotationEuler( const glm::vec3& vEuler );
	void				SetRotationEuler( const float fX, const float fY, const float fZ );
	glm::vec3			GetRotationEuler() const;

	glm::mat4			GetMatrix() const;

	glm::mat3 m_mMatrix;
	glm::vec3 m_vPosition;
	glm::vec3 m_vScale;
	glm::vec3 m_vRotationEuler; // Mainly used for editor
};

// TODO #eric we should have a way to specify which callbacks should be called (i.e. Update(), ...)
struct TransformComponent : public Component
{
	explicit TransformComponent( Entity* pEntity );

	Transform m_oTransform;
};

class Entity : public Intrusive
{
public:
	Entity();
	Entity( const uint64 uID, const char* sName );

	uint64				GetID() const;
	const char*			GetName() const;

	void				SetMatrix( const glm::mat4& mMatrix );
	glm::mat4			GetMatrix() const;

	void				SetTransform( const Transform& mMatrix );
	Transform&			GetTransform();
	const Transform&	GetTransform() const;

	glm::vec3			GetPosition() const;
	void				SetPosition( const glm::vec3& vPosition );
	void				SetPosition( const float fX, const float fY, const float fZ );

	glm::vec3			GetScale() const;
	void				SetScale( const glm::vec3& vScale );
	void				SetScale( const float fX, const float fY, const float fZ );

	glm::quat			GetRotation() const;
	void				SetRotation( const glm::quat& qRotation );
	void				SetRotation( const glm::vec3& vAxis, const float fAngle );
	void				SetRotationX( const float fAngle );
	void				SetRotationY( const float fAngle );
	void				SetRotationZ( const float fAngle );

private:
	uint64			m_uID;
	const char*		m_sName;

	using TransformHandle = ComponentHandle< TransformComponent >;
	TransformHandle	m_hTransformComponent;
};