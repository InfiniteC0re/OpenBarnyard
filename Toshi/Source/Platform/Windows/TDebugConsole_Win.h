#pragma once

namespace Toshi
{
	constexpr const char* TDebugConsoleUUID = "{1B5D8052-529A-4c88-9DDE-EE523D440B00}";

	class TDebugConsole
	{
	public:
		TDebugConsole();
		~TDebugConsole();

		bool Show(bool state);
		bool Toggle();

		// Returns true if the console is visible
		bool IsVisible() const { return m_Visible; }

		// Returns true if the console is created
		bool IsCreated() const { return m_Created; }

		// Tries to find handle of the debug console
		HWND FindConsole() const;

	private:
		bool m_Visible = false;
		bool m_Created = false;
		HWND m_HWND;
	};
}