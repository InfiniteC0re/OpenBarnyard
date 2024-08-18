#pragma once
#include "Toshi/TObject.h"
#include "Render/TRenderInterface.h"
#include "Toshi/TString8.h"

#include <dbt.h>
#include <windowsx.h>
#include "../resource.h"

TOSHI_NAMESPACE_START

class TMSWindow : public TObject
{
public:
	TDECLARE_CLASS( TMSWindow, TObject );

public:
	static constexpr UINT s_PopupStyles = WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

public:
	TMSWindow();
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

private:
	static LRESULT CALLBACK WndProc( HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam );

protected:
	static constexpr GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 0x8a, 0x6d, 0xb5, 0x4c, 0x2b, 0x4f, 0xc8, 0x35 };

	inline static TBOOL      ms_bIsFocused;
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
