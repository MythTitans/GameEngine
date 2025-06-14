#pragma once

#include <cstring>
#include <new>

#include "Common.h"
#ifdef TRACK_MEMORY
#include "MemoryTracker.h"
#endif

enum ArrayFlags
{
	STANDARD = 0,
	FAST_RESIZE = 1,
	NO_TRACKING = 1 << 1
};

class ArrayBase
{
public:
	ArrayBase();
	ArrayBase( const uint uCount, const uint uCapacity );
	~ArrayBase();

	uint Count() const;
	uint Capacity() const;
	bool Empty() const;

protected:
	uint m_uCount;
	uint m_uCapacity;
};

template < typename T, int Flags = ArrayFlags::STANDARD >
class Array : public ArrayBase
{
public:
	template < typename U, int F >
	friend class Array;

	Array()
		: m_pData( nullptr )
	{
		TrackMemory();
	}

	explicit Array( const uint uCount )
		: ArrayBase( uCount, uCount )
		, m_pData( ( T* )malloc( uCount * sizeof( T ) ) )
	{
		for( uint u = 0; u < m_uCount; ++u )
			::new( &m_pData[ u ] ) T;

		TrackMemory();
	}

	Array( const uint uCount, const T& oValue )
		: ArrayBase( uCount, uCount )
		, m_pData( ( T* )malloc( uCount * sizeof( T ) ) )
	{
		for( uint u = 0; u < m_uCount; ++u )
			::new( &m_pData[ u ] ) T( oValue );

		TrackMemory();
	}

	Array( const Array& aArray )
		: ArrayBase( aArray.m_uCount, aArray.m_uCapacity )
		, m_pData( ( T* )malloc( aArray.m_uCount * sizeof( T ) ) )
	{
		if constexpr( ( Flags & ArrayFlags::FAST_RESIZE ) != 0 )
		{
			static_assert( std::is_trivially_copyable_v< T >, "Fast resize can only be set for trivially copyable elements." );
			memcpy( m_pData, aArray.m_pData, m_uCount * sizeof( T ) );
		}
		else
		{
			for( uint u = 0; u < m_uCount; ++u )
				::new( &m_pData[ u ] ) T( aArray[ u ] );
		}

		TrackMemory();
	}

	Array& operator=( const Array& aArray )
	{
		if( &aArray == this )
			return *this;

		Destroy();

		m_pData = ( T* )malloc( aArray.m_uCount * sizeof( T ) );
		m_uCount = aArray.m_uCount;
		m_uCapacity = aArray.m_uCapacity;

		ASSERT( m_uCount <= m_uCapacity );

		if constexpr( ( Flags & ArrayFlags::FAST_RESIZE ) != 0 )
		{
			static_assert( std::is_trivially_copyable_v< T >, "Fast resize can only be set for trivially copyable elements." );
			memcpy( m_pData, aArray.m_pData, m_uCount * sizeof( T ) );
		}
		else
		{
			for( uint u = 0; u < m_uCount; ++u )
				::new( &m_pData[ u ] ) T( aArray[ u ] );
		}

		return *this;
	}

	Array( Array&& aArray ) noexcept
		: ArrayBase( aArray.m_uCount, aArray.m_uCapacity )
		, m_pData( aArray.m_pData )
	{
		aArray.m_pData = nullptr;
		aArray.m_uCount = 0;
		aArray.m_uCapacity = 0;

		TrackMemory();
	}

	Array& operator=( Array&& aArray ) noexcept
	{
		if( &aArray == this )
			return *this;

		Destroy();

		m_pData = aArray.m_pData;
		m_uCount = aArray.m_uCount;
		m_uCapacity = aArray.m_uCapacity;

		ASSERT( m_uCount <= m_uCapacity );

		aArray.m_pData = nullptr;
		aArray.m_uCount = 0;
		aArray.m_uCapacity = 0;

		return *this;
	}

	~Array()
	{
		UnTrackMemory();

		Destroy();

		m_pData = nullptr;
		m_uCount = 0;
		m_uCapacity = 0;
	}

	void PushBack()
	{
		Expand();

		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount < m_uCapacity );

