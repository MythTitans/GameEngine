#pragma once

#include <string>

#include <glm/glm.hpp>

#include "Component.h"
#include "Core/Types.h"
#include "ResourceLoader.h"

struct Mat3x4
{
	Mat3x4();
	explicit Mat3x4( const glm::mat4& mMatrix );

	glm::vec3& GetI();
	const glm::vec3& GetI() const;
	glm::vec3& GetJ();
	const glm::vec3& GetJ() const;
	glm::vec3& GetK();
	const glm::vec3& GetK() const;
	glm::vec3& GetO();
	const glm::vec3& GetO() const;

	glm::mat4			GetMatrix() const;

	glm::mat3 m_mMatrix;
	glm::vec3 m_vPosition;
};

class Entity
{
public:
	Entity( const uint64 uID, const char* sName );

	uint64			GetID() const;
	const char*		GetName() const;

	void			SetMatrix( const glm::mat4& mMatrix );
	void			SetMatrix( const Mat3x4& mMatrix );
	Mat3x4&			GetMatrix();
	const Mat3x4&	GetMatrix() const;

	glm::vec3		GetPosition() const;
	void			SetPosition( const glm::vec3& vPosition );
	void			SetPosition( const float fX, const float fY, const float fZ );

	glm::vec3		GetScale() const;
	void			SetScale( const glm::vec3& vScale );
	void			SetScale( const float fX, const float fY, const float fZ );

	glm::quat		GetRotation() const;
	void			SetRotation( const glm::quat& qRotation );
	void			SetRotation( const glm::vec3& vAxis, const float fAngle );
	void			SetRotationX( const float fAngle );
	void			SetRotationY( const float fAngle );
	void			SetRotationZ( const float fAngle );

private:
	uint64		m_uID;
	const char* m_sName;
	Mat3x4		m_mMatrix;
};

// TODO #eric turn into a class
struct Scene
{
	Scene();

	bool OnLoading();

	void Start();
	void Stop();
	void Update( const float fDeltaTime );

	ModelResPtr m_xCube;
	ModelResPtr m_xSphere;
	ModelResPtr m_xGolem;

	Array< Entity > m_aEntities;
	ComponentManager m_oComponentManager;
};

class MyFirstComponent : public Component
{
public:
	explicit MyFirstComponent( const Entity& oEntity );

	void Start() override;
	void Update( const float fDeltaTime ) override;

private:
	float m_fTotal;
	float m_fScale;
};

// TODO #eric would be better that the component create/update a graph node
class VisualComponent : public Component
{
public:
	explicit VisualComponent( const Entity& oEntity );

	void				Setup( const ModelResPtr& xResource );

	const ModelResPtr& GetResource() const;

private:
	ModelResPtr m_xResource;
};