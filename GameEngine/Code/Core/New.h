#pragma once

#include <utility>

#include "SimpleAllocator.h"


extern uint uTotalAllocated;
extern uint uTotalFreed;

#include "Logger.h"

template< typename T, typename... Args >
T* New( Args&&... oArgs )
{
	uTotalAllocated += sizeof( T );
	LOG_INFO( "Allocating {} bytes, total allocated {} bytes", sizeof( T ), uTotalAllocated );

	T* pAddress = SimpleAllocator::Allocate< T >( 1 );
	::new ( pAddress ) T( std::forward< Args >( oArgs )... );
	return pAddress;
}

template< typename T >
void Delete( T* pPtr )
{
	if( pPtr == nullptr )
		return;

	uTotalFreed += sizeof( T );
	LOG_INFO( "Freeing {} bytes, total freed {} bytes", sizeof( T ), uTotalFreed );

	pPtr->~T();
	SimpleAllocator::Deallocate( pPtr );
}
