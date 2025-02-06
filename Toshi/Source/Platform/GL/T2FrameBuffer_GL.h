#pragma once

#ifdef TRENDERINTERFACE_GL

#  include <GL/glew.h>

TOSHI_NAMESPACE_START

class T2FrameBuffer
{
public:
	T2FrameBuffer();
	~T2FrameBuffer();

	void Create( GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eInternalFormat, GLenum a_eFormat, GLenum a_ePixelType, TBOOL a_bCreateDepthTexture );
	void Destroy();

	void        Bind();
	static void Unbind();

	TBOOL  IsCreated() const { return m_uiFBO != 0; }
	GLuint GetTexture() const { return m_uiTexture; }
	GLuint GetDepthTexture() const { return m_uiDepthTexture; }

private:
	void CreateDepthTexture( GLsizei a_iWidth, GLsizei a_iHeight );

private:
	GLuint m_uiFBO;
	GLuint m_uiTexture;
	GLuint m_uiDepthTexture;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
