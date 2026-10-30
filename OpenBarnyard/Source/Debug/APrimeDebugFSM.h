#pragma once
#include <PrimeDebug.h>

#ifdef PRIMEDEBUG_DEV

//-----------------------------------------------------------------------------
// Purpose: calls the FSM script natives from the PrimeDebug console
// (fsm.call, fsm.list) and the FSM submenu of the debug menu.
// NOTE: not in retail.
//-----------------------------------------------------------------------------
class APrimeDebugFSM
{
public:
	static void Register();
	static void Unregister();

	// Runs the queued calls
	static void Update( TFLOAT a_fDeltaTime );

	// TTRUE while the command line and the dev config run (before the first frame)
	static TBOOL IsRunningCommandLine();

	// Runs the console command a_ppArgv (a_ppArgv[ 0 ] is its name) once the player has been in the world fsm.delay
	// seconds (a_bWaitForWorld) or once the debug menu has closed
	static TBOOL QueueCommand( TINT a_iArgc, TCHAR** a_ppArgv, TBOOL a_bWaitForWorld );

	// A call or a command waits for the world
	static TBOOL HasPendingWorldCalls();
};

#endif // PRIMEDEBUG_DEV
