#pragma once
#include "Toshi/TObject.h"
#include "Toshi/TSingleton.h"

TOSHI_NAMESPACE_START

// Mouse cursor of the window, TMSWindow::WndProc restores its image on WM_SETCURSOR
class TMouse_GL
    : public TObject
    , public TSingleton<TMouse_GL>
{
public:
	TDECLARE_CLASS( TMouse_GL, TObject );

public:
	TMouse_GL()
	    : m_hCursor( NULL ), m_bUnk( TFALSE )
	{
	}

	~TMouse_GL();

	// Loads the cursor image (32x32) from a_wszFileName and makes it the cursor of the window
	virtual TBOOL SetCursor( const TWCHAR* a_wszFileName );

	HCURSOR GetCursor() const { return m_hCursor; }

private:
	HCURSOR m_hCursor;
	TBOOL   m_bUnk; // never used
};

TOSHI_NAMESPACE_END
