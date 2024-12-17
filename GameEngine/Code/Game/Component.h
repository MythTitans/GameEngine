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

	explicit Component( const Entity& oEntity );

	virtual void						Initialize();
	virtual bool						IsInitialized();
	virtual void						Start();
	virtual void						Stop();
	virtual void						Update( const float fDeltaTime );

	Entity&								GetEntity() const;
	
	template < typename ComponentType >
	ComponentHandle< ComponentType >	GetComponent()
	{
		return g_pComponentManager->GetComponent< ComponentType >( m_uEntityID );
	}

private:
	uint64 m_uEntityID;
};

template < typename ComponentType >
class ComponentHandle
{
public:
	ComponentHandle()
		: m_uEntityID( UINT64_MAX )
		, m_uVersion( UINT_MAX )
		, m_iIndex( -1 )
	{
	}

	ComponentHandle( ComponentType* pComponent )
		: m_uEntityID( pComponent != nullptr ? pComponent->m_uEntityID : UINT64_MAX )
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
				if( aComponents[ u ].m_uEntityID == m_uEntityID )
				{
					m_iIndex = u;
					break;
				}
			}
		}
	}

	uint64			m_uEntityID;
	mutable uint	m_uVersion;
	mutable int		m_iIndex;
};

class MyFirstComponent : public Component
{
public:
	explicit MyFirstComponent( const Entity& oEntity );

	void Update( const float fDeltaTime ) override;

	void SetScale( const float fScale );

private:
	float m_fScale;
};

// TODO #eric would be better that the component create/update a graph node
class VisualComponent : public Component
{
public:
	explicit VisualComponent( const Entity& oEntity );

	void				Setup( const std::filesystem::path& oModelFile );
	void				Initialize() override;
	bool				IsInitialized() override;
	void				Start() override;
	void				Update( const float fDeltaTime ) override;

	const ModelResPtr&	GetResource() const;

	const glm::mat4&	GetWorldMatrix() const;

private:
	std::filesystem::path				m_oModelFile;
	ModelResPtr							m_xModel;
	ComponentHandle< MyFirstComponent > m_hTest;

	glm::mat4							m_mWorldMatrix;
};