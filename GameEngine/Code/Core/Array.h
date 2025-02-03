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
	FAST_RESIZE = 1
};

template < typename T, int Flags = ArrayFlags::STANDARD >
class Array
{
public:
	template < typename U, int F >
	friend class Array;

	Array()
		: m_pData( nullptr )
		, m_uCount( 0 )
		, m_uCapacity( 0 )
#ifdef TRACK_MEMORY
		, m_bMemoryTracked( false )
		, m_bTrackMemory( true )
#endif
	{
		TrackMemory();
	}

	explicit Array( const uint uCount )
		: m_pData( ( T* )malloc( uCount * sizeof( T ) ) )
		, m_uCount( uCount )
		, m_uCapacity( uCount )
#ifdef TRACK_MEMORY
		, m_bMemoryTracked( false )
		, m_bTrackMemory( true )
#endif
	{
		for( uint u = 0; u < m_uCount; ++u )
			::new( &m_pData[ u ] ) T;

		TrackMemory();
	}

	Array( const uint uCount, const T& oValue )
		: m_pData( ( T* )malloc( uCount * sizeof( T ) ) )
		, m_uCount( uCount )
		, m_uCapacity( uCount )
#ifdef TRACK_MEMORY
		, m_bMemoryTracked( false )
		, m_bTrackMemory( true )
#endif
	{
		for( uint u = 0; u < m_uCount; ++u )
			::new( &m_pData[ u ] ) T( oValue );

		TrackMemory();
	}

	Array( const Array& aArray )
		: m_pData( ( T* )malloc( aArray.m_uCount * sizeof( T ) ) )
		, m_uCount( aArray.m_uCount )
		, m_uCapacity( aArray.m_uCapacity )
#ifdef TRACK_MEMORY
		, m_bMemoryTracked( false )
		, m_bTrackMemory( true )
#endif
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

		UnTrackMemory();

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

		TrackMemory();

		return *this;
	}

	Array( Array&& aArray ) noexcept
		: m_pData( aArray.m_pData )
		, m_uCount( aArray.m_uCount )
		, m_uCapacity( aArray.m_uCapacity )
	{
		aArray.UnTrackMemory();

		aArray.m_pData = nullptr;
		aArray.m_uCount = 0;
		aArray.m_uCapacity = 0;

		TrackMemory();
		aArray.UnTrackMemory();
	}

	Array& operator=( Array&& aArray ) noexcept
	{
		if( &aArray == this )
			return *this;

		aArray.UnTrackMemory();
		UnTrackMemory();

		Destroy();

		m_pData = aArray.m_pData;
		m_uCount = aArray.m_uCount;
		m_uCapacity = aArray.m_uCapacity;

		ASSERT( m_uCount <= m_uCapacity );

		aArray.m_pData = nullptr;
		aArray.m_uCount = 0;
		aArray.m_uCapacity = 0;

		TrackMemory();
		aArray.TrackMemory();

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
		UnTrackMemory();
		EnableMemoryTracking( false );

		Expand();

		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount < m_uCapacity );

		::new( &m_pData[ m_uCount++ ] ) T;

		EnableMemoryTracking( true );
		TrackMemory();
	}

	void PushBack( const T& oElement )
	{
		UnTrackMemory();
		EnableMemoryTracking( false );

		Expand();

		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount < m_uCapacity );

		::new( &m_pData[ m_uCount++ ] ) T( oElement );

		EnableMemoryTracking( true );
		TrackMemory();
	}

	void PushFront( const T& oElement )
	{
		UnTrackMemory();
		EnableMemoryTracking( false );

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

		EnableMemoryTracking( true );
		*this = aPush;
	}

	void PopBack()
	{
		ASSERT( m_pData != nullptr );
		ASSERT( m_uCount > 0 );

		UnTrackMemory();

		m_pData[ --m_uCount ].~T();

		TrackMemory();
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

		UnTrackMemory();

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

		TrackMemory();
	}

	void Clear()
	{
		UnTrackMemory();

		for( uint u = 0; u < m_uCount; ++u )
			m_pData[ u ].~T();

		m_uCount = 0;

		TrackMemory();
	}

	void Swap( Array& aArray )
	{
		aArray.UnTrackMemory();
		UnTrackMemory();

		T* pData = m_pData;
		uint uCount = m_uCount;
		uint uCapacity = m_uCapacity;

		m_pData = aArray.m_pData;
		m_uCount = aArray.m_uCount;
		m_uCapacity = aArray.m_uCapacity;

		aArray.m_pData = pData;
		aArray.m_uCount = uCount;
		aArray.m_uCapacity = uCapacity;

		TrackMemory();
		aArray.TrackMemory();
	}

	void Grab( Array& aArray )
	{
		Swap( aArray );
		aArray.Clear();
	}

	void Resize( const uint uCount )
	{
		UnTrackMemory();
		EnableMemoryTracking( false );

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

		EnableMemoryTracking( true );
		TrackMemory();
	}

	void Resize( const uint uCount, const T& oValue )
	{
		UnTrackMemory();
		EnableMemoryTracking( false );

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

		EnableMemoryTracking( true );
		TrackMemory();
	}

	void Reserve( const uint uCount )
	{
		UnTrackMemory();

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

		TrackMemory();
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

		UnTrackMemory();

		m_pData = ( T* )realloc( m_pData, m_uCount * sizeof( T ) );

		m_uCapacity = m_uCount;

		TrackMemory();
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

	uint Capacity() const
	{
		return m_uCapacity;
	}

	bool Empty() const
	{
		return m_uCount == 0;
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
		if( g_pMemoryTracker != nullptr && m_bMemoryTracked == false && m_bTrackMemory )
		{
			g_pMemoryTracker->RegisterArray( typeid( T ), sizeof( T ) * Count(), sizeof( T ) * Capacity(), Count() );
			m_bMemoryTracked = true;
		}
#endif
	}

	void UnTrackMemory()
	{
#ifdef TRACK_MEMORY
		if( g_pMemoryTracker != nullptr && m_bMemoryTracked && m_bTrackMemory )
		{
			g_pMemoryTracker->UnRegisterArray( typeid( T ), sizeof( T ) * Count(), sizeof( T ) * Capacity(), Count() );
			m_bMemoryTracked = false;
		}
#endif
	}

	void EnableMemoryTracking( const bool bEnable)
	{
#ifdef TRACK_MEMORY
		m_bTrackMemory = bEnable;
#endif
	}

	T*		m_pData;
	uint	m_uCount;
	uint	m_uCapacity;

#ifdef TRACK_MEMORY
	bool	m_bMemoryTracked;
	bool	m_bTrackMemory;
#endif
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
