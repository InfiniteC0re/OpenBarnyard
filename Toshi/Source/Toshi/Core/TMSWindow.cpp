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
	
	const char* name = m_pRenderer->Class()->GetName();
	//TRender::m_sClass.GetName();
	TCString str = TCString(name);
	wndClass.hIcon = LoadIconA(0x0, (LPCSTR)IDI_APPLICATION);

	RegisterClassA(&wndClass);
	CreateWindowExA(0, name, param_2, 0x80ca0000, 100, 100, 0, 0, TNULL, TNULL, hmodule, this);
}