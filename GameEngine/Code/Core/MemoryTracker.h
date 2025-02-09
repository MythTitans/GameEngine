#pragma once

#include <mutex>
#include <typeindex>
#include <unordered_map>

#include "Types.h"

class ArrayBase;
class Intrusive;
struct ComponentsHolderBase;

struct IntrusiveMemory
{
	IntrusiveMemory();

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

struct ArrayReference
{
	ArrayReference( const ArrayBase* pArray, const std::type_index oArrayType, const uint64 uArrayTypeSize );

	const ArrayBase*	m_pArray;
	std::type_index		m_oArrayType;
	uint64				m_uArrayTypeSize;
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

	template < typename ComponentType >
	void		RegisterArray( ArrayBase* pArray )
	{
		std::unique_lock oLock( m_oMutex );

		m_lArrays.push_back( ArrayReference( pArray, typeid( ComponentType ), sizeof( ComponentType ) ) );
	}
	void		UnRegisterArray( ArrayBase* pArray );

private:
	std::list< const Intrusive* >	m_lIntrusives;
	std::vector< ComponentMemory >	m_aComponents;
	std::list< ArrayReference >		m_lArrays;

	std::mutex						m_oMutex;

	bool							m_bDisplayMemoryTracker;
};

extern MemoryTracker* g_pMemoryTracker;