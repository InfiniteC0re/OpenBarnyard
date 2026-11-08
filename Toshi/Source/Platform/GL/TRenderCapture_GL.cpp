#include "ToshiPCH.h"
#include "TRenderCapture_GL.h"
#include "TRenderAdapter_GL.h"
#include "TRenderInterface_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TRenderCaptureGL::TRenderCaptureGL()
{
	m_iUnk1   = 0;
	m_bUnk2   = TFALSE;
	m_pBuffer = TNULL;
}

TRenderCaptureGL::~TRenderCaptureGL()
{
	if ( m_pBuffer )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}
}

TBOOL TRenderCaptureGL::Create( FORMAT a_eFormat, TINT a_iWidth, TINT a_iHeight )
{
	m_eFormat = a_eFormat;
	m_iHeight = a_iHeight;
	m_iWidth  = a_iWidth;
	return TTRUE;
}

void TRenderCaptureGL::Destroy()
{
	ReleaseBuffer();
}

// NOTE: D3D8 copies the front buffer here, the default framebuffer of GL has none that can be read, so the next frame is copied by TRenderGLInterface::EndScene before it's presented and Poll waits for it
TBOOL TRenderCaptureGL::Request()
{
	ReleaseBuffer();
	TRenderGLInterface::Interface()->SetPendingCapture( this );

	return TTRUE;
}

// Copies the back buffer into the buffer, scaled down to the size of the capture
TBOOL TRenderCaptureGL::CaptureBackBuffer( TUINT a_uiBackBufferWidth, TUINT a_uiBackBufferHeight )
{
	TUINT8* pPixels = TSTATICCAST( TUINT8, TMalloc( a_uiBackBufferWidth * a_uiBackBufferHeight * 4 ) );

	if ( !pPixels )
		return TFALSE;

	glReadPixels( 0, 0, a_uiBackBufferWidth, a_uiBackBufferHeight, GL_RGBA, GL_UNSIGNED_BYTE, pPixels );

	TUINT uiBufferSize = 4 * m_iWidth * m_iHeight;
	m_pBuffer          = TSTATICCAST( TUINT8, TMalloc( uiBufferSize ) );
	TUtil::MemSet( m_pBuffer, 0xFF, uiBufferSize );

	TUINT uiStepX = a_uiBackBufferWidth / m_iWidth;
	TUINT uiStepY = a_uiBackBufferHeight / m_iHeight;

	TUINT8* pOutPixel = m_pBuffer;

	for ( TUINT uiRow = 0; uiRow < TUINT( m_iHeight ); uiRow++ )
	{
		// GL's rows start at the bottom
		const TUINT8* pInPixel = pPixels + a_uiBackBufferWidth * 4 * ( a_uiBackBufferHeight - 1 - uiRow * uiStepY );

		for ( TUINT uiColumn = 0; uiColumn < TUINT( m_iWidth ); uiColumn++ )
		{
			// Alpha stays 0xFF
			pOutPixel[ 0 ] = pInPixel[ 0 ];
			pOutPixel[ 1 ] = pInPixel[ 1 ];
			pOutPixel[ 2 ] = pInPixel[ 2 ];

			pOutPixel += 4;
			pInPixel += 4 * uiStepX;
		}
	}

	TFree( pPixels );

	return TTRUE;
}

void TRenderCaptureGL::ReleaseBuffer()
{
	if ( m_pBuffer )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}
}

TOSHI_NAMESPACE_END
