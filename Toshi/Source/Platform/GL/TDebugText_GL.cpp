#include "ToshiPCH.h"
#include "TDebugText_GL.h"
#include "TRenderInterface_GL.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

void TDebugGLText::Reset()
{
	m_pCurrent = m_aBuffer;
}

void TDebugGLText::Destroy()
{
	if ( m_pFont )
	{
		m_pFont->DeleteDeviceObjects();
		delete m_pFont;
		m_pFont = TNULL;
	}

	// NOTE: frees the memory without running the destructor, like retail
	TFree( this );
}

void TDebugGLText::Render()
{
	TRenderGLInterface* pRenderer = TSTATICCAST( TRenderGLInterface, m_pRenderer );

	if ( pRenderer->IsDeviceLost() == TTRUE || !m_bEnabled )
	{
		Reset();
		return;
	}

	TINT iLogY   = 0;
	TINT iWidth  = pRenderer->GetCurrentDisplayParams()->uiWidth;
	TINT iHeight = pRenderer->GetCurrentDisplayParams()->uiHeight;

	for ( TUINT8* pData = m_aBuffer; pData != m_pCurrent; pData += TREINTERPRETCAST( Entry*, pData )->uiLength + 13 )
	{
		Entry* pEntry = TREINTERPRETCAST( Entry*, pData );

		if ( pEntry->uiFlags & ENTRYFLAGS_LOG )
		{
			m_pFont->DrawText( 0.0f, TFLOAT( iLogY ), pEntry->uiColour, pEntry->szText, 0 );
			iLogY += 11;
		}
		else if ( pEntry->uiFlags & ENTRYFLAGS_CENTERED )
		{
			TINT iTextWidth, iTextHeight;
			m_pFont->GetTextExtent( pEntry->szText, &iTextWidth, &iTextHeight );

			TINT iX = pEntry->iX - iTextWidth / 2;
			TINT iY = pEntry->iY - iTextHeight / 2;

			// NOTE: retail subtracts the half size twice before checking if the text is on the screen
			if ( iX - iTextWidth / 2 < iWidth && iY - iTextHeight / 2 < iHeight )
				m_pFont->DrawText( TFLOAT( iX ), TFLOAT( iY ), pEntry->uiColour, pEntry->szText, 0 );
		}
		else
		{
			m_pFont->DrawText( TFLOAT( pEntry->iX ), TFLOAT( pEntry->iY ), pEntry->uiColour, pEntry->szText, 0 );
		}
	}

	Reset();
}

TDebugText::Entry* TDebugGLText::AllocateEntry( TUINT16 a_uiLength )
{
	TUINT8* pEntry = m_pCurrent;

	if ( TUINT( ( pEntry - m_aBuffer ) + a_uiLength + 12 ) >= TUINT( m_iBufferSize ) )
		return TNULL;

	m_pCurrent = pEntry + a_uiLength + 12;
	return TREINTERPRETCAST( Entry*, pEntry );
}

TBOOL TDebugGLText::OnDeviceLost()
{
	m_pFont->InvalidateDeviceObjects();
	return TTRUE;
}

TBOOL TDebugGLText::OnDeviceFound()
{
	m_pFont->RestoreDeviceObjects();
	return TTRUE;
}

TOSHI_NAMESPACE_END
