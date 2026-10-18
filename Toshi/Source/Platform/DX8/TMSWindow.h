#pragma once
#include "Toshi/TObject.h"
#include "Render/TRenderInterface.h"
#include "Toshi/TString8.h"

#include <dbt.h>
#include <windowsx.h>

TOSHI_NAMESPACE_START

class TMSWindow : public TObject
{
public:
	TDECLARE_CLASS( TMSWindow, TObject );

public:
	static constexpr UINT s_PopupStyles = WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

public:
	TMSWindow();
	~TMSWindow();
	TMSWindow( const TMSWindow& ) = default;

	void Update( TFLOAT a_fDeltaTime );

	TBOOL Create( TRenderInterface* renderer, LPCSTR title );
	void  UnregisterWindowClass();

	void SetPosition( UINT x, UINT y, UINT width, UINT height );

	void SetWindowed()
	{
		TASSERT( GetHWND() != TNULL );
		m_bWindowed = TTRUE;
	}

	void SetFullscreen()
	{
		TASSERT( GetHWND() != TNULL );
		m_bWindowed = TFALSE;
	}

	TBOOL IsDestroyed() const
	{
		return m_bDestroyed;
	}

	TBOOL IsWindowed() const
	{
		return m_bWindowed;
	}

	HWND GetHWND() const
	{
		return m_HWND;
	}

	static void SetFocused( TBOOL a_bFocused );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-background" keeps the window of unattended test runs out of the user's way.
	// The window is placed outside of the desktop, never activated and not shown in the taskbar,
	// and the game keeps running (no pause) while it isn't the active application.
	static TBOOL IsBackgroundMode();

	// Development: "-nopause" keeps a visible game running while it isn't the active application (watched test runs)
	static TBOOL IsNoPauseMode();

	// Development: while set, a windowed game keeps running when it isn't the active application (the game sets it
	// while one-shot dev switches such as "-devcheat" wait for the sim). Clearing it while the game is inactive
	// pauses the game as the deactivation would have done.
	static void Dev_SetKeepRunning( TBOOL a_bKeepRunning );

private:
	inline static TBOOL ms_bDevKeepRunning  = TFALSE;
	inline static TBOOL ms_bDevSkippedPause = TFALSE; // deactivated while ms_bDevKeepRunning was set

public:
#endif

private:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
	static constexpr GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 0x8a, 0x6d, 0xb5, 0x4c, 0x2b, 0x4f, 0xc8, 0x35 };

	inline static TBOOL      ms_bIsFocused = TTRUE; // 0x0077d188
	inline static BOOL       ms_bIsFullscreen;
	inline static STICKYKEYS ms_StickyKeys;
	inline static HDEVNOTIFY ms_hDeviceNotify;

private:
	HWND              m_HWND;         // 0x04
	TRenderInterface* m_pRenderer;    // 0x08
	TBOOL             m_bWindowed;    // 0x0C
	TBOOL             m_bDestroyed;   // 0x0D
	TBOOL             m_bIsFocused;   // 0x0E
	HMODULE           m_ModuleHandle; // 0x10
};

TOSHI_NAMESPACE_END
