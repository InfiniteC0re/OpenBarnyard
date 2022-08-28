#pragma once
#include "TObject.h"
#include "TRender.h"
#include "Toshi/Typedefs.h"
#include "Toshi/Strings/TCString.h"

namespace Toshi
{
	class TMSWindow : TObject
	{
		HWND* hwnd; // 0x4
		TRender* m_pRenderer; // Name is a guess
		TBOOL m_bIsEnabled; // this + 0xC ; Name is a guess
		TBOOL m_bIsWindowed; // 0xD Possibly?
		HMODULE hmodule;

		void Enable();
		void Disable();
		void Create(TRender* renderer, char* param_2);
		
		inline HWND* GetHWND() const { return hwnd; }
		void Update();
	};
}


