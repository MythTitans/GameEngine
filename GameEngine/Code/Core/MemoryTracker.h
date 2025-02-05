#pragma once

#include <mutex>
#include <typeindex>
#include <unordered_map>

#include "Types.h"

class Intrusive;
struct ComponentsHolderBase;

struct ObjectMemory
{
	ObjectMemory();

	uint64	m_uBytes;
	uint	m_uCount;
};

struct ComponentMemory
{
	ComponentMemory( const std::type_index oComponentType, const uint64 uComponentSize, const ComponentsHolderBase* pComponentHolder );

	std::type_index				m_oComponentType;
	uint64						m_uComponentSize;
	const ComponentsHolderBase*	m_pComponentHolder;
};

struct ArrayMemory
{
	ArrayMemory();

	uint64	m_uUsedBytes;
	uint64	m_uReservedBytes;
	uint	m_uArrayCount;
	uint	m_uElementCount;
};

class MemoryTracker
{
public:
	MemoryTracker();
	~MemoryTracker();

	void		Display();

	void		RegisterIntrusive( const Intrusive* pIntrusive );
	void		UnRegisterIntrusive( const Intrusive* pIntrusive );

	template < typename ComponentType >
	void		RegisterComponent( const ComponentsHolderBase* pComponentHolder )
	{
		m_aComponents.push_back( ComponentMemory( typeid( ComponentType ), sizeof( ComponentType ), pComponentHolder ) );
	}

	void		RegisterArray( const std::type_index oTypeIndex, const uint64 uUsedMemory, const uint64 uReservedMemory, const int uElementCount );
	void		UnRegisterArray( const std::type_index oTypeIndex, const uint64 uUsedMemory, const uint64 uReservedMemory, const int uElementCount );

private:
	std::unordered_map< std::type_index, ArrayMemory >	m_mArrays;
	std::vector< ComponentMemory >						m_aComponents;
	std::list< const Intrusive* >						m_lIntrusives;

	std::mutex											m_oMutex;

	bool												m_bDisplayMemoryTracker;
};

extern MemoryTracker* g_pMemoryTracker;