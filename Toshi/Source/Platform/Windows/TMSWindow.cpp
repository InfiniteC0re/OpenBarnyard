#include "ToshiPCH.h"
#include "TMSWindow.h"
#include "Toshi/Core/TApplication.h"
#include "Toshi/Core/TSystem.h"
#include "Input/TInputInterface_Win.h"

#include "Platform/DX8/TRenderInterface_DX8.h"

namespace Toshi {

	TMSWindow::TMSWindow()
	{
		m_HWND = NULL;
		m_pRenderer = TNULL;
		m_bWindowed = TTRUE;
		m_bDestroyed = TFALSE;
		m_bIsFocused = TFALSE;

		TString8 rendererName = TGetClass(TRenderD3DInterface).GetName();
		UnregisterWindowClass();

		ms_StickyKeys.cbSize = sizeof(STICKYKEYS);
		SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);

		STICKYKEYS newStickyKeys = { sizeof(STICKYKEYS) };
		newStickyKeys.dwFlags = 0;
		SystemParametersInfoA(0x3b, sizeof(STICKYKEYS), &newStickyKeys, 0);
	}

	void TMSWindow::Update(TFLOAT a_fDeltaTime)
	{
		MSG msg;

		if (!m_bDestroyed)
		{
			if (!m_bIsFocused)
			{
				if (!TSystemManager::GetSingleton()->IsPaused())
				{
					if (FALSE != PeekMessageA(&msg, NULL, 0, 0, 0))
					{
						while (FALSE != GetMessageA(&msg, NULL, 0, 0))
						{
							TranslateMessage(&msg);
							DispatchMessageA(&msg);

							if (FALSE == PeekMessageA(&msg, NULL, 0, 0, 0))
							{
								return;
							}
						}
					}
				}

				m_bIsFocused = TTRUE;
				LONG uStyle = GetWindowLongA(m_HWND, GWL_STYLE);
				SetWindowLongA(m_HWND, GWL_STYLE, uStyle | WS_SYSMENU);
				return;
			}
		}

		while (TSystemManager::GetSingleton()->IsPaused())
		{
			if (FALSE == GetMessageA(&msg, NULL, 0, 0))
			{
				m_bDestroyed = TTRUE;
				m_bIsFocused = TFALSE;
				return;
			}

			TranslateMessage(&msg);
			DispatchMessageA(&msg);
		}

		m_bIsFocused = TFALSE;

		LONG uStyle = GetWindowLongA(m_HWND, GWL_STYLE);
		SetWindowLongA(m_HWND, GWL_STYLE, uStyle & (~WS_SYSMENU));

		if (FALSE != PeekMessageA(&msg, NULL, 0, 0, 0))
		{
			while (FALSE != GetMessageA(&msg, NULL, 0, 0))
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);

				if (FALSE == PeekMessageA(&msg, NULL, 0, 0, 0))
				{
					return;
				}
			}
		}
	}

	void TMSWindow::UnregisterWindowClass()
	{
		if (m_HWND != NULL)
		{
			if (ms_bIsFocused)
			{
				ms_bIsFocused = TFALSE;
				SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);
			}

			DestroyWindow(m_HWND);
			m_bDestroyed = TTRUE;
			m_HWND = NULL;
		}

		if (m_pRenderer != TNULL)
		{
			UnregisterClassA(TGetClass(TRenderD3DInterface).GetName(), m_ModuleHandle);
			m_pRenderer = TNULL;
		}
	}

	void TMSWindow::SetPosition(UINT x, UINT y, UINT width, UINT height)
	{
		SetWindowPos(m_HWND, HWND_TOP, x, y, width, height, 0);
	}

	TBOOL TMSWindow::Create(TRenderInterface* renderer, LPCSTR title)
	{
		m_ModuleHandle = GetModuleHandleA(NULL);
		UnregisterWindowClass();

		m_pRenderer = renderer;
		
		WNDCLASSA wndClass = {
			.style = 3,
			.lpfnWndProc = WndProc,
			.cbClsExtra = 0,
			.cbWndExtra = 0,
			.hInstance = m_ModuleHandle,
			.hIcon = NULL,
			.hCursor = LoadCursorA(NULL, IDC_ARROW),
			.hbrBackground = (HBRUSH)GetStockObject(1),
			.lpszMenuName = NULL,
			.lpszClassName = TGetClass(TRenderD3DInterface).GetName()
		};

		RegisterClassA(&wndClass);
		m_HWND = CreateWindowExA(
			0,
			TGetClass(TRenderD3DInterface).GetName(),
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

		if (!m_HWND) return TFALSE;

		EnableWindow(m_HWND, TRUE);
		ShowWindow(m_HWND, SW_SHOW);

		return TTRUE;
	}

	LRESULT CALLBACK TMSWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		return DefWindowProcA(hWnd, uMsg, wParam, lParam);
	}
}