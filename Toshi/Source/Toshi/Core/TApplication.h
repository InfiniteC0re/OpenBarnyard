#pragma once
#include "Toshi/Core/TDebugConsole.h"
#include "Toshi/Strings/TString8.h"

namespace Toshi
{
	class TDebugConsole;

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
		
		virtual bool OnCreate(int argc, char** argv);
		virtual bool OnUpdate(float deltaTime);
		virtual bool OnDestroy();

		// Returns true if success
		bool Create(const char* appName, int argc, char** argv);

		// Destroys the application and sets the flag
		void Destroy();

		// Returns true if the application is created
		bool IsCreated() { return m_Flags & TApplicationFlag_Created; }

		// Returns true if the application is destroyed
		bool IsDestroyed() { return m_Flags & TApplicationFlag_Destroyed; }

		// Sets new state
		void SetVerbose(bool state) { m_IsVerbose = state; }

		// Executes the application and starts the loop
		bool Execute();

	private:
		TString8 m_Name;                       // 0x4
		uint32_t m_Flags;                      // 0x10
		bool m_IsVerbose;                      // 0x14
		TDebugConsole* m_pDebugConsole;
	};
}