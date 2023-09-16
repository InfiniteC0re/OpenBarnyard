#include "ToshiPCH.h"
#include "TRender_SDL.h"

#include <SDL/SDL_opengl.h>

namespace Toshi {

	TBOOL TRenderSDL::CreateDisplay(DisplayParams* a_pDisplayParams)
	{
		TASSERT(IsCreated() == TTRUE);
		TASSERT(IsDisplayCreated() == TFALSE);

		if (TRender::CreateDisplay() == TFALSE)
		{
			ShowDisplayError();
			return TFALSE;
		}

		m_DisplayParams = *a_pDisplayParams;

		GetCurrentRenderContext()->SetParams({
			.fX = 0.0f,
			.fY = 0.0f,
			.fWidth = (float)m_DisplayParams.Width,
			.fHeight = (float)m_DisplayParams.Height,
			.fMinZ = 0.0f,
			.fMaxZ = 1.0f
		});

		TTextureManager::CreateSingleton();

		return TFALSE;
	}

	void TRenderSDL::Update(float a_fDeltaTime)
	{
		TASSERT(TTRUE == IsCreated());

		m_Window.Update();
		TRender::Update(a_fDeltaTime);
	}

	void TRenderSDL::BeginScene()
	{
		TASSERT(TTRUE == IsDisplayCreated());
		TASSERT(TFALSE == IsInScene());

		TRender::BeginScene();
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glViewport(0, 0, m_DisplayParams.Width, m_DisplayParams.Height);
		m_bInScene = TTRUE;
	}

	void TRenderSDL::EndScene()
	{
		TASSERT(TTRUE == IsDisplayCreated());
		TASSERT(TTRUE == IsInScene());

		m_uiNumDrawnFrames += 1;

		SDL_GL_SwapWindow(m_Window.GetNativeWindow());

		m_bInScene = TFALSE;
		TRender::EndScene();
	}

	TBOOL TRenderSDL::Create(const char* a_szName)
	{
		TASSERT(TFALSE == IsCreated());
		TBOOL bResult = TRender::Create();

		if (bResult)
		{
			TUtil::Log("Creating TRenderSDL");
			TUtil::LogUp();

			SDL_Init(SDL_INIT_VIDEO);
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

			if (m_hAccel != NULL)
			{
				DestroyAcceleratorTable(m_hAccel);
				m_hAccel = NULL;
			}

			ACCEL accels[2];
			accels[0].fVirt = FVIRTKEY;
			accels[0].key = VK_ESCAPE;

			accels[1].fVirt = FALT;
			accels[1].key = VK_RETURN;
			accels[1].cmd = 1;

			m_hAccel = CreateAcceleratorTableA(accels, sizeof(accels) / sizeof(*accels));

			if (m_Window.Create(this, a_szName))
			{
				m_GLContext = SDL_GL_CreateContext(m_Window.GetNativeWindow());
				m_eAspectRatio = ASPECT_RATIO_16_9;
				return TTRUE;
			}

			TUtil::Log("Failed to create Window");
		}

		return TFALSE;
	}

	TBOOL TRenderSDL::RecreateDisplay(DisplayParams* a_pDisplayParams)
	{
		TASSERT(TFALSE, "Not implemented");
		return TFALSE;
	}

	void TRenderSDL::ShowDeviceError()
	{
		MessageBoxA(NULL, "Initialization failure", "Failed to create SDL device", MB_OK);
	}

	void TRenderSDL::ShowDisplayError()
	{
		MessageBoxA(NULL, "Initialization failure", "Failed to create the display. Please run the Barnyard setup program and reconfigure", MB_OK);
	}

}

