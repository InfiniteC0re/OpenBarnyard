#pragma once

TOSHI_NAMESPACE_START

constexpr const TCHAR* TDebugConsoleUUID = "{1B5D8052-529A-4c88-9DDE-EE523D440B00}";

class TDebugConsole
{
public:
	TDebugConsole();
	~TDebugConsole();

	TBOOL Show( TBOOL state );
	TBOOL Toggle();

	// Returns TTRUE if the console is visible
	TBOOL IsVisible() const { return m_Visible; }

	// Returns TTRUE if the console is created
	TBOOL IsCreated() const { return m_Created; }

	// Tries to find handle of the debug console
	HWND FindConsole() const;

private:
	// Gets called after logging system printed something to a log file
	static TBOOL OnLog( TDebugConsole* a_pCaller, TUtil* a_pOwner, TUtil::LogEvent* pLogEvent );

private:
	TBOOL                                            m_Visible = TFALSE;
	TBOOL                                            m_Created = TFALSE;
	HWND                                             m_HWND;
	TListener<TUtil, TUtil::LogEvent, TDebugConsole> m_LogListener;
};

TOSHI_NAMESPACE_END
