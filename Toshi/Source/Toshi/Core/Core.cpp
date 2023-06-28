#include "ToshiPCH.h"
#include "Core.h"
#include "Toshi/Strings/TString8.h"
#include "Platform/Windows/DX11/TRender_DX11.h"
#include "../resource.h"

using namespace Toshi;

#ifdef TOSHI_ENABLE_ASSERTS

#ifdef TOSHI_PLATFORM_WINDOWS
struct AssertionData
{
	const char* FileName;
	size_t LineNumber;
	const char* Expression;
};

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

		AssertionData* pData = TREINTERPRETCAST(AssertionData*, lParam);
		SetWindowTextA(GetDlgItem(hwnd, 8204), pData->FileName);
		SetWindowTextA(GetDlgItem(hwnd, 8205), TString8::Format("%u", pData->LineNumber));
		SetWindowTextA(GetDlgItem(hwnd, 8206), pData->Expression);
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
	AssertionData data;
	data.FileName = file;
	data.LineNumber = line;
	data.Expression = expression;
	
	return
		TSTATICCAST(
			AssertionAction,
			DialogBoxParamA(
				GetModuleHandleA(NULL),
				MAKEINTRESOURCEA(IDD_ASSERT),
				NULL,
				AssertionDlgProc,
				TREINTERPRETCAST(LPARAM, &data)
			)
		);
}
#else // TOSHI_PLATFORM_WINDOWS
AssertionAction AssertionCallback(const char* file, int line, const char* expression)
{
	return AssertionAction::Break;
}
#endif // TOSHI_PLATFORM_WINDOWS

#endif // TOSHI_ENABLE_ASSERTS
