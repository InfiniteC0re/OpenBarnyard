#include "ToshiPCH.h"
#include "TDebugConsole_Win.h"
#include "Toshi/TString8.h"

#include <conio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

namespace Toshi
{
	TDebugConsole::TDebugConsole()
	{
		m_HWND = FindConsole();
		m_Created = m_HWND != TNULL;
	}

	TDebugConsole::~TDebugConsole()
	{
		if (IsCreated())
		{
			if (IsVisible())
			{
				TWARN("Press any key to exit...\n");
				_getch();
			}
		}
	}

	TBOOL TDebugConsole::Show(TBOOL state)
	{
		TBOOL oldState = m_Visible;
		m_Visible = state;

		ShowWindow(m_HWND, state);
		UpdateWindow(m_HWND);

		return oldState;
	}

	TBOOL TDebugConsole::Toggle()
	{
		Show(!m_Visible);
		return m_Visible;
	}

	HWND TDebugConsole::FindConsole() const
	{
		TString8 str = TString8(TDebugConsoleUUID);

		// Generate title for the console so we can find it's HWND
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		TCHAR timeHex[sizeof(time.QuadPart) + 1] = { 0 };
		_ultoa_s((unsigned long)time.QuadPart, timeHex, 0x10);
		str += timeHex;

		// Save the current title
		TCHAR lpConsoleTitle[256];
		DWORD titleLength = GetConsoleTitleA(lpConsoleTitle, 256);
		if (titleLength == 0) { return TNULL; }

		// Set the genereated title and wait for 50ms so it's applied
		SetConsoleTitleA(str);
		Sleep(50);

		// Find a window with the generated title and restore it
		HWND consoleHWND = FindWindowA(TNULL, str);
		SetConsoleTitleA(lpConsoleTitle);

		return consoleHWND;

		/*  Stuff de blob does Reminder: puVar5 = buf
		* Start of code
		pcVar3 = DAT_009823b0;
	  do {
		cVar1 = *pcVar3;
		pcVar3 = pcVar3 + 1;
		pcVar3[(TINT)&uStack229 - (TINT)DAT_009823b0] = cVar1;
	  } while (cVar1 != '\0');
	  puVar5 = local_1c;
	  do {
		cVar1 = *(TCHAR *)puVar5;
		puVar5 = (undefined4 *)((TINT)puVar5 + 1);
	  } while (cVar1 != '\0');
	  uVar6 = (TINT)puVar5 - (TINT)local_1c;
	  puVar5 = &uStack229;
	  do {
		pcVar3 = (TCHAR *)((TINT)puVar5 + 1);
		puVar5 = (undefined4 *)((TINT)puVar5 + 1);
	  } while (*pcVar3 != '\0');
	  puVar7 = local_1c;
	  for (uVar4 = uVar6 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
		*puVar5 = *puVar7;
		puVar7 = puVar7 + 1;
		puVar5 = puVar5 + 1;
	  }
	  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
		*(undefined *)puVar5 = *(undefined *)puVar7;
		puVar7 = (undefined4 *)((TINT)puVar7 + 1);
		puVar5 = (undefined4 *)((TINT)puVar5 + 1);
	  }

	  */
	}
}
