#include "ToshiPCH.h"
#include "TTextureResourceHAL_GL.h"

#include "T2Texture_GL.h"
#include "TRenderInterface_GL.h"

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
	m_uiTexture       = 0;
	m_eAddressUState  = ADDRESSINGMODE_WRAP;
	m_eAddressVState  = ADDRESSINGMODE_WRAP;
	m_pLockPixels     = TNULL;
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

// NOTE: only movie frames lock textures. The pixels are BGRA like the D3D texture, and are sent on Unlock
TBOOL TTextureResourceHAL::Lock( LOCKSTATE& a_rLockState )
{
	if ( m_uiTexture && !m_pLockPixels )
	{
		m_pLockPixels = TSTATICCAST( TUINT8, TMalloc( m_ImageInfo.Width * m_ImageInfo.Height * 4 ) );

		a_rLockState.Pitch = m_ImageInfo.Width * 4;
		a_rLockState.pBits = m_pLockPixels;
		m_iNumLocks += 1;

		return TTRUE;
	}

	return TFALSE;
}

void TTextureResourceHAL::Unlock()
{
	TASSERT( m_iNumLocks != 0 );

	if ( m_iNumLocks != 0 && m_uiTexture && m_pLockPixels )
	{
		const TUINT uiNumPixels = m_ImageInfo.Width * m_ImageInfo.Height;

		for ( TUINT i = 0; i < uiNumPixels; i++ )
		{
			const TUINT8 uiBlue         = m_pLockPixels[ i * 4 + 0 ];
			m_pLockPixels[ i * 4 + 0 ] = m_pLockPixels[ i * 4 + 2 ];
			m_pLockPixels[ i * 4 + 2 ] = uiBlue;

			if ( m_ImageInfo.Format == GL_RGB8 )
				m_pLockPixels[ i * 4 + 3 ] = 0xFF;
		}

		TRenderGLInterface::Interface()->UpdateTexture( m_uiTexture, m_ImageInfo.Width, m_ImageInfo.Height, m_pLockPixels );

		TFree( m_pLockPixels );
		m_pLockPixels = TNULL;
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
	m_uiTexture       = a_pTexture->GetGLTexture();

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
	m_uiMipFlags      = MIPFLAGS_FULL_CHAIN;
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
	m_uiMipFlags     = MIPFLAGS_FULL_CHAIN;

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
	TUINT uiMipLevels = ( m_uiMipFlags & MIPFLAGS_FULL_CHAIN ) ? 0 : m_uiMipLevels;

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

template <class TSourcePixel, class Converter>
static TBOOL CreateFromMemoryImpl( GLuint& a_ruiTexture, TGLTextureInfo& a_rImageInfo, GLenum a_eFormat, TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData, Converter a_fnConvert )
{
	const TUINT   uiNumPixels = a_uiWidth * a_uiHeight;
	TUINT8*       pPixels     = new TUINT8[ uiNumPixels * 4 ];
	TSourcePixel* pSource     = TSTATICCAST( TSourcePixel, a_pData );

	for ( TUINT i = 0; i < uiNumPixels; i++ )
		a_fnConvert( pPixels + i * 4, pSource );

	const TBOOL bGenerateMipmaps = a_uiLevels == 0;

	a_ruiTexture = TRenderGLInterface::Interface()->CreateTexture( a_uiWidth, a_uiHeight, pPixels, bGenerateMipmaps ? TRenderGLInterface::TEXTUREFLAGS_GENERATEMIPMAPS : TRenderGLInterface::TEXTUREFLAGS_NONE );
	delete[] pPixels;

	if ( !a_ruiTexture )
	{
		TERROR( "Unable to create a texture!\n" );
		return TTRUE;
	}

	TUINT uiMipLevels = 1;

	if ( bGenerateMipmaps )
		for ( TUINT uiSize = TMath::Max( a_uiWidth, a_uiHeight ) >> 1; uiSize != 0; uiSize >>= 1 )
			uiMipLevels++;

	a_rImageInfo.Width     = a_uiWidth;
	a_rImageInfo.Height    = a_uiHeight;
	a_rImageInfo.MipLevels = uiMipLevels;
	a_rImageInfo.Format    = a_eFormat;

	return TTRUE;
}

TBOOL TTextureResourceHAL::CreateFromMemory8888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	return CreateFromMemoryImpl<TUINT8>( m_uiTexture, m_ImageInfo, GL_RGBA8, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT8* a_pOut, TUINT8*& a_rpIn ) {
		a_pOut[ 0 ] = a_rpIn[ 0 ];
		a_pOut[ 1 ] = a_rpIn[ 1 ];
		a_pOut[ 2 ] = a_rpIn[ 2 ];
		a_pOut[ 3 ] = a_rpIn[ 3 ];
		a_rpIn += 4;
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemory888( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	return CreateFromMemoryImpl<TUINT8>( m_uiTexture, m_ImageInfo, GL_RGB8, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT8* a_pOut, TUINT8*& a_rpIn ) {
		a_pOut[ 0 ] = a_rpIn[ 0 ];
		a_pOut[ 1 ] = a_rpIn[ 1 ];
		a_pOut[ 2 ] = a_rpIn[ 2 ];
		a_pOut[ 3 ] = 0xFF;
		a_rpIn += 3;
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemory5551( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	return CreateFromMemoryImpl<TUINT16>( m_uiTexture, m_ImageInfo, GL_RGB5_A1, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT8* a_pOut, TUINT16*& a_rpIn ) {
		const TUINT16 uiPixel = *( a_rpIn++ );
		a_pOut[ 0 ]           = TUINT8( ( uiPixel & 0x1F ) * 255 / 31 );
		a_pOut[ 1 ]           = TUINT8( ( ( uiPixel >> 5 ) & 0x1F ) * 255 / 31 );
		a_pOut[ 2 ]           = TUINT8( ( ( uiPixel >> 10 ) & 0x1F ) * 255 / 31 );
		a_pOut[ 3 ]           = ( uiPixel >> 15 ) ? 0xFF : 0;
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemory4444( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	return CreateFromMemoryImpl<TUINT16>( m_uiTexture, m_ImageInfo, GL_RGBA4, a_uiWidth, a_uiHeight, a_uiLevels, a_pData, []( TUINT8* a_pOut, TUINT16*& a_rpIn ) {
		const TUINT16 uiPixel = *( a_rpIn++ );
		a_pOut[ 0 ]           = TUINT8( ( uiPixel & 0xF ) * 17 );
		a_pOut[ 1 ]           = TUINT8( ( ( uiPixel >> 4 ) & 0xF ) * 17 );
		a_pOut[ 2 ]           = TUINT8( ( ( uiPixel >> 8 ) & 0xF ) * 17 );
		a_pOut[ 3 ]           = TUINT8( ( uiPixel >> 12 ) * 17 );
	} );
}

TBOOL TTextureResourceHAL::CreateFromMemoryDDS( TUINT a_uiWidth, TUINT a_uiHeight, TUINT a_uiLevels, void* a_pData )
{
	m_uiTexture = TRenderGLInterface::Interface()->CreateTextureFromFileInMemory( a_pData, m_uiDataSize, &m_ImageInfo );
	return m_uiTexture != 0;
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

	m_uiTexture = TRenderGLInterface::Interface()->CreateTextureFromFileInMemory( pData, uiSize, &m_ImageInfo );
	delete[] pData;

	return m_uiTexture != 0;
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

	if ( m_bLoadFromMemory )
	{
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
		if ( m_pNameEntry && !IsPPM( m_pNameEntry->GetName() ) )
		{
			CreateFromFileDDS( m_uiWidth, m_uiHeight, -1, m_pNameEntry->GetName() );
		}
	}

	return TResource::Validate();
}

void TTextureResourceHAL::Invalidate()
{
	if ( IsCreated() && IsValid() )
	{
		if ( m_bIsToshi2 == TTRUE )
		{
			m_uiTexture = 0;
		}
		else if ( m_uiTexture )
		{
			if ( TRenderGLInterface* pRender = TRenderGLInterface::Interface() )
				pRender->DestroyTexture( m_uiTexture );

			m_uiTexture = 0;
		}

		TResource::Invalidate();
	}
}

TOSHI_NAMESPACE_END
