#pragma once

#ifdef TRENDERINTERFACE_GL

#  include <GL/glew.h>

TOSHI_NAMESPACE_START

class T2FrameBuffer
{
public:
	T2FrameBuffer();
	~T2FrameBuffer();

	void Create();
	void CreateAttachment( TINT a_iAttachment, GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eInternalFormat, GLenum a_eFormat, GLenum a_ePixelType );
	void CreateDepthTexture( GLsizei a_iWidth, GLsizei a_iHeight );
	
	void Destroy();

	void        Bind();
	static void Unbind();

	TBOOL  IsCreated() const { return m_uiFBO != 0; }
	GLuint GetAttachment( TINT a_iAttachment ) const { return m_aAttachments[ a_iAttachment ]; }
	GLuint GetDepthTexture() const { return m_uiDepthTexture; }

private:
	GLuint m_uiFBO;
	GLuint m_aAttachments[4];
	GLuint m_uiDepthTexture;
};

TOSHI_NAMESPACE_END

#endif // TRENDERINTERFACE_GL
