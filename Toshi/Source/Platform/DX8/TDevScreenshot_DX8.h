#pragma once

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )

#  include <d3d8.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Development: in-game screenshots for unattended test runs (not in Final builds).
//
//  -devshot <prefix>         enables it; every shot is saved as <prefix>_<seconds>.png
//                            (quote the prefix if it contains spaces)
//  -devshotat <s1,s2,...>    seconds after the process started at which to take the shots
//  -devshotevery <seconds>   or take one every N seconds (default: every 15 seconds)
//
// The frame is read back from the back buffer after the scene has ended and before
// it is presented, so it includes the GUI/HUD and works when the window isn't visible.
//-----------------------------------------------------------------------------
class TDevScreenshot
{
public:
	// Called once per frame by TRenderD3DInterface::EndScene, right before Present
	static void Update( IDirect3DDevice8* a_pDevice );

	// Saves the current back buffer as a PNG file
	static TBOOL Capture( IDirect3DDevice8* a_pDevice, const TCHAR* a_szFileName );
};

TOSHI_NAMESPACE_END

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
