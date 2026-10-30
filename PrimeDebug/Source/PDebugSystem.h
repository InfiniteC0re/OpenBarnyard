#pragma once
#include "PrimeDebug.h"

#ifdef PRIMEDEBUG
#  include <Math/TVector4.h>

class PDebugRenderer;

//-----------------------------------------------------------------------------
// Purpose: starts, updates and draws the debug tools in the order Prime's
// application does (gfApplication and its test bed). The game owns the
// renderer and the key states (PKeyboard).
//-----------------------------------------------------------------------------
class PDebugSystem
{
public:
	static constexpr TUINT MAX_TEXT_VERTICES = 0x10000 * 6;

public:
	// The console, and outside Final builds the development tools drawing with a_pRenderer
	static void Startup( PDebugRenderer* a_pRenderer );
	static void Shutdown();

#  ifdef PRIMEDEBUG_DEV
	// The bindings and the menu (reading PKeyboard), the end of the frame of the keys and the profile counters
	static void Update( TFLOAT a_fDeltaTime );

	// The frame stats (in seconds)
	static void UpdateStats( TFLOAT a_fCpuTime, TFLOAT a_fFrameTime );

	// The primitives in the world
	static void Render3D();

	// The screen lines, the windows (a_pEye: the camera, for the windows in the world), the menu, the frame stats when
	// debug.stats is on and the debug text queued in the frame
	static void Render2D( const Toshi::TVector4* a_pEye );
#  endif
};

#endif // PRIMEDEBUG
