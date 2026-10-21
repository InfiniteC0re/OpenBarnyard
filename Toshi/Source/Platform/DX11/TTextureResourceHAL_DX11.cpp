#include "ToshiPCH.h"
#include "TTextureResourceHAL_DX11.h"

#include "T2Texture_DX11.h"
#include "TRenderInterface_DX11.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

TDEFINE_CLASS( TTextureResourceHAL );
TDEFINE_FREELIST_ALLOCATOR( TTextureResourceHAL );

TTextureResourceHAL::TTextureResourceHAL()
{
	m_iNumLocks       = 0;
	m_bLoadFromMemory = TFALSE;
	m_bIsToshi2       = TFALSE;
	m_pData           = TNULL;
	m_uiDataSize      = 0;
	m_pD3DTexture     = TNULL;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;
	m_pLockTexture    = TNULL;
	TUtil::MemClear( &m_ImageInfo, sizeof( m_ImageInfo ) );
}

TTextureResourceHAL::~TTextureResourceHAL()
{
}

TUINT TTextureResourceHAL::GetWidth()
{
	Validate();
	return m_ImageInfo.Width;
}

TUINT TTextureResourceHAL::GetHeight()
{
	Validate();
	return m_ImageInfo.Height;
}

TBOOL TTextureResourceHAL::Lock( LOCKSTATE& a_rLockState )
{
	if ( m_pD3DTexture && !m_pLockTexture )
	{
		auto pRenderer      = TRenderD3DInterface::Interface();
		auto pDeviceContext = pRenderer->GetDeviceContext();

		ID3D11Texture2D* pTexture = TNULL;
		m_pD3DTexture->GetResource( (ID3D11Resource**)&pTexture );

		D3D11_TEXTURE2D_DESC oDesc;
		pTexture->GetDesc( &oDesc );
		oDesc.MipLevels      = 1;
		oDesc.Usage          = D3D11_USAGE_STAGING;
		oDesc.BindFlags      = 0;
		oDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ | D3D11_CPU_ACCESS_WRITE;
		oDesc.MiscFlags      = 0;

		HRESULT hRes = pRenderer->GetDevice()->CreateTexture2D( &oDesc, TNULL, &m_pLockTexture );

		if ( SUCCEEDED( hRes ) )
		{
			pDeviceContext->CopySubresourceRegion( m_pLockTexture, 0, 0, 0, 0, pTexture, 0, TNULL );

			D3D11_MAPPED_SUBRESOURCE oMapped;
			hRes = pDeviceContext->Map( m_pLockTexture, 0, D3D11_MAP_READ_WRITE, 0, &oMapped );

			if ( SUCCEEDED( hRes ) )
			{
				a_rLockState.Pitch = oMapped.RowPitch;
				a_rLockState.pBits = oMapped.pData;
				m_iNumLocks += 1;
			}
			else
			{
				m_pLockTexture->Release();
				m_pLockTexture = TNULL;
			}
		}

		pTexture->Release();

		if ( FAILED( hRes ) )
		{
			TRenderD3DInterface::PrintError( hRes, "Unable to lock texture!" );
			return TFALSE;
		}

		return TTRUE;
	}

	return TFALSE;
}

void TTextureResourceHAL::Unlock()
{
	TASSERT( m_iNumLocks != 0 );

	if ( m_iNumLocks != 0 && m_pD3DTexture && m_pLockTexture )
	{
		auto pDeviceContext = TRenderD3DInterface::Interface()->GetDeviceContext();
		pDeviceContext->Unmap( m_pLockTexture, 0 );

		ID3D11Resource* pTexture = TNULL;
		m_pD3DTexture->GetResource( &pTexture );
		pDeviceContext->CopySubresourceRegion( pTexture, 0, 0, 0, 0, m_pLockTexture, 0, TNULL );
		pTexture->Release();

		m_pLockTexture->Release();
		m_pLockTexture = TNULL;
		m_iNumLocks -= 1;
	}
}

