#include "ToshiPCH.h"
#include "TMSWindow.h"
#include "Toshi/Core/TSystem.h"
#include "DX11/TRender_DX11.h"
#include "Input/TInputInterface_Win.h"

namespace Toshi
{
	TBOOL TMSWindow::ms_bIsFocused = TFALSE;
	BOOL TMSWindow::ms_bIsFullscreen = TFALSE;
	STICKYKEYS TMSWindow::ms_StickyKeys = { 0, 0 };
	HDEVNOTIFY TMSWindow::ms_hDeviceNotify = { 0 };

	void TMSWindow::Enable()
	{
		TASSERT(m_HWND != TNULL, "HWND is NULL");
		m_IsWindowed = TTRUE;
	}

	void TMSWindow::Disable()
	{
		TASSERT(m_HWND != NULL, "HWND is NULL");
		m_IsWindowed = TFALSE;
	}

	void TMSWindow::Update()
	{
		MSG msg;
		if (!m_IsDestroyed && PeekMessageA(&msg, NULL, 0, 0, 0) == TRUE)
		{
			while (GetMessageA(&msg, NULL, 0, 0) != FALSE)
			{
				TranslateMessage(&msg);
				DispatchMessageA(&msg);
				if (PeekMessageA(&msg, NULL, 0, 0, 0) == FALSE)
				{
					return;
				}
			}

			m_IsDestroyed = TTRUE;
		}
	}

	void TMSWindow::UnregisterWindowClass()
	{
		if (m_HWND != NULL)
		{
			if (ms_bIsFocused)
			{
				SetThreadExecutionState(ES_CONTINUOUS);
				ShowCursor(TTRUE);
				TSystemManager::GetSingletonSafe()->Pause(TTRUE);
				SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);
				ms_bIsFocused = TFALSE;
			}

			DestroyWindow(m_HWND);
			m_IsDestroyed = TTRUE;
			m_HWND = NULL;
		}

