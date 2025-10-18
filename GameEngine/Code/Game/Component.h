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
	friend class ComponentsHolder;

	template < typename ComponentType >
	friend class ComponentHandle;

	template < typename ComponentType >
	friend class ComponentSubTypeHandle;

	explicit Component( Entity* pEntity );

	virtual void						Initialize();
	virtual bool						IsInitialized() const;
	virtual void						Start();
	virtual void						Stop();
	virtual void						Tick();
	virtual void						BeforePhysics();
	virtual void						AfterPhysics();
	virtual void						Update( const GameContext& oGameContext );
	virtual void						Dispose();

	virtual void						DisplayGizmos( const bool bSelected );
#ifdef EDITOR
	virtual void						DisplayInspector();
	virtual void						OnPropertyChanged( const std::string& sProperty );
#endif

	Entity*								GetEntity();
	const Entity*						GetEntity() const;
	uint64								GetEntityID() const;
	
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
class ComponentHandleImplBase
{
public:
	virtual void			Refresh( const Entity* pEntity, uint& uVersion, int& iIndex ) const = 0;
	virtual ComponentType*	GetComponentFromIndex( const int iIndex ) const = 0;
};

template < typename ComponentType, typename RealComponentType >
class ComponentHandleImpl : public ComponentHandleImplBase< ComponentType >
{
public:
	void Refresh( const Entity* pEntity, uint& uVersion, int& iIndex ) const override
	{
		const uint uCurrentVersion = g_pComponentManager->GetVersion< RealComponentType >();
		if( uVersion != uCurrentVersion )
		{
			iIndex = -1;
			uVersion = uCurrentVersion;

			iIndex = g_pComponentManager->GetComponentIndexFromEntity< RealComponentType >( pEntity );
		}
	}

	ComponentType* GetComponentFromIndex( const int iIndex ) const override
	{
		return g_pComponentManager->GetComponentFromIndex< RealComponentType >( iIndex );
	}
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

		return GetComponentFromIndex( m_iIndex );
	}

	const ComponentType* operator->() const
	{
		Refresh();

		return GetComponentFromIndex( m_iIndex );
	}

	ComponentType& operator*()
	{
		Refresh();

		return *GetComponentFromIndex( m_iIndex );
	}

	const ComponentType& operator*() const
	{
		Refresh();

		return *GetComponentFromIndex( m_iIndex );
	}

	operator ComponentType*()
	{
		Refresh();

		if( m_iIndex == -1 )
			return nullptr;

		return GetComponentFromIndex( m_iIndex );
	}

	operator const ComponentType*() const
	{
		Refresh();

		if( m_iIndex == -1 )
			return nullptr;

		return GetComponentFromIndex( m_iIndex );
	}

	bool IsValid() const
	{
		Refresh();

		return m_iIndex != -1;
	}

	int GetIndex() const
	{
		Refresh();

		return m_iIndex;
	}

private:
	static ComponentHandleImpl< ComponentType, ComponentType > s_oComponentHandleImpl;

	void Refresh() const
	{
		s_oComponentHandleImpl.Refresh( m_pEntity, m_uVersion, m_iIndex );
	}

	ComponentType* GetComponentFromIndex( const int iIndex ) const
	{
		return s_oComponentHandleImpl.GetComponentFromIndex( m_iIndex );
	}

	Entity*			m_pEntity;
	mutable uint	m_uVersion;
	mutable int		m_iIndex;
};

template < typename ComponentType >
ComponentHandleImpl< ComponentType, ComponentType > ComponentHandle< ComponentType >::s_oComponentHandleImpl;

template < typename ComponentType >
class ComponentSubTypeHandle
{
public:
	ComponentSubTypeHandle()
		: m_pEntity( nullptr )
		, m_uVersion( UINT_MAX )
		, m_iIndex( -1 )
	{
	}

	ComponentSubTypeHandle( ComponentType* pComponent )
		: m_pComponentSubTypeHandleImpl( nullptr )
		, m_pEntity( pComponent != nullptr ? pComponent->m_pEntity : nullptr )
		, m_uVersion( UINT_MAX )
		, m_iIndex( -1 )
	{
		Refresh();
	}

	template < typename RealComponentType >
	void SetComponentSubType()
	{
		m_pComponentSubTypeHandleImpl.reset( new ComponentHandleImpl< ComponentType, RealComponentType > );
	}

	ComponentType* operator->()
	{
		Refresh();

		return GetComponentFromIndex( m_iIndex );
	}

	const ComponentType* operator->() const
	{
		Refresh();

		return GetComponentFromIndex( m_iIndex );
	}

	ComponentType& operator*()
	{
		Refresh();

		return *GetComponentFromIndex( m_iIndex );
	}

	const ComponentType& operator*() const
	{
		Refresh();

		return *GetComponentFromIndex( m_iIndex );
	}

	operator ComponentType* ( )
	{
		Refresh();

		if( m_iIndex == -1 )
			return nullptr;

		return GetComponentFromIndex( m_iIndex );
	}

	operator const ComponentType* ( ) const
	{
		Refresh();

		if( m_iIndex == -1 )
			return nullptr;

		return GetComponentFromIndex( m_iIndex );
	}

	bool IsValid() const
	{
		Refresh();

		return m_iIndex != -1;
	}

	int GetIndex() const
	{
		Refresh();

		return m_iIndex;
	}

private:
	void Refresh() const
	{
		if( m_pComponentSubTypeHandleImpl != nullptr )
			m_pComponentSubTypeHandleImpl->Refresh( m_pEntity, m_uVersion, m_iIndex );
		else
			m_iIndex = -1;
	}

	ComponentType* GetComponentFromIndex( const int iIndex ) const
	{
		if( m_pComponentSubTypeHandleImpl != nullptr )
			return m_pComponentSubTypeHandleImpl->GetComponentFromIndex( iIndex );

		return nullptr;
	}

	using ComponentHandleImplPtr = std::unique_ptr< ComponentHandleImplBase< ComponentType > >;
	ComponentHandleImplPtr	m_pComponentSubTypeHandleImpl;
	Entity*					m_pEntity;
	mutable uint			m_uVersion;
	mutable int				m_iIndex;
};
