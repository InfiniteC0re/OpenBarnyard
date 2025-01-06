#pragma once

#ifdef TRENDERINTERFACE_GL
#  include "Render/T2RenderCommon.h"
#  include "T2RenderBuffer_GL.h"

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Note: This is a custom class which is not the same as the one used in TOSHI 2.0
// since it was never actually used in any games but in some tools like TXSViewer which
// is not publicly available, so it's not possible to match this class.
//-----------------------------------------------------------------------------
class T2Render
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

public:
	static T2VertexBuffer CreateVertexBuffer( const void* a_pData, GLuint a_uiSize, GLenum a_eUsage );
	static T2IndexBuffer  CreateIndexBuffer( const TUINT16* a_pIndices, GLuint a_uiCount, GLenum a_eUsage );
	static T2VertexArray  CreateVertexArray();
	static T2VertexArray  CreateVertexArray( T2VertexBuffer a_VertexBuffer, T2IndexBuffer a_IndexBuffer );

private:
	void OnDeviceReset();
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
