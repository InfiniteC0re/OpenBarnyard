#pragma once
#include "TObject.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TString8.h"
#include <dbt.h>
#include <windowsx.h>

namespace Toshi
{
	class TMSWindow :
		public TGenericClassDerived<TMSWindow, TObject, "TMSWindow", TMAKEVERSION(1, 0), false>
	{

		static constexpr GUID WceusbshGUID = { 0x25dbce51, 0x6c8f, 0x4a72,
					  0x8a,0x6d,0xb5,0x4c,0x2b,0x4f,0xc8,0x35 };



		HWND* m_pHwnd; // 0x4
		TRenderInterface* m_pRenderer; // Name is a guess
		TBOOL m_bIsEnabled; // this + 0xC ; Name is a guess
		TBOOL m_bIsWindowed; // 0xD Possibly?
		HMODULE hmodule;
		TBOOL m_bUnk; // 0x14
		int m_xPos; // 0x18
		int m_yPos; // 0x1C
		TBOOL m_bUnk2; // 0x20

		static HDEVNOTIFY ms_hDeviceNotify;
		static bool ms_bUnkStatic;
		static STICKYKEYS ms_StickyKeys;
		
	public:
		void Enable();
		void Disable();
		void Create(TRenderInterface* renderer, char* param_2);

		inline bool IsWindowed() { return m_bIsWindowed; }
		inline bool IsEnabled() { return m_bIsEnabled; } // Not confirmed

		static LRESULT WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
		
		inline HWND* GetHWND() const { return m_pHwnd; }
		void Update();
	};
}


