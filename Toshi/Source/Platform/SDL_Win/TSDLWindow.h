#pragma once
#include "Toshi/Core/TObject.h"
#include "Toshi/Render/TRender.h"
#include "Toshi/Strings/TString8.h"

#include <SDL/SDL.h>

namespace Toshi {

	class TSDLWindow :
		public TGenericClassDerived<TSDLWindow, TObject, "TSDLWindow", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TSDLWindow() = default;
		
		TBOOL Create(TRender* a_pRender, const char* a_szTitle);
		
		void Update();

		SDL_Window* GetNativeWindow() const
		{
			return m_pWindow;
		}

	private:
		SDL_Window* m_pWindow = TNULL;
		TRender* m_pRender;
	};

}