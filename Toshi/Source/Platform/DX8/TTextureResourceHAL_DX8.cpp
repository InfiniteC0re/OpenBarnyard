#include "ToshiPCH.h"
#include "TTextureResourceHAL_DX8.h"

#include "T2Texture_DX8.h"
#include "TRenderInterface_DX8.h"

#include <d3dtypes.h>

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
}

// $Barnyard: FUNCTION 006c0d50
TUINT TTextureResourceHAL::GetWidth()
{
	Validate();
	return m_ImageInfo.Width;
}

// $Barnyard: FUNCTION 006c0d60
TUINT TTextureResourceHAL::GetHeight()
{
	Validate();
	return m_ImageInfo.Height;
}

// $Barnyard: FUNCTION 006c0d70
TBOOL TTextureResourceHAL::Lock( LOCKSTATE& a_rLockState )
{
	if ( m_pD3DTexture )
	{
		D3DLOCKED_RECT rect;

		HRESULT hRes = m_pD3DTexture->LockRect( 0, &rect, NULL, 0 );
		TASSERT( SUCCEEDED( hRes ) );

		if ( FAILED( hRes ) )
		{
			TRenderD3DInterface::PrintError( hRes, "Unable to lock texture!" );
		}

		if ( SUCCEEDED( hRes ) )
		{
			a_rLockState.Pitch = rect.Pitch;
			a_rLockState.pBits = rect.pBits;
			m_iNumLocks += 1;

			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006c0dd0
void TTextureResourceHAL::Unlock()
{
	TASSERT( m_iNumLocks != 0 );

	if ( m_iNumLocks != 0 && m_pD3DTexture )
	{
		m_pD3DTexture->UnlockRect( 0 );
		m_iNumLocks -= 1;
	}
}

// $Barnyard: FUNCTION 006c0cc0
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

// $Barnyard: FUNCTION 006c0bb0
TBOOL TTextureResourceHAL::Create( void* a_pData, TUINT a_uiDataSize, TUINT a_eTextureFlags, TUINT a_uiWidth, TUINT a_uiHeight )
{
	TWARN( "Deprecated! Please use: TTextureResourceHAL::CreateEx()\n" );

	if ( !CreateResource( a_pData, a_uiDataSize, a_eTextureFlags, a_uiWidth, a_uiHeight ) )
	{
		return TFALSE;
	}

	m_eTextureFlags   = a_eTextureFlags;
	m_uiWidth         = a_uiWidth;
	m_uiHeight        = a_uiHeight;
	m_uiMipLevels     = 0;
	m_eResourceFormat = TTEXTURERESOURCEFORMAT::Unknown;
	m_uiMipFlags      = 1;
	m_bLoadFromMemory = TTRUE;
	m_pData           = a_pData;
	m_uiDataSize      = a_uiDataSize;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;

	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 006c0b70
TBOOL TTextureResourceHAL::Create( const TCHAR* a_szFileName, TUINT a_eTextureFlags )
{
	if ( !CreateResource( a_szFileName, a_eTextureFlags ) )
	{
		return TFALSE;
	}

	m_eTextureFlags  = a_eTextureFlags;
	m_eAddressUState = ADDRESSINGMODE_WRAP;
	m_eAddressVState = ADDRESSINGMODE_WRAP;
	m_uiMipFlags     = 1;

	return TTRUE;
}

// $Barnyard: FUNCTION 006c0c30
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
	m_eTextureFlags   = 0x40;
	m_uiMipLevels     = a_uiMipLevels;
	m_eResourceFormat = a_eFormat;
	m_uiMipFlags      = a_uiMipMapFlags;
	m_bLoadFromMemory = TTRUE;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;

	Validate();
	return TTRUE;
}

// $Barnyard: FUNCTION 006c1280
TBOOL TTextureResourceHAL::CreateFromFormat()
{
	TUINT uiMipLevels = ( m_uiMipFlags & 1 ) ? m_uiMipLevels : 0;

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

// $Barnyard: FUNCTION 006c0ff0
TBOOL TTextureResourceHAL::CreateFromMemory8888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	TIMPLEMENT();
	return TFALSE;
}

// $Barnyard: FUNCTION 006c10f0
TBOOL TTextureResourceHAL::CreateFromMemory888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, GetRenderer() );

	HRESULT hCreateRes = pRenderer->GetDirect3DDevice()->CreateTexture(
	    a_uiWidth,
	    a_uiHeight,
	    a_uiLevels,
	    0,
	    D3DFMT_X8R8G8B8,
	    D3DPOOL_MANAGED,
	    &m_pD3DTexture
	);

	if ( SUCCEEDED( hCreateRes ) )
	{
		D3DLOCKED_RECT rect;
		m_pD3DTexture->LockRect( 0, &rect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK );

		if ( a_uiHeight != 0 )
		{
			TUINT8* pTexPixel  = TSTATICCAST( TUINT8, rect.pBits );
			TUINT8* pDataPixel = TSTATICCAST( TUINT8, a_pData );

			for ( TUINT i = a_uiHeight; i != 0; i-- )
			{
				TUINT8 uiDataPixel = *pDataPixel;
				pTexPixel[ 0 ]     = pDataPixel[ 2 ];
				pTexPixel[ 1 ]     = pDataPixel[ 1 ];
				pTexPixel[ 2 ]     = pDataPixel[ 0 ];
				pTexPixel[ 3 ]     = 255;

				pTexPixel += 4;
				pDataPixel += 3;
			}
		}

		m_pD3DTexture->UnlockRect( 0 );

		if ( a_uiLevels == 0 )
		{
			TFIXME( "Call some D3DX? function with 4 parameters with the first one being IDirect3DTexture (most likely not related to the engine)" );
			TUtil::MemClear( &m_ImageInfo, sizeof( m_ImageInfo ) );
			m_ImageInfo.Width  = a_uiWidth;
			m_ImageInfo.Height = a_uiHeight;
		}

		return TTRUE;
	}
	else
	{
		TRenderD3DInterface::PrintError( hCreateRes, TNULL );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c0df0
TBOOL TTextureResourceHAL::CreateFromMemory5551( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	TIMPLEMENT();
	return TFALSE;
}

// $Barnyard: FUNCTION 006c0ef0
TBOOL TTextureResourceHAL::CreateFromMemory4444( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, GetRenderer() );

	HRESULT hCreateRes = pRenderer->GetDirect3DDevice()->CreateTexture(
	    a_uiWidth,
	    a_uiHeight,
	    a_uiLevels,
	    0,
	    D3DFMT_A4R4G4B4,
	    D3DPOOL_MANAGED,
	    &m_pD3DTexture
	);

	if ( SUCCEEDED( hCreateRes ) )
	{
		D3DLOCKED_RECT rect;
		m_pD3DTexture->LockRect( 0, &rect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK );

		if ( a_uiHeight != 0 )
		{
			TUINT16* pTexPixel  = TSTATICCAST( TUINT16, rect.pBits );
			TUINT16* pDataPixel = TSTATICCAST( TUINT16, a_pData );

			for ( TUINT i = a_uiHeight; i != 0; i-- )
			{
				TUINT16 uiDataPixel = *pDataPixel;
				*pTexPixel          = ( ( ( ( uiDataPixel >> 12 ) << 4 | uiDataPixel & 15 ) << 4 | uiDataPixel >> 4 & 15 ) << 4 ) | uiDataPixel >> 8 & 15;
				pTexPixel++;
				pDataPixel++;
			}
		}

		m_pD3DTexture->UnlockRect( 0 );

		if ( a_uiLevels == 0 )
		{
			TFIXME( "Call some D3DX? function with 4 parameters with the first one being IDirect3DTexture (most likely not related to the engine)" );
			TUtil::MemClear( &m_ImageInfo, sizeof( m_ImageInfo ) );
			m_ImageInfo.Width  = a_uiWidth;
			m_ImageInfo.Height = a_uiHeight;
		}

		return TTRUE;
	}
	else
	{
		TRenderD3DInterface::PrintError( hCreateRes, TNULL );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c11e0
TBOOL TTextureResourceHAL::CreateFromMemoryDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, GetRenderer() );

	HRESULT hRes = D3DXCreateTextureFromFileInMemoryEx(
	    pRenderer->GetDirect3DDevice(),
	    a_pData,
	    m_uiDataSize,
	    -1,
	    -1,
	    a_uiLevels,
	    0,
	    D3DFMT_UNKNOWN,
	    D3DPOOL_MANAGED,
	    D3DFILTER_LINEARMIPNEAREST,
	    D3DFILTER_LINEARMIPNEAREST,
	    0,
	    &m_ImageInfo,
	    NULL,
	    &m_pD3DTexture
	);

	if ( FAILED( hRes ) )
	{
		TRenderD3DInterface::PrintError( hRes, TNULL );
	}

	return hRes == S_OK;
}

TBOOL TTextureResourceHAL::CreateFromFileDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, const TCHAR* a_szFile )
{
	auto pRenderer = TSTATICCAST( TRenderD3DInterface, GetRenderer() );

	HRESULT hRes = D3DXCreateTextureFromFileExA(
	    pRenderer->GetDirect3DDevice(),
	    a_szFile,
	    -1,
	    -1,
	    a_uiLevels,
	    0,
	    D3DFMT_UNKNOWN,
	    D3DPOOL_MANAGED,
	    D3DFILTER_LINEARMIPNEAREST,
	    D3DFILTER_LINEARMIPNEAREST,
	    0,
	    &m_ImageInfo,
	    NULL,
	    &m_pD3DTexture
	);

	if ( FAILED( hRes ) )
	{
		TRenderD3DInterface::PrintError( hRes, TNULL );
	}

	return hRes == S_OK;
}

// $Barnyard: FUNCTION 006c1240
TBOOL TTextureResourceHAL::IsPPM( const TCHAR* a_szName )
{
	auto iLen = TStringManager::String8Length( a_szName );

	if ( iLen >= 4 )
	{
		return TStringManager::String8Compare( a_szName + iLen - 4, ".ppm" ) == 0;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006c13f0
TBOOL TTextureResourceHAL::Validate()
{
	if ( IsCreated() && IsValid() )
	{
		return TTRUE;
	}

	auto pRenderer = TSTATICCAST( TRenderD3DInterface, GetRenderer() );

	if ( m_bLoadFromMemory )
	{
		// Load from memory
		if ( m_pData && m_uiDataSize != 0 )
		{
			if ( HASANYFLAG( m_eTextureFlags, 0x40 ) )
			{
				if ( !CreateFromFormat() )
				{
					return TFALSE;
				}
			}
			else
			{
				TUINT uiLayout = m_eTextureFlags & 0x38;

				if ( uiLayout == 8 )
				{
					CreateFromMemory5551( m_uiWidth, m_uiHeight, 0, m_pData );
				}
				else if ( uiLayout == 16 )
				{
					CreateFromMemory4444( m_uiWidth, m_uiHeight, 0, m_pData );
				}
				else if ( uiLayout == 32 )
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

	TFIXME( "Doesn't affect anything but here should be some manipulations with s_iHALMemoryUsage" );
	return TResource::Validate();
}

// $Barnyard: FUNCTION 006c1580
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
			TFIXME( "Doesn't affect anything but here should be some manipulations with s_iHALMemoryUsage" );
			m_pD3DTexture->Release();
		}

		TResource::Invalidate();
	}
}

TOSHI_NAMESPACE_END
