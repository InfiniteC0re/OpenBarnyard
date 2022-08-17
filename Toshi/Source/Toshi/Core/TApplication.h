#pragma once
#include <Windows.h>

namespace Toshi
{
	constexpr const char* TDebugConsoleUUID = "{1B5D8052-529A-4c88-9DDE-EE523D440B00}";

	enum TApplicationFlag
	{
		TApplicationFlag_Created	= 1 << 0,
		TApplicationFlag_Destroyed	= 1 << 1,
	};

	class TApplication
	{
	public:
		TApplication();
		virtual ~TApplication();
		
		virtual bool OnCreate();
		virtual bool OnUpdate();
		virtual bool OnDestroy();

		// Returns true if success
		bool Create();

		// Destroys the application and sets the flag
		void Destroy();

		// Returns true if the console was previously visible
		bool ShowConsole(bool state);

		// Returns true if the console became visible
		bool ToggleConsole();

		// Returns true if the application is created
		inline bool IsCreated() { return m_Flags & TApplicationFlag_Created; }

		// Returns true if the application is destroyed
		inline bool IsDestroyed() { return m_Flags & TApplicationFlag_Destroyed; }

		// Sets new state
		void SetVerbose(bool state) { m_IsVerbose = state; }

	private:
		HWND CreateDebugConsole();

	private:
		uint32_t m_Unk1;		// 0x4
		uint32_t m_Flags;		// 0x10
		bool m_IsVerbose;		// 0x14
		bool m_ConsoleVisible;	// 0x15
		HWND* m_DebugConsole;	// 0x18
	};
	
	// this has to be used by application
	TApplication* CreateApplication(int argc, char** argv);
}