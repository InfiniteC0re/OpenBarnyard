#include "ToshiPCH.h"
#include "TSDLWindow.h"
#include "Toshi/Core/TApplication.h"

namespace Toshi {

	TBOOL TSDLWindow::Create(TRender* a_pRender, const char* a_szTitle)
	{
		m_pRender = a_pRender;
		m_pWindow = SDL_CreateWindow(a_szTitle, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		
		return m_pWindow != TNULL;
	}

	void TSDLWindow::Update()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			if (m_pListener)
			{
				if (m_pListener->OnEvent(event))
					continue;
			}

			if (event.type == SDL_EventType::SDL_QUIT)
			{
				TGlobalEmitter<TApplicationExitEvent>::Throw({ false });
			}
		}
	}

	void TSDLWindow::SetFullscreen(TBOOL a_bFullScreen)
	{
		SDL_SetWindowFullscreen(m_pWindow, a_bFullScreen ? SDL_WINDOW_FULLSCREEN : 0);
	}

	void TSDLWindow::SetPosition(int a_iX, int a_iY, int a_iWidth, int a_iHeight)
	{
		SDL_SetWindowPosition(m_pWindow, a_iX, a_iY);
		SDL_SetWindowSize(m_pWindow, a_iWidth, a_iHeight);
	}

}
