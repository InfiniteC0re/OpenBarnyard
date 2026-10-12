#pragma once
#include "Toshi/TObject.h"
#include "Toshi/TSingleton.h"

#include <d3d8.h>

TOSHI_NAMESPACE_START

// Hardware mouse cursor drawn by the Direct3D device.
// TMSWindow::WndProc restores its cursor image on WM_SETCURSOR.
class TMouse_D3D8_Win
    : public TObject
    , public TSingleton<TMouse_D3D8_Win>
{
public:
	TDECLARE_CLASS( TMouse_D3D8_Win, TObject );

public:
	TMouse_D3D8_Win()
	    : m_pCursorSurface( TNULL ), m_bUnk( TFALSE )
	{
	}

	~TMouse_D3D8_Win();

	// Loads the cursor image (32x32) from a_wszFileName and makes it the cursor of the device
	virtual TBOOL SetCursor( const TWCHAR* a_wszFileName ); // +0x0c

	IDirect3DSurface8* GetCursorSurface() const { return m_pCursorSurface; }

private:
	IDirect3DSurface8* m_pCursorSurface; // 0x04
	TBOOL              m_bUnk;           // 0x08, never used
	TUINT8             PADDING[ 8 ];     // 0x0c
};

TOSHI_NAMESPACE_END
