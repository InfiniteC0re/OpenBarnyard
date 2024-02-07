#pragma once
#include <Toshi/TObject.h>
#include <Render/TRenderInterface.h>
#include <Toshi/TString8.h>

#include <SDL/SDL.h>

class ASDLWindowListener
{
public:
	virtual TBOOL OnEvent(const SDL_Event& a_rEvent) = 0;
};

class ASDLWindow
{
public:
	ASDLWindow() = default;

	TBOOL Create(Toshi::TRenderInterface* a_pRender, const char* a_szTitle);

	void Update();

	void SetFullscreen(TBOOL a_bFullScreen);
	void SetPosition(int a_iX, int a_iY, int a_iWidth, int a_iHeight);

	void SetListener(ASDLWindowListener* a_pListener)
	{
		m_pListener = a_pListener;
	}

	SDL_Window* GetNativeWindow() const
	{
		return m_pWindow;
	}

private:
	SDL_Window* m_pWindow = TNULL;
	ASDLWindowListener* m_pListener;
	Toshi::TRenderInterface* m_pRender;
};