void TTextureResourceHAL::CreateFromT2Texture( T2Texture* a_pTexture )
{
	m_bIsToshi2       = TTRUE;
	m_uiWidth         = a_pTexture->GetWidth();
	m_uiHeight        = a_pTexture->GetHeight();
	m_uiMipLevels     = a_pTexture->GetMipLevels();
	m_uiMipFlags      = 0;
	m_eResourceFormat = TTEXTURERESOURCEFORMAT::Unknown;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;
	m_ImageInfo       = a_pTexture->GetImageInfo();
	m_pD3DTexture     = a_pTexture->GetD3DTexture();

	TResource::Create();
	TResource::Validate();
}

TBOOL TTextureResourceHAL::Create( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight )
{
	if ( !CreateResource( a_pData, a_uiDataSize, a_eTextureFlags, a_uiWidth, a_uiHeight ) )
	{
		return TFALSE;
	}

	m_eTextureFlags   = a_eTextureFlags;
	m_uiWidth         = a_uiWidth;
	m_uiHeight        = a_uiHeight;
	m_uiMipLevels     = 0;
	m_eResourceFormat = TTEXTURERESOURCEFORMAT::Unknown;
	m_uiMipFlags      = MIPFLAGS_USE_MIPLEVELS;
	m_bLoadFromMemory = TTRUE;
	m_pData           = a_pData;
	m_uiDataSize      = a_uiDataSize;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;

	Validate();
	return TTRUE;
}

TBOOL TTextureResourceHAL::Create( const TCHAR* a_szFileName, TUINT a_eTextureFlags )
{
	if ( !CreateResource( a_szFileName, a_eTextureFlags ) )
	{
		return TFALSE;
	}

	m_eTextureFlags  = a_eTextureFlags;
	m_eAddressUState = ADDRESSINGMODE_WRAP;
	m_eAddressVState = ADDRESSINGMODE_WRAP;
	m_uiMipFlags     = MIPFLAGS_USE_MIPLEVELS;

	return TTRUE;
}

TBOOL TTextureResourceHAL::CreateEx( void* a_pData, TUINT a_uiDataSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiMipLevels, TTEXTURERESOURCEFORMAT a_eFormat, TUINT a_uiMipMapFlags )
{
	if ( !CreateResource( a_pData, a_uiDataSize, a_uiWidth, a_uiHeight, a_uiMipLevels, a_eFormat, a_uiMipMapFlags ) )
	{
		return TFALSE;
	}

	m_uiWidth         = a_uiWidth;
	m_pData           = a_pData;
	m_uiHeight        = a_uiHeight;
	m_uiDataSize      = a_uiDataSize;
	m_eTextureFlags   = TEXTUREFLAGS_RESOURCEFORMAT;
	m_uiMipLevels     = a_uiMipLevels;
	m_eResourceFormat = a_eFormat;
	m_uiMipFlags      = a_uiMipMapFlags;
	m_bLoadFromMemory = TTRUE;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;

	Validate();
	return TTRUE;
}

TBOOL TTextureResourceHAL::CreateFromFormat()
{
	TUINT uiMipLevels = ( m_uiMipFlags & MIPFLAGS_USE_MIPLEVELS ) ? m_uiMipLevels : 0;

	switch ( m_eResourceFormat )
	{
		case TTEXTURERESOURCEFORMAT::R8G8B8A8:
			return CreateFromMemory8888( m_uiWidth, m_uiHeight, uiMipLevels, m_pData );
		case TTEXTURERESOURCEFORMAT::R8G8B8:
			return CreateFromMemory888( m_uiWidth, m_uiHeight, uiMipLevels, m_pData );
		case TTEXTURERESOURCEFORMAT::R5G5B5A1:
			return CreateFromMemory5551( m_uiWidth, m_uiHeight, uiMipLevels, m_pData );
		case TTEXTURERESOURCEFORMAT::DDS:
			return CreateFromMemoryDDS( m_uiWidth, m_uiHeight, uiMipLevels, m_pData );
		case TTEXTURERESOURCEFORMAT::R4G4B4A4:
			return CreateFromMemory4444( m_uiWidth, m_uiHeight, uiMipLevels, m_pData );
		default:
			TASSERT( TFALSE, "Unknown format!" );
			return TFALSE;
	}
}

