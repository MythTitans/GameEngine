#pragma once

#include "ComponentManager.h"
#include "Core/Array.h"

class Entity;
struct GameContext;

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
	virtual bool						IsInitialized() const;
	virtual void						Start();
	virtual void						Stop();
	virtual void						Tick();
	virtual void						BeforePhysics();
	virtual void						AfterPhysics();
	virtual void						Update( const GameContext& oGameContext );

	virtual void						DisplayGizmos( const bool bSelected );
	virtual void						DisplayInspector();
	virtual void						OnPropertyChanged( const std::string& sProperty );

	Entity*								GetEntity();
	const Entity*						GetEntity() const;
	
	template < typename ComponentType >
	ComponentHandle< ComponentType >	GetComponent()
	{
		return g_pComponentManager->GetComponent< ComponentType >( m_pEntity );
	}

	template < typename ComponentType >
	const ComponentHandle< ComponentType >	GetComponent() const
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

	operator ComponentType*()
	{
		Refresh();

		ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();

		if( m_iIndex == -1 )
			return nullptr;

		ASSERT( m_iIndex >= 0 && m_iIndex < ( int )aComponents.Count() );
		return &aComponents[ m_iIndex ];
	}

	operator const ComponentType*() const
	{
		Refresh();

		ArrayView< ComponentType > aComponents = g_pComponentManager->GetComponents< ComponentType >();

		if( m_iIndex == -1 )
			return nullptr;

		ASSERT( m_iIndex >= 0 && m_iIndex < ( int )aComponents.Count() );
		return &aComponents[ m_iIndex ];
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
