#include "ToshiPCH.h"
#include "TRenderCapture_DX8.h"
#include "TRenderAdapter_DX8.h"
#include "TRenderInterface_DX8.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: inlined into TRenderD3DInterface::CreateCapture (006c6240)
TRenderCaptureD3D::TRenderCaptureD3D()
{
	m_iUnk1   = 0;
	m_bUnk2   = TFALSE;
	m_pBuffer = TNULL;
}

// $Barnyard: FUNCTION 006d9d40
TRenderCaptureD3D::~TRenderCaptureD3D()
{
	if ( m_pBuffer )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}
}

// $Barnyard: FUNCTION 006d9ae0
TBOOL TRenderCaptureD3D::Create( FORMAT a_eFormat, TINT a_iWidth, TINT a_iHeight )
{
	m_eFormat = a_eFormat;
	m_iHeight = a_iHeight;
	m_iWidth  = a_iWidth;
	return TTRUE;
}

// $Barnyard: FUNCTION 006d9b00
void TRenderCaptureD3D::Destroy()
{
	ReleaseBuffer();
}

// $Barnyard: FUNCTION 006d9b10
// Copies the front buffer into the buffer, scaled down to the size of the capture
TBOOL TRenderCaptureD3D::Request()
{
	ReleaseBuffer();

	TRenderD3DInterface* pRender  = TRenderD3DInterface::Interface();
	IDirect3DDevice8*    pDevice  = pRender->GetDirect3DDevice();
	TD3DAdapter::Mode*   pMode    = TSTATICCAST( TD3DAdapter::Mode, pRender->GetCurrentDevice()->GetMode() );
	TD3DAdapter*         pAdapter = TSTATICCAST( TD3DAdapter, pMode->GetAdapter() );

	// The front buffer has the size of the desktop
	const D3DDISPLAYMODE& rcDesktopMode = pAdapter->GetMode().GetD3DDisplayMode();

	IDirect3DSurface8* pSurface;
	if ( FAILED( pDevice->CreateImageSurface( rcDesktopMode.Width, rcDesktopMode.Height, D3DFMT_A8R8G8B8, &pSurface ) ) ||
	     FAILED( pDevice->GetFrontBuffer( pSurface ) ) )
	{
		return TFALSE;
	}

	RECT oRect;
	GetWindowRect( pRender->GetMSWindow()->GetHWND(), &oRect );

	if ( pRender->GetCurrentDisplayParams()->bWindowed != TTRUE )
	{
		oRect.left   = 0;
		oRect.top    = 0;
		oRect.right  = pMode->GetWidth();
		oRect.bottom = pMode->GetHeight();
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original reads outside of the surface when the window isn't fully on the primary monitor
	TMath::Clip<LONG>( oRect.left, 0, LONG( rcDesktopMode.Width ) - 1 );
	TMath::Clip<LONG>( oRect.right, oRect.left, LONG( rcDesktopMode.Width ) );
	TMath::Clip<LONG>( oRect.top, 0, LONG( rcDesktopMode.Height ) - 1 );
	TMath::Clip<LONG>( oRect.bottom, oRect.top, LONG( rcDesktopMode.Height ) );
#endif // BARNYARD_COMMUNITY_PATCH

	D3DLOCKED_RECT oLockedRect;
	if ( FAILED( pSurface->LockRect( &oLockedRect, TNULL, D3DLOCK_READONLY ) ) )
	{
		pSurface->Release();
		return TFALSE;
	}

	TUINT uiBufferSize = 4 * m_iWidth * m_iHeight;
	m_pBuffer          = TSTATICCAST( TUINT8, TMalloc( uiBufferSize ) );
	TUtil::MemSet( m_pBuffer, 0xFF, uiBufferSize );

	TUINT uiStepX = TUINT( oRect.right - oRect.left ) / m_iWidth;
	TUINT uiStepY = TUINT( oRect.bottom - oRect.top ) / m_iHeight;

	TUINT8* pOutPixel = m_pBuffer;
	TUINT   uiY       = 0;
	TUINT   uiRow     = 0;

	do
	{
		const TUINT8* pInPixel = TSTATICCAST( TUINT8, oLockedRect.pBits ) + oLockedRect.Pitch * ( uiY + oRect.top ) + 4 * oRect.left;

		for ( TUINT uiColumn = 0; uiColumn < TUINT( m_iWidth ); uiColumn++ )
		{
			// BGRA to RGBA, alpha stays 0xFF
			pOutPixel[ 0 ] = pInPixel[ 2 ];
			pOutPixel[ 1 ] = pInPixel[ 1 ];
			pOutPixel[ 2 ] = pInPixel[ 0 ];

			pOutPixel += 4;
			pInPixel += 4 * uiStepX;
		}

		uiY += uiStepY;
		uiRow++;
	} while ( uiRow < TUINT( m_iHeight ) );

	pSurface->UnlockRect();
	pSurface->Release();

	return TTRUE;
}

// $Barnyard: FUNCTION 006d9d20
void TRenderCaptureD3D::ReleaseBuffer()
{
	if ( m_pBuffer )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}
}

TOSHI_NAMESPACE_END
