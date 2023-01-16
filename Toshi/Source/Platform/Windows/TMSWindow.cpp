#include "ToshiPCH.h"
#include "TMSWindow.h"

namespace Toshi
{
	bool TMSWindow::ms_bIsFocused = false;
	STICKYKEYS TMSWindow::ms_StickyKeys = { 0, 0 };
	HDEVNOTIFY TMSWindow::ms_hDeviceNotify = { 0 };

	void TMSWindow::Enable()
	{
		TASSERT(m_HWND != TNULL, "HWND is NULL");
		m_IsWindowed = true;
	}

	void TMSWindow::Disable()
	{
		TASSERT(m_HWND != NULL, "HWND is NULL");
		m_IsWindowed = false;
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

			m_IsDestroyed = true;
		}
	}

	void TMSWindow::UnregisterWindowClass()
	{
		if (m_HWND != NULL)
		{
			if (ms_bIsFocused)
			{
				SetThreadExecutionState(ES_CONTINUOUS);
				ShowCursor(true);
				TSystemManager::GetSingleton()->Pause(true);
				SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);
				ms_bIsFocused = false;
			}

			DestroyWindow(m_HWND);
			m_IsDestroyed = true;
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

	bool TMSWindow::RegisterWindowClass(TRender* renderer, LPCSTR title)
	{
		UnregisterWindowClass();

		m_Render = renderer;
		m_ModuleHandle = GetModuleHandleA(NULL);
		
		WNDCLASSA wndClass = { };
		wndClass.hIcon = LoadIconA(m_ModuleHandle, MAKEINTRESOURCEA(IDI_ICON1));
		wndClass.hInstance = m_ModuleHandle;
		wndClass.lpfnWndProc = (WNDPROC)WndProc;
		wndClass.lpszClassName = TMSWindow::GetClassStatic()->GetName();
		wndClass.style = CS_VREDRAW | CS_HREDRAW;
		wndClass.cbWndExtra = 4;
		wndClass.hCursor = LoadCursorA(NULL, MAKEINTRESOURCEA(IDC_ARROW));
		wndClass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
		RegisterClassA(&wndClass);

		DWORD dwStyle = IsPopup() ? s_PopupStyles : 0;
		m_HWND = CreateWindowExA(0, TMSWindow::GetClassStatic()->GetName(), title, dwStyle, 100, 100, 0, 0, NULL, NULL, m_ModuleHandle, this);
		
		if (m_HWND == NULL)
		{
			return false;
		}

		EnableWindow(m_HWND, TRUE);
		ShowWindow(m_HWND, SW_SHOW);
		SetForegroundWindow(m_HWND);

		m_IsDestroyed = false;
		
		if (GetForegroundWindow() != m_HWND)
		{
			TOSHI_INFO("Not foreground window, Pausing Systems!\n");
			TSystemManager::GetSingleton()->Pause(true);
		}

		return true;
	}

	LRESULT CALLBACK TMSWindow::WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		TMSWindow* window = reinterpret_cast<TMSWindow*>(GetWindowLongA(hWnd, GWL_USERDATA));

		RECT rect;
		bool bFlag1, bLockCursor;
		auto pDisplayParams = Toshi::TRender::GetSingleton()->GetCurrentDisplayParams();

		if (window == NULL || pDisplayParams->IsFullscreen)
		{
			bLockCursor = false;
		}
		else
		{
			bLockCursor = true;

			if (pDisplayParams->Unk5 != false)
			{
				bLockCursor = false;
			}
		}

		bFlag1 = window != NULL && pDisplayParams->IsFullscreen != false && pDisplayParams->Unk5 != false;

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
							SetCursor(0);
							return 1;
						}
					}
					else if (uMsg == WM_SYSCOMMAND)
					{
						if (wParam == SC_CLOSE)
						{
							/*if ((window != 0) && (*(char*)(window + 0x21) != '\0')) {
								*(undefined*)(window + 0x22) = 1;
								FUN_006b17e0(&local_39);
							}*/
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
				else
				{					
					if (window->m_IsWindowed)
					{
						if (!window->m_Flag2)
						{
							ShowCursor(false);
							window->m_Flag2 = true;

							if (!window->m_Flag3)
							{
								TRACKMOUSEEVENT tme;
								tme.cbSize = sizeof(TRACKMOUSEEVENT);
								tme.dwFlags = TME_LEAVE;
								tme.hwndTrack = hWnd;

								if (TrackMouseEvent(&tme))
								{
									window->m_Flag3 = true;
								}
							}
						}

						// To obtain the position coordinates in screen coordinates
						window->m_xPos = GET_X_LPARAM(lParam); // horizontal position 
						window->m_yPos = GET_Y_LPARAM(lParam); // vertical position
					}
				}
			}
			else if (uMsg == WM_DEVICECHANGE)
			{
				if (wParam == DBT_DEVNODES_CHANGED)
				{
					TTODO("Toshi::TInputDXInterface::FUN_00680030()");
				}
			}
			else
			{
				if (uMsg != WM_MOUSELEAVE) return DefWindowProcA(hWnd, uMsg, wParam, lParam);
				window->m_Flag2 = false;
				window->m_Flag3 = false;
				ShowCursor(true);
			}

			return 1;
		}

		if (uMsg == WM_ACTIVATEAPP)
		{
			auto pSystemManager = TSystemManager::GetSingletonWeak();

			if (bFlag1 || bLockCursor)
			{
				GetWindowRect(hWnd, &rect);
				ClipCursor(&rect);
			}

			if (window->IsDestroyed()) return 0;
			if (pSystemManager == TNULL) return 0;

			if (wParam == TRUE)
			{
				// Window was activated
				pSystemManager->Pause(false);
			}
			else
			{
				// Window was deactivated
				pSystemManager->Pause(true);
			}

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
			if (bLockCursor)
			{
				GetWindowRect(hWnd, &rect);
				ClipCursor(&rect);
				return DefWindowProcA(hWnd, uMsg, wParam, lParam);
			}
		case WM_ACTIVATE:
			if (LOWORD(wParam) == WA_ACTIVE || LOWORD(wParam) == WA_CLICKACTIVE)
			{
				if (ms_bIsFocused != true)
				{
					SetThreadExecutionState(ES_CONTINUOUS | ES_DISPLAY_REQUIRED | ES_SYSTEM_REQUIRED);
					ShowCursor(false);
					//FUN_006616a0(DAT_009a46f0, 0);
					ms_bIsFocused = true;

					SystemParametersInfoA(SPI_GETSTICKYKEYS, sizeof(STICKYKEYS), &ms_StickyKeys, 0);

					STICKYKEYS newStickyKeys { sizeof(STICKYKEYS), 0 };
					SystemParametersInfoA(SPI_SETSTICKYKEYS, sizeof(STICKYKEYS), &newStickyKeys, 0);
				}
			}
			break;
		default:
			return DefWindowProcA(hWnd, uMsg, wParam, lParam);
		}
	}
}