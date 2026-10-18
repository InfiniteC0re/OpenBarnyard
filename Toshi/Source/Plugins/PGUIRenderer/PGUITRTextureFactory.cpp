#include "ToshiPCH.h"
#include "PGUITRTextureFactory.h"
#include "PGUITRDisplayContext.h"

#include <File/TFile.h>
#include <Render/TRenderInterface.h>
#include <Render/TTextureFactory.h>

#include <ctype.h>
#include <stdlib.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( PGUITRTextureFactory );

// Reads a number of a PPM header up to the next white space (at most 10 characters)
// NOTE: the name is unknown. With 10 characters and no white space, a_rValue isn't set.
// $JPOG: FUNCTION PGUIRenderer.dll+00006870
static TUINT8* ReadPPMNumber( TUINT8* a_pText, TINT& a_rValue )
{
	TCHAR szNumber[ 28 ];
	TINT  iLength = 0;

	while ( TTRUE )
	{
		TCHAR cChar          = *a_pText++;
		szNumber[ iLength ] = cChar;

		if ( isspace( cChar ) )
			break;

		if ( ++iLength >= 10 )
			return a_pText;
	}

	szNumber[ iLength ] = '\0';
	a_rValue            = atoi( szNumber );
	return a_pText;
}

// Highest power of two not above 2 * a_iValue - 1 (a_iValue itself if it's a power of two)
// NOTE: inlined in the original, it reads the exponent of the float
static TINT GetPowerOfTwo( TINT a_iValue )
{
	TFLOAT fValue = TFLOAT( 2 * a_iValue - 1 );
	return 1 << ( ( *TREINTERPRETCAST( TINT*, &fValue ) >> 23 ) - 127 );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007770
PGUITRTextureFactory::Texture::Texture( const TPString8& a_rcName, PGUITRTextureFactory* a_pFactory, TBOOL a_bValidate )
    : m_strName( a_rcName )
    , m_pFactory( a_pFactory )
    , m_pMaterial( TNULL )
    , m_pTexture( TNULL )
    , m_iWidth( -1 )
    , m_iHeight( -1 )
    , m_bUnk( TFALSE )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	m_uiFlags = 0;
#endif

	if ( a_bValidate )
		Validate();
}

// $JPOG: FUNCTION PGUIRenderer.dll+000077c0
PGUITRTextureFactory::Texture::Texture( const TPString8& a_rcName, PGUITRTextureFactory* a_pFactory, T2GUIMaterial* a_pMaterial, TINT a_iWidth, TINT a_iHeight )
    : m_strName( a_rcName )
    , m_pFactory( a_pFactory )
    , m_pMaterial( a_pMaterial )
    , m_pTexture( TNULL )
    , m_iWidth( a_iWidth )
    , m_iHeight( a_iHeight )
    , m_bUnk( TFALSE )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	m_uiFlags = 0;
#endif

	Validate();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007b50
