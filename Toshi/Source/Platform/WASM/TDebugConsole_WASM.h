#pragma once

TOSHI_NAMESPACE_START

// The page's console takes the log
class TDebugConsole
{
public:
	TDebugConsole();
	~TDebugConsole();

	TBOOL Show( TBOOL state );
	TBOOL Toggle();

	TBOOL IsVisible() const { return m_Visible; }
	TBOOL IsCreated() const { return m_Created; }

private:
	static TBOOL OnLog( TDebugConsole* a_pCaller, TUtil* a_pOwner, TUtil::LogEvent* a_pLogEvent );

private:
	TBOOL m_Visible;
	TBOOL m_Created;

	TListener<TUtil, TUtil::LogEvent, TDebugConsole> m_LogListener;
};

TOSHI_NAMESPACE_END
