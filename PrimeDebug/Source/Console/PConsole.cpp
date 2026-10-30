#include "PrimeDebugPCH.h"
#include "PConsole.h"
#include "PKeyboard.h"
#include "Util/PString.h"

#ifdef PRIMEDEBUG

#  include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TBOOL        PConsole::ms_bStartedUp;
void*        PConsole::ms_pContext;
TINT         PConsole::ms_iNumCompletions;
PCVar        PConsole::ms_oExitCodeVar;
PCFunc*      PConsole::ms_pCommands;
PCVar*       PConsole::ms_pVariables;
PCFunc       PConsole::ms_oHelpFunc;
PCFunc       PConsole::ms_oListCommandsFunc;
PCFunc       PConsole::ms_oListVarsFunc;
PCFunc       PConsole::ms_oListFunc;
PCFunc       PConsole::ms_oExitFunc;
PCFunc       PConsole::ms_oCalcFunc;
PCFunc       PConsole::ms_oKeysListFunc;
PCFunc       PConsole::ms_oSaveFunc;
PCFunc       PConsole::ms_oLoadFunc;
PCFunc       PConsole::ms_oSetFunc;
const TCHAR* PConsole::ms_apCompletions[ MAX_COMPLETIONS ];
TCHAR        PConsole::ms_aCompletionNames[ MAX_COMPLETIONS ][ ENUM_NAME_SIZE ];

//-----------------------------------------------------------------------------
// Auto-completion
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 0080bb83
void PConsole::CollectCompletion( const TCHAR* a_szName, void* a_pUserData )
{
	if ( ms_iNumCompletions < MAX_COMPLETIONS )
	{
#  ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: Prime keeps the pointer it gets, which can be Enumerate's temporary buffer (see AutoComplete)
		PString::Copy( ms_aCompletionNames[ ms_iNumCompletions ], ENUM_NAME_SIZE, a_szName );
		a_szName = ms_aCompletionNames[ ms_iNumCompletions ];
#  endif
		ms_apCompletions[ ms_iNumCompletions ] = a_szName;
		ms_iNumCompletions++;
	}
}

// NOTE: de Blob 2 formats the value but doesn't print it; the line is printed here
// $deBlob2: FUNCTION 0080bd01
void PConsole::PrintVariable( PCVar* a_pVar )
{
	TCHAR szValue[ 256 ];
	a_pVar->FormatValue( szValue, sizeof( szValue ) );
	TINFO( "%s = %s\n", a_pVar->m_szName, szValue );
}

//-----------------------------------------------------------------------------
// Built-in commands
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 0080bd25
TBOOL PConsole::ExitCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	return TTRUE;
}

// NOTE: de Blob 2 looks the name up but prints nothing; the description and usage are printed here
// $deBlob2: FUNCTION 0080bd29
TBOOL PConsole::HelpCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc == 2 )
	{
		if ( PCFunc* pFunc = FindCommandUnlocked( a_ppArgv[ 1 ] ) )
		{
			TINFO( "%s: %s\nUsage: %s\n", pFunc->m_szName, pFunc->m_szDescription, pFunc->m_szUsage );
		}
		else if ( PCVar* pVar = FindVariableUnlocked( a_ppArgv[ 1 ] ) )
		{
			TINFO( "%s: %s\n", pVar->m_szName, pVar->m_szDescription );
			PrintVariable( pVar );
		}
	}
	else
	{
		TINFO( "Type 'list {prefix}' for the commands and variables, 'help <name>' for one of them\n" );
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080bd67
TBOOL PConsole::ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 1 && a_iArgc != 2 )
		return TFALSE;

	TBOOL bCommands = ListCommandsCommand( a_iArgc, a_ppArgv, a_pFunc );
	TBOOL bVars     = ListVarsCommand( a_iArgc, a_ppArgv, a_pFunc );
	return bVars | bCommands;
}

// Pads each name to a column.
// NOTE: de Blob 2 builds the padding but prints nothing; the lines are printed here
// $deBlob2: FUNCTION 0080bde7
void PConsole::ListCommands( const TCHAR* a_szPrefix )
{
	TSIZE uiLength = strlen( a_szPrefix );

	for ( PCFunc* pFunc = ms_pCommands; pFunc != TNULL; pFunc = pFunc->m_pNext )
	{
		TINT iCompare = _strnicmp( pFunc->m_szName, a_szPrefix, uiLength );

		if ( iCompare == 0 )
		{
			TCHAR szPadding[ LIST_COLUMN ];
			TINT8 iNameLength = TINT8( strlen( pFunc->m_szName ) );

			if ( iNameLength < LIST_COLUMN )
			{
				memset( szPadding, ' ', LIST_COLUMN - 1 - iNameLength );
				szPadding[ LIST_COLUMN - 1 - iNameLength ] = '\0';
			}
			else
			{
				szPadding[ 0 ] = '\0';
			}

			TINFO( "%s%s %s\n", pFunc->m_szName, szPadding, pFunc->m_szDescription );
		}
		else if ( iCompare > 0 )
		{
			return;
		}
	}
}

