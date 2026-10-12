#include "ToshiPCH.h"
#include "TDebugConsole_Win.h"
#include "Toshi/TString8.h"

#include <conio.h>
#include <windows.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

static constexpr const WORD kTypeColours[]{
	FOREGROUND_GREEN,
	FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN,
	FOREGROUND_INTENSITY | FOREGROUND_RED,
	BACKGROUND_RED | FOREGROUND_INTENSITY | FOREGROUND_RED | FOREGROUND_GREEN
};

TSTATICASSERT( Toshi::TUtil::LogType_NUMOF == TARRAYSIZE( kTypeColours ) );

TOSHI_NAMESPACE_START

TDebugConsole::TDebugConsole()
{
	m_HWND    = FindConsole();
	m_Created = m_HWND != TNULL;

	m_LogListener.Connect(
	    TUtil::GetLogEmitter(),
	    this,
	    TDebugConsole::OnLog,
	    1
	);
}

TDebugConsole::~TDebugConsole()
{
	if ( IsCreated() )
	{
		if ( IsVisible() )
		{
			TWARN( "Press any key to exit...\n" );
			_getch();
		}
	}
}

TBOOL TDebugConsole::Show( TBOOL state )
{
	TBOOL oldState = m_Visible;
	m_Visible      = state;

	ShowWindow( m_HWND, state );
	UpdateWindow( m_HWND );

	return oldState;
}

TBOOL TDebugConsole::Toggle()
{
	Show( !m_Visible );
	return m_Visible;
}

// $Barnyard: FUNCTION 006c1600
HWND TDebugConsole::FindConsole() const
{
	// Generate a unique title for the console so we can find its HWND
	TCHAR szTimeHex[ 32 ];
	_ultoa( timeGetTime(), szTimeHex, 16 );

	static TCHAR s_szWindowName[ 256 ];
	strcpy( s_szWindowName, TDebugConsoleUUID );
	strcat( s_szWindowName, szTimeHex );

	// Save the current title
	TCHAR szConsoleTitle[ 512 ];
	if ( GetConsoleTitleA( szConsoleTitle, sizeof( szConsoleTitle ) ) == 0 )
		return TNULL;

	SetConsoleTitleA( s_szWindowName );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Wait for the new title to be applied
	ThreadSleep( 50 );
#endif // BARNYARD_COMMUNITY_PATCH

	// Find the window with the generated title and restore the title
	HWND hConsole = FindWindowA( TNULL, s_szWindowName );
	SetConsoleTitleA( szConsoleTitle );

	return hConsole;
}

static TBOOL s_bIsSimpleMode = TFALSE;

TBOOL __stdcall TDebugConsole::OnLog( TDebugConsole* a_pCaller, TUtil* a_pOwner, TUtil::LogEvent* pLogEvent )
{
	TASSERT( pLogEvent );

	HANDLE hStd;
	WORD   wOldColorAttrs;

	hStd = GetStdHandle( STD_OUTPUT_HANDLE );

	CONSOLE_SCREEN_BUFFER_INFO csbiInfo;
	GetConsoleScreenBufferInfo( hStd, &csbiInfo );
	wOldColorAttrs = csbiInfo.wAttributes;

	SetConsoleTextAttribute( hStd, kTypeColours[ pLogEvent->m_eType ] );

	if ( s_bIsSimpleMode )
	{
		TDebug_FinalPrintf(
		    "[%s]: %s",
		    TUtil::LogTypeToString( pLogEvent->m_eType ),
		    pLogEvent->m_szString
		);
	}
	else
	{
		auto& appParams = TUtil::GetToshiParameters();

		TCHAR strTime[ 9 ];
		_strtime( strTime );

		TDebug_FinalPrintf(
		    "[%s] [%s/%s] [%s]: %s",
		    strTime,
		    appParams.szLogAppName,
		    appParams.szLogAppDirName != TNULL ? appParams.szLogAppDirName : "",
		    TUtil::LogTypeToString( pLogEvent->m_eType ),
		    pLogEvent->m_szString
		);
	}

	SetConsoleTextAttribute( hStd, wOldColorAttrs );

	return true;
}

TOSHI_NAMESPACE_END
