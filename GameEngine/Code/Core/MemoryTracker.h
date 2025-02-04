#pragma once

#include <mutex>
#include <typeindex>
#include <unordered_map>

#include "Types.h"

class Intrusive;

struct ObjectMemory
{
	ObjectMemory();

	uint64	m_uBytes;
	uint	m_uCount;
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

	void		RegisterArray( const std::type_index oTypeIndex, const uint64 uUsedMemory, const uint64 uReservedMemory, const int uElementCount );
	void		UnRegisterArray( const std::type_index oTypeIndex, const uint64 uUsedMemory, const uint64 uReservedMemory, const int uElementCount );

private:
	std::unordered_map< std::type_index, ArrayMemory >	m_mArrays;
	std::list< const Intrusive* >						m_lIntrusives;

	std::mutex											m_oMutex;

	bool												m_bDisplayMemoryTracker;
};

extern MemoryTracker* g_pMemoryTracker;