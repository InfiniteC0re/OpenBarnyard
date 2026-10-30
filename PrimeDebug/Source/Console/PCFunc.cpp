#include "PrimeDebugPCH.h"
#include "PCFunc.h"
#include "PConsole.h"

#ifdef PRIMEDEBUG

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TSTATICASSERT( sizeof( PCFunc ) == 0x20 );

// $deBlob2: FUNCTION 0080c586
TBOOL PCFunc::Create( const TCHAR* a_szName, const TCHAR* a_szDescription, const TCHAR* a_szUsage, PCFuncCallback a_fnCallback, TINT a_iNumArgs, void* a_pUserData )
{
	if ( !PConsole::ms_bStartedUp )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Command executor not initialised.\n" );
		return TFALSE;
	}

	if ( strchr( a_szName, ' ' ) )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Callback names cannot contain spaces.\n" );
		return TFALSE;
	}

	if ( strchr( a_szDescription, ';' ) )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Descriptions cannot contain ';' symbols.\n" );
		return TFALSE;
	}

	if ( a_fnCallback == TNULL )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Passed CR_NULL for callback.\n" );
		return TFALSE;
	}

	if ( a_iNumArgs < ANY_NUM_ARGS )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Invalid number of callback parameters.\n" );
		return TFALSE;
	}

	if ( PConsole::HasCommand( this ) )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Command callback already added.\n" );
		return TFALSE;
	}

	if ( PConsole::FindCommandUnlocked( a_szName ) )
	{
#  ifdef BARNYARD_COMMUNITY_PATCH
		PConsole::Error( "[crCFunc_Create] ERROR: Command callback for with name '%s' already added.\n", a_szName );
#  else
		// NOTE: the name isn't passed to the message's %s
		PConsole::Error( "[crCFunc_Create] ERROR: Command callback for with name '%s' already added.\n" );
#  endif
		return TFALSE;
	}

	if ( PConsole::FindVariableUnlocked( a_szName ) )
	{
		PConsole::Error( "[crCFunc_Create] ERROR: Command variable for with name '%s' already added.\n", a_szName );
		return TFALSE;
	}

	m_szDescription = a_szDescription;
	m_szUsage       = a_szUsage ? a_szUsage : "";
	m_pNext         = TNULL;
	m_pPrev         = TNULL;
	m_iNumArgs      = a_iNumArgs;
	m_szName        = a_szName;
	m_fnCallback    = a_fnCallback;
	m_pUserData     = a_pUserData;
	PConsole::AddCommand( this );
	return TTRUE;
}

// $deBlob2: FUNCTION 0080c6c6
void PCFunc::Remove()
{
	if ( !PConsole::ms_bStartedUp )
		return;

	PCFunc* pNext = m_pNext;
	PCFunc* pPrev = m_pPrev;
	m_pNext       = TNULL;
	m_pPrev       = TNULL;

	if ( pNext )
		pNext->m_pPrev = pPrev;

#  ifdef BARNYARD_COMMUNITY_PATCH
	if ( pPrev )
		pPrev->m_pNext = pNext;
	else if ( this == PConsole::ms_pCommands )
		PConsole::ms_pCommands = pNext;
#  else
	// NOTE: removing a command that isn't in the list (no previous one) replaces the whole list with its next one
	if ( pPrev )
	{
		pPrev->m_pNext = pNext;
		pNext          = PConsole::ms_pCommands;
	}

	PConsole::ms_pCommands = pNext;
#  endif
}

#endif // PRIMEDEBUG
