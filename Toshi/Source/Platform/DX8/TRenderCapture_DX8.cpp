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
#ifdef BARNYARD_COMMUNITY_PATCH
	if ( ms_pPendingCapture == this )
		ms_pPendingCapture = TNULL;
#endif

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

#ifdef BARNYARD_COMMUNITY_PATCH
	ms_pPendingCapture = this;
	return TTRUE;
#else
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
#ifdef BARNYARD_COMMUNITY_PATCH
	GetClientRect( pRender->GetMSWindow()->GetHWND(), &oRect );
	MapWindowPoints( pRender->GetMSWindow()->GetHWND(), HWND_DESKTOP, (LPPOINT)&oRect, 2 );
#else
	GetWindowRect( pRender->GetMSWindow()->GetHWND(), &oRect );
#endif // BARNYARD_COMMUNITY_PATCH

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
#endif // BARNYARD_COMMUNITY_PATCH
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

#ifdef BARNYARD_COMMUNITY_PATCH

void TRenderCaptureD3D::CapturePending( IDirect3DDevice8* a_pDevice )
{
	if ( !ms_pPendingCapture )
		return;

	TRenderCaptureD3D* pCapture = ms_pPendingCapture;
	ms_pPendingCapture          = TNULL;

	if ( !pCapture->CaptureBackBuffer( a_pDevice ) )
		TWARN( "TRenderCaptureD3D: couldn't read the back buffer\n" );
}

// Copies the back buffer into the buffer, scaled down to the size of the capture
TBOOL TRenderCaptureD3D::CaptureBackBuffer( IDirect3DDevice8* a_pDevice )
{
	IDirect3DSurface8* pBackBuffer = TNULL;
	if ( FAILED( a_pDevice->GetBackBuffer( 0, D3DBACKBUFFER_TYPE_MONO, &pBackBuffer ) ) )
		return TFALSE;

	D3DSURFACE_DESC oDesc;
	pBackBuffer->GetDesc( &oDesc );

	const TBOOL b32Bit = oDesc.Format == D3DFMT_X8R8G8B8 || oDesc.Format == D3DFMT_A8R8G8B8;
	const TBOOL b565   = oDesc.Format == D3DFMT_R5G6B5;
	const TBOOL b555   = oDesc.Format == D3DFMT_X1R5G5B5 || oDesc.Format == D3DFMT_A1R5G5B5;

	if ( !b32Bit && !b565 && !b555 )
	{
		TWARN( "TRenderCaptureD3D: unsupported back buffer format %d\n", oDesc.Format );
		pBackBuffer->Release();
		return TFALSE;
	}

	IDirect3DSurface8* pSurface = TNULL;
	if ( FAILED( a_pDevice->CreateImageSurface( oDesc.Width, oDesc.Height, oDesc.Format, &pSurface ) ) )
	{
		pBackBuffer->Release();
		return TFALSE;
	}

	HRESULT hRes = a_pDevice->CopyRects( pBackBuffer, TNULL, 0, pSurface, TNULL );
	pBackBuffer->Release();

	D3DLOCKED_RECT oLockedRect;
	if ( FAILED( hRes ) || FAILED( pSurface->LockRect( &oLockedRect, TNULL, D3DLOCK_READONLY ) ) )
	{
		pSurface->Release();
		return TFALSE;
	}

	TUINT uiBufferSize = 4 * m_iWidth * m_iHeight;
	m_pBuffer          = TSTATICCAST( TUINT8, TMalloc( uiBufferSize ) );
	TUtil::MemSet( m_pBuffer, 0xFF, uiBufferSize );

	const TUINT uiBytesPerPixel = b32Bit ? 4 : 2;
	const TUINT uiStepX         = oDesc.Width / m_iWidth;
	const TUINT uiStepY         = oDesc.Height / m_iHeight;

	TUINT8* pOutPixel = m_pBuffer;

	for ( TUINT uiRow = 0; uiRow < TUINT( m_iHeight ); uiRow++ )
	{
		const TUINT8* pInPixel = TSTATICCAST( TUINT8, oLockedRect.pBits ) + oLockedRect.Pitch * ( uiRow * uiStepY );

		for ( TUINT uiColumn = 0; uiColumn < TUINT( m_iWidth ); uiColumn++ )
		{
			// To RGBA, alpha stays 0xFF
			if ( b32Bit )
			{
				pOutPixel[ 0 ] = pInPixel[ 2 ];
				pOutPixel[ 1 ] = pInPixel[ 1 ];
				pOutPixel[ 2 ] = pInPixel[ 0 ];
			}
			else
			{
				const TUINT16 uiPixel = *TREINTERPRETCAST( const TUINT16*, pInPixel );

				if ( b565 )
				{
					pOutPixel[ 0 ] = TUINT8( ( ( uiPixel >> 11 ) & 0x1F ) * 255 / 31 );
					pOutPixel[ 1 ] = TUINT8( ( ( uiPixel >> 5 ) & 0x3F ) * 255 / 63 );
				}
				else
				{
					pOutPixel[ 0 ] = TUINT8( ( ( uiPixel >> 10 ) & 0x1F ) * 255 / 31 );
					pOutPixel[ 1 ] = TUINT8( ( ( uiPixel >> 5 ) & 0x1F ) * 255 / 31 );
				}

				pOutPixel[ 2 ] = TUINT8( ( uiPixel & 0x1F ) * 255 / 31 );
			}

			pOutPixel += 4;
			pInPixel += uiBytesPerPixel * uiStepX;
		}
	}

	pSurface->UnlockRect();
	pSurface->Release();

	return TTRUE;
}

#endif // BARNYARD_COMMUNITY_PATCH

TOSHI_NAMESPACE_END
