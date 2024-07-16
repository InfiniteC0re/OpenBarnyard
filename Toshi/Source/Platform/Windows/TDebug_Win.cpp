#include "ToshiPCH.h"
#include "Toshi/TDebug.h"
#include "Thread/T2Mutex.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TCHAR TDebug_ScratchMem[4096];
BOOL TDebug_IsMutexCreated;
T2Mutex TDebug_Mutex;

void TDebug_AcquireMutex()
{
	if (TDebug_IsMutexCreated == FALSE)
	{
		TDebug_Mutex.Create();
		TDebug_IsMutexCreated = TRUE;
	}
	
	TDebug_Mutex.Lock();
}

void TDebug_ReleaseMutex()
{
	TDebug_Mutex.Unlock();
}

TBOOL TDebug_IsValidAddress(const void* a_pPtr)
{
	if (a_pPtr != TNULL &&
		a_pPtr != (void*)0xcdcdcdcd &&
		a_pPtr != (void*)0xdeaddead &&
		a_pPtr != (void*)0xbeefbeef &&
		a_pPtr >= (void*)0x4f)
	{
		return TTRUE;
	}

	return TFALSE;
}

void TDebug_FinalPrintf(const TCHAR* a_szFormat, ...)
{
	va_list args;
	va_start(args, a_szFormat);
	TDebug_FinalVPrintf(a_szFormat, args);
	va_end(args);
}

void TDebug_FinalVPrintf(const TCHAR* a_szFormat, va_list a_Args)
{
	TDebug_FinalVSPrintf(a_szFormat, a_Args);
	TDebug_OutputDebugString(TDebug_ScratchMem);
}

void TDebug_FinalSPrintf(const TCHAR* a_szFormat, ...)
{
	va_list args;
	va_start(args, a_szFormat);
	TDebug_FinalVSPrintf(a_szFormat, args);
	va_end(args);
}

void TDebug_FinalVSPrintf(const TCHAR* a_szFormat, va_list a_Args)
{
	TDebug_AcquireMutex();
	vsprintf(TDebug_ScratchMem, a_szFormat, a_Args);
	TDebug_ReleaseMutex();
}

void TDebug_OutputDebugString(const TCHAR* a_szString)
{
	OutputDebugStringA(a_szString);
	printf("%s", a_szString);

	TDebug_BroadcastDebugString(a_szString);
}

void TDebug_BroadcastDebugString(const TCHAR* a_szString)
{
	//TIMPLEMENT();
}

#include "../resource.h"

const TCHAR* TDebug_szAssertFilename;
const TCHAR* TDebug_szAssertExpression;
TUINT TDebug_uiAssertLineNumber;

BOOL CALLBACK TDebug_AssertionDlgProc(HWND hwnd, UINT Message, WPARAM wParam, LPARAM lParam)
{
	switch (Message)
	{
	case WM_INITDIALOG:
	{
		ShowCursor(TRUE);
		SetActiveWindow(hwnd);
		SetForegroundWindow(hwnd);
		SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);

		if (!TDebug_szAssertFilename)
			TDebug_szAssertFilename = "***INVALID***";

		if (!TDebug_szAssertExpression)
			TDebug_szAssertExpression = "***INVALID***";

		SetDlgItemTextA(hwnd, 8204, TDebug_szAssertFilename);
		SetDlgItemInt(hwnd, 8205, TDebug_uiAssertLineNumber, TRUE);
		SetDlgItemTextA(hwnd, 8206, TDebug_szAssertExpression);
		return TRUE;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case 8200: // Quit
			ExitProcess(1);
			break;
		case 8201: // Debug
			EndDialog(hwnd, 1);
			break;
		case 8202: // Ignore
			ShowCursor(FALSE);
			EndDialog(hwnd, 0);
			break;
		}
		break;
	default:
		return FALSE;
	}

	return TRUE;
}

TINT TDebug_AssertHandler(const TCHAR* a_szExpression, const TCHAR* a_szFileName, TUINT a_uiLineNumber, const TCHAR* a_szDescription)
{
	TDebug_szAssertFilename = a_szFileName;
	TDebug_szAssertExpression = a_szExpression;
	TDebug_uiAssertLineNumber = a_uiLineNumber;

	HWND hWnd = GetActiveWindow();

	if (hWnd != NULL)
		hWnd = GetLastActivePopup(hWnd);

	INT_PTR result = DialogBoxParamA(
		GetModuleHandleA(NULL),
		MAKEINTRESOURCEA(IDD_ASSERT),
		hWnd,
		TDebug_AssertionDlgProc,
		NULL
	);

	return (result != -1) ? result : 1;
}