		::new( &m_pData[ m_uCount++ ] ) T;
	}

	void PushBack( const T& oElement )
	{
		Expand();

		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount < m_uCapacity );

		::new( &m_pData[ m_uCount++ ] ) T( oElement );
	}

	void PushFront( const T& oElement )
	{
		Array< T, Flags > aPush;
		if( m_uCapacity < m_uCount + 1 )
			aPush.Reserve( m_uCount + 1 );
		else
			aPush.Reserve( m_uCapacity );

		aPush.PushBack( oElement );

		if constexpr( ( Flags & ArrayFlags::FAST_RESIZE ) != 0 )
		{
			static_assert( std::is_trivially_copyable_v< T >, "Fast resize can only be set for trivially copyable elements." );
			memcpy( &aPush.m_pData[ 1 ], m_pData, m_uCount * sizeof( T ) );
			aPush.m_uCount = m_uCount + 1;
		}
		else
		{
			for( uint u = 0; u < m_uCount; ++u )
				aPush.PushBack( m_pData[ u ] );
		}

		*this = aPush;
	}

	void PopBack()
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		m_pData[ --m_uCount ].~T();
	}

	void PopFront()
	{
		Remove( 0 );
	}

	void Remove( const uint uIndex )
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );
		ASSERT( uIndex >= 0 && uIndex < m_uCount );

		m_pData[ uIndex ].~T();

		if constexpr( ( Flags & ArrayFlags::FAST_RESIZE ) != 0 )
		{
			static_assert( std::is_trivially_copyable_v< T >, "Fast resize can only be set for trivially copyable elements." );
			memmove( &m_pData[ uIndex ], &m_pData[ uIndex + 1 ], ( m_uCount - uIndex - 1 ) * sizeof( T ) );
		}
		else
		{
			for( uint u = uIndex + 1; u < m_uCount; ++u )
			{
				m_pData[ u - 1 ] = m_pData[ u ];
				m_pData[ u ].~T();
			}
		}

		--m_uCount;
	}

	void Clear()
	{
		for( uint u = 0; u < m_uCount; ++u )
			m_pData[ u ].~T();

		m_uCount = 0;
	}

	void Swap( Array& aArray )
	{
		T* pData = m_pData;
		uint uCount = m_uCount;
		uint uCapacity = m_uCapacity;

		m_pData = aArray.m_pData;
		m_uCount = aArray.m_uCount;
		m_uCapacity = aArray.m_uCapacity;

		aArray.m_pData = pData;
		aArray.m_uCount = uCount;
		aArray.m_uCapacity = uCapacity;
	}

	void Grab( Array& aArray )
	{
		Swap( aArray );
		aArray.Clear();
	}

	void Resize( const uint uCount )
	{
		if( m_uCount < uCount )
		{
			const uint uExpansion = uCount - m_uCount;
			Reserve( uCount );

			for( uint u = 0; u < uExpansion; ++u )
				PushBack();
		}
		else if( m_uCount > uCount )
		{
			const uint uShrink = m_uCount - uCount;

			for( uint u = 0; u < uShrink; ++u )
				PopBack();
		}
	}

	void Resize( const uint uCount, const T& oValue )
	{
		if( m_uCount < uCount )
		{
			const uint uExpansion = uCount - m_uCount;
			Reserve( uCount );

			for( uint u = 0; u < uExpansion; ++u )
				PushBack( oValue );
		}
		else if( m_uCount > uCount )
		{
			const uint uShrink = m_uCount - uCount;

			for( uint u = 0; u < uShrink; ++u )
				PopBack();
		}
	}

	void Reserve( const uint uCount )
	{
		if( m_uCapacity < uCount )
		{
			T* pData = ( T* )malloc( uCount * sizeof( T ) );

			if constexpr( ( Flags & ArrayFlags::FAST_RESIZE ) != 0 )
			{
				static_assert( std::is_trivially_copyable_v< T >, "Fast resize can only be set for trivially copyable elements." );
				memcpy( pData, m_pData, m_uCount * sizeof( T ) );
			}
			else
			{
				for( uint u = 0; u < m_uCount; ++u )
					::new ( &pData[ u ] ) T( m_pData[ u ] );
			}

			Destroy();

			m_pData = pData;
			m_uCapacity = uCount;

			ASSERT( m_uCount <= m_uCapacity );
		}
	}

	void Expand( uint uBy = 1 )
	{
		if( m_uCapacity >= m_uCount + uBy )
			return;

		Reserve( m_uCount + uBy );
	}

	void ShrinkToFit()
	{
		if( m_uCapacity == m_uCount )
			return;

		m_pData = ( T* )realloc( m_pData, m_uCount * sizeof( T ) );

		m_uCapacity = m_uCount;
	}

	T& Back()
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ m_uCount - 1 ];
	}

	const T& Back() const
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ m_uCount - 1 ];
	}

	T& Front()
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ 0 ];
	}

	const T& Front() const
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ 0 ];
	}

	T& operator[]( const uint uIndex )
	{
		ASSERT( m_pData != nullptr );
		ASSERT( uIndex >= 0 && uIndex < m_uCount );

		return m_pData[ uIndex ];
	}

	const T& operator[]( const uint uIndex ) const
	{
		ASSERT( m_pData != nullptr );
		ASSERT( uIndex >= 0 && uIndex < m_uCount );

		return m_pData[ uIndex ];
	}

	T* Data()
	{
		return m_pData;
	}

	const T* Data() const
	{
		return m_pData;
	}

	T* begin()
	{
		return &m_pData[ 0 ];
	}

	const T* begin() const
	{
		return &m_pData[ 0 ];
	}

	T* end()
	{
		return &m_pData[ m_uCount ];
	}

	const T* end() const
	{
		return &m_pData[ m_uCount ];
	}

