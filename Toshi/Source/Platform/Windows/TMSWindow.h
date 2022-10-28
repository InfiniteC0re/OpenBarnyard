#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Strings/TString8.h"

#include <dbt.h>
#include <windowsx.h>
#include <resource.h>

namespace Toshi
{
	class TMSWindow :
		public TGenericClassDerived<TMSWindow, TObject, "TMSWindow", TMAKEVERSION(1, 0), false>
	{
	public:
		static constexpr GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72,
					  0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };



		HWND m_pHwnd; // 0x4
		TRenderInterface* m_pRenderer; // Name is a guess
		TBOOL m_bIsEnabled; // this + 0xC ; Name is a guess
		TBOOL m_bIsWindowed; // 0xD Possibly?
		HMODULE hmodule;
		TBOOL m_bUnk; // 0x14
		int m_xPos; // 0x18
		int m_yPos; // 0x1C
		TBOOL m_bUnk2; // 0x20

		static inline HDEVNOTIFY ms_hDeviceNotify = NULL;
		static inline bool ms_bUnkStatic = false;
		static inline STICKYKEYS ms_StickyKeys = {0,0};
		
	public:
		void Enable();
		void Disable();
		bool Create(TRenderInterface* renderer, const char* param_2);

		inline bool IsWindowed() { return m_bIsWindowed; }
		inline bool IsEnabled() { return m_bIsEnabled; } // Not confirmed

		static LRESULT CALLBACK WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		
		inline HWND GetHWND() const { return m_pHwnd; }
		
	};
}


