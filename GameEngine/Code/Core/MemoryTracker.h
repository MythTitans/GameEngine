#pragma once

#include <mutex>
#include <typeindex>
#include <unordered_map>

#include "Types.h"

struct ArrayMemory
{
	ArrayMemory();

	uint64	m_uUsedBytes;
	uint64	m_uReservedBytes;
	int		m_uArrayCount;
	int		m_uElementCount;
};

class MemoryTracker
{
public:
	MemoryTracker();
	~MemoryTracker();

	void		Display();

	void		RegisterArray( const std::type_index oTypeIndex, const uint64 uUsedMemory, const uint64 uReservedMemory, const int uElementCount );
	void		UnRegisterArray( const std::type_index oTypeIndex, const uint64 uUsedMemory, const uint64 uReservedMemory, const int uElementCount );

private:
	std::unordered_map< std::type_index, ArrayMemory >	m_mArrays;

	std::mutex											m_oMutex;

	bool												m_bDisplayMemoryTracker;
};

extern MemoryTracker* g_pMemoryTracker;