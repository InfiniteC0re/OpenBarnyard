#pragma once
#include "TSDLWindow.h"

#include "Toshi/Render/TRender.h"
#include "Toshi/Render/TRenderAdapter.h"
#include "Toshi/Render/Shaders/TToneMap.h"
#include "Toshi/Render/Shaders/TFXAA.h"
#include "Toshi/Render/TOrderTable.h"
#include "Toshi2/T2RedBlackTree.h"
#include "Toshi2/T2Pair.h"

#include <SDL/SDL.h>

namespace Toshi {

	class TGLShaderRef
	{
	public:
		TGLShaderRef() : m_uiId(0) { }

		TGLShaderRef(GLuint a_uiId) :
			m_uiId(a_uiId) { }

		TGLShaderRef(const TGLShaderRef& a_rOther) :
			m_uiId(a_rOther.m_uiId) { }

		GLuint GetId() const { return m_uiId; }

		operator bool() const { return m_uiId != 0; }

	private:
		GLuint m_uiId;
	};

	class TGLShaderProgram
	{
	public:
		TGLShaderProgram() : m_uiProgram(0), m_VShader(0), m_FShader(0) { }

		TGLShaderProgram(const TGLShaderRef& a_VShader, const TGLShaderRef& a_FShader) :
			m_uiProgram(0),
			m_VShader(a_VShader),
			m_FShader(a_FShader)
		{ }

		TGLShaderProgram(const TGLShaderProgram& a_rOther) :
			m_uiProgram(a_rOther.m_uiProgram),
			m_VShader(a_rOther.m_VShader),
			m_FShader(a_rOther.m_FShader)
		{ }

		void Create()
		{
			TASSERT(m_uiProgram == 0);

			if (m_VShader && m_FShader)
			{
				m_uiProgram = glCreateProgram();
				TASSERT(m_uiProgram != 0, "Couldn't create GL shader program");

				glAttachShader(m_uiProgram, m_VShader.GetId());
				glAttachShader(m_uiProgram, m_FShader.GetId());
				glLinkProgram(m_uiProgram);
				glValidateProgram(m_uiProgram);
			}
		}

		void Destroy(TBOOL a_bDeleteShaders)
		{
			glDeleteProgram(m_uiProgram);

			if (a_bDeleteShaders)
			{
				glDeleteShader(m_VShader.GetId());
				glDeleteShader(m_FShader.GetId());
			}
		}

		void Use() const
		{
			TASSERT(m_uiProgram != 0);
			glUseProgram(m_uiProgram);
		}

		void SetUniform(const char* a_szSlotName, const TMatrix44& a_rMatrix)
		{
			GLuint loc = glGetUniformLocation(m_uiProgram, a_szSlotName);
			glUniformMatrix4fv(loc, 1, GL_FALSE, &a_rMatrix.m_f11);
		}

		void SetUniform(const char* a_szSlotName, const TVector4& a_rVector)
		{
			GLuint loc = glGetUniformLocation(m_uiProgram, a_szSlotName);
			glUniform4fv(loc, 1, &a_rVector.x);
		}

		void SetUniform(const char* a_szSlotName, const TVector3& a_rVector)
		{
			GLuint loc = glGetUniformLocation(m_uiProgram, a_szSlotName);
			glUniform3fv(loc, 1, &a_rVector.x);
		}

		void SetUniform(const char* a_szSlotName, float a_fValue)
		{
			GLuint loc = glGetUniformLocation(m_uiProgram, a_szSlotName);
			glUniform1f(loc, a_fValue);
		}

		TGLShaderRef GetVertexShader()
		{
			return m_VShader;
		}

		TGLShaderRef GetFragmentShader()
		{
			return m_FShader;
		}

	private:
		GLuint m_uiProgram;
		TGLShaderRef m_VShader;
		TGLShaderRef m_FShader;
	};

	template <GLenum Type>
	class TGLBufferRef
	{
	public:
		TGLBufferRef() : m_uiId(0) { }

		TGLBufferRef(GLuint a_uiId) :
			m_uiId(a_uiId) { }

		TGLBufferRef(const TGLBufferRef& a_rOther) :
			m_uiId(a_rOther.m_uiId) { }

		GLuint GetId() const { return m_uiId; }

		void SetData(const void* a_pData, GLuint a_uiSize, GLenum a_eUsage)
		{
			TASSERT(m_uiId != 0);
			glBindBuffer(Type, m_uiId);
			glBufferData(Type, a_uiSize, a_pData, a_eUsage);
			glBindBuffer(Type, 0);
		}

		void Bind() { glBindBuffer(Type, m_uiId); }
		void Unbind() { glBindBuffer(Type, 0); }
		void Destroy() { glDeleteBuffers(1, &m_uiId); }

		operator bool() const { return m_uiId != 0; }

	private:
		GLuint m_uiId;
	};

	using TGLVertexBuffer = TGLBufferRef<GL_ARRAY_BUFFER>;
	using TGLIndexBuffer = TGLBufferRef<GL_ELEMENT_ARRAY_BUFFER>;

	class TGLVertexArrayRef
	{
	public:
		TGLVertexArrayRef() : m_uiId(0) { }

