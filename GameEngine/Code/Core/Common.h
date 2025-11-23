#pragma once

#define NOMINMAX
#include <Windows.h>

inline HHOOK g_hAssertHook;

LRESULT CALLBACK AssertHook( INT iCode, WPARAM wParam, LPARAM lParam );

bool DisplayAssert( const wchar_t* sExpression, const wchar_t* sFile, const wchar_t* sFunction, int iLine );

#ifdef _DEBUG
#define ASSERT( bExpression )																	\
if( ( bExpression ) == false )																	\
{																								\
	static bool s_bAlwaysIgnore = false;														\
	if( s_bAlwaysIgnore == false )																\
		s_bAlwaysIgnore = DisplayAssert( L""#bExpression, __FILEW__, __FUNCTIONW__, __LINE__ );	\
}
#else
#define ASSERT( bExpression ) ( void )( bExpression );
#endif
