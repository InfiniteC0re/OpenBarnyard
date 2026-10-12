#include "ToshiPCH.h"
#include "TMSWindow.h"
#include "Toshi/TApplication.h"
#include "Toshi/TSystem.h"

#include "Platform/DX8/Input/TInputInterface_Win.h"
#include "Platform/DX8/Input/TInputDeviceMouse_Win.h"
#include "Platform/DX8/TRenderInterface_DX8.h"
#include "Platform/DX8/TMouse_D3D8_Win.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: GetClass 006da250, CreateObject 006da5e0, CreateObjectInPlace 006da600
TDEFINE_CLASS( TMSWindow );

// $Barnyard: FUNCTION 006da520
TMSWindow::TMSWindow()
{
	m_HWND       = NULL;
	m_pRenderer  = TNULL;
	m_bWindowed  = TTRUE;
	m_bDestroyed = TFALSE;
	m_bIsFocused = TFALSE;

	TString8 rendererName = TGetClass( TRenderD3DInterface ).GetName();
	UnregisterWindowClass();

#ifdef BARNYARD_COMMUNITY_PATCH
	// Background test runs don't touch the user's sticky keys setting (several runs at once would mix it up)
	if ( IsBackgroundMode() )
	{
		ms_bIsFocused = TFALSE; // nothing to restore in UnregisterWindowClass
		return;
	}
#endif

	ms_StickyKeys.cbSize = sizeof( STICKYKEYS );
	SystemParametersInfoA( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );

	STICKYKEYS newStickyKeys = { sizeof( STICKYKEYS ) };
	newStickyKeys.dwFlags    = 0;
	SystemParametersInfoA( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &newStickyKeys, 0 );
}

// $Barnyard: FUNCTION 006da2e0
// $Barnyard: FUNCTION 006da5b0
TMSWindow::~TMSWindow()
{
}

// $Barnyard: FUNCTION 006da380
void TMSWindow::Update( TFLOAT a_fDeltaTime )
{
	MSG msg;

	if ( m_bDestroyed ) return;

	// NOTE: m_bIsFocused is set while the system is paused (the window is inactive)
	if ( !m_bIsFocused )
	{
		if ( g_oSystemManager.IsPaused() )
		{
			// Got paused, allow the system menu while the game isn't running
			m_bIsFocused = TTRUE;
			LONG uStyle  = GetWindowLongA( m_HWND, GWL_STYLE );
			SetWindowLongA( m_HWND, GWL_STYLE, uStyle | WS_SYSMENU );
			return;
		}
	}
	else
	{
		// Wait for the system to be unpaused
		while ( g_oSystemManager.IsPaused() )
		{
			if ( FALSE == GetMessageA( &msg, NULL, 0, 0 ) )
			{
				m_bDestroyed = TTRUE;
				m_bIsFocused = TFALSE;
				return;
			}

			TranslateMessage( &msg );
			DispatchMessageA( &msg );
		}

		m_bIsFocused = TFALSE;
		LONG uStyle  = GetWindowLongA( m_HWND, GWL_STYLE );
		SetWindowLongA( m_HWND, GWL_STYLE, uStyle & ( ~WS_SYSMENU ) );
	}

	if ( FALSE != PeekMessageA( &msg, NULL, 0, 0, 0 ) )
	{
		while ( FALSE != GetMessageA( &msg, NULL, 0, 0 ) )
		{
			TranslateMessage( &msg );
			DispatchMessageA( &msg );

			if ( FALSE == PeekMessageA( &msg, NULL, 0, 0, 0 ) )
			{
				return;
			}
		}

		// WM_QUIT
		m_bDestroyed = TTRUE;
	}
}

// $Barnyard: FUNCTION 006da2f0
void TMSWindow::UnregisterWindowClass()
{
	if ( m_HWND != NULL )
	{
		if ( ms_bIsFocused )
		{
			ms_bIsFocused = TFALSE;
			SystemParametersInfoA( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );
		}

		DestroyWindow( m_HWND );
		m_bDestroyed = TTRUE;
		m_HWND       = NULL;
	}

	if ( m_pRenderer != TNULL )
	{
		UnregisterClassA( TGetClass( TRenderD3DInterface ).GetName(), m_ModuleHandle );
		m_pRenderer = TNULL;
	}
}

// $Barnyard: FUNCTION 006da260
void TMSWindow::SetFocused( TBOOL a_bFocused )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	if ( IsBackgroundMode() ) return;
#endif

	if ( ms_bIsFocused != a_bFocused )
	{
		ms_bIsFocused = a_bFocused;

		if ( ms_bIsFocused )
		{
			SystemParametersInfoA( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );

			STICKYKEYS newStickyKeys = { sizeof( STICKYKEYS ) };
			newStickyKeys.dwFlags    = 0;
			SystemParametersInfoA( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &newStickyKeys, 0 );
		}
		else
		{
			SystemParametersInfoA( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );
		}
	}
}

// $Barnyard: FUNCTION 006da4d0
void TMSWindow::SetPosition( UINT x, UINT y, UINT width, UINT height )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	if ( IsBackgroundMode() )
	{
		// Above and to the left of the whole virtual desktop (all monitors), so it never covers anything
		const TINT iX = GetSystemMetrics( SM_XVIRTUALSCREEN ) - TINT( width ) - 256;
		const TINT iY = GetSystemMetrics( SM_YVIRTUALSCREEN ) - TINT( height ) - 256;
		SetWindowPos( m_HWND, HWND_BOTTOM, iX, iY, width, height, SWP_NOACTIVATE );
		return;
	}
