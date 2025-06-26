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
	m_uiFBO          = 0;
	m_uiDepthTexture = 0;
	
	for ( TINT i = 0; i < TARRAYSIZE( m_aAttachments ); i++ )
	{
		m_aAttachments[ i ] = 0;
	}
}

T2FrameBuffer::~T2FrameBuffer()
{
	Destroy();
}

void T2FrameBuffer::Create()
{
	TASSERT( m_uiFBO == 0 );
	TASSERT( !IsCreated() );

	glGenFramebuffers( 1, &m_uiFBO );
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
		m_uiFBO = 0;

		for ( TINT i = 0; i < TARRAYSIZE( m_aAttachments ); i++ )
		{
			if ( m_aAttachments[ i ] != 0 )
			{
				T2Render::DestroyTexture( m_aAttachments[ i ] );
				m_aAttachments[ i ] = 0;
			}
		}

		if ( m_uiDepthTexture != 0 )
		{
			T2Render::DestroyTexture( m_uiDepthTexture );
			m_uiDepthTexture = 0;
		}
	}
}

void T2FrameBuffer::CreateDepthTexture( GLsizei a_iWidth, GLsizei a_iHeight )
{
	TASSERT( IsCreated() );
	TASSERT( a_iWidth > 0 );
	TASSERT( a_iHeight > 0 );
	TASSERT( m_uiDepthTexture == 0 );

	T2FrameBuffer::Bind();
	GLuint uiTexture = T2Render::CreateTexture( a_iWidth, a_iHeight, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT, TFALSE, TNULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );

	TFLOAT borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
	glTexParameterfv( GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, uiTexture, 0 );
	m_uiDepthTexture = uiTexture;
}

void T2FrameBuffer::CreateAttachment( TINT a_iAttachment, GLsizei a_iWidth, GLsizei a_iHeight, GLenum a_eInternalFormat, GLenum a_eFormat, GLenum a_ePixelType )
{
	TASSERT( IsCreated() );
	TASSERT( a_iWidth > 0 );
	TASSERT( a_iHeight > 0 );
	TASSERT( m_aAttachments[ a_iAttachment ] == 0 );

	T2FrameBuffer::Bind();
	GLuint uiTexture = T2Render::CreateTexture( a_iWidth, a_iHeight, a_eInternalFormat, a_eFormat, a_ePixelType, TFALSE, TNULL );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR );
	glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR );

	glFramebufferTexture2D( GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0 + a_iAttachment, GL_TEXTURE_2D, uiTexture, 0 );
	m_aAttachments[ a_iAttachment ] = uiTexture;
}

void T2FrameBuffer::SetDrawBuffer( GLenum a_eDrawBuffer )
{
	T2FrameBuffer::Bind();
	glDrawBuffer( GL_NONE );
	glReadBuffer( GL_NONE );
}

TOSHI_NAMESPACE_END
