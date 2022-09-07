#pragma once
#include "Toshi/Core/TDebugConsole.h"

namespace Toshi
{
	constexpr const char* TDebugConsoleUUID = "{1B5D8052-529A-4c88-9DDE-EE523D440B00}";

	class TDebugConsoleWindows : public TDebugConsole
	{
	public:
		TDebugConsoleWindows();
		~TDebugConsoleWindows();

		virtual bool Show(bool state) override;
		virtual bool Toggle() override;

		// Returns true if the console is visible
		inline bool IsVisible() const { return m_Visible; }

		// Returns true if the console is created
		inline bool IsCreated() const { return m_Created; }

		// Tries to find handle of the debug console
		HWND FindConsole() const;

	private:
		/* 
		*  The functions below are taken from
		*  https://stackoverflow.com/a/55875595
		*/

		bool RedirectConsoleIO();
		bool ReleaseConsole();
		void AdjustConsoleBuffer(int16_t minLength);
		void CreateNewConsole(int16_t minLength);

	private:
		bool m_Visible = false;
		bool m_Created = false;
		HWND m_HWND;
	};
}