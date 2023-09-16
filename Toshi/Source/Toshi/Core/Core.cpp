#include "ToshiPCH.h"
#include "Core.h"
#include "Toshi/Strings/TString8.h"
#include "../resource.h"

using namespace Toshi;

#ifdef TOSHI_ENABLE_ASSERTS

#ifdef TOSHI_SKU_WINDOWS
const char* g_szAssertFilename;
size_t g_uiAssertLineNumber;
const char* g_szAssertExpression;

BOOL CALLBACK AssertionDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		ShowCursor(TRUE);
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		if (!g_szAssertFilename)
			g_szAssertFilename = "***INVALID***";

		if (!g_szAssertExpression)
			g_szAssertExpression = "***INVALID***";

		SetDlgItemTextA(hwnd, 8204, g_szAssertFilename);
		SetDlgItemInt(hwnd, 8205, g_uiAssertLineNumber, TRUE);
		SetDlgItemTextA(hwnd, 8206, g_szAssertExpression);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 8200: // Quit
			ExitProcess(1);
			break;
		case 8201: // Debug
			EndDialog(hwnd, AssertionAction::Break);
			break;
		case 8202: // Ignore
			ShowCursor(FALSE);
			EndDialog(hwnd, AssertionAction::Ignore);
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

AssertionAction AssertionCallback(const char* file, int line, const char* expression)
{
	g_szAssertFilename = file;
	g_uiAssertLineNumber = line;
	g_szAssertExpression = expression;

	HWND hWnd = GetActiveWindow();
	
	if (hWnd != NULL)
		hWnd = GetLastActivePopup(hWnd);

	INT_PTR result = DialogBoxParamA(
		GetModuleHandleA(NULL),
		MAKEINTRESOURCEA(IDD_ASSERT),
		hWnd,
		AssertionDlgProc,
		NULL
	);
	
	return result != -1 ? TSTATICCAST(AssertionAction, result) : AssertionAction::Break;
}
#else // TOSHI_SKU_WINDOWS
AssertionAction AssertionCallback(const char* file, int line, const char* expression)
{
	return AssertionAction::Break;
}
#endif // TOSHI_SKU_WINDOWS

#endif // TOSHI_ENABLE_ASSERTS
