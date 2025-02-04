#include "Intrusive.h"

#ifdef TRACK_MEMORY
#include "MemoryTracker.h"
#endif

Intrusive::Intrusive()
	: m_pWeakLink( nullptr )
	, m_uReferenceCount( 0 )
{
	TrackMemory();
}

Intrusive::~Intrusive()
{
	UnTrackMemory();

	ASSERT( m_uReferenceCount == 0 );
	m_uReferenceCount = 0;

	WeakPtrBase* pLink = m_pWeakLink;
	while( pLink != nullptr )
	{
		ASSERT( pLink->m_pPtr != nullptr );
		pLink->m_pPtr = nullptr;
		pLink = pLink->m_pNext;
	}
}

uint Intrusive::GetReferenceCount() const
{
	return m_uReferenceCount;
}

uint Intrusive::CountWeakReferences() const
{
	uint uCount = 0;

	WeakPtrBase* pLink = m_pWeakLink;
	while( pLink != nullptr )
	{
		++uCount;
		pLink = pLink->m_pNext;
	}

	return uCount;
}

void Intrusive::TrackMemory()
{
#ifdef TRACK_MEMORY
	if( g_pMemoryTracker != nullptr )
		g_pMemoryTracker->RegisterIntrusive( this );
#endif
}

void Intrusive::UnTrackMemory()
{
#if TRACK_MEMORY
	if( g_pMemoryTracker != nullptr )
		g_pMemoryTracker->UnRegisterIntrusive( this );
#endif
}

StrongPtrBase::StrongPtrBase()
	: m_pPtr( nullptr )
{
}

StrongPtrBase::StrongPtrBase( Intrusive* pPtr )
	: m_pPtr( pPtr )
{
	if( m_pPtr != nullptr )
		++( m_pPtr->m_uReferenceCount );
}

StrongPtrBase::StrongPtrBase( const StrongPtrBase& xPtr )
	: m_pPtr( xPtr.m_pPtr )
{
	AddReference();
}

StrongPtrBase& StrongPtrBase::operator=( const StrongPtrBase& xPtr )
{
	if( &xPtr == this )
		return *this;

	RemoveReference();

	m_pPtr = xPtr.m_pPtr;

	AddReference();

	return *this;
}

StrongPtrBase::StrongPtrBase( StrongPtrBase&& xPtr ) noexcept
	: m_pPtr( xPtr.m_pPtr )
{
	if( m_pPtr != nullptr )
		ASSERT( m_pPtr->m_uReferenceCount > 0 );

	xPtr.m_pPtr = nullptr;
}

StrongPtrBase& StrongPtrBase::operator=( StrongPtrBase&& xPtr ) noexcept
{
	if( &xPtr == this )
		return *this;

	RemoveReference();

	m_pPtr = xPtr.m_pPtr;
	if( m_pPtr != nullptr )
		ASSERT( m_pPtr->m_uReferenceCount > 0 );

	xPtr.m_pPtr = nullptr;

	return *this;
}

StrongPtrBase::~StrongPtrBase()
{
	RemoveReference();
}

void StrongPtrBase::AddReference()
{
	if( m_pPtr != nullptr )
	{
		ASSERT( m_pPtr->m_uReferenceCount > 0 );
		++( m_pPtr->m_uReferenceCount );
	}
}

void StrongPtrBase::RemoveReference()
{
	if( m_pPtr != nullptr )
	{
		ASSERT( m_pPtr->m_uReferenceCount > 0 );
		if( --( m_pPtr->m_uReferenceCount ) == 0 )
			delete m_pPtr;;
	}
}

WeakPtrBase::WeakPtrBase()
	: m_pPtr( nullptr )
	, m_pNext( nullptr )
{
}

WeakPtrBase::WeakPtrBase( Intrusive* pPtr )
	: m_pPtr( pPtr )
	, m_pNext( nullptr )
{
	AddLink();
}

WeakPtrBase::WeakPtrBase( const WeakPtrBase& xPtr )
	: m_pPtr( xPtr.m_pPtr )
	, m_pNext( nullptr )
{
	AddLink();
}

WeakPtrBase& WeakPtrBase::operator=( const WeakPtrBase& xPtr )
{
	if( &xPtr == this )
		return *this;

	RemoveLink();

	m_pPtr = xPtr.m_pPtr;

	AddLink();

	return *this;
}

WeakPtrBase::~WeakPtrBase()
{
	RemoveLink();
}

void WeakPtrBase::AddLink()
{
	if( m_pPtr != nullptr )
	{
		ASSERT( m_pPtr->m_uReferenceCount > 0 );
		m_pNext = m_pPtr->m_pWeakLink;
		m_pPtr->m_pWeakLink = this;
	}
}

void WeakPtrBase::RemoveLink()
{
	if( m_pPtr != nullptr && m_pPtr->m_pWeakLink != nullptr )
	{
		if( m_pPtr->m_pWeakLink == this )
		{
			m_pPtr->m_pWeakLink = m_pNext;
		}
		else
		{
			WeakPtrBase* pLink = m_pPtr->m_pWeakLink;
			while( pLink->m_pNext != nullptr )
			{
				if( pLink->m_pNext == this )
					pLink->m_pNext = m_pNext;
				else
					pLink = pLink->m_pNext;
			}
		}
	}
}
