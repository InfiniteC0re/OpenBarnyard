#pragma once
#include <PrimeDebug.h>

#ifdef PRIMEDEBUG

class APrimeDebug
{
public:
	// Before the game systems (AApplication::OnCreate): the console and the settings read while the systems are created,
	// set from the dev config and the command line ("set" lines of the settings only)
	static void CreateConsole();

	// After the renderer and its development singletons (ADebugOverlay, AWorldDebugDraw)
	static void Create();
	static void Destroy();

#  ifdef PRIMEDEBUG_DEV
	// Once a frame, after the input events (AInputHandler)
	static void Update( TFLOAT a_fDeltaTime );

	// In the main pass of the world (with its depth), after AWorldDebugDraw
	static void Render3D();

	// Over everything, after the GUI and the debug overlay
	static void Render2D();

	// The averaged frame time and its frames a second (two second intervals), TFALSE until the first interval
	static TBOOL GetFrameStats( TFLOAT& a_rfMilliseconds, TFLOAT& a_rfFPS );

	// editor.network_save_path: where the network editor saves (empty: nowhere)
	static const TCHAR* GetNetworkSavePath();
#  endif

private:
	// Runs the console lines of the command line (only the ones a_fnFilter accepts if given)
	static void RunCommandLine( TBOOL ( *a_fnFilter )( const TCHAR* a_szLine ) );
	static void RegisterSettings();
	static void UnregisterSettings();

#  ifdef PRIMEDEBUG_DEV
	static void RegisterGame();
	static void UnregisterGame();
	static void ReadKeyboard();
	static void SyncVars();
#  endif
};

#endif // PRIMEDEBUG
