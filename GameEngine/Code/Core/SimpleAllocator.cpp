#include "SimpleAllocator.h"

#include "ArrayUtils.h"

void* SimpleAllocator::AllocateRaw( const std::size_t uBytes )
{
	return ::operator new( uBytes );
}

void SimpleAllocator::DeallocateRaw( void* pAddress )
{
	::operator delete( pAddress );
}
