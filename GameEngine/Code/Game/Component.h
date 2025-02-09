#pragma once

#include "ComponentManager.h"
#include "Core/Array.h"
#include "ResourceLoader.h"

class Entity;

// This class is not polymorphic, it just defines an interface for subclasses to follow
class Component
{
public:
	template < typename ComponentType >
	friend struct ComponentsHolder;

	template < typename ComponentType >
	friend class ComponentHandle;

	explicit Component( Entity* pEntity );

	virtual void						Initialize();
	virtual bool						IsInitialized();
	virtual void						Start();
	virtual void						Stop();
	virtual void						Update( const float fDeltaTime );

	virtual void						DisplayGizmos();

	Entity*								GetEntity();
	const Entity*						GetEntity() const;
	
	template < typename ComponentType >
	ComponentHandle< ComponentType >	GetComponent()
	{
		return g_pComponentManager->GetComponent< ComponentType >( m_pEntity );
	}

private:
	Entity* m_pEntity;
};

template < typename ComponentType >
class ComponentHandle
{
public:
	ComponentHandle()
		: m_pEntity( nullptr )
		, m_uVersion( UINT_MAX )
		, m_iIndex( -1 )
	{
	}

	ComponentHandle( ComponentType* pComponent )
		: m_pEntity( pComponent != nullptr ? pComponent->m_pEntity : nullptr )
		, m_uVersion( UINT_MAX )
		, m_iIndex( -1 )
	{
		Refresh();
	}

	ComponentType* operator->()
	{
		Refresh();

		ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();

		ASSERT( m_iIndex >= 0 && m_iIndex < ( int )aComponents.Count() );
		return &aComponents[ m_iIndex ];
	}

	const ComponentType* operator->() const
	{
		Refresh();

		ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();

		ASSERT( m_iIndex >= 0 && m_iIndex < ( int )aComponents.Count() );
		return &aComponents[ m_iIndex ];
	}

	ComponentType& operator*()
	{
		Refresh();

		ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();

		ASSERT( m_iIndex >= 0 && m_iIndex < ( int )aComponents.Count() );
		return aComponents[ m_iIndex ];
	}

	const ComponentType& operator*() const
	{
		Refresh();

		ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();

		ASSERT( m_iIndex >= 0 && m_iIndex < ( int )aComponents.Count() );
		return aComponents[ m_iIndex ];
	}

	bool IsValid() const
	{
		Refresh();

		return m_iIndex != -1;
	}

private:
	void Refresh() const
	{
		const uint uVersion = g_pComponentManager->GetVersion< ComponentType >();
		if( m_uVersion != uVersion )
		{
			m_iIndex = -1;
			m_uVersion = uVersion;

			ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();
			for( uint u = 0; u < aComponents.Count(); ++u )
			{
				if( aComponents[ u ].m_pEntity == m_pEntity )
				{
					m_iIndex = u;
					break;
				}
			}
		}
	}

	Entity*			m_pEntity;
	mutable uint	m_uVersion;
	mutable int		m_iIndex;
};

// TODO #eric would be better that the component create/update a graph node
class VisualComponent : public Component
{
public:
	explicit VisualComponent( Entity* pEntity );

	void				Setup( const char* sModelFile );
	void				Initialize() override;
	bool				IsInitialized() override;
	void				Update( const float fDeltaTime ) override;

	const ModelResPtr&	GetResource() const;

	const glm::mat4&	GetWorldMatrix() const;

private:
	std::string	m_sModelFile;
	ModelResPtr	m_xModel;

	glm::mat4	m_mWorldMatrix;
};

class DirectionalLightComponent : public Component
{
public:
	explicit DirectionalLightComponent( Entity* pEntity );

	void DisplayGizmos() override;

	glm::vec3	m_vDirection;
	glm::vec3	m_vColor;
	float		m_fIntensity;
};

class PointLightComponent : public Component
{
public:
	explicit PointLightComponent( Entity* pEntity );

	void				Update( const float fDeltaTime ) override;

	void				DisplayGizmos() override;

	const glm::vec3&	GetPosition() const;

private:
	glm::vec3	m_vPosition;

public:
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fFalloffFactor;
};

class SpotLightComponent : public Component
{
public:
	explicit SpotLightComponent( Entity* pEntity );

	void				Update( const float fDeltaTime ) override;

	void				DisplayGizmos() override;

	const glm::vec3&	GetPosition() const;

private:
	glm::vec3	m_vPosition;

public:
	glm::vec3	m_vDirection;
	glm::vec3	m_vColor;
	float		m_fIntensity;
	float		m_fInnerAngle;
	float		m_fOuterAngle;
	float		m_fFalloffFactor;
};