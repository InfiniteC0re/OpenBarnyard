#pragma once
#include "Render/TRenderInterface.h"

#include <SDL/SDL.h>

TOSHI_NAMESPACE_START

class T2Render;

//-----------------------------------------------------------------------------
// Note: This is a custom class which is not the same as the one used in TOSHI 2.0
// since it was never actually used in any games but in some tools like TXSViewer which
// is not publicly available, so it's not possible to match this class.
//-----------------------------------------------------------------------------
class T2Window
{
public:
	class EventListener
	{
	public:
		virtual TBOOL OnEvent( const SDL_Event& a_rEvent ) = 0;
	};

public:
	T2Window()  = default;
	~T2Window() = default;

	TBOOL Create( T2Render* a_pRender, const TCHAR* a_szTitle );
	void  Update();

	void SetFullscreen( TBOOL a_bFullScreen );
	void SetPosition( TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight );

	void        SetListener( EventListener* a_pListener ) { m_pListener = a_pListener; }
	SDL_Window* GetNativeWindow() const { return m_pWindow; }

	TINT GetWidth() const { return m_iWidth; }
	TINT GetHeight() const { return m_iHeight; }

private:
	SDL_Window*    m_pWindow = TNULL;
	EventListener* m_pListener;
	T2Render*      m_pRender;

	TINT m_iWidth;
	TINT m_iHeight;
};

TOSHI_NAMESPACE_END
