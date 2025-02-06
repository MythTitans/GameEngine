#include "Common.h"

LRESULT CALLBACK AssertHook( INT iCode, WPARAM wParam, LPARAM lParam )
{
	SetDlgItemTextA( ( HWND )wParam, IDCANCEL, "Ignore" );
	SetDlgItemTextA( ( HWND )wParam, IDTRYAGAIN, "Always" );
	SetDlgItemTextA( ( HWND )wParam, IDCONTINUE, "Break" );

	return CallNextHookEx( g_hAssertHook, iCode, wParam, lParam );
}
