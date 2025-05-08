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

	glm::mat4			GetMatrixTR() const;
	glm::mat4			GetMatrixTRS() const;

	glm::mat3	m_mMatrix;
	glm::vec3	m_vPosition;
	glm::vec3	m_vScale;

	bool		m_bDirtyRotation;
	bool		m_bUniformScale;
};

// TODO #eric we should have a way to specify which callbacks should be called (i.e. Update(), ...)
struct TransformComponent : public Component
{
	explicit TransformComponent( Entity* pEntity );

	Transform m_oTransform;
};

struct EulerComponent : public Component
{
	explicit EulerComponent( Entity* pEntity );

	void		Start() override;
	void		Update( const GameContext& oGameContext ) override;

	void		SetRotationEuler( const glm::vec3& vEuler );
 	void		SetRotationEuler( const float fX, const float fY, const float fZ );
 	glm::vec3	GetRotationEuler() const;

	using TransformHandle = ComponentHandle< TransformComponent >;
	TransformHandle	m_hTransformComponent;
	
	glm::vec3		m_vRotationEuler;
};

class Entity : public Intrusive
{
public:
	friend class Scene;

	Entity();
	Entity( const uint64 uID, const std::string& sName );

	uint64				GetID() const;
	const std::string&	GetName() const;

	Entity*				GetParent();
	const Entity*		GetParent() const;

	void				SetWorldTransform( const Transform& oTransform );
	Transform			GetWorldTransform() const;
	
	void				SetWorldPosition( const glm::vec3& vPosition );
	void				SetWorldPosition( const float fX, const float fY, const float fZ );
	glm::vec3			GetWorldPosition() const;

	void				SetRotation( const glm::quat& qRotation );
	void				SetRotation( const glm::vec3& vAxis, const float fAngle );
	void				SetRotationX( const float fAngle );
	void				SetRotationY( const float fAngle );
	void				SetRotationZ( const float fAngle );
	glm::quat			GetRotation() const;

	void				SetTransform( const Transform& oTransform );
	Transform&			GetTransform();
	const Transform&	GetTransform() const;

	glm::vec3			GetPosition() const;
	void				SetPosition( const glm::vec3& vPosition );
	void				SetPosition( const float fX, const float fY, const float fZ );

	glm::vec3			GetScale() const;
	void				SetScale( const glm::vec3& vScale );
	void				SetScale( const float fX, const float fY, const float fZ );

private:
	uint64				m_uID;
	std::string			m_sName;

	Entity*				m_pParent;
	Array< Entity* >	m_aChildren;

	using TransformHandle = ComponentHandle< TransformComponent >;
	TransformHandle		m_hTransformComponent;
};