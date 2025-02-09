#pragma once

#include <typeindex>
#include <unordered_map>

#include "Core/MemoryTracker.h"
#include "Core/Profiler.h"
#include "Scene.h"

struct ComponentsHolderBase
{
	ComponentsHolderBase();
	virtual ~ComponentsHolderBase();

	virtual void InitializeComponents() = 0;
	virtual bool AreComponentsInitialized() = 0;
	virtual void StartComponents() = 0;
	virtual void StopComponents() = 0;
	virtual void UpdateComponents( const float fDeltaTime ) = 0;

	virtual void DisplayGizmos() = 0;

	virtual uint GetCount() const = 0;

	uint m_uVersion;
};

template < typename ComponentType >
struct ComponentsHolder : ComponentsHolderBase
{
	ComponentsHolder()
	{
#ifdef TRACK_MEMORY
		g_pMemoryTracker->RegisterComponent< ComponentType >( this );
#endif
	}

	void InitializeComponents() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Initialize();
	}

	bool AreComponentsInitialized() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
		{
			if( oComponent.IsInitialized() == false )
				return false;
		}

		return true;
	}

	void StartComponents() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Start();
	}

	void StopComponents() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Stop();
	}

	void UpdateComponents( const float fDeltaTime ) override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Update( fDeltaTime );
	}

	void DisplayGizmos() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.DisplayGizmos();
	}

	ComponentType* GetComponent( const Entity* pEntity )
	{
		for( ComponentType& oComponent : m_aComponents )
		{
			if( oComponent.m_pEntity == pEntity )
				return &oComponent;
		}

		return nullptr;
	}

	uint GetCount() const
	{
		return m_aComponents.Count();
	}

	std::string PROFILER_BLOCK_NAME = []() {
		const std::string sName = std::string( typeid( ComponentType ).name() );
		return sName.substr( sName.find( " " ) + 1 );
	}();

	Array< ComponentType > m_aComponents;
};

// TODO #eric don't forget the version when dealing with destroyed components
class ComponentManager
{
public:
	template < typename ComponentType >
	friend class ComponentHandle;

	ComponentManager();
	~ComponentManager();

	// TODO #eric maybe we should always return component handles ?
	template < typename ComponentType >
	ComponentType& CreateComponent( Entity* oEntity )
	{
		ComponentsHolderBase*& pComponentsHolderBase = m_mComponentsHolders[ std::type_index( typeid( ComponentType ) ) ];
		if( pComponentsHolderBase == nullptr )
			pComponentsHolderBase = new ComponentsHolder< ComponentType >;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( pComponentsHolderBase );
		pComponentsHolder->m_aComponents.PushBack( ComponentType( oEntity ) );

		return pComponentsHolder->m_aComponents.Back();
	}

	template < typename ComponentType >
	ComponentType* GetComponent( const Entity* pEntity )
	{
		auto it = m_mComponentsHolders.find( std::type_index( typeid( ComponentType ) ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return nullptr;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponent( pEntity );
	}

	template < typename ComponentType >
	ArrayView< ComponentType > GetComponents()
	{
		auto it = m_mComponentsHolders.find( std::type_index( typeid( ComponentType ) ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return ArrayView< ComponentType >();

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->m_aComponents;
	}

	void InitializeComponents();
	bool AreComponentsInitialized() const;
	void StartComponents();
	void StopComponents();
	void UpdateComponents( const float fDeltaTime );

	void DisplayGizmos();

private:
	template < typename ComponentType >
	uint GetVersion()
	{
		auto it = m_mComponentsHolders.find( std::type_index( typeid( ComponentType ) ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return UINT_MAX;

		return it->second->m_uVersion;
	}

	std::unordered_map< std::type_index, ComponentsHolderBase* > m_mComponentsHolders;
};

extern ComponentManager* g_pComponentManager;