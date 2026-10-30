#pragma once
#include "PCFunc.h"
#include "PCVar.h"

#ifdef PRIMEDEBUG

//-----------------------------------------------------------------------------
// Purpose: Prime's console (crConsole): the commands and variables, sorted by
// name, and the lines that run them. A line is a command with its arguments
// ("bind.add \"debug.overlay 1\" F1") or a variable assignment ("name value")
// or query ("name"); '#' starts a comment, arguments with spaces are quoted.
// Built-in commands: help, list_commands, list_vars, list, exit, calc,
// keys.list, save, load, set; variable exitCode.
// NOTE: Prime locks a critical section around the lists; this port runs on
// the game's thread only, so there's no lock.
//-----------------------------------------------------------------------------
class PConsole
{
public:
	enum RESULT
	{
		RESULT_OK       = 0,
		RESULT_ERROR    = 1,
		RESULT_NOTFOUND = 2,
	};

	static constexpr TINT MAX_ARGS        = 50;
	static constexpr TINT MAX_COMPLETIONS = 0x800;
	static constexpr TINT ENUM_NAME_SIZE  = 0x80;
	static constexpr TINT LIST_COLUMN     = 20;
	static constexpr TINT MAX_LINE        = 512;

	typedef void ( *EnumCallback )( const TCHAR* a_szName, void* a_pUserData );
	typedef void ( *CommandCallback )( void* a_pUserData, PCFunc* a_pFunc );
	typedef void ( *VariableCallback )( void* a_pUserData, PCVar* a_pVar );

public:
	static TBOOL Startup();
	static void  Shutdown();
	static TBOOL IsStarted() { return ms_bStartedUp; }

	// Runs one line: a command, or a variable assignment ("name value") / query ("name"); '#' starts a comment
	static TBOOL  Execute( const TCHAR* a_szLine );
	static RESULT ExecuteCommand( TCHAR** a_ppArgv, TINT a_iArgc );
	static RESULT SetVariable( TCHAR** a_ppArgv, TINT a_iArgc );

	static PCFunc* FindCommand( const TCHAR* a_szName );
	static PCVar*  FindVariable( const TCHAR* a_szName );

	// Calls a_fnCallback for the commands and variables starting with a_szPrefix (with a_bFullNames TFALSE: only
	// the name up to one level below the prefix, once per distinct name); returns the number of calls
	static TINT Enumerate( const TCHAR* a_szPrefix, TINT a_iMaxResults, TBOOL a_bFullNames, EnumCallback a_fnCallback, void* a_pUserData );

	// Completes a_szPartial to the next matching name (one level), cycling through the matches
	static TBOOL AutoComplete( const TCHAR* a_szPartial, TCHAR* a_szResult );

	static void* SetContext( void* a_pContext );
	static void* GetContext() { return ms_pContext; }

	static void ForEachCommand( CommandCallback a_fnCallback, void* a_pUserData );
	static void ForEachVariable( VariableCallback a_fnCallback, void* a_pUserData );

	// Splits a_szString in place into arguments separated by spaces, tabs or new lines ("quoted" ones may have them)
	static TBOOL ParseArguments( TCHAR* a_szString, TCHAR** a_ppArgs, TINT a_iMaxArgs, TINT* a_pNumArgs );

	// Prime's hash of a string (CRC-32 of the text and its terminator)
	static TUINT32 HashString( const TCHAR* a_szString );

	// Runs every line of a file (lines starting with "//" are skipped, and the ones a_fnFilter rejects); TFALSE if it
	// can't be read
	static TBOOL ExecuteFile( const TCHAR* a_szFileName, TBOOL ( *a_fnFilter )( const TCHAR* a_szLine ) = TNULL );

private:
	friend class PCVar;
	friend class PCFunc;

	static void    PrintVariable( PCVar* a_pVar );
	static PCFunc* FindCommandUnlocked( const TCHAR* a_szName );
	static PCVar*  FindVariableUnlocked( const TCHAR* a_szName );
	static void    Error( const TCHAR* a_szFormat, ... );
	static void    AddCommand( PCFunc* a_pFunc );
	static TBOOL   HasCommand( PCFunc* a_pFunc );
	static void    AddVariable( PCVar* a_pVar );
	static TBOOL   HasVariable( PCVar* a_pVar );
	static void    ListCommands( const TCHAR* a_szPrefix );
	static void    ListVars( const TCHAR* a_szPrefix );
	static void    GetNameLevels( const TCHAR* a_szName, TINT a_iNumLevels, TCHAR* a_szResult );
	static void    CollectCompletion( const TCHAR* a_szName, void* a_pUserData );

	static TBOOL ExitCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL HelpCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ListCommandsCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ListVarsCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL LoadCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL SaveCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL SetCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL CalcCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL KeysListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );

private:
	static TBOOL        ms_bStartedUp;
	static void*        ms_pContext;
	static TINT         ms_iNumCompletions;
	static PCVar        ms_oExitCodeVar;
	static PCFunc*      ms_pCommands;
	static PCVar*       ms_pVariables;
	static PCFunc       ms_oHelpFunc;
	static PCFunc       ms_oListCommandsFunc;
	static PCFunc       ms_oListVarsFunc;
	static PCFunc       ms_oListFunc;
	static PCFunc       ms_oExitFunc;
	static PCFunc       ms_oCalcFunc;
	static PCFunc       ms_oKeysListFunc;
	static PCFunc       ms_oSaveFunc;
	static PCFunc       ms_oLoadFunc;
	static PCFunc       ms_oSetFunc;
	static const TCHAR* ms_apCompletions[ MAX_COMPLETIONS ];
	static TCHAR        ms_aCompletionNames[ MAX_COMPLETIONS ][ ENUM_NAME_SIZE ];
};

#endif // PRIMEDEBUG
