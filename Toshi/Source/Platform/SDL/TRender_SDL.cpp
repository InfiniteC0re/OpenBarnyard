#include "ToshiPCH.h"
#include <gl/glew.h>

#include "TRender_SDL.h"
#include "TRenderContext_SDL.h"

namespace Toshi {

	TRenderContext* TRender::CreateRenderContext()
	{
		return new TRenderContextSDL(this);
	}

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

		if (m_DisplayParams.IsFullscreen)
		{
			m_Window.SetFullscreen(TTRUE);
		}

		m_Window.SetPosition(100, 100, m_DisplayParams.Width, m_DisplayParams.Height);

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

			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 4);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
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
				SDL_GL_MakeCurrent(m_Window.GetNativeWindow(), m_GLContext);

				GLenum glewStatus = glewInit();
				TASSERT(glewStatus == GLEW_OK, glewGetErrorString(glewStatus));

				m_eAspectRatio = ASPECT_RATIO_16_9;
				return TTRUE;
			}

			TUtil::Log("Failed to create Window");
		}

		return TFALSE;
	}

	void TRenderSDL::WaitForEndOfRender()
	{
		TIMPLEMENT();
	}

	void TRenderSDL::FlushShaders()
	{
		TASSERT(TTRUE == IsInScene());

		for (auto it = m_OrderTables.Begin(); it != m_OrderTables.End(); it++)
		{
			it->Flush();
		}
	}

	GLuint TRenderSDL::CreateTexture(GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eFormat, TBOOL a_bGenerateMipmap, const void* a_pData)
	{
		GLuint uiTexture;
		glGenTextures(1, &uiTexture);

		glBindTexture(GL_TEXTURE_2D, uiTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, a_eFormat, a_iWidth, a_iHeight, 0, a_eFormat, GL_UNSIGNED_BYTE, a_pData);

		if (a_bGenerateMipmap)
		{
			glGenerateMipmap(GL_TEXTURE_2D);
		}

		return uiTexture;
	}

	TGLShaderRef TRenderSDL::CompileShader(GLenum a_eShader, const char* a_szSource)
	{
		TGLShaderRef shader = glCreateShader(a_eShader);
		TASSERT(shader, "Couldn't create GL shader");

		glShaderSource(shader.GetId(), 1, &a_szSource, TNULL);
		glCompileShader(shader.GetId());

		GLint compileSuccess = 0;
		glGetShaderiv(shader.GetId(), GL_COMPILE_STATUS, &compileSuccess);

		if (compileSuccess == GL_FALSE)
		{
			GLint logLength;
			glGetShaderiv(shader.GetId(), GL_INFO_LOG_LENGTH, &logLength);

			GLchar* log = new GLchar[logLength - 1];
			glGetShaderInfoLog(shader.GetId(), logLength - 1, nullptr, log);

			TOSHI_CORE_ERROR("Unable to compile shader");
			TOSHI_CORE_ERROR("Error: {0}", log);
			delete[] log;
		}

		return shader;
	}

	TGLShaderRef TRenderSDL::CompileShaderFromFile(GLenum a_eShader, const char* a_szFileName)
	{
		TFile* pFile = TFile::Create(a_szFileName);
		DWORD fileSize = pFile->GetSize();
		char* srcData = new char[fileSize + 1];
		pFile->Read(srcData, fileSize);
		srcData[fileSize] = '\0';
		pFile->Destroy();

		TGLShaderRef shader = CompileShader(a_eShader, srcData);
		delete[] srcData;

		return shader;
	}

	TGLShaderProgram TRenderSDL::CreateShaderProgram(TGLShaderRef a_VertexShader, TGLShaderRef a_FragmentShader)
	{
		TGLShaderProgram program(a_VertexShader, a_FragmentShader);
		program.Create();

		return program;
	}

	TGLVertexBuffer TRenderSDL::CreateVertexBuffer(const void* a_pData, GLuint a_uiSize, GLenum a_eUsage)
	{
		GLuint bufferId;
		glGenBuffers(1, &bufferId);
		TASSERT(bufferId != 0);

		TGLVertexBuffer buffer = bufferId;

		if (a_uiSize != 0)
		{
			buffer.SetData(a_pData, a_uiSize, a_eUsage);
		}

		return buffer;
	}

	TGLIndexBuffer TRenderSDL::CreateIndexBuffer(const uint16_t* a_pIndices, unsigned int a_uiCount, GLenum a_eUsage)
	{
		GLuint bufferId;
		glGenBuffers(1, &bufferId);
		TASSERT(bufferId != 0);

		TGLIndexBuffer buffer = bufferId;

		if (a_uiCount != 0)
		{
			buffer.SetData(a_pIndices, sizeof(*a_pIndices) * a_uiCount, a_eUsage);
		}

		return buffer;
	}

	TGLVertexArrayRef TRenderSDL::CreateVertexArray()
	{
		GLuint vaoId;
		glGenVertexArrays(1, &vaoId);
		TASSERT(vaoId != 0);

		return TGLVertexArrayRef(vaoId);
	}

	TGLVertexArrayRef TRenderSDL::CreateVertexArray(TGLVertexBuffer a_VertexBuffer, TGLIndexBuffer a_IndexBuffer)
	{
		GLuint vaoId;
		glGenVertexArrays(1, &vaoId);
		TASSERT(vaoId != 0);

		TGLVertexArrayRef vao(vaoId);
		vao.SetVertexBuffer(a_VertexBuffer);
		vao.SetIndexBuffer(a_IndexBuffer);

		return vao;
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