#endif

	SetWindowPos( m_HWND, HWND_TOP, x, y, width, height, 0 );
}

#ifdef BARNYARD_COMMUNITY_PATCH

TBOOL TMSWindow::IsBackgroundMode()
{
	static const TBOOL s_bBackground = T2String8::FindString( GetCommandLineA(), "-background" ) != TNULL;
	return s_bBackground;
}

TBOOL TMSWindow::IsNoPauseMode()
{
	static const TBOOL s_bNoPause = T2String8::FindString( GetCommandLineA(), "-nopause" ) != TNULL;
	return s_bNoPause;
}

#endif // BARNYARD_COMMUNITY_PATCH

// $Barnyard: FUNCTION 006da8a0
TBOOL TMSWindow::Create( TRenderInterface* renderer, LPCSTR title )
{
	m_ModuleHandle = GetModuleHandleA( NULL );
	UnregisterWindowClass();

	m_pRenderer = renderer;

	WNDCLASSA wndClass = {
		.style         = 3,
		.lpfnWndProc   = WndProc,
		.cbClsExtra    = 0,
		.cbWndExtra    = 4,
		.hInstance     = m_ModuleHandle,
		.hIcon         = NULL,
		.hCursor       = LoadCursorA( NULL, IDC_ARROW ),
		.hbrBackground = (HBRUSH)GetStockObject( 1 ),
		.lpszMenuName  = NULL,
		.lpszClassName = TGetClass( TRenderD3DInterface ).GetName()
	};

	RegisterClassA( &wndClass );

	DWORD uiExStyle = 0;

#ifdef BARNYARD_COMMUNITY_PATCH
	// Not in the taskbar/alt-tab and never activated by clicks
	if ( IsBackgroundMode() ) uiExStyle = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;
#endif

	m_HWND = CreateWindowExA(
	    uiExStyle,
	    TGetClass( TRenderD3DInterface ).GetName(),
	    title,
	    0x86c00000,
	    100,
	    100,
	    0,
	    0,
	    NULL,
	    NULL,
	    m_ModuleHandle,
	    this
	);

	if ( !m_HWND ) return TFALSE;

	EnableWindow( m_HWND, TRUE );

#ifdef BARNYARD_COMMUNITY_PATCH
	if ( IsBackgroundMode() )
	{
		// Move it out of the desktop before it's shown, and don't take the focus
		SetWindowPos( m_HWND, HWND_BOTTOM, GetSystemMetrics( SM_XVIRTUALSCREEN ) - 4096, GetSystemMetrics( SM_YVIRTUALSCREEN ) - 4096, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
		ShowWindow( m_HWND, SW_SHOWNOACTIVATE );
		return TTRUE;
	}
#endif

	ShowWindow( m_HWND, SW_SHOW );

	return TTRUE;
}

// $Barnyard: FUNCTION 006da620
LRESULT CALLBACK TMSWindow::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static TBOOL s_bIsPaused;

	switch ( uMsg )
	{
		case WM_CREATE:
			SetWindowLongA( hWnd, GWL_USERDATA, *(LONG*)lParam );
			return 0;

		case WM_ACTIVATE:
			SetFocused( wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE );
			return DefWindowProcA( hWnd, WM_ACTIVATE, wParam, lParam );

		case WM_QUIT:
			DestroyWindow( hWnd );
			return DefWindowProcA( hWnd, WM_QUIT, wParam, lParam );

		case WM_ACTIVATEAPP:
		{
			TMSWindow*           pWindow   = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );
			TRenderD3DInterface* pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );

			if ( pWindow->m_bDestroyed )
				return 0;

#ifdef BARNYARD_COMMUNITY_PATCH
			// Background test runs keep running while another application is active
			if ( IsBackgroundMode() || IsNoPauseMode() ) return 0;
#endif

			const TBOOL bPaused = wParam != TRUE;
			g_oSystemManager.Pause( bPaused );
			s_bIsPaused = bPaused;

			// The device of a fullscreen window is lost while the game isn't active
			if ( !pWindow->m_bWindowed && pRenderer->GetDirect3DDevice() )
			{
				if ( bPaused )
					pRenderer->OnD3DDeviceLost();
				else
					pRenderer->OnD3DDeviceFound();
			}

			return 0;
		}

		case WM_SETCURSOR:
		{
			TRenderD3DInterface* pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
			pRenderer->GetDirect3DDevice()->ShowCursor( TRUE );

			if ( TMouse_D3D8_Win* pMouse = TMouse_D3D8_Win::GetSingleton() )
				pRenderer->GetDirect3DDevice()->SetCursorProperties( 0, 0, pMouse->GetCursorSurface() );

			return DefWindowProcA( hWnd, uMsg, wParam, lParam );
		}

		case WM_SYSCOMMAND:
			if ( wParam == SC_CLOSE )
			{
				TRenderD3DInterface* pRenderer = TSTATICCAST( TRenderD3DInterface, TRenderInterface::GetSingleton() );
				pRenderer->Exit();

				g_oSystemManager.Destroy();
				return 0;
			}

			if ( wParam == SC_SCREENSAVE || wParam == SC_MONITORPOWER )
				return 0;

			return DefWindowProcA( hWnd, uMsg, wParam, lParam );

		case WM_MOUSEMOVE:
		{
			auto pMouse = TInputInterface::GetSingletonSafe()->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
			pMouse->SetCurrentPosition( LOWORD( lParam ), HIWORD( lParam ) );
			return 1;
		}
	}

	return DefWindowProcA( hWnd, uMsg, wParam, lParam );
}

TOSHI_NAMESPACE_END
