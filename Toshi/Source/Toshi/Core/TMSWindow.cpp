#include "pch.h"
#include "Core.h"
#include "TMSWindow.h"

void __thiscall Toshi::TMSWindow::Enable()
{
	TASSERT(hwnd != TNULL, "HWND shouldn't be TNULL");
	m_bIsEnabled = 1;
}

void __thiscall Toshi::TMSWindow::Disable()
{
	TASSERT(hwnd != TNULL, "HWND shouldn't be TNULL");
	m_bIsEnabled = 0;
}

void __thiscall Toshi::TMSWindow::Create(TRender* renderer, char* param_2)
{
	WNDCLASSA wndClass;

	HMODULE moduleHandle = GetModuleHandleA(0x0);
	hmodule = moduleHandle;
	//Destory()
	m_pRenderer = renderer;
	LoadIconA(0x0, (LPCSTR)0x7F00);
	//TRender::m_sClass.GetName();
}