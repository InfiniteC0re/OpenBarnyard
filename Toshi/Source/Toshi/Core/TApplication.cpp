#include "pch.h"
#include "TApplication.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Core/TMemory.h"
#include "Toshi/Strings/TCString.h"

#include <timeapi.h>

namespace Toshi
{
	TApplication::TApplication()
	{
		m_Unk1 = 0;
		m_Flags = 0;

		m_DebugConsole = (HWND*)tmalloc(sizeof(HWND));
		*m_DebugConsole = CreateDebugConsole();

#ifdef TOSHI_DEBUG
		ShowConsole(true);
#else
		ShowConsole(false);
#endif
	}

	TApplication::~TApplication()
	{
		Destroy();
	}

	bool TApplication::Create()
	{
		// todo..
		// TApplication.dll: 0x10001260

		// temporary solution
		OnCreate();
		OnUpdate();

		return true;
	}

	void TApplication::Destroy()
	{
		m_Flags |= TApplicationFlag_Destroyed;
		OnDestroy();
	}

	bool TApplication::OnCreate()
	{
		m_Flags |= TApplicationFlag_Created;
		return true;
	}

	bool TApplication::OnUpdate()
	{
		return (m_Flags & TApplicationFlag_Destroyed) == 0;
	}

	bool TApplication::OnDestroy()
	{
		return true;
	}

	bool TApplication::ShowConsole(bool state)
	{
		bool oldState = m_ConsoleVisible;
		m_ConsoleVisible = state;

		ShowWindow(*m_DebugConsole, state);
		UpdateWindow(*m_DebugConsole);

		return oldState;
	}

	bool TApplication::ToggleConsole()
	{
		ShowConsole(!m_ConsoleVisible);
		return m_ConsoleVisible;
	}

	HWND TApplication::CreateDebugConsole()
	{
#ifdef TOSHI_PLATFORM_WINDOWS
		TCString str = TCString(TDebugConsoleUUID);

		// timeGetTime is not precise and requires linking external library
		// so let's use performance counter for Windows and some other function
		// for any other OS
		// 
		// probably there's a reason to make different version of TApplication
		// for every platform in the future

		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);

		char timeHex[sizeof(time.QuadPart) + 1] = { 0 };
		_ultoa_s((unsigned long)time.QuadPart, timeHex, 0x10);

		TOSHI_INFO("Debug Start Time: {0}", timeHex);

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

		str += timeHex;
		char lpConsoleTitle[0x200];
		DWORD unk3 = GetConsoleTitleA(lpConsoleTitle, 0x200);

		if (unk3 == 0)
		{
			return TNULL;
		}

		SetConsoleTitleA(str);
		HWND consoleHWND = FindWindowA(TNULL, str);
		SetConsoleTitleA(lpConsoleTitle);

		return consoleHWND;
#else
		return TNULL;
#endif
	}

}