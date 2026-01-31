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
	explicit Transform( const glm::mat4x3& mTRSMatrix );
	Transform( const glm::mat4x3& mTRMatrix, const glm::vec3& vScale );

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

	glm::mat4x3			GetMatrixTR() const;
	glm::mat4x3			GetMatrixTRS() const;

	bool				IsUniformScale() const;

	glm::mat3	m_mMatrix;
	glm::vec3	m_vPosition;
	glm::vec3	m_vScale;

	bool		m_bUniformScale;
};

class TransformComponent : public Component
{
public:
	friend class Entity;

	explicit TransformComponent( Entity* pEntity );

	void		Update( const GameContext& oGameContext ) override;
	void		Finalize() override;

#ifdef EDITOR
	void		SetRotationEuler( const glm::vec3& vEuler );
	void		SetRotationEuler( const float fX, const float fY, const float fZ );
	glm::vec3	GetRotationEuler() const;
#endif

private:
	Transform	m_oTransform;
	bool		m_bDirty;

#ifdef EDITOR
	glm::vec3	m_vRotationEuler;
	bool		m_bDirtyRotation;
#endif
};

class Entity : public Intrusive
{
public:
	friend class Scene;

	Entity();
	Entity( const uint64 uID, const std::string& sName );
	~Entity();

	uint64					GetSize() const override;

	uint64					GetID() const;

	void					SetName( const std::string& sName );
	const std::string&		GetName() const;

	Entity*					GetParent() const;
	const Array< Entity* >&	GetChildren() const;

	void					SetWorldTransform( const Transform& oTransform );
	Transform				GetWorldTransform() const;
	
	void					SetWorldPosition( const glm::vec3& vPosition );
	void					SetWorldPosition( const float fX, const float fY, const float fZ );
	glm::vec3				GetWorldPosition() const;

	void					SetRotation( const glm::quat& qRotation );
	void					SetRotation( const glm::vec3& vAxis, const float fAngle );
	void					SetRotationX( const float fAngle );
	void					SetRotationY( const float fAngle );
	void					SetRotationZ( const float fAngle );
	glm::quat				GetRotation() const;

	void					SetTransform( const Transform& oTransform );
	const Transform&		GetTransform() const;

	glm::vec3				GetPosition() const;
	void					SetPosition( const glm::vec3& vPosition );
	void					SetPosition( const float fX, const float fY, const float fZ );

	glm::vec3				GetScale() const;
	void					SetScale( const glm::vec3& vScale );
	void					SetScale( const float fX, const float fY, const float fZ );

	bool					IsDirty() const;

private:
	uint64				m_uID;
	std::string			m_sName;

	Entity*				m_pParent;
	Array< Entity* >	m_aChildren;

	using TransformHandle = ComponentHandle< TransformComponent >;
	TransformHandle		m_hTransformComponent;
};