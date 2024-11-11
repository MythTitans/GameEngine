#include <chrono>
#include <iostream>
#include <string>
#include <vector>

#include "Array.h"
#include "Intrusive.h"
#include "Logger.h"

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

struct MyStruct : Intrusive
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
	LOG_INFO( "Starting application" );
	LOG_INFO( "Loading resources" );
	LOG_WARN( "Failed to find resource {}", "config.cfg" );
	LOG_ERROR( "Could not start application, shutting down..." );

// 	MyStruct* pPtr = new MyStruct( 1, 2.f, true );
// 	StrongPtr< MyStruct > xStrongPtr = pPtr;
// 	WeakPtr< MyStruct > xWeakPtr = pPtr;
// 
// 	Array< MyStruct* > aArray( 1, pPtr );
// 
// 	aArray[ 58 ]->m_fFloat = 3.f;
// 
// 	pPtr->m_iInt;
}