// $deBlob2: FUNCTION 0080bda1
TBOOL PConsole::ListCommandsCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	const TCHAR* szPrefix;

	if ( a_iArgc == 1 )
		szPrefix = "";
	else if ( a_iArgc == 2 )
		szPrefix = a_ppArgv[ 1 ];
	else
		return TFALSE;

	ListCommands( szPrefix );
	return TTRUE;
}

// NOTE: de Blob 2 builds the padding but prints nothing; the lines are printed here
// $deBlob2: FUNCTION 0080beb0
void PConsole::ListVars( const TCHAR* a_szPrefix )
{
	TSIZE uiLength = strlen( a_szPrefix );

	for ( PCVar* pVar = ms_pVariables; pVar != TNULL; pVar = pVar->m_pNext )
	{
		TINT iCompare = _strnicmp( pVar->m_szName, a_szPrefix, uiLength );

		if ( iCompare == 0 )
		{
			TCHAR szPadding[ LIST_COLUMN ];
			TINT8 iNameLength = TINT8( strlen( pVar->m_szName ) );

			if ( iNameLength < LIST_COLUMN )
			{
				memset( szPadding, ' ', LIST_COLUMN - 1 - iNameLength );
				szPadding[ LIST_COLUMN - 1 - iNameLength ] = '\0';
			}
			else
			{
				szPadding[ 0 ] = '\0';
			}

			TCHAR szValue[ 256 ];
			pVar->FormatValue( szValue, sizeof( szValue ) );
			TINFO( "%s%s %s\n", pVar->m_szName, szPadding, szValue );
		}
		else if ( iCompare > 0 )
		{
			return;
		}
	}
}

// $deBlob2: FUNCTION 0080be6a
TBOOL PConsole::ListVarsCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	const TCHAR* szPrefix;

	if ( a_iArgc == 1 )
		szPrefix = "";
	else if ( a_iArgc == 2 )
		szPrefix = a_ppArgv[ 1 ];
	else
		return TFALSE;

	ListVars( szPrefix );
	return TTRUE;
}

// $deBlob2: FUNCTION 0080bf33
TBOOL PConsole::LoadCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 2 )
		return TFALSE;

	return ExecuteFile( a_ppArgv[ 1 ] );
}

// Runs every line of a file as a console command; lines starting with "//" are skipped.
// NOTE: Prime reads the file through its file system; here it's a path for the C library
TBOOL PConsole::ExecuteFile( const TCHAR* a_szFileName, TBOOL ( *a_fnFilter )( const TCHAR* a_szLine ) )
{
	FILE* pFile = fopen( a_szFileName, "rb" );

	if ( !pFile )
		return TFALSE;

	fseek( pFile, 0, SEEK_END );
	TINT iSize = TINT( ftell( pFile ) );
	fseek( pFile, 0, SEEK_SET );

	if ( iSize < 0 )
	{
		fclose( pFile );
		return TFALSE;
	}

	TCHAR* pBuffer   = new TCHAR[ iSize + 1 ];
	iSize            = TINT( fread( pBuffer, 1, iSize, pFile ) );
	pBuffer[ iSize ] = '\0';
	fclose( pFile );

	TCHAR* pCursor = pBuffer;
	TCHAR* pLine   = PString::Tokenize( pBuffer, "\r\n" );

	while ( pLine != TNULL )
	{
		pCursor = pLine + strlen( pLine );

		if ( pCursor - pBuffer < iSize )
		{
			do
			{
				pCursor++;
			} while ( *pCursor == '\r' || *pCursor == '\n' );
		}

		while ( *pLine == ' ' || *pLine == '\t' )
			pLine++;

		if ( ( pLine[ 0 ] != '/' || pLine[ 1 ] != '/' ) && ( !a_fnFilter || a_fnFilter( pLine ) ) )
			Execute( pLine );

		if ( pCursor - pBuffer >= iSize )
			break;

		pLine = PString::Tokenize( pCursor, "\r\n" );
	}

	delete[] pBuffer;
	return TTRUE;
}

