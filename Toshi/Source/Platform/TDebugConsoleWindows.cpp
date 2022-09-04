#include "ToshiPCH.h"
#include "TDebugConsoleWindows.h"
#include "Toshi/Strings/TCString.h"

namespace Toshi
{
	TDebugConsoleWindows::TDebugConsoleWindows()
	{
		m_HWND = (HWND*)tmalloc(sizeof(HWND));
		*m_HWND = CreateDebugConsole();
	}

	TDebugConsoleWindows::~TDebugConsoleWindows()
	{
		tfree(m_HWND);
	}

	HWND TDebugConsoleWindows::CreateDebugConsole()
	{
		TCString str = TCString(TDebugConsoleUUID);

		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);

		char timeHex[sizeof(time.QuadPart) + 1] = { 0 };
		_ultoa_s((unsigned long)time.QuadPart, timeHex, 0x10);

		TOSHI_INFO("Debug Start Time: {0}", timeHex);
		str += timeHex;

		char lpConsoleTitle[0x200];
		DWORD titleLength = GetConsoleTitleA(lpConsoleTitle, 0x200);
		if (titleLength == 0) { return TNULL; }

		SetConsoleTitleA(str);
		HWND consoleHWND = FindWindowA(TNULL, str);
		SetConsoleTitleA(lpConsoleTitle);

		return consoleHWND;

		/*  Stuff de blob does Reminder: puVar5 = buf
		* Start of code
		pcVar3 = DAT_009823b0;
	  do {
		cVar1 = *pcVar3;
		pcVar3 = pcVar3 + 1;
		pcVar3[(int)&uStack229 - (int)DAT_009823b0] = cVar1;
	  } while (cVar1 != '\0');
	  puVar5 = local_1c;
	  do {
		cVar1 = *(char *)puVar5;
		puVar5 = (undefined4 *)((int)puVar5 + 1);
	  } while (cVar1 != '\0');
	  uVar6 = (int)puVar5 - (int)local_1c;
	  puVar5 = &uStack229;
	  do {
		pcVar3 = (char *)((int)puVar5 + 1);
		puVar5 = (undefined4 *)((int)puVar5 + 1);
	  } while (*pcVar3 != '\0');
	  puVar7 = local_1c;
	  for (uVar4 = uVar6 >> 2; uVar4 != 0; uVar4 = uVar4 - 1) {
		*puVar5 = *puVar7;
		puVar7 = puVar7 + 1;
		puVar5 = puVar5 + 1;
	  }
	  for (uVar6 = uVar6 & 3; uVar6 != 0; uVar6 = uVar6 - 1) {
		*(undefined *)puVar5 = *(undefined *)puVar7;
		puVar7 = (undefined4 *)((int)puVar7 + 1);
		puVar5 = (undefined4 *)((int)puVar5 + 1);
	  }

	  */
	}

	bool TDebugConsoleWindows::Show(bool state)
	{
		bool oldState = m_Visible;
		m_Visible = state;

		ShowWindow(*m_HWND, state);
		UpdateWindow(*m_HWND);

		return oldState;
	}

	bool TDebugConsoleWindows::Toggle()
	{
		Show(!m_Visible);
		return m_Visible;
	}
}
