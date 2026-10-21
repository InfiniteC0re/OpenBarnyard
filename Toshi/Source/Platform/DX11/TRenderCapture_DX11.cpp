#include "ToshiPCH.h"
#include "TRenderCapture_DX11.h"
#include "TRenderAdapter_DX11.h"
#include "TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TRenderCaptureD3D::TRenderCaptureD3D()
{
	m_iUnk1   = 0;
	m_bUnk2   = TFALSE;
	m_pBuffer = TNULL;
}

TRenderCaptureD3D::~TRenderCaptureD3D()
{
	if ( m_pBuffer )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}
}

TBOOL TRenderCaptureD3D::Create( FORMAT a_eFormat, TINT a_iWidth, TINT a_iHeight )
{
	m_eFormat = a_eFormat;
	m_iHeight = a_iHeight;
	m_iWidth  = a_iWidth;
	return TTRUE;
}

void TRenderCaptureD3D::Destroy()
{
	ReleaseBuffer();
}

// NOTE: D3D8 copies the front buffer here, the flip model swap chain of D3D11 has none that can be read,
// so the next frame is copied by TRenderD3DInterface::EndScene before it's presented and Poll waits for it
TBOOL TRenderCaptureD3D::Request()
{
	ReleaseBuffer();
	TRenderD3DInterface::Interface()->SetPendingCapture( this );

	return TTRUE;
}

// Copies the back buffer into the buffer, scaled down to the size of the capture
TBOOL TRenderCaptureD3D::CaptureBackBuffer( ID3D11Device* a_pDevice, ID3D11DeviceContext* a_pDeviceContext, ID3D11Texture2D* a_pBackBuffer )
{
	D3D11_TEXTURE2D_DESC oDesc;
	a_pBackBuffer->GetDesc( &oDesc );

	oDesc.Usage          = D3D11_USAGE_STAGING;
	oDesc.BindFlags      = 0;
	oDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	oDesc.MiscFlags      = 0;

	ID3D11Texture2D* pSurface;
	if ( FAILED( a_pDevice->CreateTexture2D( &oDesc, TNULL, &pSurface ) ) )
		return TFALSE;

	a_pDeviceContext->CopyResource( pSurface, a_pBackBuffer );

	D3D11_MAPPED_SUBRESOURCE oMapped;
	if ( FAILED( a_pDeviceContext->Map( pSurface, 0, D3D11_MAP_READ, 0, &oMapped ) ) )
	{
		pSurface->Release();
		return TFALSE;
	}

	TUINT uiBufferSize = 4 * m_iWidth * m_iHeight;
	m_pBuffer          = TSTATICCAST( TUINT8, TMalloc( uiBufferSize ) );
	TUtil::MemSet( m_pBuffer, 0xFF, uiBufferSize );

	TUINT uiStepX = oDesc.Width / m_iWidth;
	TUINT uiStepY = oDesc.Height / m_iHeight;

	// The output is RGBA
	const TBOOL bBGRA = oDesc.Format == DXGI_FORMAT_B8G8R8A8_UNORM;

	TUINT8* pOutPixel = m_pBuffer;
	TUINT   uiY       = 0;
	TUINT   uiRow     = 0;

	do
	{
		const TUINT8* pInPixel = TSTATICCAST( TUINT8, oMapped.pData ) + oMapped.RowPitch * uiY;

		for ( TUINT uiColumn = 0; uiColumn < TUINT( m_iWidth ); uiColumn++ )
		{
			// Alpha stays 0xFF
			pOutPixel[ 0 ] = pInPixel[ bBGRA ? 2 : 0 ];
			pOutPixel[ 1 ] = pInPixel[ 1 ];
			pOutPixel[ 2 ] = pInPixel[ bBGRA ? 0 : 2 ];

			pOutPixel += 4;
			pInPixel += 4 * uiStepX;
		}

		uiY += uiStepY;
		uiRow++;
	} while ( uiRow < TUINT( m_iHeight ) );

	a_pDeviceContext->Unmap( pSurface, 0 );
	pSurface->Release();

	return TTRUE;
}

void TRenderCaptureD3D::ReleaseBuffer()
{
	if ( m_pBuffer )
	{
		TFree( m_pBuffer );
		m_pBuffer = TNULL;
	}
}

TOSHI_NAMESPACE_END