private:
	void Destroy()
	{
		for( uint u = 0; u < m_uCount; ++u )
			m_pData[ u ].~T();

		free( m_pData );
	}

	void TrackMemory()
	{
#ifdef TRACK_MEMORY
		if constexpr( ( Flags & ArrayFlags::NO_TRACKING ) == 0 )
		{
			if( g_pMemoryTracker != nullptr )
				g_pMemoryTracker->RegisterArray< T >( this );
		}
#endif
	}

	void UnTrackMemory()
	{
#ifdef TRACK_MEMORY
		if constexpr( ( Flags & ArrayFlags::NO_TRACKING ) == 0 )
		{
			if( g_pMemoryTracker != nullptr )
				g_pMemoryTracker->UnRegisterArray( this );
		}
#endif
	}

	T*		m_pData;
};

// TODO #eric should rename, this is not really an array view
template < typename T >
class ArrayView
{
public:
	ArrayView()
		: m_pData( nullptr )
		, m_uCount( 0 )
	{
	}

	ArrayView( Array< T >& aArray )
		: m_pData( aArray.Data() )
		, m_uCount( aArray.Count() )
	{
	}

	T& Back()
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ m_uCount - 1 ];
	}

	const T& Back() const
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ m_uCount - 1 ];
	}

	T& Front()
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ 0 ];
	}

	const T& Front() const
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		return m_pData[ 0 ];
	}

	T& operator[]( const uint uIndex )
	{
		ASSERT( m_pData != nullptr );
		ASSERT( uIndex >= 0 && uIndex < m_uCount );

		return m_pData[ uIndex ];
	}

	const T& operator[]( const uint uIndex ) const
	{
		ASSERT( m_pData != nullptr );
		ASSERT( uIndex >= 0 && uIndex < m_uCount );

		return m_pData[ uIndex ];
	}

	uint Count() const
	{
		return m_uCount;
	}

	bool Empty() const
	{
		return m_uCount == 0;
	}

// 	T* Data()
// 	{
// 		return m_pData;
// 	}

	const T* Data() const
	{
		return m_pData;
	}

	T* begin()
	{
		return &m_pData[ 0 ];
	}

	const T* begin() const
	{
		return &m_pData[ 0 ];
	}

	T* end()
	{
		return &m_pData[ m_uCount ];
	}

	const T* end() const
	{
		return &m_pData[ m_uCount ];
	}

private:
	T*		m_pData;
	uint	m_uCount;
};
