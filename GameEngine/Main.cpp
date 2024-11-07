#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "Array.h"

class ProfilerBlock
{
public:
	ProfilerBlock( const char* sName )
		: m_sName( sName )
		, m_oStart( std::chrono::high_resolution_clock::now() )
	{
	}

	~ProfilerBlock()
	{
		std::cout<<m_sName << " : " << std::chrono::duration_cast<std::chrono::milliseconds>( std::chrono::high_resolution_clock::now() - m_oStart ).count() << " ms" << std::endl;
	}

private:
	std::string										m_sName;
	std::chrono::high_resolution_clock::time_point	m_oStart;
};

struct MyStruct
{
	MyStruct( const int iInt, const float fFloat, const bool bBool )
		: m_iInt( iInt )
		, m_fFloat( fFloat )
		, m_bBool( bBool )
	{
	}

	int		m_iInt;
	float	m_fFloat;
	bool	m_bBool;
};

int main()
{
	{
		ProfilerBlock oBlk( "Vector pushback primitive 10K" );
		std::vector<int> aTest;
		for( int i = 0; i < 10000; ++i )
			aTest.push_back( i );
	}

	{
		ProfilerBlock oBlk( "Array pushback primitive 10K" );
		Array<int> aTest;
		for( int i = 0; i < 10000; ++i )
			aTest.PushBack( i );
	}

	{
		ProfilerBlock oBlk( "Vector reserve + pushback primitive 1M" );
		std::vector<int> aTest;
		aTest.reserve( 1000000 );
		for( int i = 0; i < 1000000; ++i )
			aTest.push_back( i );
	}

	{
		ProfilerBlock oBlk( "Array reserve + pushback primitive 1M" );
		Array<int> aTest;
		aTest.Reserve( 1000000 );
		for( int i = 0; i < 1000000; ++i )
			aTest.PushBack( i );
	}

	{
		ProfilerBlock oBlk( "Vector reserve + pushback struct 1M" );
		std::vector<MyStruct> aTest;
		aTest.reserve( 1000000 );
		for( int i = 0; i < 1000000; ++i )
			aTest.push_back( MyStruct( i, i * 1.f, i % 2 == 0 ) );
	}

	{
		ProfilerBlock oBlk( "Array reserve + pushback struct 1M" );
		Array<MyStruct> aTest;
		aTest.Reserve( 1000000 );
		for( int i = 0; i < 1000000; ++i )
			aTest.PushBack( MyStruct( i, i * 1.f, i % 2 == 0 ) );
	}



	{
		ProfilerBlock oBlk( "Array reserve + pushfront primitive 10K" );
		Array<int> aTest;
		aTest.Reserve( 10000 );
		for( int i = 0; i < 10000; ++i )
			aTest.PushFront( i );
	}

	{
		ProfilerBlock oBlk( "Array reserve + pushfront struct 10K" );
		Array<MyStruct> aTest;
		aTest.Reserve( 10000 );
		for( int i = 0; i < 10000; ++i )
			aTest.PushFront( MyStruct( i, i * 1.f, i % 2 == 0 ) );
	}

	{
		ProfilerBlock oBlk( "Array reserve + pushfront primitive fast resize 10K" );
		Array<int, ArrayFlags::FAST_RESIZE> aTest;
		aTest.Reserve( 10000 );
		for( int i = 0; i < 10000; ++i )
			aTest.PushFront( i );
	}

	{
		ProfilerBlock oBlk( "Array reserve + pushfront struct fast resize 10K" );
		Array<MyStruct, ArrayFlags::FAST_RESIZE> aTest;
		aTest.Reserve( 10000 );
		for( int i = 0; i < 10000; ++i )
			aTest.PushFront( MyStruct( i, i * 1.f, i % 2 == 0 ) );
	}
}