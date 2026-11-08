#include "ToshiPCH.h"
#include "TMSWindow.h"
#include "Toshi/TApplication.h"
#include "Toshi/TSystem.h"

#include "Platform/Windows/Input/TInputInterface_Win.h"
#include "Platform/Windows/Input/TInputDeviceMouse_Win.h"
#include "Platform/GL/TRenderInterface_GL.h"
#include "Platform/GL/TMouse_GL.h"
#include <GL/wglew.h>

#include <dbt.h>
#include <windowsx.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TMSWindow );

TMSWindow::TMSWindow()
{
	m_HWND       = NULL;
	m_pRenderer  = TNULL;
	m_bWindowed  = TTRUE;
	m_bDestroyed = TFALSE;
	m_bIsFocused = TFALSE;
	m_hDC        = NULL;
	m_hGLRC      = NULL;

	m_bDisplayModeChanged = TFALSE;

	TString8 rendererName = TGetClass( TRenderGLInterface ).GetName();
	UnregisterWindowClass();

	// Background test runs don't touch the user's sticky keys setting (several runs at once would mix it up)
	if ( IsBackgroundMode() )
	{
		ms_bIsFocused = TFALSE; // nothing to restore in UnregisterWindowClass
		return;
	}

	ms_StickyKeys.cbSize = sizeof( STICKYKEYS );
	SystemParametersInfoA( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );

	STICKYKEYS newStickyKeys = { sizeof( STICKYKEYS ) };
	newStickyKeys.dwFlags    = 0;
	SystemParametersInfoA( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &newStickyKeys, 0 );
}

TMSWindow::~TMSWindow()
{
}

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
		UnregisterClassA( TGetClass( TRenderGLInterface ).GetName(), m_ModuleHandle );
		m_pRenderer = TNULL;
	}
}

