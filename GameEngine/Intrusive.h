#pragma once

#include <type_traits>

#include "Common.h"

class Intrusive
{
public:
	template < typename T >
	friend class StrongPtr;
	friend class StrongPtrBase;
	template < typename T >
	friend class WeakPtr;
	friend class WeakPtrBase;

			Intrusive();
	virtual ~Intrusive();

	uint	GetReferenceCount() const;
	uint	CountWeakReferences() const;

private:
	WeakPtrBase*	m_pWeakLink;
	uint			m_uReferenceCount;
};

class StrongPtrBase
{
public:
			StrongPtrBase();
			StrongPtrBase( Intrusive* pPtr );

			StrongPtrBase( const StrongPtrBase& xPtr );
			StrongPtrBase& operator=( const StrongPtrBase& xPtr );

			StrongPtrBase( StrongPtrBase&& xPtr ) noexcept;
			StrongPtrBase& operator=( StrongPtrBase&& xPtr ) noexcept;

			~StrongPtrBase();

protected:
	void	AddReference();
	void	RemoveReference();

	Intrusive* m_pPtr;
};

template < typename T >
class StrongPtr : public StrongPtrBase
{
public:
	StrongPtr()
		: StrongPtrBase( nullptr )
	{
	}

	StrongPtr( T* pPtr )
		: StrongPtrBase( ( Intrusive* )pPtr )
	{
		static_assert( std::is_base_of_v< Intrusive, T >, "Trying to make a StrongPtr to a non-intrusive object." );
	}

	T* operator->()
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return ( T* )m_pPtr;
	}

	const T* operator->() const
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return ( T* ) m_pPtr;
	}

	T& operator*()
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return *( ( T* ) m_pPtr );
	}

	const T& operator*() const
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return *( ( T* ) m_pPtr );
	}

	bool operator!() const
	{
		return !m_pPtr;
	}

	bool operator==( T* pPtr ) const
	{
		return m_pPtr == pPtr;
	}

	bool operator!=( T* pPtr ) const
	{
		return m_pPtr != pPtr;
	}

	T* GetPtr()
	{
		return ( T* )m_pPtr;
	}

	const T* GetPtr() const
	{
		return ( T* ) m_pPtr;
	}
};

class WeakPtrBase
{
public:
	friend class Intrusive;

			WeakPtrBase();
			WeakPtrBase( Intrusive* pPtr );

			WeakPtrBase( const WeakPtrBase& xPtr );
			WeakPtrBase& operator=( const WeakPtrBase& xPtr );

			WeakPtrBase( WeakPtrBase&& xPtr ) = delete;
			WeakPtrBase& operator=( WeakPtrBase&& xPtr ) = delete;

			~WeakPtrBase();

protected:
	void	AddLink();
	void	RemoveLink();

	Intrusive*		m_pPtr;
	WeakPtrBase*	m_pNext;
};

template < typename T >
class WeakPtr : public WeakPtrBase
{
public:
	WeakPtr()
		: WeakPtrBase( nullptr )
	{
	}

	WeakPtr( T* pPtr )
		: WeakPtrBase( pPtr )
	{
		static_assert( std::is_base_of_v< Intrusive, T >, "Trying to make a WeakPtr to a non-intrusive object." );
	}

	T* operator->()
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return ( T* ) m_pPtr;
	}

	const T* operator->() const
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return ( T* ) m_pPtr;
	}

	T& operator*()
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return *( ( T* ) m_pPtr );
	}

	const T& operator*() const
	{
		ASSERT( m_pPtr != nullptr && m_pPtr->m_uReferenceCount > 0 );
		return *( ( T* ) m_pPtr );
	}

	bool operator!() const
	{
		return !m_pPtr;
	}

	bool operator==( T* pPtr ) const
	{
		return m_pPtr == pPtr;
	}

	bool operator!=( T* pPtr ) const
	{
		return m_pPtr != pPtr;
	}

	T* GetPtr()
	{
		return ( T* ) m_pPtr;
	}

	const T* GetPtr() const
	{
		return ( T* ) m_pPtr;
	}
};