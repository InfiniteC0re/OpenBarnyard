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

// NOTE: GetClass 006c0b60, CreateObject 006c1320, CreateObjectInPlace 006c1370
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

// $Barnyard: FUNCTION 006c13c0
TTextureResourceHAL::~TTextureResourceHAL()
{
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
	// NOTE: retail has no deprecation warning here. It still creates its invalid texture through this
	// (TRenderD3DInterface::CreateDisplay -> TTextureFactoryHAL::CreateTextureFromMemory)

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
	m_uiMipFlags     = MIPFLAGS_USE_MIPLEVELS;

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

// $Barnyard: FUNCTION 006c1280
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

// Creates a managed texture of a_eFormat, lets a_fnConvert fill the top level with converted
// pixels of a_pData and generates the mip levels if a_uiLevels is 0 (all levels).
// NOTE: the four CreateFromMemory functions below share this code in the original (inlined),
// the pitch of the locked rect is ignored.
template <class TPixel, class TSourcePixel, class Converter>
static TBOOL CreateFromMemoryImpl( TTextureResourceHAL* a_pTexture, IDirect3DTexture8*& a_rpD3DTexture, D3DXIMAGE_INFO& a_rImageInfo, D3DFORMAT a_eFormat, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData, Converter a_fnConvert )
{
	TRenderD3DInterface* pRenderer = TSTATICCAST( TRenderD3DInterface, a_pTexture->GetRenderer() );

	HRESULT hCreateRes = pRenderer->GetDirect3DDevice()->CreateTexture(
	    a_uiWidth,
	    a_uiHeight,
	    a_uiLevels,
	    0,
	    a_eFormat,
	    D3DPOOL_MANAGED,
	    &a_rpD3DTexture
	);

	if ( FAILED( hCreateRes ) )
	{
		TRenderD3DInterface::PrintError( hCreateRes, TNULL );
		return TTRUE;
	}

	D3DLOCKED_RECT rect;
	a_rpD3DTexture->LockRect( 0, &rect, NULL, D3DLOCK_NO_DIRTY_UPDATE | D3DLOCK_NOSYSLOCK );

	TPixel*       pTexPixel  = TSTATICCAST( TPixel, rect.pBits );
	TSourcePixel* pDataPixel = TSTATICCAST( TSourcePixel, a_pData );

	for ( TUINT i = a_uiHeight; i != 0; i-- )
	{
		for ( TUINT j = a_uiWidth; j != 0; j-- )
			a_fnConvert( pTexPixel, pDataPixel );
	}

	a_rpD3DTexture->UnlockRect( 0 );

	if ( a_uiLevels == 0 )
	{
		D3DXFilterTexture( a_rpD3DTexture, NULL, 0, D3DX_DEFAULT );
		TUtil::MemClear( &a_rImageInfo, sizeof( a_rImageInfo ) );
		a_rImageInfo.Width  = a_uiWidth;
		a_rImageInfo.Height = a_uiHeight;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006c0ff0
TBOOL TTextureResourceHAL::CreateFromMemory8888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels
	return CreateFromMemoryImpl<TUINT32, TUINT32>( this, m_pD3DTexture, m_ImageInfo, D3DFMT_A8R8G8B8, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT32*& a_rpOut, TUINT32*& a_rpIn ) {
		TUINT32 uiPixel = *( a_rpIn++ );
		*( a_rpOut++ )  = ( uiPixel & 0xFF00FF00 ) | ( ( uiPixel & 0xFF ) << 16 ) | ( ( uiPixel >> 16 ) & 0xFF );
	} );
}

// $Barnyard: FUNCTION 006c10f0
TBOOL TTextureResourceHAL::CreateFromMemory888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels and adds an opaque alpha
	return CreateFromMemoryImpl<TUINT8, TUINT8>( this, m_pD3DTexture, m_ImageInfo, D3DFMT_X8R8G8B8, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT8*& a_rpOut, TUINT8*& a_rpIn ) {
		a_rpOut[ 0 ] = a_rpIn[ 2 ];
		a_rpOut[ 1 ] = a_rpIn[ 1 ];
		a_rpOut[ 2 ] = a_rpIn[ 0 ];
		a_rpOut[ 3 ] = 0xFF;

		a_rpOut += 4;
		a_rpIn += 3;
	} );
}

// $Barnyard: FUNCTION 006c0df0
TBOOL TTextureResourceHAL::CreateFromMemory5551( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels
	return CreateFromMemoryImpl<TUINT16, TUINT16>( this, m_pD3DTexture, m_ImageInfo, D3DFMT_A1R5G5B5, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT16*& a_rpOut, TUINT16*& a_rpIn ) {
		TUINT16 uiPixel = *( a_rpIn++ );
		*( a_rpOut++ )  = TUINT16( ( ( uiPixel >> 10 ) & 0x1F ) | ( ( ( uiPixel >> 5 ) & 0x1F ) << 5 ) | ( ( uiPixel & 0x1F ) << 10 ) | ( ( uiPixel >> 15 ) << 15 ) );
	} );
}

// $Barnyard: FUNCTION 006c0ef0
TBOOL TTextureResourceHAL::CreateFromMemory4444( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	// Swaps the red and blue channels
	return CreateFromMemoryImpl<TUINT16, TUINT16>( this, m_pD3DTexture, m_ImageInfo, D3DFMT_A4R4G4B4, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT16*& a_rpOut, TUINT16*& a_rpIn ) {
		TUINT16 uiPixel = *( a_rpIn++ );
		*( a_rpOut++ )  = TUINT16( ( ( uiPixel >> 8 ) & 0xF ) | ( ( ( uiPixel >> 4 ) & 0xF ) << 4 ) | ( ( uiPixel & 0xF ) << 8 ) | ( ( uiPixel >> 12 ) << 12 ) );
	} );
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
		return TStringManager::String8CompareNoCase( a_szName + iLen - 4, ".ppm" ) == 0;
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

	TMemory::HALMemInfo memInfoHAL;
	TMemory::GetHALMemInfo( memInfoHAL );

	auto pRenderer = TSTATICCAST( TRenderD3DInterface, GetRenderer() );

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
			TMemory::HALMemInfo memInfoHAL;
			TMemory::GetHALMemInfo( memInfoHAL );

			m_pD3DTexture->Release();
			m_pD3DTexture = TNULL;

			TMemory::GetHALMemInfo( memInfoHAL );
			s_iHALMemoryUsage -= memInfoHAL.m_uiMemUsage;
		}

		TResource::Invalidate();
	}
}

TOSHI_NAMESPACE_END