template <class TPixel, class TSourcePixel, class Converter>
static TBOOL CreateFromMemoryImpl( ID3D11ShaderResourceView*& a_rpD3DTexture, D3D11_TEXTURE2D_DESC& a_rImageInfo, DXGI_FORMAT a_eFormat, TUINT a_uiPixelSize, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData, Converter a_fnConvert )
{
	auto pRenderer = TRenderD3DInterface::Interface();

	TBYTE*        pPixels    = new TBYTE[ a_uiWidth * a_uiHeight * a_uiPixelSize ];
	TPixel*       pTexPixel  = TREINTERPRETCAST( TPixel*, pPixels );
	TSourcePixel* pDataPixel = TSTATICCAST( TSourcePixel, a_pData );

	for ( TUINT i = a_uiHeight; i != 0; i-- )
	{
		for ( TUINT j = a_uiWidth; j != 0; j-- )
			a_fnConvert( pTexPixel, pDataPixel );
	}

	UINT uiSupport = 0;
	pRenderer->GetDevice()->CheckFormatSupport( a_eFormat, &uiSupport );

	const TBOOL bGenerateMipmaps = a_uiLevels == 0 && HASANYFLAG( uiSupport, D3D11_FORMAT_SUPPORT_MIP_AUTOGEN );

	a_rpD3DTexture = pRenderer->CreateTexture( a_uiWidth, a_uiHeight, a_eFormat, pPixels, bGenerateMipmaps ? TRenderD3DInterface::TEXTUREFLAGS_GENERATEMIPMAPS : TRenderD3DInterface::TEXTUREFLAGS_NONE, D3D11_USAGE_DEFAULT, 0, 1 );
	delete[] pPixels;

	if ( !a_rpD3DTexture )
	{
		TRenderD3DInterface::PrintError( E_FAIL, "Unable to create a texture!" );
		return TTRUE;
	}

	ID3D11Texture2D* pTexture = TNULL;
	a_rpD3DTexture->GetResource( (ID3D11Resource**)&pTexture );
	pTexture->GetDesc( &a_rImageInfo );
	pTexture->Release();

	return TTRUE;
}