void TMSWindow::SetFocused( TBOOL a_bFocused )
{
	if ( IsBackgroundMode() ) return;

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

void TMSWindow::SetBorderless( TBOOL a_bBorderless )
{
	const LONG uStyle    = GetWindowLongA( m_HWND, GWL_STYLE );
	const LONG uNewStyle = a_bBorderless ? ( uStyle & ~WS_CAPTION ) : ( uStyle | WS_CAPTION );

	if ( uNewStyle == uStyle )
		return;

	SetWindowLongA( m_HWND, GWL_STYLE, uNewStyle );
	SetWindowPos( m_HWND, NULL, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
}

void TMSWindow::SetPosition( UINT x, UINT y, UINT width, UINT height )
{
	if ( m_bWindowed )
	{
		RECT oRect = { 0, 0, LONG( width ), LONG( height ) };

		if ( AdjustWindowRectEx( &oRect, GetWindowLongA( m_HWND, GWL_STYLE ), FALSE, GetWindowLongA( m_HWND, GWL_EXSTYLE ) ) )
		{
			width  = UINT( oRect.right - oRect.left );
			height = UINT( oRect.bottom - oRect.top );
		}
	}

	if ( IsBackgroundMode() )
	{
		// Above and to the left of the whole virtual desktop (all monitors), so it never covers anything
		const TINT iX = GetSystemMetrics( SM_XVIRTUALSCREEN ) - TINT( width ) - 256;
		const TINT iY = GetSystemMetrics( SM_YVIRTUALSCREEN ) - TINT( height ) - 256;
		SetWindowPos( m_HWND, HWND_BOTTOM, iX, iY, width, height, SWP_NOACTIVATE );
		return;
	}

	SetWindowPos( m_HWND, HWND_TOP, x, y, width, height, 0 );
}


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

void TMSWindow::Dev_SetKeepRunning( TBOOL a_bKeepRunning )
{
	ms_bDevKeepRunning = a_bKeepRunning;

	// The game was deactivated meanwhile: pause it now, as the deactivation would have
	if ( !a_bKeepRunning && ms_bDevSkippedPause )
	{
		ms_bDevSkippedPause = TFALSE;
		g_oSystemManager.Pause( TTRUE );
	}
}


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
		.lpszClassName = TGetClass( TRenderGLInterface ).GetName()
	};

	RegisterClassA( &wndClass );

	DWORD uiExStyle = 0;

	// Not in the taskbar/alt-tab and never activated by clicks
	if ( IsBackgroundMode() ) uiExStyle = WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE;

	m_HWND = CreateWindowExA(
	    uiExStyle,
	    TGetClass( TRenderGLInterface ).GetName(),
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

	if ( IsBackgroundMode() )
	{
		// Move it out of the desktop before it's shown, and don't take the focus
		SetWindowPos( m_HWND, HWND_BOTTOM, GetSystemMetrics( SM_XVIRTUALSCREEN ) - 4096, GetSystemMetrics( SM_YVIRTUALSCREEN ) - 4096, 0, 0, SWP_NOSIZE | SWP_NOACTIVATE );
		ShowWindow( m_HWND, SW_SHOWNOACTIVATE );
		return TTRUE;
	}

	ShowWindow( m_HWND, SW_SHOW );

	return TTRUE;
}

TBOOL TMSWindow::CreateContext()
{
	m_hDC = GetDC( m_HWND );

	PIXELFORMATDESCRIPTOR oFormat = {};
	oFormat.nSize                 = sizeof( oFormat );
	oFormat.nVersion              = 1;
	oFormat.dwFlags               = PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
	oFormat.iPixelType            = PFD_TYPE_RGBA;
	oFormat.cColorBits            = 32;
	oFormat.cAlphaBits            = 8;
	oFormat.cDepthBits            = 24;
	oFormat.cStencilBits          = 8;
	oFormat.iLayerType            = PFD_MAIN_PLANE;

	if ( !SetPixelFormat( m_hDC, ChoosePixelFormat( m_hDC, &oFormat ), &oFormat ) )
		return TFALSE;

	// wglCreateContextAttribsARB can only be loaded with a context current
	HGLRC hOldContext = wglCreateContext( m_hDC );

	if ( !hOldContext || !wglMakeCurrent( m_hDC, hOldContext ) )
		return TFALSE;

	glewExperimental = GL_TRUE;

	if ( glewInit() != GLEW_OK || !WGLEW_ARB_create_context )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( hOldContext );
		return TFALSE;
	}

	const int aAttributes[] = {
		WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
		WGL_CONTEXT_MINOR_VERSION_ARB, 3,
		WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
		0
	};

	m_hGLRC = wglCreateContextAttribsARB( m_hDC, NULL, aAttributes );

	wglMakeCurrent( NULL, NULL );
	wglDeleteContext( hOldContext );

	if ( !m_hGLRC || !wglMakeCurrent( m_hDC, m_hGLRC ) )
		return TFALSE;

	// Functions of the old context can't be trusted on the new one
	glewExperimental = GL_TRUE;

	if ( glewInit() != GLEW_OK )
		return TFALSE;

	glGetError();

	if ( WGLEW_EXT_swap_control )
		wglSwapIntervalEXT( IsBackgroundMode() ? 0 : 1 );

	return TTRUE;
}

void TMSWindow::DestroyContext()
{
	if ( m_hGLRC )
	{
		wglMakeCurrent( NULL, NULL );
		wglDeleteContext( m_hGLRC );
		m_hGLRC = NULL;
	}

	if ( m_hDC )
	{
		ReleaseDC( m_HWND, m_hDC );
		m_hDC = NULL;
	}
}

void TMSWindow::SwapBuffers()
{
	::SwapBuffers( m_hDC );
}

TBOOL TMSWindow::IsExtensionSupported( const TCHAR* a_szExtension ) const
{
	return glewIsSupported( a_szExtension ) == GL_TRUE;
}

