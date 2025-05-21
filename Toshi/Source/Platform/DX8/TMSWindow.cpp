#include "ToshiPCH.h"
#include "TMSWindow.h"
#include "Toshi/TApplication.h"
#include "Toshi/TSystem.h"

#include "Platform/DX8/Input/TInputInterface_Win.h"
#include "Platform/DX8/Input/TInputDeviceMouse_Win.h"
#include "Platform/DX8/TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

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

	ms_StickyKeys.cbSize = sizeof( STICKYKEYS );
	SystemParametersInfoA( SPI_GETSTICKYKEYS, sizeof( STICKYKEYS ), &ms_StickyKeys, 0 );

	STICKYKEYS newStickyKeys = { sizeof( STICKYKEYS ) };
	newStickyKeys.dwFlags    = 0;
	SystemParametersInfoA( SPI_SETSTICKYKEYS, sizeof( STICKYKEYS ), &newStickyKeys, 0 );
}

// $Barnyard: FUNCTION 006da380
void TMSWindow::Update( TFLOAT a_fDeltaTime )
{
	MSG msg;

	if ( m_bDestroyed ) return;

	if ( !m_bIsFocused )
	{
		if ( !g_oSystemManager.IsPaused() )
		{
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

				m_bDestroyed = TTRUE;
			}

			return;
		}
		else
		{
			m_bIsFocused = TTRUE;
			LONG uStyle  = GetWindowLongA( m_HWND, GWL_STYLE );
			SetWindowLongA( m_HWND, GWL_STYLE, uStyle | WS_SYSMENU );
			return;
		}
	}

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

	LONG uStyle = GetWindowLongA( m_HWND, GWL_STYLE );
	SetWindowLongA( m_HWND, GWL_STYLE, uStyle & ( ~WS_SYSMENU ) );

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
	SetWindowPos( m_HWND, HWND_TOP, x, y, width, height, 0 );
}

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
		.cbWndExtra    = 0,
		.hInstance     = m_ModuleHandle,
		.hIcon         = NULL,
		.hCursor       = LoadCursorA( NULL, IDC_ARROW ),
		.hbrBackground = (HBRUSH)GetStockObject( 1 ),
		.lpszMenuName  = NULL,
		.lpszClassName = TGetClass( TRenderD3DInterface ).GetName()
	};

	RegisterClassA( &wndClass );
	m_HWND = CreateWindowExA(
	    0,
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
	ShowWindow( m_HWND, SW_SHOW );

	return TTRUE;
}

// $Barnyard: FUNCTION 006da620
LRESULT CALLBACK TMSWindow::WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
	static TBOOL s_bIsPaused;

	if ( uMsg <= WM_ACTIVATEAPP )
	{
		if ( uMsg == WM_ACTIVATEAPP )
		{
			TMSWindow*           pWindow   = TREINTERPRETCAST( TMSWindow*, GetWindowLongA( hWnd, GWL_USERDATA ) );
			TRenderD3DInterface* pRenderer = TREINTERPRETCAST( TRenderD3DInterface*, pWindow->m_pRenderer );

			if ( !pWindow->m_bDestroyed )
			{
				const TBOOL bPaused = !wParam;

				g_oSystemManager.Pause( bPaused );
				s_bIsPaused = bPaused;

				if ( wParam && !pWindow->m_bWindowed && pRenderer->GetDirect3DDevice() )
					pRenderer->OnD3DDeviceFound();
				else if ( !wParam && !pWindow->m_bWindowed && pRenderer->GetDirect3DDevice() )
					pRenderer->OnD3DDeviceLost();

				return 0;
			}
		}

		if ( uMsg == WM_CREATE )
		{
			SetWindowLongA( hWnd, GWL_USERDATA, *(LONG*)lParam );
			return 0;
		}

		if ( uMsg == WM_ACTIVATE )
		{
			SetFocused( ( wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE ) );

			return DefWindowProcA( hWnd, WM_ACTIVATE, wParam, lParam );
		}

		if ( uMsg == WM_QUIT )
		{
			DestroyWindow( hWnd );

			return DefWindowProcA( hWnd, WM_QUIT, wParam, lParam );
		}
	}
	else if ( uMsg == WM_SETCURSOR )
	{
		TRenderD3DInterface* pRenderer = TREINTERPRETCAST( TRenderD3DInterface*, TRenderInterface::GetSingleton() );

		pRenderer->GetDirect3DDevice()->ShowCursor( TRUE );
		TFIXME( "Use data from TMouse_D3D8_Win to set cursor properties" );
	}
	else if ( uMsg == WM_SYSCOMMAND )
	{
		if ( wParam == SC_CLOSE )
		{
			TRenderD3DInterface* pRenderer = TREINTERPRETCAST( TRenderD3DInterface*, TRenderInterface::GetSingleton() );
			pRenderer->Exit();
			TTODO( "FUN_006bbb80" );

			return 0;
		}
		else if ( wParam == SC_SCREENSAVE )
		{
			return 0;
		}
		else if ( wParam == SC_MONITORPOWER )
		{
			return 0;
		}
	}
	else if ( uMsg == WM_MOUSEFIRST )
	{
		auto pMouse = TInputInterface::GetSingletonSafe()->GetDeviceByIndex<TInputDXDeviceMouse>( 0 );
		pMouse->SetCurrentPosition( GET_X_LPARAM( lParam ), GET_Y_LPARAM( lParam ) );

		return 1;
	}

	return DefWindowProcA( hWnd, uMsg, wParam, lParam );
}

TOSHI_NAMESPACE_END