		if (m_Render != TNULL)
		{
			UnregisterClassA(TMSWindow::GetClassStatic()->GetName(), m_ModuleHandle);
			m_Render = TNULL;
		}
	}

	void TMSWindow::SetPosition(UINT x, UINT y, UINT width, UINT height)
	{
		RECT rect;
		rect.right = width;
		rect.bottom = height;
		rect.left = 0;
		rect.top = 0;

		DWORD dwStyle = IsPopup() ? (s_PopupStyles & (~WS_MINIMIZEBOX)) : 0;
		
		AdjustWindowRect(&rect, dwStyle, FALSE);
		SetWindowPos(m_HWND, HWND_TOP, x, y, rect.right - rect.left, rect.bottom - rect.top, 0);
	}

	TBOOL TMSWindow::Create(TRender* renderer, LPCSTR title)
	{
		UnregisterWindowClass();

		m_Render = renderer;
		m_ModuleHandle = GetModuleHandleA(NULL);

		const char* className = TGetClass(TRenderDX11)->GetName();
		
		WNDCLASSA wndClass = { };
		wndClass.hIcon = LoadIconA(m_ModuleHandle, MAKEINTRESOURCEA(IDI_APPLICATION));
		wndClass.hInstance = m_ModuleHandle;
		wndClass.lpfnWndProc = (WNDPROC)WndProc;
		wndClass.lpszClassName = className;
		wndClass.style = CS_VREDRAW | CS_HREDRAW;
		wndClass.cbWndExtra = 4;
		wndClass.hCursor = LoadCursor(NULL, IDC_ARROW); 
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassA(&wndClass);

		DWORD dwStyle = IsPopup() ? s_PopupStyles : 0;
		m_HWND = CreateWindowExA(0, className, title, dwStyle, 100, 100, 0, 0, NULL, NULL, m_ModuleHandle, this);
		
		if (m_HWND == NULL)
		{
			return TFALSE;
		}

		EnableWindow(m_HWND, TRUE);
		ShowWindow(m_HWND, SW_SHOW);
		SetForegroundWindow(m_HWND);

		m_IsDestroyed = TFALSE;
		
		if (GetForegroundWindow() != m_HWND)
		{
			TOSHI_INFO("Not foreground window, Pausing Systems!\n");
			TSystemManager::GetSingletonSafe()->Pause(TTRUE);
		}

		return TTRUE;
	}

	LRESULT CALLBACK TMSWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		TMSWindow* window = reinterpret_cast<TMSWindow*>(GetWindowLongA(hWnd, GWL_USERDATA));

		if (s_fnWndProcHandler != TNULL)
		{
			if (s_fnWndProcHandler(hWnd, uMsg, wParam, lParam))
			{
				return TRUE;
			}
		}

		RECT rect;
		TBOOL bFlag1, bLockCursor;
		auto pRenderer = TRenderDX11::Interface();
		auto pDisplayParams = pRenderer->GetCurrentDisplayParams();
		auto pSystemManager = TSystemManager::GetSingletonSafe();

		if (window == NULL || pDisplayParams->IsFullscreen)
		{
			bLockCursor = TFALSE;
		}
		else
		{
			bLockCursor = TTRUE;

			if (pDisplayParams->Unk5)
			{
				bLockCursor = TFALSE;
			}
		}

		bFlag1 = window != NULL && pDisplayParams->IsFullscreen != TFALSE && pDisplayParams->Unk5 != TFALSE;

		if (WM_ACTIVATEAPP < uMsg)
		{
			if (uMsg < WM_LBUTTONDOWN)
			{
				if (uMsg != WM_MOUSEMOVE)
				{
					if (uMsg == WM_SETCURSOR)
					{
						if (lParam == 1)
						{
							SetCursor(NULL);
							return 1;
						}
					}
					else if (uMsg == WM_SYSCOMMAND)
					{
						if (wParam == SC_CLOSE)
						{
							if (window->m_IsPopup) 
							{
								window->m_Flag5 = TTRUE;
								TTODO("FUN_006b17e0(&bLockCursor)");
							}
						}
						else
						{
							if ((wParam != SC_TASKLIST) && (wParam != SC_MINIMIZE)) return DefWindowProcA(hWnd, uMsg, wParam, lParam);

							if (window->m_IsWindowed)
							{
								ShowWindow(hWnd, SW_MINIMIZE);
								return 0;
							}
						}

						return 0;
					}

					return DefWindowProcA(hWnd, uMsg, wParam, lParam);
				}

				if (window->m_IsWindowed)
				{
					if (!window->m_Flag2)
					{
						ShowCursor(TFALSE);
						window->m_Flag2 = TTRUE;

						if (!window->m_Flag3)
						{
							TRACKMOUSEEVENT tme;
							tme.cbSize = sizeof(TRACKMOUSEEVENT);
							tme.dwFlags = TME_LEAVE;
							tme.hwndTrack = hWnd;

							if (TrackMouseEvent(&tme))
							{
								window->m_Flag3 = TTRUE;
							}
						}
					}

					// To obtain the position coordinates in screen coordinates
					window->m_xPos = GET_X_LPARAM(lParam); // horizontal position 
					window->m_yPos = GET_Y_LPARAM(lParam); // vertical position
				}
			}
			else if (uMsg == WM_DEVICECHANGE)
			{
				if (wParam == DBT_DEVNODES_CHANGED)
				{
					TInputDXInterface::GetInterface()->LostDevice();
				}
			}
			else
			{
				// Mouse Left the Screen
				if (uMsg != WM_MOUSELEAVE) return DefWindowProcA(hWnd, uMsg, wParam, lParam);
				window->m_Flag2 = TFALSE;
				window->m_Flag3 = TFALSE;
				ShowCursor(TTRUE);
			}

			return 1;
		}

		if (uMsg == WM_ACTIVATEAPP)
		{
			if (bFlag1 || bLockCursor)
			{
				GetWindowRect(hWnd, &rect);
				ClipCursor(&rect);
			}

			if (window->IsDestroyed()) return 0;

			if (!window->IsWindowed())
			{
				if (pSystemManager == TNULL) return 0;
				if (wParam == TRUE)
				{
					// Window was activated
					pSystemManager->Pause(TFALSE);
					return 0;
				}
			}
			else
			{
				if (wParam == TRUE)
				{
					// Window was activated
					pSystemManager->Pause(TFALSE);
					return 0;
				}
			}
			pSystemManager->Pause(TTRUE);
			return 0;
		}

		switch (uMsg)
		{
		case WM_CREATE:
			SetWindowLongA(hWnd, GWL_USERDATA, lParam);

			DEV_BROADCAST_DEVICEINTERFACE NotificationFilter;

			ZeroMemory(&NotificationFilter, sizeof(NotificationFilter));
			NotificationFilter.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
			NotificationFilter.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
			NotificationFilter.dbcc_classguid = WceusbshGUID;

			ms_hDeviceNotify = RegisterDeviceNotificationA(hWnd, &NotificationFilter, 0);
			if (ms_hDeviceNotify != NULL) return 0;
			ExitProcess(1);
		case WM_SIZE:
			if (bFlag1 || bLockCursor)
			{
				GetWindowRect(hWnd, &rect);
				ClipCursor(&rect);
				return DefWindowProcA(hWnd, uMsg, wParam, lParam);
			}
			break;
		case WM_ACTIVATE:
			if (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE)
			{
				if (!ms_bIsFocused)
				{
					SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
					ShowCursor(TFALSE);
					pSystemManager->Pause(TTRUE);
					ms_bIsFocused = TTRUE;

					SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);

					STICKYKEYS newStickyKeys { sizeof(STICKYKEYS), 0 };
					SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &newStickyKeys, 0);
				}
				if (pRenderer->m_SwapChain != TNULL && ms_bIsFullscreen)
				{
					TUtil::Log("#####  Restore fullscreen");
					ms_bIsFullscreen = TFALSE;
					pRenderer->m_SwapChain->SetFullscreenState(TTRUE, TNULL);
					return DefWindowProcA(hWnd, uMsg, wParam, lParam);;
				}
			}
			else
			{
				if (pRenderer->m_SwapChain != TNULL)
				{
					pRenderer->m_SwapChain->GetFullscreenState(&ms_bIsFullscreen, TNULL);
					if (pDisplayParams->IsFullscreen && ms_bIsFullscreen)
					{
						TUtil::Log("#####  Disable fullscreen");
						pRenderer->m_SwapChain->SetFullscreenState(TFALSE, TNULL);
					}
				}
				if (ms_bIsFocused)
				{
					SetThreadExecutionState(ES_CONTINUOUS);
					ShowCursor(TTRUE);
					pSystemManager->Pause(TTRUE);
					ms_bIsFocused = TFALSE;

					SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);
					return DefWindowProcA(hWnd, uMsg, wParam, lParam);;
				}
			}
			break;
		case WM_SETFOCUS:
			window->m_bIsFocused = TTRUE;
			ShowCursor(TFALSE);
			pSystemManager->Pause(TFALSE);
			return 1;
		case WM_KILLFOCUS:
			window->m_bIsFocused = TFALSE;
			ShowCursor(TTRUE);
			pSystemManager->Pause(TTRUE);
			if (bLockCursor)
			{
				TUtil::Log("Minimize window");
				ShowWindow(hWnd, SW_MINIMIZE);
			}
			return 1;
		default:
			return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		}
	}
}