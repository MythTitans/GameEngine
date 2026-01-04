#include "Common.h"

#include <filesystem>
#include <format>
#include <xstring>

LRESULT CALLBACK AssertHook( INT iCode, WPARAM wParam, LPARAM lParam )
{
	SetDlgItemTextA( ( HWND )wParam, IDCANCEL, "Ignore" );
	SetDlgItemTextA( ( HWND )wParam, IDTRYAGAIN, "Always" );
	SetDlgItemTextA( ( HWND )wParam, IDCONTINUE, "Break" );

	return CallNextHookEx( g_hAssertHook, iCode, wParam, lParam );
}

bool DisplayAssert( const wchar_t* sExpression, const wchar_t* sFile, const wchar_t* sFunction, int iLine )
{
	bool bAlwaysIgnore = false;

	const std::wstring sFilePath = std::wstring( sFile );
	const std::wstring sWorkingDir = std::filesystem::current_path().wstring();
	const std::wstring sFinalFile = sFilePath.starts_with( sWorkingDir ) ? sFilePath.substr( sWorkingDir.length() + 1 ) : sFilePath;
	const std::wstring sContent = std::format( L"Expression\n\n    {}\n\nfailed when executing\n\n{}\n{} ({})", sExpression, sFunction, sFinalFile, iLine );
	g_hAssertHook = SetWindowsHookEx( WH_CBT, &AssertHook, nullptr, GetCurrentThreadId() );
	switch( MessageBox( nullptr, sContent.c_str(), L"Assertion failed", MB_ICONERROR | MB_CANCELTRYCONTINUE | MB_DEFBUTTON3 ) )
	{
	case IDCANCEL:
		break;
	case IDTRYAGAIN:
		bAlwaysIgnore = true;
		break;
	case IDCONTINUE:
		__debugbreak();
		break;
	}

	UnhookWindowsHookEx( g_hAssertHook );

	return bAlwaysIgnore;
}