// Writes "name value" lines for all writable variables
// $deBlob2: FUNCTION 0080c058
TBOOL PConsole::SaveCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc < 2 )
		return TFALSE;

	// NOTE: the prefix and extra name arguments are parsed by Prime but not used, they're skipped here

	FILE* pFile = fopen( a_ppArgv[ 1 ], "wb" );

	if ( !pFile )
		return TFALSE;

	for ( PCVar* pVar = ms_pVariables; pVar != TNULL; )
	{
#  ifdef BARNYARD_COMMUNITY_PATCH
		if ( pVar->m_uiFlags & PCVar::FLAGS_READONLY )
		{
			pVar = pVar->m_pNext;
			continue;
		}
#  else
		// NOTE: Prime doesn't move to the next variable after a read-only one, so saving hangs
		if ( pVar->m_uiFlags & PCVar::FLAGS_READONLY )
			continue;
#  endif

		TCHAR szValue[ 256 ];
		TCHAR szLine[ 256 ];
		pVar->FormatValueRaw( szValue, sizeof( szValue ) );
		PString::Copy( szLine, sizeof( szLine ), pVar->m_szName );
		PString::Concat( szLine, sizeof( szLine ), " " );
		PString::Concat( szLine, sizeof( szLine ), szValue );
#  ifdef BARNYARD_COMMUNITY_PATCH
		PString::Concat( szLine, sizeof( szLine ), "\n" );
#  else
		// NOTE: Prime writes no line break, so "load" can't read the file back
#  endif
		fwrite( szLine, 1, strlen( szLine ), pFile );

		pVar = pVar->m_pNext;
	}

	fclose( pFile );
	return TTRUE;
}

// "set <cvar> <value> [time]": the time isn't used
// $deBlob2: FUNCTION 0080c179
TBOOL PConsole::SetCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc < 3 )
		return TFALSE;

	PCVar* pVar   = FindVariableUnlocked( a_ppArgv[ 1 ] );
	TBOOL  bFound = pVar != TNULL;

	if ( bFound )
		SetVariable( a_ppArgv + 1, a_iArgc - 1 );

	return bFound;
}

//-----------------------------------------------------------------------------
// Lists
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 0080c1cd
void PConsole::AddCommand( PCFunc* a_pFunc )
{
	PCFunc* pPrev = TNULL;
	PCFunc* pNext = ms_pCommands;

	if ( ms_pCommands == TNULL )
	{
		ms_pCommands = a_pFunc;
		return;
	}

	do
	{
		if ( _stricmp( pNext->m_szName, a_pFunc->m_szName ) >= 0 )
			break;

		pPrev = pNext;
		pNext = pNext->m_pNext;
	} while ( pNext != TNULL );

	if ( pPrev == TNULL )
	{
		ms_pCommands = a_pFunc;
	}
	else
	{
		pPrev->m_pNext   = a_pFunc;
		a_pFunc->m_pPrev = pPrev;
	}

	if ( pNext != TNULL )
	{
		a_pFunc->m_pNext = pNext;
		pNext->m_pPrev   = a_pFunc;
	}
}

// $deBlob2: FUNCTION 0080c227
TBOOL PConsole::HasCommand( PCFunc* a_pFunc )
{
	for ( PCFunc* pFunc = ms_pCommands; pFunc != TNULL; pFunc = pFunc->m_pNext )
	{
		if ( a_pFunc == pFunc )
			return TTRUE;
	}

	return TFALSE;
}

// $deBlob2: FUNCTION 0080c244
void PConsole::AddVariable( PCVar* a_pVar )
{
	PCVar* pPrev = TNULL;
	PCVar* pNext = ms_pVariables;

	if ( ms_pVariables == TNULL )
	{
		ms_pVariables = a_pVar;
		return;
	}

	do
	{
		if ( _stricmp( pNext->m_szName, a_pVar->m_szName ) >= 0 )
			break;

		pPrev = pNext;
		pNext = pNext->m_pNext;
	} while ( pNext != TNULL );

	if ( pPrev == TNULL )
	{
		ms_pVariables = a_pVar;
	}
	else
	{
		pPrev->m_pNext  = a_pVar;
		a_pVar->m_pPrev = pPrev;
	}

	if ( pNext != TNULL )
	{
		a_pVar->m_pNext = pNext;
		pNext->m_pPrev  = a_pVar;
	}
}

// $deBlob2: FUNCTION 0080c29e
TBOOL PConsole::HasVariable( PCVar* a_pVar )
{
	for ( PCVar* pVar = ms_pVariables; pVar != TNULL; pVar = pVar->m_pNext )
	{
		if ( a_pVar == pVar )
			return TTRUE;
	}

	return TFALSE;
}

// NOTE: de Blob 2 formats the message but doesn't print it; it's a warning here
// $deBlob2: FUNCTION 0080c2bb
void PConsole::Error( const TCHAR* a_szFormat, ... )
{
	va_list args;
	va_start( args, a_szFormat );

	TCHAR szMessage[ 1024 ];
	PString::FormatV( szMessage, sizeof( szMessage ), a_szFormat, args );
	va_end( args );

	TWARN( "%s", szMessage );
}

