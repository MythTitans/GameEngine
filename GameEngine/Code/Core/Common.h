#pragma once

#include <windows.h>

#include <filesystem>
#include <format>
#include <string>

#include "Types.h"

inline HHOOK g_hAssertHook;

LRESULT CALLBACK AssertHook( INT iCode, WPARAM wParam, LPARAM lParam );

#ifdef _DEBUG
#define ASSERT( bExpression )																																			\
if( ( bExpression ) == false )																																			\
{																																										\
	static bool s_bAlwaysIgnore = false;																																\
	if( s_bAlwaysIgnore == false )																																		\
	{																																									\
		const std::wstring sFilePath = std::wstring( __FILEW__ );																										\
		const std::wstring sWorkingDir = std::filesystem::current_path().wstring();																						\
		const std::wstring sFile = sFilePath.starts_with( sWorkingDir ) ? sFilePath.substr( sWorkingDir.length() + 1 ) : sFilePath;										\
		const std::wstring sContent = std::format( L"Expression\n\n    {}\n\nfailed when executing\n\n{}\n{} ({})", L""#bExpression, __FUNCTIONW__, sFile, __LINE__ );	\
		g_hAssertHook = SetWindowsHookEx( WH_CBT, &AssertHook, nullptr, GetCurrentThreadId() );																			\
		switch( MessageBox( nullptr, sContent.c_str(), L"Assertion failed", MB_ICONERROR | MB_CANCELTRYCONTINUE | MB_DEFBUTTON3 ) )										\
		{																																								\
		case IDCANCEL:																																					\
			break;																																						\
		case IDTRYAGAIN:																																				\
			s_bAlwaysIgnore = true;																																		\
			break;																																						\
		case IDCONTINUE:																																				\
			__debugbreak();																																				\
			break;																																						\
		}																																								\
		UnhookWindowsHookEx( g_hAssertHook );																															\
	}																																									\
}
#else
#define ASSERT( bExpression ) ( void )( bExpression );
#endif
