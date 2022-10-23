#include "ToshiPCH.h"
#include "TDebugConsole_Win.h"
#include "Toshi/Strings/TString8.h"
#include <conio.h>

namespace Toshi
{
	TDebugConsole_Win::TDebugConsole_Win()
	{
		CreateNewConsole(1024);
		m_HWND = FindConsole();
	}

	TDebugConsole_Win::~TDebugConsole_Win()
	{
		if (IsCreated())
		{
			if (IsVisible())
			{
				TOSHI_CORE_WARN("Press any key to exit...");
				_getch();
			}

			ReleaseConsole();
		}
	}

	HWND TDebugConsole_Win::FindConsole() const
	{
		TString8 str = TString8(TDebugConsoleUUID);

		// Generate title for the console so we can find it's HWND
		LARGE_INTEGER time;
		QueryPerformanceCounter(&time);
		char timeHex[sizeof(time.QuadPart) + 1] = { 0 };
		_ultoa_s((unsigned long)time.QuadPart, timeHex, 0x10);
		str += timeHex;

		// Save the current title
		char lpConsoleTitle[256];
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

	void TDebugConsole_Win::CreateNewConsole(int16_t minLength)
	{
		// Release any current console and redirect IO to NUL
		ReleaseConsole();

		// Attempt to create new console
		if (AllocConsole())
		{
			Toshi::TLog::Create();
			AdjustConsoleBuffer(minLength);
			m_Created = RedirectConsoleIO();
		}
	}

	bool TDebugConsole_Win::RedirectConsoleIO()
	{
		bool result = true;
		FILE* fp;

		// Redirect STDIN if the console has an input handle
		if (GetStdHandle(STD_INPUT_HANDLE) != INVALID_HANDLE_VALUE)
		{
			if (freopen_s(&fp, "CONIN$", "r", stdin) != 0)
			{
				result = false;
			}
			else
			{
				setvbuf(stdin, NULL, _IONBF, 0);
			}
		}

		// Redirect STDOUT if the console has an output handle
		if (GetStdHandle(STD_OUTPUT_HANDLE) != INVALID_HANDLE_VALUE)
		{
			if (freopen_s(&fp, "CONOUT$", "w", stdout) != 0)
			{
				result = false;
			}
			else
			{
				setvbuf(stdout, NULL, _IONBF, 0);
			}
		}

		// Redirect STDERR if the console has an error handle
		if (GetStdHandle(STD_ERROR_HANDLE) != INVALID_HANDLE_VALUE)
		{
			if (freopen_s(&fp, "CONOUT$", "w", stderr) != 0)
			{
				result = false;
			}
			else
			{
				setvbuf(stderr, NULL, _IONBF, 0);
			}
		}

		// Make C++ standard streams point to console as well.
		std::ios::sync_with_stdio(true);

		// Clear the error state for each of the C++ standard streams.
		std::cout.clear();
		std::cerr.clear();
		std::cin.clear();

		return result;
	}

	bool TDebugConsole_Win::ReleaseConsole()
	{
		bool result = true;
		FILE* fp;

		// Just to be safe, redirect standard IO to NUL before releasing.

		// Redirect STDIN to NUL
		if (freopen_s(&fp, "NUL:", "r", stdin) != 0)
		{
			result = false;
		}
		else
		{
			setvbuf(stdin, NULL, _IONBF, 0);
		}

		// Redirect STDOUT to NUL
		if (freopen_s(&fp, "NUL:", "w", stdout) != 0)
		{
			result = false;
		}
		else
		{
			setvbuf(stdout, NULL, _IONBF, 0);
		}

		// Redirect STDERR to NUL
		if (freopen_s(&fp, "NUL:", "w", stderr) != 0)
		{
			result = false;
		}
		else
		{
			setvbuf(stderr, NULL, _IONBF, 0);
		}

		// Detach from console
		if (!FreeConsole())
		{
			result = false;
		}

		return result;
	}

	void TDebugConsole_Win::AdjustConsoleBuffer(int16_t minLength)
	{
		// Set the screen buffer to be big enough to scroll some text
		CONSOLE_SCREEN_BUFFER_INFO conInfo;
		GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &conInfo);
		
		if (conInfo.dwSize.Y < minLength)
		{
			conInfo.dwSize.Y = minLength;
		}

		SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), conInfo.dwSize);
	}

	bool TDebugConsole_Win::Show(bool state)
	{
		bool oldState = m_Visible;
		m_Visible = state;

		ShowWindow(m_HWND, state);
		UpdateWindow(m_HWND);

		return oldState;
	}

	bool TDebugConsole_Win::Toggle()
	{
		Show(!m_Visible);
		return m_Visible;
	}
}
