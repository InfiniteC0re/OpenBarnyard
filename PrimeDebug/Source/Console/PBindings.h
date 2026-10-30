#pragma once
#include "PCFunc.h"

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Purpose: console commands bound to key combinations (Prime's crBindings):
// "bind.add <command> <keys...>", "bind.remove <keys...>", "bind.list".
// Key names are PKeyboard's ("keys.list"), not case sensitive, up to 4 keys.
// Update runs the commands whose keys are all down since this frame.
//-----------------------------------------------------------------------------
class PBindings
{
public:
	static constexpr TINT MAX_BINDINGS = 256;
	static constexpr TINT MAX_KEYS     = 4;
	static constexpr TINT COMMAND_SIZE = 0x80;

public:
	static void Startup();
	static void Shutdown();

	// Runs the commands whose keys all just went down
	static void Update();

	static TINT GetNumBindings() { return ms_iNumBindings; }

private:
	struct Binding
	{
		TCHAR szCommand[ COMMAND_SIZE ]; // 0x00
		TINT8 iNumKeys;                  // 0x80
		TINT  aKeys[ MAX_KEYS ];         // 0x84
		TBOOL bWasDown;                  // 0x94
	};

	static TBOOL AddCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL RemoveCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );
	static TBOOL ListCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc );

private:
	static TINT    ms_iNumBindings;
	static PCFunc  ms_oAddFunc;
	static PCFunc  ms_oRemoveFunc;
	static PCFunc  ms_oListFunc;
	static Binding ms_aBindings[ MAX_BINDINGS ];
};

#endif // PRIMEDEBUG_DEV
