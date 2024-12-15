#pragma once

#include <typeindex>
#include <unordered_map>

#include "Core/Profiler.h"
#include "Scene.h"

struct ComponentsHolderBase
{
	ComponentsHolderBase();
	virtual ~ComponentsHolderBase();

	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void Update( const float fDeltaTime ) = 0;

	uint m_uVersion;
};

template < typename ComponentType >
struct ComponentsHolder : ComponentsHolderBase
{
	void Start() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Start();
	}

	void Stop() override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Stop();
	}

	void Update( const float fDeltaTime ) override
	{
		ProfilerBlock oBlock( PROFILER_BLOCK_NAME.c_str() );

		for( ComponentType& oComponent : m_aComponents )
			oComponent.Update( fDeltaTime );
	}

	ComponentType* GetComponent( const uint64 uEntityID )
	{
		for( ComponentType& oComponent : m_aComponents )
		{
			if( oComponent.m_uEntityID == uEntityID )
				return &oComponent;
		}

		return nullptr;
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

	template < typename ComponentType >
	ComponentType& CreateComponent( const Entity& oEntity )
	{
		ComponentsHolderBase*& pComponentsHolderBase = m_mComponentsHolders[ std::type_index( typeid( ComponentType ) ) ];
		if( pComponentsHolderBase == nullptr )
			pComponentsHolderBase = new ComponentsHolder< ComponentType >;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( pComponentsHolderBase );
		pComponentsHolder->m_aComponents.PushBack( ComponentType( oEntity ) );

		return pComponentsHolder->m_aComponents.Back();
	}

	template < typename ComponentType >
	ComponentType* GetComponent( const uint64 uEntityID )
	{
		auto it = m_mComponentsHolders.find( std::type_index( typeid( ComponentType ) ) );
		if( it == m_mComponentsHolders.end() || it->second == nullptr )
			return nullptr;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( it->second );
		return pComponentsHolder->GetComponent( uEntityID );
	}

	template < typename ComponentType >
	ComponentType* GetComponent( const Entity& oEntity )
	{
		return GetComponent( oEntity.GetID() );
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

	void Start();
	void Stop();
	void Update( const float fDeltaTime );

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