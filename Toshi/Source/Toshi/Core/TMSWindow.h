#pragma once
#include "TObject.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TString8.h"

namespace Toshi
{
	class TMSWindow :
		public TGenericClassDerived<TMSWindow, TObject, "TMSWindow", TMAKEVERSION(1, 0), false>
	{
		HWND* hwnd; // 0x4
		TRenderInterface* m_pRenderer; // Name is a guess
		TBOOL m_bIsEnabled; // this + 0xC ; Name is a guess
		TBOOL m_bIsWindowed; // 0xD Possibly?
		HMODULE hmodule;

		void Enable();
		void Disable();
		void Create(TRenderInterface* renderer, char* param_2);
		
		inline HWND* GetHWND() const { return hwnd; }
		void Update();
	};
}


