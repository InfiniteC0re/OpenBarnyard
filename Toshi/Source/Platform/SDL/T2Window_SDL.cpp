#include "ToshiPCH.h"
#include "T2Window_SDL.h"
#include "Toshi/TApplication.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TBOOL T2Window::Create( T2Render* a_pRender, const TCHAR* a_szTitle, TBOOL a_bHeadless )
{
	m_pRender = a_pRender;
	m_pWindow = SDL_CreateWindow(
	    a_szTitle,
	    SDL_WINDOWPOS_CENTERED,
	    SDL_WINDOWPOS_CENTERED,
	    0,
	    0,
	    SDL_WINDOW_SHOWN | ( a_bHeadless ? SDL_WINDOW_HIDDEN : 0 )
#ifdef TRENDERINTERFACE_GL
	        | SDL_WINDOW_OPENGL
#endif
	);

	m_iWidth    = -1;
	m_iHeight   = -1;
	m_bHeadless = a_bHeadless;

	return m_pWindow != TNULL;
}

void T2Window::Update()
{
	SDL_Event event;

	while ( SDL_PollEvent( &event ) )
	{
		if ( m_pListener )
		{
			if ( m_pListener->OnEvent( event ) )
				continue;
		}

		if ( event.type == SDL_EventType::SDL_QUIT )
		{
			TGlobalEmitter<TApplicationExitEvent>::Throw( { TFALSE } );
		}
	}
}

void T2Window::SetFullscreen( TBOOL a_bFullScreen )
{
	SDL_SetWindowFullscreen( m_pWindow, a_bFullScreen ? SDL_WINDOW_FULLSCREEN : 0 );
}

void T2Window::SetPosition( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight )
{
	SDL_SetWindowPosition( m_pWindow, a_iX, a_iY );
	SDL_SetWindowSize( m_pWindow, a_iWidth, a_iHeight );

	m_iWidth  = a_iWidth;
	m_iHeight = a_iHeight;
}

TOSHI_NAMESPACE_END