//-----------------------------------------------------------------------------
// Execution
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 0080c2e3
PConsole::RESULT PConsole::ExecuteCommand( TCHAR** a_ppArgv, TINT a_iArgc )
{
	PCFunc* pFunc = FindCommandUnlocked( a_ppArgv[ 0 ] );

	if ( pFunc == TNULL )
		return RESULT_NOTFOUND;

	if ( pFunc->m_iNumArgs != PCFunc::ANY_NUM_ARGS && pFunc->m_iNumArgs != a_iArgc - 1 )
		return RESULT_ERROR;

	return pFunc->m_fnCallback( a_iArgc, a_ppArgv, pFunc ) ? RESULT_OK : RESULT_ERROR;
}

// $deBlob2: FUNCTION 0080c33a
PConsole::RESULT PConsole::SetVariable( TCHAR** a_ppArgv, TINT a_iArgc )
{
	PCVar* pVar = FindVariableUnlocked( a_ppArgv[ 0 ] );

	if ( pVar == TNULL )
		return RESULT_NOTFOUND;

	if ( a_iArgc == 1 )
	{
		PrintVariable( pVar );
		return RESULT_OK;
	}

	if ( pVar->m_uiFlags & PCVar::FLAGS_READONLY )
		return RESULT_ERROR;

	TBOOL bSet = TFALSE;

	switch ( pVar->m_uiType )
	{
		case PCVar::TYPE_INT:
			if ( a_iArgc != 2 )
				return RESULT_ERROR;

			if ( pVar->m_uiFlags & PCVar::FLAGS_HASH )
				bSet = pVar->SetInt( TINT( HashString( a_ppArgv[ 1 ] ) ), TNULL );
			else
				bSet = pVar->SetInt( PString::ToInt( a_ppArgv[ 1 ], TNULL, 10 ), TNULL );
			break;

		case PCVar::TYPE_FLOAT:
			if ( a_iArgc != 2 )
				return RESULT_ERROR;

			bSet = pVar->SetFloat( PString::ToFloat( a_ppArgv[ 1 ], TNULL ), TNULL );
			break;

		case PCVar::TYPE_FLOATSPLAT:
			if ( a_iArgc != 2 )
				return RESULT_ERROR;

			bSet = pVar->SetFloatSplat( PString::ToFloat( a_ppArgv[ 1 ], TNULL ), TNULL );
			break;

		case PCVar::TYPE_VECTOR4:
		{
			if ( a_iArgc != 5 )
				return RESULT_ERROR;

			TFLOAT aValue[ 4 ];
			for ( TINT i = 0; i < 4; i++ )
				aValue[ i ] = PString::ToFloat( a_ppArgv[ 1 + i ], TNULL );

			bSet = pVar->SetVector4( aValue, TNULL );
			break;
		}

		case PCVar::TYPE_MATRIX:
		{
			if ( a_iArgc != 17 )
				return RESULT_ERROR;

			TFLOAT aValues[ 16 ];
			for ( TINT i = 0; i < 16; i++ )
				aValues[ i ] = PString::ToFloat( a_ppArgv[ 1 + i ], TNULL );

			bSet = pVar->SetMatrix( aValues, TNULL );
			break;
		}

		case PCVar::TYPE_STRING:
			if ( a_iArgc != 2 )
				return RESULT_ERROR;

			bSet = pVar->SetString( a_ppArgv[ 1 ] );
			break;

		default:
			return RESULT_ERROR;
	}

	return bSet ? RESULT_OK : RESULT_ERROR;
}

// Lists are sorted by name, so the search stops at the first greater name
// $deBlob2: FUNCTION 0080c50b
PCFunc* PConsole::FindCommandUnlocked( const TCHAR* a_szName )
{
	for ( PCFunc* pFunc = ms_pCommands; pFunc != TNULL; pFunc = pFunc->m_pNext )
	{
		TINT iCompare = _stricmp( pFunc->m_szName, a_szName );

		if ( iCompare == 0 )
			return pFunc;

		if ( iCompare > 0 )
			return TNULL;
	}

	return TNULL;
}

// $deBlob2: FUNCTION 0080c53a
PCVar* PConsole::FindVariableUnlocked( const TCHAR* a_szName )
{
	for ( PCVar* pVar = ms_pVariables; pVar != TNULL; pVar = pVar->m_pNext )
	{
		TINT iCompare = _stricmp( pVar->m_szName, a_szName );

		if ( iCompare == 0 )
			return pVar;

		if ( iCompare > 0 )
			return TNULL;
	}

	return TNULL;
}

// $deBlob2: FUNCTION 0080c569
static int __cdecl PConsole_CompareNames( const void* a_pName1, const void* a_pName2 )
{
	return _stricmp( *(const TCHAR**)a_pName1, *(const TCHAR**)a_pName2 );
}

