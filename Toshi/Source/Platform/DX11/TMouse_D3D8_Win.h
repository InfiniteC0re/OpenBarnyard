#pragma once
#include "Toshi/TObject.h"
#include "Toshi/TSingleton.h"

TOSHI_NAMESPACE_START

// Mouse cursor of the window, a Direct3D device cursor on D3D8.
// TMSWindow::WndProc restores its cursor image on WM_SETCURSOR.
class TMouse_D3D8_Win
    : public TObject
    , public TSingleton<TMouse_D3D8_Win>
{
public:
	TDECLARE_CLASS( TMouse_D3D8_Win, TObject );

public:
	TMouse_D3D8_Win()
	    : m_hCursor( NULL ), m_bUnk( TFALSE )
	{
	}

	~TMouse_D3D8_Win();

	// Loads the cursor image (32x32) from a_wszFileName and makes it the cursor of the window
	virtual TBOOL SetCursor( const TWCHAR* a_wszFileName );

	HCURSOR GetCursor() const { return m_hCursor; }

private:
	HCURSOR m_hCursor;
	TBOOL   m_bUnk; // never used
};

TOSHI_NAMESPACE_END
