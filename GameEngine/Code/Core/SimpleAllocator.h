#pragma once

#include <cstddef>

#include "Types.h"

class SimpleAllocator
{
public:
	static void* AllocateRaw( const std::size_t uBytes );
	static void DeallocateRaw( void* pAddress );

	template < typename T >
	static T* Allocate(const uint uCount);

	template < typename T >
	static void Deallocate( T*& pMemory );
};

template < typename T >
T* SimpleAllocator::Allocate( const uint uCount )
{
	return static_cast< T* >( AllocateRaw( uCount * sizeof( T ) ) );
}

template < typename T >
void SimpleAllocator::Deallocate( T*& pMemory )
{
	DeallocateRaw( pMemory );
	pMemory = nullptr;
}