// $deBlob2: FUNCTION 0080d22d
void* PConsole::SetContext( void* a_pContext )
{
	void* pPrevious = ms_pContext;
	ms_pContext     = a_pContext;
	return pPrevious;
}

//-----------------------------------------------------------------------------
// Console
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 0080d5e2
TBOOL PConsole::Execute( const TCHAR* a_szLine )
{
	if ( !ms_bStartedUp )
		return TFALSE;

	if ( strlen( a_szLine ) != 0 )
	{
		TCHAR szLine[ MAX_LINE ];
		PString::Copy( szLine, sizeof( szLine ), a_szLine );

		for ( TCHAR* p = szLine; *p != '\0'; p++ )
		{
			if ( *p == '#' )
			{
				*p = '\0';
				break;
			}
		}

		TCHAR* apArgs[ MAX_ARGS ];
		TINT   iNumArgs;

		if ( !ParseArguments( szLine, apArgs, MAX_ARGS, &iNumArgs ) )
			return TFALSE;

		if ( iNumArgs != 0 )
		{
			RESULT eResult = ExecuteCommand( apArgs, iNumArgs );

			if ( eResult != RESULT_OK )
			{
				if ( eResult == RESULT_ERROR )
					return TFALSE;

				if ( SetVariable( apArgs, iNumArgs ) != RESULT_OK )
					return TFALSE;
			}
		}
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0080d6a6
PCFunc* PConsole::FindCommand( const TCHAR* a_szName )
{
	return FindCommandUnlocked( a_szName );
}

// $deBlob2: FUNCTION 0080d6c4
PCVar* PConsole::FindVariable( const TCHAR* a_szName )
{
	return FindVariableUnlocked( a_szName );
}

// The part of a_szName down to a_iNumLevels + 1 dot-separated levels
void PConsole::GetNameLevels( const TCHAR* a_szName, TINT a_iNumLevels, TCHAR* a_szResult )
{
	TSIZE uiNameLength = strlen( a_szName );

	TCHAR szName[ ENUM_NAME_SIZE ];
	PString::Copy( szName, sizeof( szName ), a_szName );
	PString::Copy( a_szResult, ENUM_NAME_SIZE, "" );

	TCHAR* pLevel = szName;

	if ( a_iNumLevels != -1 && a_iNumLevels + 1 > 0 )
	{
		for ( TINT i = 0; i < a_iNumLevels + 1; i++ )
		{
			TCHAR* pToken = PString::Tokenize( pLevel, "." );

			if ( !pToken )
				break;

			PString::Concat( a_szResult, ENUM_NAME_SIZE, pLevel );
			pLevel += strlen( pToken ) + 1;

			if ( TINT( uiNameLength ) <= pLevel - szName )
				break;

			PString::Concat( a_szResult, ENUM_NAME_SIZE, "." );
		}
	}
}

// $deBlob2: FUNCTION 0080d6e8
TINT PConsole::Enumerate( const TCHAR* a_szPrefix, TINT a_iMaxResults, TBOOL a_bFullNames, EnumCallback a_fnCallback, void* a_pUserData )
{
	TINT  iNumLevels = 0;
	TINT  iCount     = 0;
	TSIZE uiLength   = strlen( a_szPrefix );

	if ( strchr( a_szPrefix, ' ' ) )
		return 0;

	TCHAR szLast[ ENUM_NAME_SIZE ];
	TCHAR szPrefix[ ENUM_NAME_SIZE ];
	PString::Copy( szLast, sizeof( szLast ), "" );
	PString::Copy( szPrefix, sizeof( szPrefix ), a_szPrefix );

	// Count the levels of the prefix
	TCHAR* pLevel = szPrefix;

	if ( TINT( uiLength ) > 0 )
	{
		do
		{
			TCHAR* pToken = PString::Tokenize( pLevel, ".\n" );

			if ( !pToken )
				break;

			pLevel += strlen( pToken ) + 1;
			iNumLevels++;
		} while ( pLevel - szPrefix < TINT( uiLength ) );
	}

	for ( PCFunc* pFunc = ms_pCommands; pFunc != TNULL && ( iCount < a_iMaxResults || a_iMaxResults == -1 ); pFunc = pFunc->m_pNext )
	{
		TINT iCompare = _strnicmp( pFunc->m_szName, a_szPrefix, uiLength );

		if ( iCompare == 0 )
		{
			if ( !a_bFullNames )
			{
				TCHAR szName[ ENUM_NAME_SIZE ];
				GetNameLevels( pFunc->m_szName, iNumLevels, szName );

				if ( a_fnCallback && _stricmp( szName, szLast ) != 0 )
				{
					// NOTE: the callback gets a buffer that only lives during this call
					a_fnCallback( szName, a_pUserData );
					iCount++;
					PString::Copy( szLast, sizeof( szLast ), szName );
				}
			}
			else if ( a_fnCallback )
			{
				a_fnCallback( pFunc->m_szName, a_pUserData );
				iCount++;
			}
		}
		else if ( iCompare > 0 )
		{
			break;
		}
	}

	PString::Copy( szLast, sizeof( szLast ), "" );

	for ( PCVar* pVar = ms_pVariables; pVar != TNULL && ( iCount < a_iMaxResults || a_iMaxResults == -1 ); pVar = pVar->m_pNext )
	{
		TINT iCompare = _strnicmp( pVar->m_szName, a_szPrefix, uiLength );

		if ( iCompare == 0 )
		{
			if ( !a_bFullNames )
			{
				TCHAR szName[ ENUM_NAME_SIZE ];
				GetNameLevels( pVar->m_szName, iNumLevels, szName );

				if ( a_fnCallback && _stricmp( szName, szLast ) != 0 )
				{
					a_fnCallback( szName, a_pUserData );
					iCount++;
					PString::Copy( szLast, sizeof( szLast ), szName );
				}
			}
			else if ( a_fnCallback )
			{
				a_fnCallback( pVar->m_szName, a_pUserData );
				iCount++;
			}
		}
		else if ( iCompare > 0 )
		{
			break;
		}
	}

	return iCount;
}

// NOTE: in Prime the collected names point to Enumerate's temporary buffer, which is gone when they're compared
// (fixed in CollectCompletion)
// $deBlob2: FUNCTION 0080dab0
TBOOL PConsole::AutoComplete( const TCHAR* a_szPartial, TCHAR* a_szResult )
{
	ms_iNumCompletions = 0;

	TSIZE uiLength = strlen( a_szPartial );
	TSIZE uiBase   = uiLength;

	TCHAR szPrefix[ 256 ];

	if ( TINT( uiLength ) > 0 )
	{
		for ( ; TINT( uiBase ) > 0; uiBase-- )
		{
			if ( a_szPartial[ uiBase - 1 ] == '.' )
				break;
		}

		if ( TINT( uiBase ) > 0 )
			memcpy( szPrefix, a_szPartial, uiBase );
	}

	szPrefix[ uiBase ] = '\0';

	TINT iNumMatches = Enumerate( szPrefix, MAX_COMPLETIONS, TFALSE, CollectCompletion, TNULL );

	if ( iNumMatches == 0 )
		return TFALSE;

	qsort( ms_apCompletions, iNumMatches, sizeof( const TCHAR* ), PConsole_CompareNames );

	TINT i = 0;

	for ( ; i < iNumMatches; i++ )
	{
		if ( _strnicmp( ms_apCompletions[ i ], a_szPartial, uiLength ) == 0 )
			break;
	}

	if ( i == iNumMatches )
		return TFALSE;

	TSIZE uiMatchLength = strlen( ms_apCompletions[ i ] );

	// Already complete: move on to the next match
	if ( uiMatchLength == uiLength || ( TINT( uiLength ) < TINT( uiMatchLength ) && ms_apCompletions[ i ][ uiLength ] == '.' ) )
	{
		while ( i < iNumMatches && _strnicmp( ms_apCompletions[ i ], a_szPartial, uiLength ) == 0 )
		{
			const TCHAR* szMatch = ms_apCompletions[ i ];
			TSIZE        uiEnd   = uiBase;

			while ( szMatch[ uiEnd ] != '.' && szMatch[ uiEnd ] != '\0' )
				uiEnd++;

			if ( uiEnd != uiLength )
				break;

			i++;
		}

		if ( i == iNumMatches )
			i = 0;
	}

	const TCHAR* szMatch = ms_apCompletions[ i ];
	TSIZE        uiEnd   = uiBase;

	while ( szMatch[ uiEnd ] != '.' && szMatch[ uiEnd ] != '\0' )
		uiEnd++;

	memcpy( a_szResult, szMatch, uiEnd );
	a_szResult[ uiEnd ] = '\0';
	return TTRUE;
}

// $deBlob2: FUNCTION 0080dc76
void PConsole::Shutdown()
{
	if ( !ms_bStartedUp )
		return;

	ms_oCalcFunc.Remove();
	ms_oKeysListFunc.Remove();
	ms_oExitFunc.Remove();
	ms_oListVarsFunc.Remove();
	ms_oListCommandsFunc.Remove();
	ms_oHelpFunc.Remove();
	ms_oSaveFunc.Remove();
	ms_oLoadFunc.Remove();
	ms_oSetFunc.Remove();

	for ( PCVar* pVar = ms_pVariables; pVar != TNULL; pVar = pVar->m_pNext )
	{
		if ( pVar->m_uiType == PCVar::TYPE_STRING )
			pVar->SetString( "" );
	}

	ms_pCommands  = TNULL;
	ms_pVariables = TNULL;
	ms_bStartedUp = TFALSE;
}

// $deBlob2: FUNCTION 0080deeb
void PConsole::ForEachCommand( CommandCallback a_fnCallback, void* a_pUserData )
{
	for ( PCFunc* pFunc = ms_pCommands; pFunc != TNULL; pFunc = pFunc->m_pNext )
		a_fnCallback( a_pUserData, pFunc );
}

// $deBlob2: FUNCTION 0080df0a
void PConsole::ForEachVariable( VariableCallback a_fnCallback, void* a_pUserData )
{
	for ( PCVar* pVar = ms_pVariables; pVar != TNULL; pVar = pVar->m_pNext )
		a_fnCallback( a_pUserData, pVar );
}

// $deBlob2: FUNCTION 0080dd31
TBOOL PConsole::Startup()
{
	// NOTE: the game starts the console before PDebugSystem (APrimeDebug::CreateConsole)
	if ( ms_bStartedUp )
		return TTRUE;

	ms_bStartedUp = TTRUE;

	ms_oHelpFunc.Create( "help", "General help or help on specific command.", "help [command/variable]", HelpCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oListCommandsFunc.Create( "list_commands", "Lists all available commands. A prefix can be given to limit the list.", "list_commands {prefix}", ListCommandsCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oListVarsFunc.Create( "list_vars", "Lists all available variables. A prefix can be given to limit the list.", "list_vars {prefix}", ListVarsCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oListFunc.Create( "list", "Lists all available commands and variables. A prefix can be given to limit the list.", "list {prefix}", ListCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oExitFunc.Create( "exit", "Closes an application.", "exit", ExitCommand, 0, TNULL );
	ms_oCalcFunc.Create( "calc", "Calculates the value of given integer arithmetic expression. No brackets accepted, no priority of * and / operators over + and - ones.", "calc<expression>", CalcCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oKeysListFunc.Create( "keys.list", "Lists all available key names that can be used to bind to any command.", "keys.list", KeysListCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oSaveFunc.Create( "save", "Saves all modified variables to the specified file.", "save <file>", SaveCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oLoadFunc.Create( "load", "Loads batch file containing console commands.", "load <file>", LoadCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	ms_oSetFunc.Create( "set", "Sets a cvar, conditionally may include over how long to lerp to it", "set <cvar> <value> [time]", SetCommand, PCFunc::ANY_NUM_ARGS, TNULL );

	PCVarValue oDefault;
	oDefault.iValue = 0;
	oDefault.iMin   = 0;
	oDefault.iMax   = 1;
	oDefault.iStep  = 1;
	ms_oExitCodeVar.Create( "exitCode", "The error code that is returned when the app exits.", 0, PCVar::TYPE_INT, &oDefault, TNULL, TNULL, TNULL );

	return TTRUE;
}

// Integer arithmetic over the arguments, left to right (no precedence, no brackets); division by zero gives 0
// $deBlob2: FUNCTION 0080ecfc
TBOOL PConsole::CalcCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	enum OPERATOR
	{
		OPERATOR_NONE = -1,
		OPERATOR_ADD,
		OPERATOR_SUBTRACT,
		OPERATOR_MULTIPLY,
		OPERATOR_DIVIDE,
	};

	TINT iResult   = 0;
	TINT eOperator = OPERATOR_NONE;

	for ( TINT iArg = 1; iArg < a_iArgc; iArg++ )
	{
		const TCHAR* p = a_ppArgv[ iArg ];

		while ( *p != '\0' )
		{
			while ( *p == ' ' )
				p++;

			TCHAR c = *p;

			if ( c == '\0' )
				break;

			if ( ( c < '0' || c > '9' ) && c != '+' && c != '-' && c != '*' && c != '/' )
			{
				TINFO( "Invalid character in expression: '%c'\n", c );
				return TTRUE;
			}

			TINT         iNumber = 0;
			const TCHAR* pEnd    = p;

			if ( c > '/' )
			{
				do
				{
					if ( c > '9' )
						break;

					iNumber = iNumber * 10 + ( c - '0' );
					pEnd++;
					c = *pEnd;
				} while ( c > '/' );

				if ( p != pEnd )
				{
					TINT iPrevious = iResult;
					iResult        = iNumber;

					switch ( eOperator )
					{
						case OPERATOR_NONE:
							break;
						case OPERATOR_ADD:
							iResult = iPrevious + iNumber;
							break;
						case OPERATOR_SUBTRACT:
							iResult = iPrevious - iNumber;
							break;
						case OPERATOR_MULTIPLY:
							iResult = iPrevious * iNumber;
							break;
						case OPERATOR_DIVIDE:
							iResult = iNumber == 0 ? 0 : iPrevious / iNumber;
							break;
						default:
							iResult = iPrevious;
							break;
					}
				}
			}

			while ( *pEnd == ' ' )
				pEnd++;

			if ( *pEnd == '\0' )
				break;

			if ( *pEnd == '+' )
			{
				eOperator = OPERATOR_ADD;
				pEnd++;
			}

			if ( *pEnd == '-' )
			{
				eOperator = OPERATOR_SUBTRACT;
				pEnd++;
			}

			if ( *pEnd == '*' )
			{
				eOperator = OPERATOR_MULTIPLY;
				pEnd++;
			}

			if ( *pEnd == '/' )
			{
				eOperator = OPERATOR_DIVIDE;
				pEnd++;
			}

			p = pEnd;
		}
	}

	TINFO( "Result: %d\n", iResult );
	return TTRUE;
}

// $deBlob2: FUNCTION 0080ee20
TBOOL PConsole::KeysListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	TINFO( "Available keys (%d):\n", TINT( PKeyboard::KEY_NUMOF ) );

	// NOTE: Prime prints the names one by one; TOSHI's log starts a line each time, so they're gathered in lines
	TCHAR szLine[ 256 ];
	szLine[ 0 ] = '\0';

	for ( TINT i = 0; i < PKeyboard::KEY_NUMOF; i++ )
	{
		PString::Concat( szLine, sizeof( szLine ), PKeyboard::GetKeyName( i ) );
		PString::Concat( szLine, sizeof( szLine ), " " );

		if ( i != 0 && i % 9 == 0 )
		{
			TINFO( "%s\n", szLine );
			szLine[ 0 ] = '\0';
		}
	}

	TINFO( "%s\n", szLine );
	return TTRUE;
}

//-----------------------------------------------------------------------------
// Strings
//-----------------------------------------------------------------------------

// $deBlob2: FUNCTION 0057dfbd
TBOOL PConsole::ParseArguments( TCHAR* a_szString, TCHAR** a_ppArgs, TINT a_iMaxArgs, TINT* a_pNumArgs )
{
	*a_pNumArgs = 0;

	while ( *a_szString != '\0' )
	{
		while ( *a_szString == ' ' || *a_szString == '\t' || *a_szString == '\n' )
			a_szString++;

		if ( *a_szString == '\0' )
			return TTRUE;

		if ( *a_szString == '"' )
		{
			a_szString++;

			if ( *a_pNumArgs == a_iMaxArgs )
				return TFALSE;

			a_ppArgs[ *a_pNumArgs ] = a_szString;
			( *a_pNumArgs )++;

			while ( *a_szString != '\0' && *a_szString != '"' )
				a_szString++;

			if ( *a_szString != '"' )
				return TFALSE;

			*a_szString++ = '\0';

			if ( *a_szString == '\0' )
				return TTRUE;
		}
		else
		{
			if ( *a_pNumArgs == a_iMaxArgs )
				return TFALSE;

			a_ppArgs[ *a_pNumArgs ] = a_szString;
			( *a_pNumArgs )++;

			if ( *a_szString == '\0' )
				return TTRUE;

			while ( *a_szString != ' ' && *a_szString != '\t' && *a_szString != '\n' )
			{
				a_szString++;

				if ( *a_szString == '\0' )
					break;
			}

			if ( *a_szString == '\0' )
				return TTRUE;

			if ( *a_szString != ' ' && *a_szString != '\t' && *a_szString != '\n' )
				return TFALSE;

			*a_szString++ = '\0';
		}
	}

	return TTRUE;
}

// $deBlob2: FUNCTION 0057be97
// $deBlob2: FUNCTION 0057b96b
TUINT32 PConsole::HashString( const TCHAR* a_szString )
{
	static TUINT32 s_aCRC32Table[ 256 ];
	static TBOOL   s_bTable = TFALSE;

	if ( !s_bTable )
	{
		for ( TUINT32 i = 0; i < 256; i++ )
		{
			TUINT32 uiCRC = i;

			for ( TINT j = 0; j < 8; j++ )
				uiCRC = ( uiCRC & 1 ) ? ( uiCRC >> 1 ) ^ 0xEDB88320 : uiCRC >> 1;

			s_aCRC32Table[ i ] = uiCRC;
		}

		s_bTable = TTRUE;
	}

	TUINT32 uiCRC = 0xFFFFFFFF;

	if ( a_szString )
	{
		TUINT8 c;
		do
		{
			c     = TUINT8( *a_szString );
			uiCRC = ( uiCRC >> 8 ) ^ s_aCRC32Table[ TUINT8( uiCRC ) ^ c ];
			a_szString++;
		} while ( c != 0 );
	}

	return ~uiCRC;
}

#endif // PRIMEDEBUG
