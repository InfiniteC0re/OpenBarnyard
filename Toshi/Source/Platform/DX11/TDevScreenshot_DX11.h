#pragma once

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )

#  include <d3d11.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Development: in-game screenshots for unattended test runs (not in Final builds).
//  -devshot <prefix>         enables it; every shot is saved as <prefix>_<seconds>.png
//                            (quote the prefix if it contains spaces)
//  -devshotat <s1,s2,...>    seconds after the process started at which to take the shots
//  -devshotevery <seconds>   or take one every N seconds (default: every 15 seconds)
// The finished frame is read back from the back buffer after the scene has ended and before
// it is presented, so it includes the GUI/HUD and works when the window isn't visible.
// The PNG files are written with WIC.
//-----------------------------------------------------------------------------
class TDevScreenshot
{
public:
	// Called once per frame by TRenderD3DInterface::EndScene, right before Present
	static void Update( ID3D11Device* a_pDevice, ID3D11DeviceContext* a_pContext, ID3D11Texture2D* a_pBackBuffer );

	// Saves the texture as a PNG file
	static TBOOL Capture( ID3D11Device* a_pDevice, ID3D11DeviceContext* a_pContext, ID3D11Texture2D* a_pTexture, const TCHAR* a_szFileName );
};

TOSHI_NAMESPACE_END

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