PGUITRTextureFactory::Texture::~Texture()
{
	Invalidate();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007810
void PGUITRTextureFactory::Texture::Validate()
{
	if ( !m_strName.IsEmpty() )
	{
		// NOTE: the original looks the name up with TMaterialLibraryManager::GetMaterial and keeps the material if it's a
		// TSpriteMaterialHAL. Here the T2GUI renderer finds the texture (AGUI2RendererDX8 asks the material library of the
		// game) and the factory makes a material for it.
		T2GUIRenderer* pRenderer = m_pFactory->GetShader();
		TTexture*      pTexture  = pRenderer ? pRenderer->GetTexture( m_strName.GetString() ) : TNULL;

		if ( pTexture )
		{
			if ( m_pMaterial && m_pMaterial->GetTextureResource() != pTexture )
			{
				pRenderer->DestroyMaterial( m_pMaterial );
				m_pMaterial = TNULL;
			}

			if ( !m_pMaterial )
				m_pMaterial = pRenderer->CreateMaterial( pTexture );

			m_iWidth  = pTexture->GetWidth();
			m_iHeight = pTexture->GetHeight();
		}
		else
		{
			if ( m_pMaterial && pRenderer )
				pRenderer->DestroyMaterial( m_pMaterial );

			m_pMaterial = TNULL;
		}
	}
}

// $JPOG: FUNCTION PGUIRenderer.dll+000078e0
void PGUITRTextureFactory::Texture::Invalidate()
{
	if ( m_strName.IsEmpty() )
	{
		if ( m_pMaterial )
		{
			m_pFactory->GetShader()->DestroyMaterial( m_pMaterial );
			m_pMaterial = TNULL;
		}

		if ( m_pTexture )
		{
			m_pTexture->DestroyResource();
			m_pTexture = TNULL;
		}
	}
	else if ( m_pMaterial )
	{
		// NOTE: the material of a named texture belonged to the material library in the original, here Validate made it
		m_pFactory->GetShader()->DestroyMaterial( m_pMaterial );
		m_pMaterial = TNULL;
	}
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007ad0
void PGUITRTextureFactory::Texture::SetMaterial( T2GUIMaterial* a_pMaterial, TINT a_iWidth, TINT a_iHeight )
{
	Invalidate();
	m_pMaterial = a_pMaterial;
	m_iWidth    = a_iWidth;
	m_iHeight   = a_iHeight;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007b00
void PGUITRTextureFactory::Texture::SetFlag( TextureFlag a_eFlag, TBOOL a_bValue )
{
	if ( a_bValue == TTRUE )
		m_uiFlags |= a_eFlag;
	else
#ifdef BARNYARD_COMMUNITY_PATCH
		m_uiFlags &= ~a_eFlag;
#else
		m_uiFlags &= a_eFlag;
#endif
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007b20
TBOOL PGUITRTextureFactory::Texture::GetFlag( TextureFlag a_eFlag ) const
{
	return ( m_uiFlags & a_eFlag ) != 0;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005c80
PGUITRTextureFactory::TextureSet::TextureSet( PGUITRTextureFactory* a_pFactory )
    : m_pFactory( a_pFactory )
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00005c30
PGUITRTextureFactory::TextureSet::~TextureSet()
{
	// The list deletes the nodes (not the textures)
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007930
void PGUITRTextureFactory::TextureSet::Add( const TCHAR* a_szName )
{
	TINT16   iID      = m_pFactory->ReserveTextureID( TPString8( a_szName ) );
	Texture* pTexture = m_pFactory->m_aTextures[ iID ];

	m_oTextures.InsertTail( new Node( pTexture ) );
}

// $JPOG: FUNCTION PGUIRenderer.dll+000079c0
void PGUITRTextureFactory::TextureSet::Validate()
{
	for ( auto it = m_oTextures.Begin(); it != m_oTextures.End(); it++ )
		it->pTexture->Validate();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007a00
void PGUITRTextureFactory::TextureSet::Invalidate()
{
	for ( auto it = m_oTextures.Begin(); it != m_oTextures.End(); it++ )
		it->pTexture->Invalidate();
}

// $JPOG: FUNCTION PGUIRenderer.dll+000068e0
PGUITRTextureFactory::PGUITRTextureFactory()
    : m_aTextures( 10, 0 )
{
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007da0
// $JPOG: FUNCTION PGUIRenderer.dll+00005d90
PGUITRTextureFactory::~PGUITRTextureFactory()
{
	// NOTE: the textures aren't deleted, the array only frees its buffer
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007c60
void PGUITRTextureFactory::Create( PGUITRDisplayContext* a_pDisplayContext )
{
	m_pDisplayContext = a_pDisplayContext;

	if ( m_aTextures.SizeAllocated() < 200 )
		m_aTextures.Resize( 200 );

	m_aTextures.SetSize( 0 );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006910
void PGUITRTextureFactory::ResolveMaterials()
{
	for ( TINT i = 0; i < m_aTextures.Size(); i++ )
	{
		if ( m_aTextures[ i ] )
			m_aTextures[ i ]->Validate();
	}
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006980
TINT16 PGUITRTextureFactory::GetTextureID( TTexture* a_pTexture )
{
	// NOTE: the original gets a new TSpriteMaterial from the sprite shader and creates it
	T2GUIMaterial* pMaterial = GetShader()->CreateMaterial( a_pTexture );

	TINT iWidth  = a_pTexture->GetWidth();
	TINT iHeight = a_pTexture->GetHeight();

	TINT16 iID         = AllocateTextureID();
	m_aTextures[ iID ] = new Texture( TPString8(), this, pMaterial, iWidth, iHeight );

	return iID;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006a30
void PGUITRTextureFactory::ReplaceTexture( TINT16 a_iID, TTexture* a_pTexture )
{
	Texture* pTexture = GetTexture( a_iID );

	if ( pTexture->m_pTexture )
		pTexture->m_pTexture->DestroyResource();

	// NOTE: the original changes the texture of the material, the T2GUI renderer makes a new material
	GetShader()->DestroyMaterial( pTexture->m_pMaterial );
	pTexture->m_pMaterial = GetShader()->CreateMaterial( a_pTexture );
	pTexture->m_pTexture  = a_pTexture;
	pTexture->m_iWidth   = a_pTexture->GetWidth();
	pTexture->m_iHeight  = a_pTexture->GetHeight();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006a80
void PGUITRTextureFactory::DestroyTexture( TINT16 a_iID )
{
	Texture* pTexture = GetTexture( a_iID );

	if ( pTexture )
		delete pTexture;

	m_aTextures[ a_iID ] = TNULL;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006ac0
TINT16 PGUITRTextureFactory::ReserveTextureID( const TPString8& a_rcName )
{
	TINT16 iID = FindTextureID( a_rcName );

	if ( iID == -1 )
	{
		iID                = AllocateTextureID();
		m_aTextures[ iID ] = new Texture( a_rcName, this, TFALSE );
	}

	return iID;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006b20
TINT16 PGUITRTextureFactory::GetTextureID( const TPString8& a_rcName )
{
	TINT16 iID = FindTextureID( a_rcName );

	if ( iID == -1 )
	{
		iID                = AllocateTextureID();
		m_aTextures[ iID ] = new Texture( a_rcName, this, TFALSE );
	}

	return iID;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006b80
TINT16 PGUITRTextureFactory::FindTextureID( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < m_aTextures.Size(); i++ )
	{
		Texture* pTexture = m_aTextures[ i ];

		if ( pTexture && pTexture->GetName() == a_rcName )
			return TINT16( i );
	}

	return -1;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006c10
TBOOL PGUITRTextureFactory::IsTextureIDValid( TINT16 a_iID ) const
{
	return a_iID >= 0 && a_iID < m_aTextures.Size() && m_aTextures[ a_iID ] != TNULL;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006c40
PGUITRTextureFactory::Texture* PGUITRTextureFactory::GetTexture( const TPString8& a_rcName )
{
	TINT16 iID = GetTextureID( a_rcName );

	if ( iID < 0 )
		return TNULL;

	return m_aTextures[ iID ];
}

// $JPOG: FUNCTION PGUIRenderer.dll+00006c70
TTexture* PGUITRTextureFactory::LoadTTexture( const TPString8& a_rcName, TINT& a_rWidth, TINT& a_rHeight )
{
	TString8 strPath;
	strPath = m_strTextureRoot + "/";
	strPath = strPath + a_rcName.GetString();

	TFile* pFile = TFile::Create( strPath, TFILEMODE_READ );

	if ( pFile )
	{
		TINT    iSize = pFile->GetSize();
		TUINT8* pData = TSTATICCAST( TUINT8, TMalloc( iSize ) );
		pFile->Read( pData, iSize );
		pFile->Destroy();

		TTexture* pTexture = ReadPPM( pData, iSize, a_rWidth, a_rHeight );
		TFree( pData );

		return pTexture;
	}

	return TNULL;
}

// Creates a texture from a PPM image: P5 (grey used as alpha), P6 (RGB) or P7 (RGBA, PAM without its header fields)
// $JPOG: FUNCTION PGUIRenderer.dll+00006d60
TTexture* PGUITRTextureFactory::ReadPPM( TUINT8* a_pData, TINT a_iSize, TINT& a_rWidth, TINT& a_rHeight )
{
	if ( a_iSize < 2 )
		return TNULL;

	TINT  iSrcBytesPerPixel;
	TINT  iDstBytesPerPixel;
	TBOOL bAlpha;
	TBOOL bGrey;

	if ( a_pData[ 0 ] == 'P' && a_pData[ 1 ] == '5' )
	{
		iSrcBytesPerPixel = 1;
		iDstBytesPerPixel = 4;
		bAlpha            = TTRUE;
		bGrey             = TTRUE;
	}
	else if ( a_pData[ 0 ] == 'P' && a_pData[ 1 ] == '6' )
	{
		iSrcBytesPerPixel = 3;
		iDstBytesPerPixel = 3;
		bAlpha            = TFALSE;
		bGrey             = TFALSE;
	}
	else if ( a_pData[ 0 ] == 'P' && a_pData[ 1 ] == '7' )
	{
		iSrcBytesPerPixel = 4;
		iDstBytesPerPixel = 4;
		bAlpha            = TTRUE;
		bGrey             = TFALSE;
	}
	else
	{
		return TNULL;
	}

	// Skip the end of the magic line and the comment lines
	TUINT8* pText = a_pData + 2;
	TUINT8  cChar = a_pData[ 2 ];

	do
	{
		while ( TTRUE )
		{
			pText++;

			if ( cChar == '\n' )
				break;

			cChar = *pText;
		}

		cChar = *pText;
	} while ( *pText == '#' );

	TINT iWidth;
	TINT iHeight;
	TINT iMaxValue;
	pText          = ReadPPMNumber( pText, iWidth );
	pText          = ReadPPMNumber( pText, iHeight );
	TUINT8* pSrc   = ReadPPMNumber( pText, iMaxValue );

	if ( iWidth <= 0 || iHeight <= 0 )
		return TNULL;

	// The sizes must be powers of two
	if ( GetPowerOfTwo( iWidth ) != iWidth || GetPowerOfTwo( iHeight ) != iHeight )
		return TNULL;

	if ( iWidth > 20000 || iHeight > 20000 )
		return TNULL;

	TINT iSrcPitch = iWidth * iSrcBytesPerPixel;

	if ( iSrcBytesPerPixel * iWidth * iHeight > a_iSize - ( pSrc - a_pData ) )
		return TNULL;

	// A one bit alpha fits in 16 bits
	if ( bAlpha == TTRUE && TestRGBA16( pSrc, iWidth, iHeight, iSrcPitch, iSrcBytesPerPixel ) )
		iDstBytesPerPixel = 2;

	if ( iSrcBytesPerPixel == 3 )
		iDstBytesPerPixel = 2;

	TINT iDstWidth  = GetPowerOfTwo( iWidth );
	TINT iDstHeight = GetPowerOfTwo( iHeight );
	TINT iDstPitch  = iDstBytesPerPixel * iDstWidth;
	TINT iDstSize   = iDstBytesPerPixel * iDstWidth * iDstHeight;

	a_rWidth  = iDstWidth;
	a_rHeight = iDstHeight;

	TTextureFactory* pTextureFactory = m_pDisplayContext->GetRenderInterface()->GetSystemResource<TTextureFactory>( SYSRESOURCE_TEXTUREFACTORY );
	TTexture*        pTexture        = TNULL;

	// NOTE: the TRender texture factory took the bits per pixel (8 for the 16 bits images, 32), TOSHI 2.0's takes a
	// format. The TOSHI 2.0 textures also keep the pixels to recreate themselves, so the buffers aren't freed here
	// (the original freed the 16 bits one, and the file data the 32 bits one points to is freed by LoadTTexture).
	if ( iDstBytesPerPixel == 2 )
	{
		TUINT8* pDst = TSTATICCAST( TUINT8, TMalloc( iDstSize ) );
		QuantiseRGBA16( pSrc, iWidth, iHeight, iSrcPitch, iSrcBytesPerPixel, pDst, iDstWidth, iDstHeight, iDstPitch, iDstBytesPerPixel, bGrey, bAlpha );

		pTexture = pTextureFactory->CreateEx( pDst, iDstSize, iDstWidth, iDstHeight, 1, TTEXTURERESOURCEFORMAT::R5G5B5A1, TTexture::MIPFLAGS_USE_MIPLEVELS );
	}
	else if ( iDstBytesPerPixel == 4 )
	{
		// NOTE: a P5 image with a grey alpha (not only 0 and 255) gets here with one byte per pixel and is read as a 32 bits image
		TUINT8* pDst = TSTATICCAST( TUINT8, TMalloc( iDstSize ) );
		TUtil::MemCopy( pDst, pSrc, iDstSize );

		pTexture = pTextureFactory->CreateEx( pDst, iDstSize, iDstWidth, iDstHeight, 1, TTEXTURERESOURCEFORMAT::R8G8B8A8, TTexture::MIPFLAGS_USE_MIPLEVELS );
	}

	// NOTE: the original sets the field 0x30 of the texture to 2, assumed to be the addressing mode (clamp)
	pTexture->SetAddressing( ADDRESSINGMODE_CLAMP );
	return pTexture;
}

// $JPOG: FUNCTION PGUIRenderer.dll+000070e0
TINT16 PGUITRTextureFactory::AllocateTextureID()
{
	// Reuse a free slot
	for ( TINT i = 0; i < m_aTextures.Size(); i++ )
	{
		if ( m_aTextures[ i ] == TNULL )
			return TINT16( i );
	}

	m_aTextures.Push( TNULL );
	return TINT16( m_aTextures.Size() - 1 );
}

// Converts the image to A1B5G5R5 (the alpha bit is set where the alpha isn't 0, or always without alpha)
// $JPOG: FUNCTION PGUIRenderer.dll+00007380
void PGUITRTextureFactory::QuantiseRGBA16( TUINT8* a_pSrc, TINT a_iWidth, TINT a_iHeight, TINT a_iSrcPitch, TINT a_iSrcBytesPerPixel, TUINT8* a_pDst, TINT a_iDstWidth, TINT a_iDstHeight, TINT a_iDstPitch, TINT a_iDstBytesPerPixel, TBOOL a_bGrey, TBOOL a_bAlpha )
{
	for ( TINT iY = 0; iY < a_iHeight; iY++ )
	{
		for ( TINT iX = 0; iX < a_iWidth; iX++ )
		{
			switch ( a_iSrcBytesPerPixel )
			{
				case 1:
					if ( a_bAlpha )
					{
						TUINT16 uiAlpha                 = ( *a_pSrc != 0 ) ? 0x8000 : 0;
						*TREINTERPRETCAST( TUINT16*, a_pDst ) = uiAlpha | ( *a_pSrc >> 3 ) | ( ( *a_pSrc >> 3 ) << 5 ) | ( ( *a_pSrc >> 3 ) << 10 );
					}
					else
					{
						*TREINTERPRETCAST( TUINT16*, a_pDst ) = ( *a_pSrc >> 3 ) | ( ( *a_pSrc >> 3 ) << 5 ) | ( ( *a_pSrc >> 3 ) << 10 ) | 0x8000;
					}

					a_pDst += 2;
					a_pSrc += 1;
					break;
				case 3:
					*TREINTERPRETCAST( TUINT16*, a_pDst ) = ( a_pSrc[ 0 ] >> 3 ) | ( ( a_pSrc[ 1 ] >> 3 ) << 5 ) | ( ( a_pSrc[ 2 ] >> 3 ) << 10 ) | 0x8000;
					a_pSrc += 3;
					a_pDst += 2;
					break;
				case 4:
					if ( a_bAlpha )
						*TREINTERPRETCAST( TUINT16*, a_pDst ) = ( ( a_pSrc[ 3 ] >> 7 ) << 15 ) | ( a_pSrc[ 0 ] >> 3 ) | ( ( a_pSrc[ 1 ] >> 3 ) << 5 ) | ( ( a_pSrc[ 2 ] >> 3 ) << 10 );
					else
						*TREINTERPRETCAST( TUINT16*, a_pDst ) = ( a_pSrc[ 0 ] >> 3 ) | ( ( a_pSrc[ 1 ] >> 3 ) << 5 ) | ( ( a_pSrc[ 2 ] >> 3 ) << 10 ) | 0x8000;

					a_pDst += 2;
					a_pSrc += 4;
					break;
			}
		}
	}
}

// Converts the image to 8 bits indices of a palette it fills, returns the number of colours
// NOTE: the colour keys overlap (the components are shifted by 4 bits only)
// $JPOG: FUNCTION PGUIRenderer.dll+00007540
TUINT8 PGUITRTextureFactory::QuantisePalette256( TUINT* a_pPalette, TUINT8* a_pSrc, TINT a_iWidth, TINT a_iHeight, TINT a_iSrcPitch, TUINT8* a_pDst, TINT a_iDstWidth, TINT a_iDstHeight, TINT a_iDstPitch, TINT a_iDstBytesPerPixel, TBOOL a_bGrey, TBOOL a_bAlpha )
{
	TUINT8 uiNumColours = 0;

	if ( a_bGrey )
	{
		// Transparent and white
		uiNumColours    = 2;
		a_pPalette[ 0 ] = 0;
		a_pPalette[ 1 ] = 0xFFFFFFFF;
	}

	for ( TINT iY = 0; iY < a_iHeight; iY++ )
	{
		for ( TINT iX = 0; iX < a_iSrcPitch; iX += a_iDstBytesPerPixel )
		{
			if ( a_bGrey )
			{
				a_pDst[ iX ] = ( *a_pSrc++ != 0 );
			}
			else if ( a_bAlpha )
			{
				a_pDst[ iX ] = InsertColour( a_pPalette, uiNumColours, a_pSrc[ 3 ] | ( a_pSrc[ 0 ] << 4 ) | ( a_pSrc[ 1 ] << 8 ) | ( a_pSrc[ 2 ] << 12 ) );
				a_pSrc += 4;
			}
			else
			{
				a_pDst[ iX ] = InsertColour( a_pPalette, uiNumColours, ( a_pSrc[ 0 ] << 4 ) | ( a_pSrc[ 1 ] << 8 ) | ( a_pSrc[ 2 ] << 12 ) );
				a_pSrc += 3;
			}
		}

		a_pDst += a_iDstPitch;
	}

	return uiNumColours;
}

// Returns the index of a_uiColour in the palette, adds it if it isn't there
// $JPOG: FUNCTION PGUIRenderer.dll+000076b0
TUINT8 PGUITRTextureFactory::InsertColour( TUINT* a_pPalette, TUINT8& a_rNumColours, TUINT a_uiColour )
{
	for ( TINT i = 0; i < a_rNumColours; i++ )
	{
		if ( a_pPalette[ i ] == a_uiColour )
			return TUINT8( i );
	}

	a_pPalette[ a_rNumColours ] = a_uiColour;
	return a_rNumColours++;
}

// Returns TTRUE if the alpha of each pixel (its last byte) is 0 or 255
// $JPOG: FUNCTION PGUIRenderer.dll+00007700
TBOOL PGUITRTextureFactory::TestRGBA16( TUINT8* a_pSrc, TINT a_iWidth, TINT a_iHeight, TINT a_iSrcPitch, TINT a_iSrcBytesPerPixel )
{
	TUINT8* pAlpha = a_pSrc + a_iSrcBytesPerPixel - 1;

	for ( TINT iY = 0; iY < a_iHeight; iY++ )
	{
		for ( TINT iX = 0; iX < a_iSrcPitch; iX += a_iSrcBytesPerPixel )
		{
			if ( *pAlpha != 0 && *pAlpha != 255 )
				return TFALSE;

			pAlpha += a_iSrcBytesPerPixel;
		}
	}

	return TTRUE;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007b70
T2GUIRenderer* PGUITRTextureFactory::GetShader() const
{
	return m_pDisplayContext->GetShader();
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007b90
TINT16 PGUITRTextureFactory::GetNextTextureID( TINT16 a_iID ) const
{
	if ( a_iID >= m_aTextures.Size() - 1 )
		return -1;

	return a_iID + 1;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007bb0
TINT16 PGUITRTextureFactory::GetFirstTextureID() const
{
	return 0;
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007bc0
void PGUITRTextureFactory::SetTextureFlag( TINT16 a_iID, TextureFlag a_eFlag, TBOOL a_bValue )
{
	m_aTextures[ a_iID ]->SetFlag( a_eFlag, a_bValue );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007be0
TBOOL PGUITRTextureFactory::GetTextureFlag( TINT16 a_iID, TextureFlag a_eFlag ) const
{
	return m_aTextures[ a_iID ]->GetFlag( a_eFlag );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007c00
TINT16 PGUITRTextureFactory::GetTextureHeight( TINT16 a_iID ) const
{
	return TINT16( m_aTextures[ a_iID ]->m_iHeight );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007c20
TINT16 PGUITRTextureFactory::GetTextureWidth( TINT16 a_iID ) const
{
	return TINT16( m_aTextures[ a_iID ]->m_iWidth );
}

// $JPOG: FUNCTION PGUIRenderer.dll+00007c40
const TPString8& PGUITRTextureFactory::GetTextureName( TINT16 a_iID ) const
{
	return m_aTextures[ a_iID ]->GetName();
}
