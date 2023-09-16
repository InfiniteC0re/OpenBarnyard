#include "ToshiPCH.h"
#include "TRender_SDL.h"
#include "Toshi/Render/TViewport.h"

#include <SDL/SDL_opengl.h>

namespace Toshi {

	void TViewport::BeginSKU()
	{
		glViewport(
			TSTATICCAST(GLint, GetX()),
			TSTATICCAST(GLint, GetY()),
			TSTATICCAST(GLsizei, GetWidth()),
			TSTATICCAST(GLsizei, GetHeight())
		);

		if (m_bAllowBackgroundClear || m_bAllowDepthClear)
		{
			if (m_bAllowBackgroundClear)
			{
				uint8_t r, g, b, a;
				GetBackgroundColor(r, g, b, a);

				glClearColor(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
				glClear(GL_COLOR_BUFFER_BIT);
			}

			if (m_bAllowDepthClear)
			{
				glClear(GL_DEPTH_BUFFER_BIT);
			}
		}
	}

	void TViewport::EndSKU()
	{
		TRenderSDL::Interface()->FlushShaders();
	}

	void TViewport::ChangeSKU(ChangeEvent a_eEvent)
	{

	}

}
