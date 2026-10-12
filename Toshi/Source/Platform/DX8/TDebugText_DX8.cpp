#include "ToshiPCH.h"
#include "TDebugText_DX8.h"
#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $Barnyard: FUNCTION 006da1e0
void TDebugD3DText::Reset()
{
	m_pCurrent = m_aBuffer;
}

// $Barnyard: FUNCTION 006da210
void TDebugD3DText::Destroy()
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

// $Barnyard: FUNCTION 006da070
void TDebugD3DText::Render()
{
	TRenderD3DInterface* pRenderer = TSTATICCAST( TRenderD3DInterface, m_pRenderer );

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
			SIZE oSize;
			m_pFont->GetTextExtent( pEntry->szText, &oSize );

			TINT iX = pEntry->iX - oSize.cx / 2;
			TINT iY = pEntry->iY - oSize.cy / 2;

			// NOTE: retail subtracts the half size twice before checking if the text is on the screen
			if ( iX - oSize.cx / 2 < iWidth && iY - oSize.cy / 2 < iHeight )
				m_pFont->DrawText( TFLOAT( iX ), TFLOAT( iY ), pEntry->uiColour, pEntry->szText, 0 );
		}
		else
		{
			m_pFont->DrawText( TFLOAT( pEntry->iX ), TFLOAT( pEntry->iY ), pEntry->uiColour, pEntry->szText, 0 );
		}
	}

	Reset();
}

// $Barnyard: FUNCTION 006da1b0
TDebugText::Entry* TDebugD3DText::AllocateEntry( TUINT16 a_uiLength )
{
	TUINT8* pEntry = m_pCurrent;

	if ( TUINT( ( pEntry - m_aBuffer ) + a_uiLength + 12 ) >= TUINT( m_iBufferSize ) )
		return TNULL;

	m_pCurrent = pEntry + a_uiLength + 12;
	return TREINTERPRETCAST( Entry*, pEntry );
}

// $Barnyard: FUNCTION 006da1f0
TBOOL TDebugD3DText::OnDeviceLost()
{
	m_pFont->InvalidateDeviceObjects();
	return TTRUE;
}

// $Barnyard: FUNCTION 006da200
TBOOL TDebugD3DText::OnDeviceFound()
{
	m_pFont->RestoreDeviceObjects();
	return TTRUE;
}

TOSHI_NAMESPACE_END