TBOOL TTextureResourceHAL::CreateFromMemory8888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels
	return CreateFromMemoryImpl<TUINT32, TUINT32>( m_pD3DTexture, m_ImageInfo, DXGI_FORMAT_B8G8R8A8_UNORM, 4, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT32*& a_rpOut, TUINT32*& a_rpIn ) {
		TUINT32 uiPixel = *( a_rpIn++ );
		*( a_rpOut++ )  = ( uiPixel & 0xFF00FF00 ) | ( ( uiPixel & 0xFF ) << 16 ) | ( ( uiPixel >> 16 ) & 0xFF );
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemory888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels and adds an opaque alpha
	return CreateFromMemoryImpl<TUINT8, TUINT8>( m_pD3DTexture, m_ImageInfo, DXGI_FORMAT_B8G8R8X8_UNORM, 4, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT8*& a_rpOut, TUINT8*& a_rpIn ) {
		a_rpOut[ 0 ] = a_rpIn[ 2 ];
		a_rpOut[ 1 ] = a_rpIn[ 1 ];
		a_rpOut[ 2 ] = a_rpIn[ 0 ];
		a_rpOut[ 3 ] = 0xFF;

		a_rpOut += 4;
		a_rpIn += 3;
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemory5551( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels
	return CreateFromMemoryImpl<TUINT16, TUINT16>( m_pD3DTexture, m_ImageInfo, DXGI_FORMAT_B5G5R5A1_UNORM, 2, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT16*& a_rpOut, TUINT16*& a_rpIn ) {
		TUINT16 uiPixel = *( a_rpIn++ );
		*( a_rpOut++ )  = TUINT16( ( ( uiPixel >> 10 ) & 0x1F ) | ( ( ( uiPixel >> 5 ) & 0x1F ) << 5 ) | ( ( uiPixel & 0x1F ) << 10 ) | ( ( uiPixel >> 15 ) << 15 ) );
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemory4444( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels
	return CreateFromMemoryImpl<TUINT16, TUINT16>( m_pD3DTexture, m_ImageInfo, DXGI_FORMAT_B4G4R4A4_UNORM, 2, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT16*& a_rpOut, TUINT16*& a_rpIn ) {
		TUINT16 uiPixel = *( a_rpIn++ );
		*( a_rpOut++ )  = TUINT16( ( ( uiPixel >> 8 ) & 0xF ) | ( ( ( uiPixel >> 4 ) & 0xF ) << 4 ) | ( ( uiPixel & 0xF ) << 8 ) | ( ( uiPixel >> 12 ) << 12 ) );
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemoryDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	m_pD3DTexture = TRenderD3DInterface::Interface()->CreateTextureFromFileInMemory( a_pData, m_uiDataSize, &m_ImageInfo );
	return m_pD3DTexture != TNULL;
}

TBOOL TTextureResourceHAL::CreateFromFileDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, const TCHAR* a_szFile )
{
	TFile* pFile = TFile::Create( a_szFile );

	if ( !pFile )
	{
		return TFALSE;
	}

	TUINT  uiSize = pFile->GetSize();
	TBYTE* pData  = new TBYTE[ uiSize ];
	pFile->Read( pData, uiSize );
	pFile->Destroy();

	m_pD3DTexture = TRenderD3DInterface::Interface()->CreateTextureFromFileInMemory( pData, uiSize, &m_ImageInfo );
	delete[] pData;

	return m_pD3DTexture != TNULL;
}

TBOOL TTextureResourceHAL::IsPPM( const TCHAR* a_szName )
{
	auto iLen = TStringManager::String8Length( a_szName );

	if ( iLen >= 4 )
	{
		return TStringManager::String8CompareNoCase( a_szName + iLen - 4, ".ppm" ) == 0;
	}

	return TFALSE;
}

TBOOL TTextureResourceHAL::Validate()
{
	if ( IsCreated() && IsValid() )
	{
		return TTRUE;
	}

	TMemory::HALMemInfo memInfoHAL;
	TMemory::GetHALMemInfo( memInfoHAL );

	if ( m_bLoadFromMemory )
	{
		// Load from memory
		if ( m_pData && m_uiDataSize != 0 )
		{
			if ( HASANYFLAG( m_eTextureFlags, TEXTUREFLAGS_RESOURCEFORMAT ) )
			{
				if ( !CreateFromFormat() )
				{
					return TFALSE;
				}
			}
			else
			{
				TUINT uiLayout = m_eTextureFlags & TEXTUREFLAGS_LAYOUT_MASK;

				if ( uiLayout == TEXTUREFLAGS_LAYOUT_5551 )
				{
					CreateFromMemory5551( m_uiWidth, m_uiHeight, 0, m_pData );
				}
				else if ( uiLayout == TEXTUREFLAGS_LAYOUT_4444 )
				{
					CreateFromMemory4444( m_uiWidth, m_uiHeight, 0, m_pData );
				}
				else if ( uiLayout == TEXTUREFLAGS_LAYOUT_8888 )
				{
					CreateFromMemory8888( m_uiWidth, m_uiHeight, 0, m_pData );
				}
				else
				{
					CreateFromMemoryDDS( m_uiWidth, m_uiHeight, -1, m_pData );
				}
			}
		}
	}
	else
	{
		// Load from file
		if ( m_pNameEntry && !IsPPM( m_pNameEntry->GetName() ) )
		{
			CreateFromFileDDS( m_uiWidth, m_uiHeight, -1, m_pNameEntry->GetName() );
		}
	}

	TMemory::GetHALMemInfo( memInfoHAL );
	s_iHALMemoryUsage += memInfoHAL.m_uiMemUsage;

	return TResource::Validate();
}

void TTextureResourceHAL::Invalidate()
{
	if ( IsCreated() && IsValid() )
	{
		if ( m_bIsToshi2 == TTRUE )
		{
			m_pD3DTexture = TNULL;
		}
		else if ( m_pD3DTexture )
		{
			TMemory::HALMemInfo memInfoHAL;
			TMemory::GetHALMemInfo( memInfoHAL );

			if ( TRenderD3DInterface* pRender = TRenderD3DInterface::Interface() )
				pRender->InvalidateShaderResource( m_pD3DTexture );

			m_pD3DTexture->Release();
			m_pD3DTexture = TNULL;

			TMemory::GetHALMemInfo( memInfoHAL );
			s_iHALMemoryUsage -= memInfoHAL.m_uiMemUsage;
		}

		TResource::Invalidate();
	}
}

TOSHI_NAMESPACE_END
