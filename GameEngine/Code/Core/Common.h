#pragma once

#ifndef WIDEN
#define WIDEN2( x ) L##x
#define WIDEN( x ) WIDEN2( x )
#endif

#include "Core/Types.h"

int AssertHook( int iCode, uint* wParam, uint* lParam );

bool DisplayAssert( const wchar_t* sExpression, const wchar_t* sFile, const wchar_t* sFunction, int iLine );

#ifdef _DEBUG
#define ASSERT( bExpression )																							\
if( ( bExpression ) == false )																							\
{																														\
	static bool s_bAlwaysIgnore = false;																				\
	if( s_bAlwaysIgnore == false )																						\
		s_bAlwaysIgnore = DisplayAssert( WIDEN( #bExpression ), WIDEN( __FILE__ ), WIDEN( __FUNCTION__ ), __LINE__ );	\
}
#else
#define ASSERT( bExpression ) ( void )( bExpression );
#endif