void TMSWindow::SetDisplayMode( TUINT a_uiWidth, TUINT a_uiHeight, TBOOL a_bWindowed, TBOOL a_bBorderless )
{
	RestoreDisplayMode();

	m_bWindowed = a_bWindowed;

	MONITORINFO oMonitorInfo = { sizeof( MONITORINFO ) };
	GetMonitorInfoA( MonitorFromWindow( m_HWND, MONITOR_DEFAULTTONEAREST ), &oMonitorInfo );

	const RECT& rcMonitor      = oMonitorInfo.rcMonitor;
	const TINT  iMonitorWidth  = TINT( rcMonitor.right - rcMonitor.left );
	const TINT  iMonitorHeight = TINT( rcMonitor.bottom - rcMonitor.top );

	if ( !a_bWindowed )
	{
		DEVMODEA oMode     = {};
		oMode.dmSize       = sizeof( oMode );
		oMode.dmPelsWidth  = a_uiWidth;
		oMode.dmPelsHeight = a_uiHeight;
		oMode.dmFields     = DM_PELSWIDTH | DM_PELSHEIGHT;

		m_bDisplayModeChanged = ChangeDisplaySettingsA( &oMode, CDS_FULLSCREEN ) == DISP_CHANGE_SUCCESSFUL;

		SetBorderless( TTRUE );
		SetPosition( rcMonitor.left, rcMonitor.top, a_uiWidth, a_uiHeight );
	}
	else if ( a_bBorderless )
	{
		SetBorderless( TTRUE );
		SetPosition( rcMonitor.left, rcMonitor.top, iMonitorWidth, iMonitorHeight );
	}
	else
	{
		SetBorderless( TFALSE );

		if ( IsBackgroundMode() )
			SetPosition( GetSystemMetrics( SM_XVIRTUALSCREEN ) - 4096, GetSystemMetrics( SM_YVIRTUALSCREEN ) - 4096, a_uiWidth, a_uiHeight );
		else
			SetPosition( rcMonitor.left + TMath::Max( iMonitorWidth - TINT( a_uiWidth ), 0 ) / 2, rcMonitor.top + TMath::Max( iMonitorHeight - TINT( a_uiHeight ), 0 ) / 2, a_uiWidth, a_uiHeight );
	}
}

void TMSWindow::RestoreDisplayMode()
{
	if ( m_bDisplayModeChanged )
	{
		ChangeDisplaySettingsA( NULL, 0 );
		m_bDisplayModeChanged = TFALSE;
	}
}

void TMSWindow::CenterCursor( TUINT a_uiWidth, TUINT a_uiHeight )
{
	// Background test runs leave the user's mouse cursor alone
	if ( IsBackgroundMode() )
		return;

	POINT oPoint = { TINT( a_uiWidth / 2 ), TINT( a_uiHeight / 2 ) };
	ClientToScreen( m_HWND, &oPoint );
	SetCursorPos( oPoint.x, oPoint.y );
}

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
			TMSWindow* pWindow = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );

			if ( pWindow->m_bDestroyed )
				return 0;

			// Background test runs keep running while another application is active
			if ( IsBackgroundMode() || IsNoPauseMode() ) return 0;

			// One-shot dev switches waiting for the sim keep a windowed game running (see Dev_SetKeepRunning)
			if ( pWindow->m_bWindowed )
			{
				if ( wParam != TRUE && ms_bDevKeepRunning )
				{
					ms_bDevSkippedPause = TTRUE;
					return 0;
				}

				ms_bDevSkippedPause = TFALSE;
			}

			const TBOOL bPaused = wParam != TRUE;
			g_oSystemManager.Pause( bPaused );
			s_bIsPaused = bPaused;

			if ( bPaused && !pWindow->m_bWindowed )
				ShowWindow( hWnd, SW_MINIMIZE );

			return 0;
		}

		case WM_SETCURSOR:
		{
			// NOTE: D3D8 draws its device cursor over the client area, GL has none, so the cursor of TMouse_GL is set instead
			TMouse_GL* pMouse = TMouse_GL::GetSingleton();

			if ( pMouse && pMouse->GetCursor() && LOWORD( lParam ) == HTCLIENT )
			{
				::SetCursor( pMouse->GetCursor() );
				return TRUE;
			}

			return DefWindowProcA( hWnd, uMsg, wParam, lParam );
		}

		case WM_CLOSE:
			{
				TRenderGLInterface* pRenderer = TSTATICCAST( TRenderGLInterface, TRenderInterface::GetSingleton() );
				pRenderer->Exit();

				g_oSystemManager.Destroy();
				return 0;
			}

		case WM_SYSCOMMAND:
			if ( wParam == SC_CLOSE )
			{
				TRenderGLInterface* pRenderer = TSTATICCAST( TRenderGLInterface, TRenderInterface::GetSingleton() );
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
