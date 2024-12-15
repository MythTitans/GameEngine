#pragma once

#include <typeindex>
#include <unordered_map>

#include "Core/Array.h"
#include "Core/Profiler.h"

class Entity;

struct ComponentsHolderBase
{
	virtual ~ComponentsHolderBase();

	virtual void Start() = 0;
	virtual void Stop() = 0;
	virtual void Update( const float fDeltaTime ) = 0;
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

	std::string PROFILER_BLOCK_NAME = []() {
		const std::string sName = std::string( typeid( ComponentType ).name() );
		return sName.substr( sName.find( " " ) + 1 );
	}();

	Array< ComponentType > m_aComponents;
};

// This class is not polymorphic, it just defines an interface for subclasses to follow
class Component
{
public:
	explicit Component( const Entity& oEntity );

	virtual void	Start();
	virtual void	Stop();
	virtual void	Update( const float fDeltaTime );

	Entity&			GetEntity() const;

private:
	uint64 m_uEntityID;
};

class ComponentManager
{
public:
	template < typename ComponentType >
	ComponentType& CreateComponent( Entity& oEntity )
	{
		ComponentsHolderBase*& pComponentsHolderBase = m_mComponentsHolders[ std::type_index( typeid( ComponentType ) ) ];
		if( pComponentsHolderBase == nullptr )
			pComponentsHolderBase = new ComponentsHolder< ComponentType >;

		ComponentsHolder< ComponentType >* pComponentsHolder = static_cast< ComponentsHolder< ComponentType >* >( pComponentsHolderBase );
		pComponentsHolder->m_aComponents.PushBack( ComponentType( oEntity ) );

		return pComponentsHolder->m_aComponents.Back();
	}

	template < typename ComponentType >
	ArrayView< ComponentType > GetComponentsOfType()
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
	std::unordered_map< std::type_index, ComponentsHolderBase* > m_mComponentsHolders;
};