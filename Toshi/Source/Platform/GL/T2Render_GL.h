#pragma once

#ifdef TRENDERINTERFACE_GL
#  include "Render/T2RenderCommon.h"
#  include "T2RenderBuffer_GL.h"
#  include "T2Shader_GL.h"
#  include "Platform/SDL/T2Window_SDL.h"

#  include <SDL/SDL.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Note: This is a custom class which is not the same as the one used in TOSHI 2.0
// since it was never actually used in any games but in some tools like TXSViewer
// which is not publicly available, so it's not possible to match this class.
//-----------------------------------------------------------------------------
class T2Render
    : public TSingleton<T2Render>
{
public:
	inline static constexpr T2RenderAPI API = T2RenderAPI::OpenGL;

	struct WindowParams
	{
		const TCHAR* pchTitle;
		TUINT        uiWidth;
		TUINT        uiHeight;
		TBOOL        bIsWindowed;
	};

public:
	T2Render();
	~T2Render();

	// Creates renderer and it's resources. Returns TTRUE if succeded
	TBOOL Create( const WindowParams& a_rcWindowParams );

	// Destroys renderer and everything it manages
	void Destroy();

	void Update( TFLOAT a_fDeltaTime );
	void BeginScene();
	void EndScene();

	const WindowParams& GetWindowParams() const { return m_oWindowParams; }

public:
	static GLuint           CreateTexture( GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eFormat, TBOOL a_bGenerateMipmap, const void* a_pData );
	static void             DestroyTexture( GLuint a_iId );

	static T2CompiledShader CompileShader( GLenum a_eShader, const TCHAR* a_szSource );
	static T2CompiledShader CompileShaderFromFile( GLenum a_eShader, const TCHAR* a_szFileName );
	static T2Shader         CreateShaderProgram( T2CompiledShader a_VertexShader, T2CompiledShader a_FragmentShader );
	static T2VertexBuffer   CreateVertexBuffer( const void* a_pData, GLuint a_uiSize, GLenum a_eUsage );
	static T2IndexBuffer    CreateIndexBuffer( const TUINT16* a_pIndices, GLuint a_uiCount, GLenum a_eUsage );
	static T2VertexArray    CreateVertexArray();
	static T2VertexArray    CreateVertexArray( T2VertexBuffer a_VertexBuffer, T2IndexBuffer a_IndexBuffer );

private:
	void OnDeviceReset();

private:
	T2Window*     m_pWindow    = TNULL;
	SDL_GLContext m_pGLContext = TNULL;
	WindowParams  m_oWindowParams;
	TBOOL         m_bIsInScene = TFALSE;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
