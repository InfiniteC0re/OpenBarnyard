#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Strings/TString8.h"

#include <dbt.h>
#include <windowsx.h>
#include "../resource.h"

namespace Toshi
{
	class TMSWindow :
		public TGenericClassDerived<TMSWindow, TObject, "TMSWindow", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		static constexpr UINT s_PopupStyles = WS_POPUP | WS_CAPTION | WS_MINIMIZEBOX | WS_SYSMENU;

		using t_WndProcCallback = LRESULT(*)(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	public:
		TMSWindow() = default;
		TMSWindow(const TMSWindow&) = default;

		void Enable();
		void Disable();
		void Update();

		TBOOL Create(TRender* renderer, LPCSTR title);
		void UnregisterWindowClass();

		void SetPosition(UINT x, UINT y, UINT width, UINT height);

		void SetWindowed()
		{
			TASSERT(GetHWND() != TNULL);
			m_IsWindowed = TTRUE;
		}

		void SetFullscreen()
		{
			TASSERT(GetHWND() != TNULL);
			m_IsWindowed = TFALSE;
		}

		TBOOL IsDestroyed() const
		{
			return m_IsDestroyed;
		}

		TBOOL IsPopup() const
		{
			return m_IsPopup;
		}

		TBOOL IsWindowed() const
		{
			return m_IsWindowed;
		}

		HWND GetHWND() const
		{
			return m_HWND;
		}

	public:
		// Custom function used to send events to ImGui
		// FIXME: delete it when we can catch input using Toshi input system
		inline static t_WndProcCallback s_fnWndProcHandler = TNULL;
		
	private:
		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	protected:
		static constexpr GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72, 0x8a, 0x6d, 0xb5, 0x4c, 0x2b, 0x4f, 0xc8, 0x35 };

		static TBOOL ms_bIsFocused;
		static STICKYKEYS ms_StickyKeys;
		static HDEVNOTIFY ms_hDeviceNotify;
		static BOOL ms_bIsFullscreen;

	private:
		HWND m_HWND;                // 0x04
		TRender* m_Render;          // 0x08
		TBOOL m_IsWindowed;         // 0x0C
		TBOOL m_IsDestroyed;        // 0x0D
		TBOOL m_bIsFocused;         // 0x0E
		HMODULE m_ModuleHandle;     // 0x10
		TBOOL m_Flag2;              // 0x14
	public:
		int m_xPos;                 // 0x18
		int m_yPos;                 // 0x1C
	private:
		TBOOL m_Flag3;              // 0x20
		TBOOL m_IsPopup;            // 0x21
		TBOOL m_Flag5;              // 0x22
	};
}


