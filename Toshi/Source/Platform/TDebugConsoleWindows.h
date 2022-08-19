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

		HWND CreateDebugConsole();

	private:
		bool m_Visible;
		HWND* m_HWND;
	};
}