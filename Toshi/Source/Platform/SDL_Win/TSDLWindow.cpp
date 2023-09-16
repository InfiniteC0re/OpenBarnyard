#include "ToshiPCH.h"
#include "TSDLWindow.h"

namespace Toshi {

	TBOOL TSDLWindow::Create(TRender* a_pRender, const char* a_szTitle)
	{
		m_pRender = a_pRender;
		m_pWindow = SDL_CreateWindow("", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 0, 0, SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
		
		return m_pWindow != TNULL;
	}

	void TSDLWindow::Update()
	{
		SDL_Event event;

		while (SDL_PollEvent(&event))
		{
			
		}
	}

}
