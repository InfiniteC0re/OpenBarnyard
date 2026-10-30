#include "PrimeDebugPCH.h"
#include "PBindings.h"
#include "PConsole.h"
#include "PKeyboard.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TINT               PBindings::ms_iNumBindings;
PCFunc             PBindings::ms_oAddFunc;
PCFunc             PBindings::ms_oRemoveFunc;
PCFunc             PBindings::ms_oListFunc;
PBindings::Binding PBindings::ms_aBindings[ MAX_BINDINGS ];

// $deBlob2: FUNCTION 0080e385
TBOOL PBindings::AddCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc < 3 || a_iArgc > 2 + MAX_KEYS || ms_iNumBindings == MAX_BINDINGS )
		return TFALSE;

	TINT aKeys[ MAX_KEYS ] = {};

	for ( TINT i = 2; i < a_iArgc; i++ )
	{
		if ( !PKeyboard::GetKeyFromName( &aKeys[ i - 2 ], a_ppArgv[ i ] ) )
		{
			TINFO( "Failed to bind command '%s': unknown key name '%s'.\n", a_ppArgv[ 1 ], a_ppArgv[ i ] );
			return TTRUE;
		}
	}

	TINT iNumKeys = a_iArgc - 2;

	for ( TINT i = 0; i < ms_iNumBindings; i++ )
	{
		if ( ms_aBindings[ i ].iNumKeys == iNumKeys && memcmp( aKeys, ms_aBindings[ i ].aKeys, iNumKeys * sizeof( TINT ) ) == 0 )
		{
			TINFO( "Failed to bind command '%s': the same key combination is already used.\n", a_ppArgv[ 1 ] );
			return TTRUE;
		}
	}

	Binding& rBinding = ms_aBindings[ ms_iNumBindings ];

	if ( !PString::Copy( rBinding.szCommand, COMMAND_SIZE, a_ppArgv[ 1 ] ) )
		return TFALSE;

	memcpy( rBinding.aKeys, aKeys, sizeof( aKeys ) );
	rBinding.bWasDown = TFALSE;
	ms_iNumBindings++;
	rBinding.iNumKeys = TINT8( iNumKeys );
	TINFO( "Command '%s' was bound successfully.\n", a_ppArgv[ 1 ] );
	return TTRUE;
}

// $deBlob2: FUNCTION 0080e4c2
TBOOL PBindings::ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	TINFO( "Current command bindings (%d)\n", ms_iNumBindings );

	for ( TINT i = 0; i < ms_iNumBindings; i++ )
	{
		// NOTE: Prime prints the parts one by one; TOSHI's log starts a line each time, so the line is gathered first
		TCHAR szLine[ 256 ];
		PString::Format( szLine, sizeof( szLine ), "  '%s'", ms_aBindings[ i ].szCommand );

		for ( TINT j = 0; j < ms_aBindings[ i ].iNumKeys; j++ )
		{
			PString::Concat( szLine, sizeof( szLine ), " " );
			PString::Concat( szLine, sizeof( szLine ), PKeyboard::GetKeyName( ms_aBindings[ i ].aKeys[ j ] ) );
		}

		TINFO( "%s\n", szLine );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080e552
void PBindings::Shutdown()
{
	ms_iNumBindings = 0;
	ms_oAddFunc.Remove();
	ms_oRemoveFunc.Remove();
	ms_oListFunc.Remove();
}

// $deBlob2: FUNCTION 0080e57d
void PBindings::Startup()
{
	ms_iNumBindings = 0;
	ms_oAddFunc.Create( "bind.add", "Binds console command to given key combination.\nNote 1: Command can contain spaces if enclosed by double quotation marks.\nNote 2: Use keys.list to list available key names.\nNote 3: Key names are not case sensitive.\nNote 4: You can bind command to at most 4 keys.", "bind.add <command> <key name list>", AddCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oRemoveFunc.Create( "bind.remove", "Unbinds console command from given key combination.", "bind.remove <key names list>", RemoveCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oListFunc.Create( "bind.list", "Lists all bindings of the commands.", "bind.list", ListCommand, 0, TNULL );
}

// $deBlob2: FUNCTION 0080e5ea
TBOOL PBindings::RemoveCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc < 2 || a_iArgc > 1 + MAX_KEYS )
		return TFALSE;

	TINT aKeys[ MAX_KEYS ] = {};
	TINT iNumKeys          = a_iArgc - 1;

	for ( TINT i = 1; i < a_iArgc; i++ )
	{
		if ( !PKeyboard::GetKeyFromName( &aKeys[ i - 1 ], a_ppArgv[ i ] ) )
			return TTRUE;
	}

	for ( TINT i = 0; i < ms_iNumBindings; i++ )
	{
		if ( ms_aBindings[ i ].iNumKeys == iNumKeys && memcmp( aKeys, ms_aBindings[ i ].aKeys, iNumKeys * sizeof( TINT ) ) == 0 )
		{
#  ifdef BARNYARD_COMMUNITY_PATCH
			TINFO( "Command '%s' was unbound successfully.\n", ms_aBindings[ i ].szCommand );
#  endif

			if ( i != ms_iNumBindings - 1 )
				memcpy( &ms_aBindings[ i ], &ms_aBindings[ ms_iNumBindings - 1 ], sizeof( Binding ) );

			ms_iNumBindings--;

#  ifndef BARNYARD_COMMUNITY_PATCH
			// NOTE: the last binding has moved into the removed one's slot, so the message names the moved command
			TINFO( "Command '%s' was unbound successfully.\n", ms_aBindings[ i ].szCommand );
#  endif
			return TTRUE;
		}
	}

	TINFO( "Failed to unbind command: no existing command is bound to specified keys.\n" );
	return TTRUE;
}

// $deBlob2: FUNCTION 0080e6db
void PBindings::Update()
{
	for ( TINT i = 0; i < ms_iNumBindings; i++ )
	{
		Binding& rBinding = ms_aBindings[ i ];
		TBOOL    bDown    = TTRUE;

		for ( TINT j = 0; j < rBinding.iNumKeys; j++ )
		{
			if ( !PKeyboard::IsDown( rBinding.aKeys[ j ] ) )
			{
				bDown = TFALSE;
				break;
			}
		}

		if ( !rBinding.bWasDown && bDown )
			PConsole::Execute( rBinding.szCommand );

		rBinding.bWasDown = bDown;
	}
}

#endif // PRIMEDEBUG_DEV
