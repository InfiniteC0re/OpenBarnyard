#include "ToshiPCH.h"
#include "T2FrameBuffer_GL.h"
#include "T2Render_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

T2FrameBuffer::T2FrameBuffer()
{
	m_uiFBO     = 0;
	m_uiTexture = 0;
}

T2FrameBuffer::~T2FrameBuffer()
{
	Destroy();
}

void T2FrameBuffer::Create( GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eInternalFormat, GLenum a_eFormat, GLenum a_ePixelType, TBOOL a_bCreateDepthTexture )
{
	TASSERT( m_uiFBO == 0 );
	TASSERT( a_iWidth > 0 );
	TASSERT( !IsCreated() );

	glGenFramebuffers( 1, &m_uiFBO );
	T2FrameBuffer::Bind();

	GLuint uiTexture = T2Render::CreateTexture( a_iWidth, a_iHeight, a_eInternalFormat, a_eFormat, a_ePixelType, TFALSE, TNULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, uiTexture, 0 );
	m_uiTexture = uiTexture;

	if ( a_bCreateDepthTexture )
	{
		CreateDepthTexture( a_iWidth, a_iHeight );
	}
}

GLuint g_uiBoundFBO = 0;

void T2FrameBuffer::Bind()
{
	TASSERT( IsCreated() );

	if ( g_uiBoundFBO != m_uiFBO )
	{
		g_uiBoundFBO = m_uiFBO;
		glBindFramebuffer( GL_FRAMEBUFFER, m_uiFBO );
	}
}

void T2FrameBuffer::Unbind()
{
	if ( g_uiBoundFBO != 0 )
	{
		g_uiBoundFBO = 0;
		glBindFramebuffer( GL_FRAMEBUFFER, 0 );
	}
}

void T2FrameBuffer::Destroy()
{
	if ( m_uiFBO != 0 )
	{
		glDeleteFramebuffers( 1, &m_uiFBO );
		m_uiFBO     = 0;

		T2Render::DestroyTexture( m_uiTexture );
		m_uiTexture = 0;

		if ( m_uiDepthTexture != 0 )
		{
			T2Render::DestroyTexture( m_uiDepthTexture );
			m_uiDepthTexture = 0;
		}
	}
}

void T2FrameBuffer::CreateDepthTexture( GLsizei a_iWidth, GLsizei a_iHeight )
{
	T2FrameBuffer::Bind();

	GLuint uiTexture = T2Render::CreateTexture( a_iWidth, a_iHeight, GL_DEPTH24_STENCIL8, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, TFALSE, TNULL );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, uiTexture, 0 );
	m_uiDepthTexture = uiTexture;
}

TOSHI_NAMESPACE_END
