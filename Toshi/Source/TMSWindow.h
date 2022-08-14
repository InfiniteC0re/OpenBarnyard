#pragma once
#include "Typedefs.h"
#include <Windows.h>
#include "TObject.h"
#include "TRender.h"

namespace Toshi
{
	class TMSWindow : TObject
	{
		HWND* hwnd;
		TRender* m_pRenderer; // Name is a guess
		TBOOL m_bIsEnabled; // this + 0xC ; Name is a guess
		HMODULE hmodule;

		void __thiscall Enable();
		void __thiscall Disable();
		void __thiscall Create(TRender * renderer, char * param_2)
	};
}