		TGLVertexArrayRef(GLuint a_uiId) : m_uiId(a_uiId) { }

		TGLVertexArrayRef(GLuint a_uiId, TGLVertexBuffer a_VertexBuffer, TGLIndexBuffer a_IndexBuffer) :
			m_uiId(a_uiId),
			m_VertexBuffer(a_VertexBuffer),
			m_IndexBuffer(a_IndexBuffer)
		{ }

		TGLVertexArrayRef(const TGLVertexArrayRef& a_rOther) :
			m_uiId(a_rOther.m_uiId),
			m_VertexBuffer(a_rOther.m_VertexBuffer),
			m_IndexBuffer(a_rOther.m_IndexBuffer)
		{ }

		void SetAttribPointer(GLuint a_uiIndex, GLint a_iNumComponents, GLenum a_eType, GLsizei a_iStride, const void* a_pOffset)
		{
			TASSERT(m_uiId != 0);
			glBindVertexArray(m_uiId);
			glBindBuffer(GL_ARRAY_BUFFER, m_VertexBuffer.GetId());
			glVertexAttribPointer(a_uiIndex, a_iNumComponents, a_eType, GL_FALSE, a_iStride, a_pOffset);
			glEnableVertexAttribArray(a_uiIndex);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		void SetVertexBuffer(TGLVertexBuffer a_VertexBuffer)
		{
			TASSERT(m_uiId != 0);
			glBindVertexArray(m_uiId);
			a_VertexBuffer.Bind();
			m_VertexBuffer = a_VertexBuffer;
			glBindVertexArray(0);
		}

		void SetIndexBuffer(TGLIndexBuffer a_IndexBuffer)
		{
			TASSERT(m_uiId != 0);
			glBindVertexArray(m_uiId);
			a_IndexBuffer.Bind();
			m_IndexBuffer = a_IndexBuffer;
			glBindVertexArray(0);
		}

		GLuint GetId() const { return m_uiId; }

		void Bind() { glBindVertexArray(m_uiId); }
		void Unbind() { glBindVertexArray(0); }
		void Destroy() { glDeleteVertexArrays(1, &m_uiId); }

		TGLVertexBuffer GetVertexBuffer() { return m_VertexBuffer; }
		TGLIndexBuffer GetIndexBuffer() { return m_IndexBuffer; }

		operator bool() const { return m_uiId != 0; }

	private:
		GLuint m_uiId;
		TGLVertexBuffer m_VertexBuffer;
		TGLIndexBuffer m_IndexBuffer;
	};

	class TRenderSDL :
		public TGenericClassDerived<TRenderSDL, TRender, "TRenderSDL", TMAKEVERSION(1, 0), TFALSE>
	{
	public:
		TRenderSDL() = default;

		virtual TBOOL CreateDisplay(DisplayParams* a_pDisplayParams) override;
		virtual TBOOL DestroyDisplay() override { return TTRUE; }
		virtual void Update(float a_fDeltaTime) override;
		virtual void BeginScene() override;
		virtual void EndScene() override;
		virtual void* GetCurrentDevice() override { return TNULL; }
		virtual DisplayParams* GetCurrentDisplayParams() override { return &m_DisplayParams; }
		virtual TBOOL Create() override { return Create("TOSHI SDL Application"); }

		virtual TBOOL RecreateDisplay(DisplayParams* a_pDisplayParams);
		virtual void ShowDeviceError();
		virtual void ShowDisplayError();

		TBOOL Create(const char* a_szName);

		void WaitForEndOfRender();
		void FlushShaders();

		GLuint CreateTexture(GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eFormat, TBOOL a_bGenerateMipmap, const void* a_pData);

		static TGLShaderRef CompileShader(GLenum a_eShader, const char* a_szSource);
		static TGLShaderRef CompileShaderFromFile(GLenum a_eShader, const char* a_szFileName);
		static TGLShaderProgram CreateShaderProgram(TGLShaderRef a_VertexShader, TGLShaderRef a_FragmentShader);
		static TGLVertexBuffer CreateVertexBuffer(const void* a_pData, GLuint a_uiSize, GLenum a_eUsage);
		static TGLIndexBuffer CreateIndexBuffer(const uint16_t* a_pIndices, unsigned int a_uiCount, GLenum a_eUsage);
		static TGLVertexArrayRef CreateVertexArray();
		static TGLVertexArrayRef CreateVertexArray(TGLVertexBuffer a_VertexBuffer, TGLIndexBuffer a_IndexBuffer);

		TSDLWindow* GetWindow() { return &m_Window; }
		TPriList<TOrderTable>& GetOrderTables() { return m_OrderTables; }
		SDL_GLContext GetDeviceContext() const { return m_GLContext; }

	public:
		static TRenderSDL* Interface()
		{
			return static_cast<TRenderSDL*>(TRender::GetSingleton());
		}

	private:
		TSDLWindow m_Window;
		uint32_t m_uiNumDrawnFrames = 0;
		DisplayParams m_DisplayParams;
		TPriList<TOrderTable> m_OrderTables;
		HACCEL m_hAccel = TNULL;
		SDL_GLContext m_GLContext = TNULL;
	};

}
