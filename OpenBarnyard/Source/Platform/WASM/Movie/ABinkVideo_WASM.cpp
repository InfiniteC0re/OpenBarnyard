#include "pch.h"
#include "ABinkVideo_WASM.h"
#include "ABinkTables_WASM.h"

#include <bink.h>

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TUINT32 FLAG_ALPHA = 0x00100000;
static constexpr TINT    VERSION_H  = 'h';
static constexpr TINT    VERSION_I  = 'i';
static constexpr TINT    VERSION_K  = 'k';

enum BLOCK_
{
	BLOCK_SKIP,
	BLOCK_SCALED,
	BLOCK_MOTION,
	BLOCK_RUN,
	BLOCK_RESIDUE,
	BLOCK_INTRA,
	BLOCK_FILL,
	BLOCK_INTER,
	BLOCK_PATTERN,
	BLOCK_RAW
};

static const TUINT8 s_aRleLengths[ 4 ] = { 4, 8, 12, 32 };

// For each code table: the next bits -> (length << 4) | leaf
static TUINT8 s_aVlc[ 16 ][ 128 ];
static TUINT8 s_aVlcBits[ 16 ];

// YUV to RGB, for video range (before revision k) and full range, in 16.16
struct ColorTables
{
	TINT aY[ 256 ];
	TINT aRV[ 256 ];
	TINT aGU[ 256 ];
	TINT aGV[ 256 ];
	TINT aBU[ 256 ];
};

static ColorTables s_aColors[ 2 ];
static TUINT8      s_aClamp[ 1536 ];
static TBOOL       s_bTablesReady;

static void BuildTables()
{
	for ( TINT t = 0; t < 16; t++ )
	{
		const TINT iMax = s_aBinkTreeLens[ t * 16 + 15 ];
		s_aVlcBits[ t ] = TUINT8( iMax );

		for ( TINT i = 0; i < 16; i++ )
		{
			const TINT iLength = s_aBinkTreeLens[ t * 16 + i ];
			const TINT iCode   = s_aBinkTreeBits[ t * 16 + i ];

			for ( TINT f = 0; f < 1 << ( iMax - iLength ); f++ )
				s_aVlc[ t ][ iCode | ( f << iLength ) ] = TUINT8( ( iLength << 4 ) | i );
		}
	}

	for ( TINT t = 0; t < 2; t++ )
	{
		const TBOOL   bFull  = t == 1;
		const TDOUBLE dY     = bFull ? 1.0 : 255.0 / 219.0;
		const TDOUBLE dC     = bFull ? 1.0 : 255.0 / 224.0;
		const TDOUBLE dScale = 65536.0;

		for ( TINT i = 0; i < 256; i++ )
		{
			const TDOUBLE dChroma = dC * ( i - 128 ) * dScale;

			s_aColors[ t ].aY[ i ]  = TINT( floor( ( bFull ? i : i - 16 ) * dY * dScale + 0.5 ) ) + 32768;
			s_aColors[ t ].aRV[ i ] = TINT( floor( 1.402 * dChroma + 0.5 ) );
			s_aColors[ t ].aGU[ i ] = TINT( floor( -0.344136 * dChroma + 0.5 ) );
			s_aColors[ t ].aGV[ i ] = TINT( floor( -0.714136 * dChroma + 0.5 ) );
			s_aColors[ t ].aBU[ i ] = TINT( floor( 1.772 * dChroma + 0.5 ) );
		}
	}

	for ( TINT i = 0; i < 1536; i++ )
		s_aClamp[ i ] = TUINT8( i < 512 ? 0 : ( i > 767 ? 255 : i - 512 ) );

	s_bTablesReady = TTRUE;
}

static inline TINT ILog2( TUINT32 a_uiValue )
{
	return 31 - __builtin_clz( a_uiValue );
}

// Number of bits of a bundle's count, the bundles only ever hold one row's worth of what such a count can say
static TINT GetCountLength( TINT a_iBase )
{
	return ILog2( a_iBase + 511 ) + 1;
}

TUINT32 ABinkVideo::GetMemorySize( TINT a_iWidth, TINT a_iHeight, TUINT32 a_uiFlags )
{
	const TINT iBlocksWide   = ( a_iWidth + 7 ) >> 3;
	const TINT iBlocksHigh   = ( a_iHeight + 7 ) >> 3;
	const TINT iChromaWide   = ( a_iWidth + 15 ) >> 4;
	const TINT iChromaHigh   = ( a_iHeight + 15 ) >> 4;
	const TINT iWidthAligned = TMath::Max( iBlocksWide * 8, 8 );

	// 16x16 blocks may stick out at the right and bottom when the block count is odd
	TUINT32 uiLuma   = ( iBlocksWide * 8 ) * ( iBlocksHigh * 8 + 8 ) + 16;
	TUINT32 uiChroma = ( iChromaWide * 8 ) * ( iChromaHigh * 8 + 8 ) + 16;
	TUINT32 uiSize   = ( uiLuma + uiChroma * 2 ) * 2;

	if ( a_uiFlags & FLAG_ALPHA )
		uiSize += uiLuma * 2;

	uiSize += 2 << GetCountLength( iWidthAligned >> 3 );                            // block types
	uiSize += 2 << GetCountLength( iWidthAligned >> 4 );                            // sub block types
	uiSize += 2 << GetCountLength( iBlocksWide * 64 );                              // colors
	uiSize += 2 << GetCountLength( iBlocksWide * 8 );                               // patterns
	uiSize += 2 * ( 2 << GetCountLength( iWidthAligned >> 3 ) );                    // motion
	uiSize += 2 * sizeof( TINT16 ) * ( 2 << GetCountLength( iWidthAligned >> 3 ) ); // DC
	uiSize += 2 << GetCountLength( iBlocksWide * 48 );                              // runs

	return uiSize + 64;
}

void ABinkVideo::Create( void* a_pMemory, TINT a_iWidth, TINT a_iHeight, TCHAR a_cRevision, TUINT32 a_uiFlags )
{
	if ( !s_bTablesReady )
		BuildTables();

	m_iWidth      = a_iWidth;
	m_iHeight     = a_iHeight;
	m_iVersion    = a_cRevision;
	m_bHasAlpha   = ( a_uiFlags & FLAG_ALPHA ) != 0;
	m_bSwapChroma = m_iVersion >= VERSION_H;
	m_iColorLast  = 0;
	m_iCoefCount  = 0;

	const TINT iBlocksWide   = ( a_iWidth + 7 ) >> 3;
	const TINT iBlocksHigh   = ( a_iHeight + 7 ) >> 3;
	const TINT iChromaWide   = ( a_iWidth + 15 ) >> 4;
	const TINT iChromaHigh   = ( a_iHeight + 15 ) >> 4;
	const TINT iWidthAligned = TMath::Max( iBlocksWide * 8, 8 );

	m_aiStrides[ PLANE_Y ]     = iBlocksWide * 8;
	m_aiStrides[ PLANE_U ]     = iChromaWide * 8;
	m_aiStrides[ PLANE_V ]     = iChromaWide * 8;
	m_aiStrides[ PLANE_ALPHA ] = iBlocksWide * 8;
	m_aiRows[ PLANE_Y ]        = iBlocksHigh * 8;
	m_aiRows[ PLANE_U ]        = iChromaHigh * 8;
	m_aiRows[ PLANE_V ]        = iChromaHigh * 8;
	m_aiRows[ PLANE_ALPHA ]    = iBlocksHigh * 8;

	TUINT8* pMemory = TSTATICCAST( TUINT8, a_pMemory );

	for ( TINT i = 0; i < PLANE_NUMOF; i++ )
	{
		m_apPlanes[ i ]   = TNULL;
		m_apPrevious[ i ] = TNULL;

		if ( i == PLANE_ALPHA && !m_bHasAlpha )
			continue;

		const TUINT32 uiSize = m_aiStrides[ i ] * ( m_aiRows[ i ] + 8 ) + 16;

		// Grayscale videos have no chroma, and there's nothing to see before the first frame
		const TINT iFill = i == PLANE_Y ? 0 : ( i == PLANE_ALPHA ? 255 : 128 );

		m_apPlanes[ i ] = pMemory;
		TUtil::MemSet( pMemory, iFill, uiSize );
		pMemory += uiSize;

		m_apPrevious[ i ] = pMemory;
		TUtil::MemSet( pMemory, iFill, uiSize );
		pMemory += uiSize;
	}

	const TINT aiLengths[ BUNDLE_NUMOF ] = {
		GetCountLength( iWidthAligned >> 3 ),
		GetCountLength( iWidthAligned >> 4 ),
		GetCountLength( iBlocksWide * 64 ),
		GetCountLength( iBlocksWide * 8 ),
		GetCountLength( iWidthAligned >> 3 ),
		GetCountLength( iWidthAligned >> 3 ),
		GetCountLength( iWidthAligned >> 3 ),
		GetCountLength( iWidthAligned >> 3 ),
		GetCountLength( iBlocksWide * 48 )
	};

	for ( TINT i = 0; i < BUNDLE_NUMOF; i++ )
	{
		const TBOOL bWide = i == BUNDLE_INTRA_DC || i == BUNDLE_INTER_DC;

		if ( bWide )
			pMemory = TREINTERPRETCAST( TUINT8*, ( TUINTPTR( pMemory ) + 1 ) & ~TUINTPTR( 1 ) );

		TUtil::MemClear( &m_aBundles[ i ], sizeof( Bundle ) );
		m_aBundles[ i ].pData = pMemory;

		const TUINT32 uiSize = ( 2u << aiLengths[ i ] ) * ( bWide ? sizeof( TINT16 ) : 1 );

		TUtil::MemClear( pMemory, uiSize );
		pMemory += uiSize;
	}
}

TBOOL ABinkVideo::Decode( const TUINT8* a_pData, TUINT32 a_uiSize )
{
	for ( TINT i = 0; i < PLANE_NUMOF; i++ )
	{
		TUINT8* pPlane    = m_apPrevious[ i ];
		m_apPrevious[ i ] = m_apPlanes[ i ];
		m_apPlanes[ i ]   = pPlane;
	}

	m_oBits.Init( a_pData, a_uiSize );

	if ( m_bHasAlpha )
	{
		if ( m_iVersion >= VERSION_I )
			m_oBits.Skip( 32 );

		if ( !DecodePlane( PLANE_ALPHA, TFALSE ) )
			return TFALSE;
	}

	if ( m_iVersion >= VERSION_I )
		m_oBits.Skip( 32 );

	for ( TINT iPlane = 0; iPlane < 3; iPlane++ )
	{
		const TINT iIndex = ( !iPlane || !m_bSwapChroma ) ? iPlane : iPlane ^ 3;

		if ( !DecodePlane( iIndex, iPlane != 0 ) )
			return TFALSE;

		if ( m_oBits.IsAtEnd() )
			break;
	}

	return TTRUE;
}

//-----------------------------------------------------------------------------
// Bundles
//-----------------------------------------------------------------------------

void ABinkVideo::InitLengths( TINT a_iWidth, TINT a_iBlocksWide )
{
	const TINT iWidth = ( a_iWidth + 7 ) & ~7;

	m_aBundles[ BUNDLE_BLOCK_TYPES ].iLength     = GetCountLength( iWidth >> 3 );
	m_aBundles[ BUNDLE_SUB_BLOCK_TYPES ].iLength = GetCountLength( iWidth >> 4 );
	m_aBundles[ BUNDLE_COLORS ].iLength          = GetCountLength( a_iBlocksWide * 64 );
	m_aBundles[ BUNDLE_INTRA_DC ].iLength        = GetCountLength( iWidth >> 3 );
	m_aBundles[ BUNDLE_INTER_DC ].iLength        = GetCountLength( iWidth >> 3 );
	m_aBundles[ BUNDLE_X_OFF ].iLength           = GetCountLength( iWidth >> 3 );
	m_aBundles[ BUNDLE_Y_OFF ].iLength           = GetCountLength( iWidth >> 3 );
	m_aBundles[ BUNDLE_PATTERN ].iLength         = GetCountLength( a_iBlocksWide * 8 );
	m_aBundles[ BUNDLE_RUN ].iLength             = GetCountLength( a_iBlocksWide * 48 );
}

// Two sorted halves of the source, merged as the bits say
static void MergeSymbols( ABinkBits& a_rBits, TUINT8* a_pDst, const TUINT8* a_pSrc, TINT a_iSize )
{
	const TUINT8* pSecond = a_pSrc + a_iSize;
	TINT          iSize   = a_iSize;
	TINT          iSize2  = a_iSize;

	do
	{
		if ( !a_rBits.ReadBit() )
		{
			*a_pDst++ = *a_pSrc++;
			iSize--;
		}
		else
		{
			*a_pDst++ = *pSecond++;
			iSize2--;
		}
	} while ( iSize && iSize2 );

	while ( iSize-- ) *a_pDst++ = *a_pSrc++;
	while ( iSize2-- ) *a_pDst++ = *pSecond++;
}

TBOOL ABinkVideo::ReadTree( Tree& a_rTree )
{
	if ( m_oBits.GetLeft() < 4 )
		return TFALSE;

	a_rTree.iTable = m_oBits.Read( 4 );

	TUINT8* pSymbols = a_rTree.aSymbols;

	if ( !a_rTree.iTable )
	{
		for ( TINT i = 0; i < 16; i++ )
			pSymbols[ i ] = TUINT8( i );

		return TTRUE;
	}

	if ( m_oBits.ReadBit() )
	{
		TINT   iLength = m_oBits.Read( 3 );
		TUINT8 aUsed[ 16 ];
		TUtil::MemClear( aUsed, sizeof( aUsed ) );

		for ( TINT i = 0; i <= iLength; i++ )
		{
			pSymbols[ i ]          = TUINT8( m_oBits.Read( 4 ) );
			aUsed[ pSymbols[ i ] ] = 1;
		}

		for ( TINT i = 0; i < 16 && iLength < 15; i++ )
		{
			if ( !aUsed[ i ] )
				pSymbols[ ++iLength ] = TUINT8( i );
		}
	}
	else
	{
		const TINT iLength = m_oBits.Read( 2 );

		TUINT8  aFirst[ 16 ];
		TUINT8  aSecond[ 16 ];
		TUINT8* pSrc = aFirst;
		TUINT8* pDst = aSecond;

		for ( TINT i = 0; i < 16; i++ )
			aFirst[ i ] = TUINT8( i );

		for ( TINT i = 0; i <= iLength; i++ )
		{
			const TINT iSize = 1 << i;

			for ( TINT t = 0; t < 16; t += iSize << 1 )
				MergeSymbols( m_oBits, pDst + t, pSrc + t, iSize );

			TUINT8* pSwap = pSrc;
			pSrc          = pDst;
			pDst          = pSwap;
		}

		TUtil::MemCopy( pSymbols, pSrc, 16 );
	}

	return TTRUE;
}

TBOOL ABinkVideo::ReadBundleHeader( TINT a_iBundle )
{
	Bundle& rBundle = m_aBundles[ a_iBundle ];

	if ( a_iBundle == BUNDLE_COLORS )
	{
		for ( TINT i = 0; i < 16; i++ )
		{
			if ( !ReadTree( m_aColorHigh[ i ] ) )
				return TFALSE;
		}

		m_iColorLast = 0;
	}

	if ( a_iBundle != BUNDLE_INTRA_DC && a_iBundle != BUNDLE_INTER_DC )
	{
		if ( !ReadTree( rBundle.oTree ) )
			return TFALSE;
	}

	rBundle.iCurDec = 0;
	rBundle.iCurPtr = 0;
	return TTRUE;
}

inline TINT ABinkVideo::ReadSymbol( const Tree& a_rTree )
{
	const TINT   iTable = a_rTree.iTable;
	const TUINT8 uiCode = s_aVlc[ iTable ][ m_oBits.Peek( s_aVlcBits[ iTable ] ) ];

	m_oBits.Skip( uiCode >> 4 );
	return a_rTree.aSymbols[ uiCode & 15 ];
}

// Number of values to read for this row. A bundle gets new values only when all the old ones are used, so it can
// start over from the beginning, that's what keeps them this small.
inline TINT ABinkVideo::ReadCount( Bundle& a_rBundle )
{
	if ( a_rBundle.iCurDec < 0 || a_rBundle.iCurDec > a_rBundle.iCurPtr )
		return 0;

	const TINT iCount = m_oBits.Read( a_rBundle.iLength );

	if ( !iCount )
	{
		a_rBundle.iCurDec = -1;
		return 0;
	}

	a_rBundle.iCurDec = 0;
	a_rBundle.iCurPtr = 0;
	return iCount;
}

void ABinkVideo::ReadRuns( Bundle& a_rBundle )
{
	const TINT iCount = ReadCount( a_rBundle );

	if ( !iCount )
		return;

	TUINT8* pData = a_rBundle.pData;

	if ( m_oBits.ReadBit() )
	{
		TUtil::MemSet( pData, m_oBits.Read( 4 ), iCount );
	}
	else
	{
		for ( TINT i = 0; i < iCount; i++ )
			pData[ i ] = TUINT8( ReadSymbol( a_rBundle.oTree ) );
	}

	a_rBundle.iCurDec = iCount;
}

void ABinkVideo::ReadMotionValues( Bundle& a_rBundle )
{
	const TINT iCount = ReadCount( a_rBundle );

	if ( !iCount )
		return;

	TINT8* pData = TREINTERPRETCAST( TINT8*, a_rBundle.pData );

	if ( m_oBits.ReadBit() )
	{
		TINT iValue = m_oBits.Read( 4 );

		if ( iValue && m_oBits.ReadBit() )
			iValue = -iValue;

		TUtil::MemSet( pData, iValue, iCount );
	}
	else
	{
		for ( TINT i = 0; i < iCount; i++ )
		{
			TINT iValue = ReadSymbol( a_rBundle.oTree );

			if ( iValue && m_oBits.ReadBit() )
				iValue = -iValue;

			pData[ i ] = TINT8( iValue );
		}
	}

	a_rBundle.iCurDec = iCount;
}

TBOOL ABinkVideo::ReadBlockTypes( Bundle& a_rBundle )
{
	TINT iCount = ReadCount( a_rBundle );

	if ( !iCount )
		return TTRUE;

	if ( m_iVersion == VERSION_K )
	{
		iCount ^= 0xBB;

		if ( !iCount )
		{
			a_rBundle.iCurDec = -1;
			return TTRUE;
		}
	}

	TUINT8* pData = a_rBundle.pData;

	if ( m_oBits.ReadBit() )
	{
		TUtil::MemSet( pData, m_oBits.Read( 4 ), iCount );
		a_rBundle.iCurDec = iCount;
		return TTRUE;
	}

	TINT iLast = 0;
	TINT iAt   = 0;

	while ( iAt < iCount )
	{
		const TINT iValue = ReadSymbol( a_rBundle.oTree );

		if ( iValue < 12 )
		{
			iLast          = iValue;
			pData[ iAt++ ] = TUINT8( iValue );
		}
		else
		{
			const TINT iRun = s_aRleLengths[ iValue - 12 ];

			if ( iCount - iAt < iRun )
				return TFALSE;

			TUtil::MemSet( pData + iAt, iLast, iRun );
			iAt += iRun;
		}
	}

	a_rBundle.iCurDec = iCount;
	return TTRUE;
}

TBOOL ABinkVideo::ReadPatterns( Bundle& a_rBundle )
{
	const TINT iCount = ReadCount( a_rBundle );

	if ( !iCount )
		return TTRUE;

	TUINT8* pData = a_rBundle.pData;

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( m_oBits.GetLeft() < 2 )
			return TFALSE;

		const TINT iLow = ReadSymbol( a_rBundle.oTree );
		pData[ i ]      = TUINT8( iLow | ( ReadSymbol( a_rBundle.oTree ) << 4 ) );
	}

	a_rBundle.iCurDec = iCount;
	return TTRUE;
}

TBOOL ABinkVideo::ReadColors( Bundle& a_rBundle )
{
	const TINT iCount = ReadCount( a_rBundle );

	if ( !iCount )
		return TTRUE;

	TUINT8*     pData = a_rBundle.pData;
	const TBOOL bOld  = m_iVersion < VERSION_I;
	const TBOOL bOne  = m_oBits.ReadBit() != 0;

	for ( TINT i = 0; i < iCount; i++ )
	{
		if ( m_oBits.GetLeft() < 2 )
			return TFALSE;

		m_iColorLast = ReadSymbol( m_aColorHigh[ m_iColorLast ] );

		TINT iValue = ( m_iColorLast << 4 ) | ReadSymbol( a_rBundle.oTree );

		// Before revision i: sign and magnitude around 128
		if ( bOld )
			iValue = ( iValue & 0x80 ) ? 0x80 - ( iValue & 0x7F ) : 0x80 + iValue;

		if ( bOne )
		{
			TUtil::MemSet( pData, iValue, iCount );
			break;
		}

		pData[ i ] = TUINT8( iValue );
	}

	a_rBundle.iCurDec = iCount;
	return TTRUE;
}

TBOOL ABinkVideo::ReadDCs( Bundle& a_rBundle, TINT a_iStartBits, TINT a_iHasSign )
{
	TINT iLength = ReadCount( a_rBundle );

	if ( !iLength )
		return TTRUE;

	if ( m_oBits.GetLeft() < a_iStartBits - a_iHasSign )
		return TFALSE;

	TINT16* pData  = TREINTERPRETCAST( TINT16*, a_rBundle.pData );
	TINT    iAt    = 0;
	TINT    iValue = m_oBits.Read( a_iStartBits - a_iHasSign );

	if ( iValue && a_iHasSign && m_oBits.ReadBit() )
		iValue = -iValue;

	pData[ iAt++ ] = TINT16( iValue );
	iLength--;

	for ( TINT i = 0; i < iLength; i += 8 )
	{
		const TINT iGroup = TMath::Min( iLength - i, 8 );
		const TINT iBits  = m_oBits.Read( 4 );

		if ( iBits )
		{
			for ( TINT j = 0; j < iGroup; j++ )
			{
				TINT iDelta = m_oBits.Read( iBits );

				if ( iDelta && m_oBits.ReadBit() )
					iDelta = -iDelta;

				iValue += iDelta;

				if ( iValue < -32768 || iValue > 32767 )
					return TFALSE;

				pData[ iAt++ ] = TINT16( iValue );
			}
		}
		else
		{
			for ( TINT j = 0; j < iGroup; j++ )
				pData[ iAt++ ] = TINT16( iValue );
		}
	}

	a_rBundle.iCurDec = iAt;
	return TTRUE;
}

//-----------------------------------------------------------------------------
// DCT and residue coefficients
//-----------------------------------------------------------------------------

static inline TINT ReadCoefValue( ABinkBits& a_rBits, TINT a_iBits )
{
	if ( !a_iBits )
		return 1 - TINT( a_rBits.ReadBit() << 1 );

	const TINT iValue = TINT( a_rBits.Read( a_iBits ) | ( 1u << a_iBits ) );
	return a_rBits.ReadBit() ? -iValue : iValue;
}

// Returns the quantiser index, -1 if out of data
TINT ABinkVideo::ReadDctCoeffs( TINT* a_pBlock )
{
	TUINT8 aCoefs[ 128 ];
	TUINT8 aModes[ 128 ];

	ABinkBits& rBits = m_oBits;

	if ( rBits.GetLeft() < 4 )
		return -1;

	TINT iStart = 64;
	TINT iEnd   = 64;
	TINT iCount = 0;

	aCoefs[ iEnd ] = 4, aModes[ iEnd++ ] = 0;
	aCoefs[ iEnd ] = 24, aModes[ iEnd++ ] = 0;
	aCoefs[ iEnd ] = 44, aModes[ iEnd++ ] = 0;
	aCoefs[ iEnd ] = 1, aModes[ iEnd++ ] = 3;
	aCoefs[ iEnd ] = 2, aModes[ iEnd++ ] = 3;
	aCoefs[ iEnd ] = 3, aModes[ iEnd++ ] = 3;

	for ( TINT iBits = TINT( rBits.Read( 4 ) ) - 1; iBits >= 0; iBits-- )
	{
		TINT iPos = iStart;

		while ( iPos < iEnd )
		{
			if ( !( aModes[ iPos ] | aCoefs[ iPos ] ) || !rBits.ReadBit() )
			{
				iPos++;
				continue;
			}

			TINT       iCoef = aCoefs[ iPos ];
			const TINT iMode = aModes[ iPos ];

			if ( iMode == 0 || iMode == 2 )
			{
				if ( iMode == 0 )
				{
					aCoefs[ iPos ] = TUINT8( iCoef + 4 );
					aModes[ iPos ] = 1;
				}
				else
				{
					aCoefs[ iPos ]   = 0;
					aModes[ iPos++ ] = 0;
				}

				for ( TINT i = 0; i < 4; i++, iCoef++ )
				{
					if ( rBits.ReadBit() )
					{
						aCoefs[ --iStart ] = TUINT8( iCoef );
						aModes[ iStart ]   = 3;
					}
					else
					{
						a_pBlock[ s_aBinkScan[ iCoef ] ] = ReadCoefValue( rBits, iBits );
						m_aiCoefIndices[ iCount++ ]      = iCoef;
					}
				}
			}
			else if ( iMode == 1 )
			{
				aModes[ iPos ] = 2;

				for ( TINT i = 0; i < 3; i++ )
				{
					iCoef += 4;
					aCoefs[ iEnd ]   = TUINT8( iCoef );
					aModes[ iEnd++ ] = 2;
				}
			}
			else
			{
				a_pBlock[ s_aBinkScan[ iCoef ] ] = ReadCoefValue( rBits, iBits );
				m_aiCoefIndices[ iCount++ ]      = iCoef;
				aCoefs[ iPos ]                   = 0;
				aModes[ iPos++ ]                 = 0;
			}
		}
	}

	m_iCoefCount = iCount;
	return rBits.Read( 4 );
}

inline void ABinkVideo::Unquantize( TINT* a_pBlock, const TINT* a_pQuant, TINT a_iQuant )
{
	const TINT* pQuant = a_pQuant + a_iQuant * 64;

	a_pBlock[ 0 ] = ( a_pBlock[ 0 ] * pQuant[ 0 ] ) >> 11;

	for ( TINT i = 0; i < m_iCoefCount; i++ )
	{
		const TINT iCoef = m_aiCoefIndices[ i ];
		const TINT iPos  = s_aBinkScan[ iCoef ];

		a_pBlock[ iPos ] = ( a_pBlock[ iPos ] * pQuant[ iCoef ] ) >> 11;
	}
}

// The difference after motion compensation, of a_iMasksCount coded values
void ABinkVideo::ReadResidue( TINT* a_pBlock, TINT a_iMasksCount )
{
	TUINT8 aCoefs[ 128 ];
	TUINT8 aModes[ 128 ];
	TUINT8 aNonZero[ 64 ];

	ABinkBits& rBits = m_oBits;

	TINT iStart   = 64;
	TINT iEnd     = 64;
	TINT iNonZero = 0;

	aCoefs[ iEnd ] = 4, aModes[ iEnd++ ] = 0;
	aCoefs[ iEnd ] = 24, aModes[ iEnd++ ] = 0;
	aCoefs[ iEnd ] = 44, aModes[ iEnd++ ] = 0;
	aCoefs[ iEnd ] = 0, aModes[ iEnd++ ] = 2;

	for ( TINT iMask = 1 << rBits.Read( 3 ); iMask; iMask >>= 1 )
	{
		for ( TINT i = 0; i < iNonZero; i++ )
		{
			if ( !rBits.ReadBit() )
				continue;

			TINT& rValue = a_pBlock[ aNonZero[ i ] ];
			rValue += rValue < 0 ? -iMask : iMask;

			if ( --a_iMasksCount < 0 )
				return;
		}

		TINT iPos = iStart;

		while ( iPos < iEnd )
		{
			if ( !( aCoefs[ iPos ] | aModes[ iPos ] ) || !rBits.ReadBit() )
			{
				iPos++;
				continue;
			}

			TINT       iCoef = aCoefs[ iPos ];
			const TINT iMode = aModes[ iPos ];

			if ( iMode == 0 || iMode == 2 )
			{
				if ( iMode == 0 )
				{
					aCoefs[ iPos ] = TUINT8( iCoef + 4 );
					aModes[ iPos ] = 1;
				}
				else
				{
					aCoefs[ iPos ]   = 0;
					aModes[ iPos++ ] = 0;
				}

				for ( TINT i = 0; i < 4; i++, iCoef++ )
				{
					if ( rBits.ReadBit() )
					{
						aCoefs[ --iStart ] = TUINT8( iCoef );
						aModes[ iStart ]   = 3;
					}
					else
					{
						const TINT iAt = s_aBinkScan[ iCoef ];

						aNonZero[ iNonZero++ ] = TUINT8( iAt );
						a_pBlock[ iAt ]        = rBits.ReadBit() ? -iMask : iMask;

						if ( --a_iMasksCount < 0 )
							return;
					}
				}
			}
			else if ( iMode == 1 )
			{
				aModes[ iPos ] = 2;

				for ( TINT i = 0; i < 3; i++ )
				{
					iCoef += 4;
					aCoefs[ iEnd ]   = TUINT8( iCoef );
					aModes[ iEnd++ ] = 2;
				}
			}
			else
			{
				const TINT iAt = s_aBinkScan[ iCoef ];

				aNonZero[ iNonZero++ ] = TUINT8( iAt );
				a_pBlock[ iAt ]        = rBits.ReadBit() ? -iMask : iMask;
				aCoefs[ iPos ]         = 0;
				aModes[ iPos++ ]       = 0;

				if ( --a_iMasksCount < 0 )
					return;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Block operations
//-----------------------------------------------------------------------------

static inline void Copy8( TUINT8* a_pDst, const TUINT8* a_pSrc, TINT a_iStride )
{
	for ( TINT y = 0; y < 8; y++, a_pDst += a_iStride, a_pSrc += a_iStride )
	{
		TUINT64 uiRow;
		memcpy( &uiRow, a_pSrc, 8 );
		memcpy( a_pDst, &uiRow, 8 );
	}
}

// Wraps around like 8-bit arithmetic
static inline void Add8( TUINT8* a_pDst, TINT a_iStride, const TINT* a_pBlock )
{
	for ( TINT y = 0; y < 8; y++, a_pDst += a_iStride, a_pBlock += 8 )
	{
		for ( TINT x = 0; x < 8; x++ )
			a_pDst[ x ] = TUINT8( a_pDst[ x ] + a_pBlock[ x ] );
	}
}

static inline void Fill8( TUINT8* a_pDst, TINT a_iStride, TINT a_iValue )
{
	const TUINT64 uiRow = 0x0101010101010101ull * TUINT8( a_iValue );

	for ( TINT y = 0; y < 8; y++, a_pDst += a_iStride )
		memcpy( a_pDst, &uiRow, 8 );
}

// 8x8 -> 16x16, each pixel doubled
static void ScaleBlock( const TUINT8* a_pSrc, TUINT8* a_pDst, TINT a_iStride )
{
	for ( TINT y = 0; y < 8; y++, a_pDst += 2 * a_iStride, a_pSrc += 8 )
	{
		for ( TINT x = 0; x < 8; x++ )
		{
			const TUINT8 uiValue = a_pSrc[ x ];
			TUINT8*      pOut    = a_pDst + 2 * x;

			pOut[ 0 ]             = uiValue;
			pOut[ 1 ]             = uiValue;
			pOut[ a_iStride ]     = uiValue;
			pOut[ a_iStride + 1 ] = uiValue;
		}
	}
}

#define BINK_MUL( X, Y ) ( ( ( X ) * ( Y ) ) >> 11 )

#define BINK_IDCT_TERMS( S0, S1, S2, S3, S4, S5, S6, S7 ) \
	const TINT a0 = ( S0 ) + ( S4 );                      \
	const TINT a1 = ( S0 ) - ( S4 );                      \
	const TINT a2 = ( S2 ) + ( S6 );                      \
	const TINT a3 = BINK_MUL( 2896, ( S2 ) - ( S6 ) );    \
	const TINT a4 = ( S5 ) + ( S3 );                      \
	const TINT a5 = ( S5 ) - ( S3 );                      \
	const TINT a6 = ( S1 ) + ( S7 );                      \
	const TINT a7 = ( S1 ) - ( S7 );                      \
	const TINT b0 = a4 + a6;                              \
	const TINT b1 = BINK_MUL( 3784, a5 + a7 );            \
	const TINT b2 = BINK_MUL( -5352, a5 ) - b0 + b1;      \
	const TINT b3 = BINK_MUL( 2896, a6 - a4 ) - b2;       \
	const TINT b4 = BINK_MUL( 2217, a7 ) + b3 - b1;

static inline void IdctColumns( const TINT* a_pBlock, TINT* a_pTemp )
{
	for ( TINT i = 0; i < 8; i++ )
	{
		const TINT s0 = a_pBlock[ i ];
		const TINT s1 = a_pBlock[ i + 8 ];
		const TINT s2 = a_pBlock[ i + 16 ];
		const TINT s3 = a_pBlock[ i + 24 ];
		const TINT s4 = a_pBlock[ i + 32 ];
		const TINT s5 = a_pBlock[ i + 40 ];
		const TINT s6 = a_pBlock[ i + 48 ];
		const TINT s7 = a_pBlock[ i + 56 ];

		if ( ( s1 | s2 | s3 | s4 | s5 | s6 | s7 ) == 0 )
		{
			a_pTemp[ i ]      = s0;
			a_pTemp[ i + 8 ]  = s0;
			a_pTemp[ i + 16 ] = s0;
			a_pTemp[ i + 24 ] = s0;
			a_pTemp[ i + 32 ] = s0;
			a_pTemp[ i + 40 ] = s0;
			a_pTemp[ i + 48 ] = s0;
			a_pTemp[ i + 56 ] = s0;
			continue;
		}

		BINK_IDCT_TERMS( s0, s1, s2, s3, s4, s5, s6, s7 );

		a_pTemp[ i ]      = a0 + a2 + b0;
		a_pTemp[ i + 8 ]  = a1 + a3 - a2 + b2;
		a_pTemp[ i + 16 ] = a1 - a3 + a2 + b3;
		a_pTemp[ i + 24 ] = a0 - a2 - b4;
		a_pTemp[ i + 32 ] = a0 - a2 + b4;
		a_pTemp[ i + 40 ] = a1 - a3 + a2 - b3;
		a_pTemp[ i + 48 ] = a1 + a3 - a2 - b2;
		a_pTemp[ i + 56 ] = a0 + a2 - b0;
	}
}

// Rows of the temp block, with the final rounding shift. Pixels are stored modulo 256.
template <class T>
static inline void IdctRows( const TINT* a_pTemp, T* a_pOut, TINT a_iStride )
{
	for ( TINT i = 0; i < 8; i++, a_pTemp += 8, a_pOut += a_iStride )
	{
		BINK_IDCT_TERMS( a_pTemp[ 0 ], a_pTemp[ 1 ], a_pTemp[ 2 ], a_pTemp[ 3 ], a_pTemp[ 4 ], a_pTemp[ 5 ], a_pTemp[ 6 ], a_pTemp[ 7 ] );

		a_pOut[ 0 ] = T( ( a0 + a2 + b0 + 0x7F ) >> 8 );
		a_pOut[ 1 ] = T( ( a1 + a3 - a2 + b2 + 0x7F ) >> 8 );
		a_pOut[ 2 ] = T( ( a1 - a3 + a2 + b3 + 0x7F ) >> 8 );
		a_pOut[ 3 ] = T( ( a0 - a2 - b4 + 0x7F ) >> 8 );
		a_pOut[ 4 ] = T( ( a0 - a2 + b4 + 0x7F ) >> 8 );
		a_pOut[ 5 ] = T( ( a1 - a3 + a2 - b3 + 0x7F ) >> 8 );
		a_pOut[ 6 ] = T( ( a1 + a3 - a2 - b2 + 0x7F ) >> 8 );
		a_pOut[ 7 ] = T( ( a0 + a2 - b0 + 0x7F ) >> 8 );
	}
}

// Writes an intra block, a_iCoefs is how many coefficients there are other than DC
static inline void IdctPut( TUINT8* a_pDst, TINT a_iStride, TINT* a_pBlock, TINT a_iCoefs )
{
	if ( a_iCoefs == 0 )
	{
		Fill8( a_pDst, a_iStride, ( a_pBlock[ 0 ] + 0x7F ) >> 8 );
		return;
	}

	TINT aiTemp[ 64 ];
	IdctColumns( a_pBlock, aiTemp );
	IdctRows<TUINT8>( aiTemp, a_pDst, a_iStride );
}

// In place, the block ends up as differences
static inline void IdctInPlace( TINT* a_pBlock )
{
	TINT aiTemp[ 64 ];
	IdctColumns( a_pBlock, aiTemp );
	IdctRows<TINT>( aiTemp, a_pBlock, 8 );
}

//-----------------------------------------------------------------------------
// Plane
//-----------------------------------------------------------------------------

TBOOL ABinkVideo::DecodePlane( TINT a_iPlane, TBOOL a_bChroma )
{
	ABinkBits& rBits = m_oBits;

	const TINT iBlocksWide = a_bChroma ? ( m_iWidth + 15 ) >> 4 : ( m_iWidth + 7 ) >> 3;
	const TINT iBlocksHigh = a_bChroma ? ( m_iHeight + 15 ) >> 4 : ( m_iHeight + 7 ) >> 3;
	const TINT iWidth      = a_bChroma ? m_iWidth >> 1 : m_iWidth;
	const TINT iHeight     = a_bChroma ? m_iHeight >> 1 : m_iHeight;

	TUINT8*       pDst    = m_apPlanes[ a_iPlane ];
	const TUINT8* pPrev   = m_apPrevious[ a_iPlane ];
	const TINT    iStride = m_aiStrides[ a_iPlane ];

	if ( m_iVersion == VERSION_K && rBits.ReadBit() )
	{
		const TINT iFill = rBits.Read( 8 );

		for ( TINT y = 0; y < iHeight; y++ )
			TUtil::MemSet( pDst + y * iStride, iFill, iWidth );

		rBits.Align32();
		return TTRUE;
	}

	InitLengths( TMath::Max( iWidth, 8 ), iBlocksWide );

	for ( TINT i = 0; i < BUNDLE_NUMOF; i++ )
	{
		if ( !ReadBundleHeader( i ) )
			return TFALSE;
	}

	const TINT iRefEnd = ( iBlocksWide - 1 + iStride * ( iBlocksHigh - 1 ) ) * 8;

	Bundle& rTypes    = m_aBundles[ BUNDLE_BLOCK_TYPES ];
	Bundle& rSubTypes = m_aBundles[ BUNDLE_SUB_BLOCK_TYPES ];
	Bundle& rColors   = m_aBundles[ BUNDLE_COLORS ];
	Bundle& rPatterns = m_aBundles[ BUNDLE_PATTERN ];
	Bundle& rXOffs    = m_aBundles[ BUNDLE_X_OFF ];
	Bundle& rYOffs    = m_aBundles[ BUNDLE_Y_OFF ];
	Bundle& rIntraDC  = m_aBundles[ BUNDLE_INTRA_DC ];
	Bundle& rInterDC  = m_aBundles[ BUNDLE_INTER_DC ];
	Bundle& rRuns     = m_aBundles[ BUNDLE_RUN ];

	const TUINT8* pColors   = rColors.pData;
	const TUINT8* pPatterns = rPatterns.pData;
	const TINT8*  pXOffs    = TREINTERPRETCAST( const TINT8*, rXOffs.pData );
	const TINT8*  pYOffs    = TREINTERPRETCAST( const TINT8*, rYOffs.pData );
	const TINT16* pIntraDC  = TREINTERPRETCAST( const TINT16*, rIntraDC.pData );
	const TINT16* pInterDC  = TREINTERPRETCAST( const TINT16*, rInterDC.pData );
	const TUINT8* pRuns     = rRuns.pData;

	TINT   aiBlock[ 64 ];
	TUINT8 aScaled[ 64 ];

	for ( TINT iBlockY = 0; iBlockY < iBlocksHigh; iBlockY++ )
	{
		if ( !ReadBlockTypes( rTypes ) || !ReadBlockTypes( rSubTypes ) || !ReadColors( rColors ) || !ReadPatterns( rPatterns ) )
			return TFALSE;

		ReadMotionValues( rXOffs );
		ReadMotionValues( rYOffs );

		if ( !ReadDCs( rIntraDC, 11, 0 ) || !ReadDCs( rInterDC, 11, 1 ) )
			return TFALSE;

		ReadRuns( rRuns );

		if ( rBits.GetLeft() < -64 )
			return TFALSE;

		for ( TINT iBlockX = 0; iBlockX < iBlocksWide; iBlockX++ )
		{
			const TINT iOffset = 8 * ( iBlockY * iStride + iBlockX );
			TUINT8*    pBlock  = pDst + iOffset;
			TINT       iType   = rTypes.pData[ rTypes.iCurPtr++ ];

			// The other three quarters of a 16x16 block
			if ( ( ( iBlockY | iBlockX ) & 1 ) && iType == BLOCK_SCALED )
			{
				iBlockX++;
				continue;
			}

			switch ( iType )
			{
				case BLOCK_SKIP:
					Copy8( pBlock, pPrev + iOffset, iStride );
					break;
				case BLOCK_SCALED:
				{
					iType = rSubTypes.pData[ rSubTypes.iCurPtr++ ];

					switch ( iType )
					{
						case BLOCK_RUN:
						{
							if ( rBits.GetLeft() < 4 )
								return TFALSE;

							const TUINT8* pScan = s_aBinkPatterns + rBits.Read( 4 ) * 64;
							TINT          iAt   = 0;

							do
							{
								const TINT iRun = pRuns[ rRuns.iCurPtr++ ] + 1;
								iAt += iRun;

								if ( iAt > 64 )
									return TFALSE;

								if ( rBits.ReadBit() )
								{
									const TUINT8 uiValue = pColors[ rColors.iCurPtr++ ];

									for ( TINT j = 0; j < iRun; j++ )
										aScaled[ *pScan++ ] = uiValue;
								}
								else
								{
									for ( TINT j = 0; j < iRun; j++ )
										aScaled[ *pScan++ ] = pColors[ rColors.iCurPtr++ ];
								}
							} while ( iAt < 63 );

							if ( iAt == 63 )
								aScaled[ *pScan++ ] = pColors[ rColors.iCurPtr++ ];

							break;
						}
						case BLOCK_INTRA:
						{
							TUtil::MemClear( aiBlock, sizeof( aiBlock ) );
							aiBlock[ 0 ] = pIntraDC[ rIntraDC.iCurPtr++ ];

							const TINT iQuant = ReadDctCoeffs( aiBlock );

							if ( iQuant < 0 )
								return TFALSE;

							Unquantize( aiBlock, s_aBinkIntraQuant, iQuant );
							IdctPut( aScaled, 8, aiBlock, m_iCoefCount );
							break;
						}
						case BLOCK_FILL:
						{
							const TINT iValue = pColors[ rColors.iCurPtr++ ];

							for ( TINT y = 0; y < 16; y++ )
								TUtil::MemSet( pBlock + y * iStride, iValue, 16 );

							break;
						}
						case BLOCK_PATTERN:
						{
							const TUINT8 uiColor0 = pColors[ rColors.iCurPtr++ ];
							const TUINT8 uiColor1 = pColors[ rColors.iCurPtr++ ];

							for ( TINT j = 0; j < 8; j++ )
							{
								TINT iBits = pPatterns[ rPatterns.iCurPtr++ ];

								for ( TINT i = 0; i < 8; i++, iBits >>= 1 )
									aScaled[ i + j * 8 ] = ( iBits & 1 ) ? uiColor1 : uiColor0;
							}

							break;
						}
						case BLOCK_RAW:
							TUtil::MemCopy( aScaled, pColors + rColors.iCurPtr, 64 );
							rColors.iCurPtr += 64;
							break;
						default:
							return TFALSE;
					}

					if ( iType != BLOCK_FILL )
						ScaleBlock( aScaled, pBlock, iStride );

					iBlockX++;
					break;
				}
				case BLOCK_MOTION:
				{
					const TINT iRef = iOffset + pXOffs[ rXOffs.iCurPtr++ ] + pYOffs[ rYOffs.iCurPtr++ ] * iStride;

					if ( iRef < 0 || iRef > iRefEnd )
						return TFALSE;

					Copy8( pBlock, pPrev + iRef, iStride );
					break;
				}
				case BLOCK_RUN:
				{
					const TUINT8* pScan = s_aBinkPatterns + rBits.Read( 4 ) * 64;
					TINT          iAt   = 0;

					do
					{
						const TINT iRun = pRuns[ rRuns.iCurPtr++ ] + 1;
						iAt += iRun;

						if ( iAt > 64 )
							return TFALSE;

						if ( rBits.ReadBit() )
						{
							const TUINT8 uiValue = pColors[ rColors.iCurPtr++ ];

							for ( TINT j = 0; j < iRun; j++ )
							{
								const TINT iPos                                  = *pScan++;
								pBlock[ ( iPos & 7 ) + ( iPos >> 3 ) * iStride ] = uiValue;
							}
						}
						else
						{
							for ( TINT j = 0; j < iRun; j++ )
							{
								const TINT iPos                                  = *pScan++;
								pBlock[ ( iPos & 7 ) + ( iPos >> 3 ) * iStride ] = pColors[ rColors.iCurPtr++ ];
							}
						}
					} while ( iAt < 63 );

					if ( iAt == 63 )
					{
						const TINT iPos                                  = *pScan++;
						pBlock[ ( iPos & 7 ) + ( iPos >> 3 ) * iStride ] = pColors[ rColors.iCurPtr++ ];
					}

					break;
				}
				case BLOCK_RESIDUE:
				{
					const TINT iRef = iOffset + pXOffs[ rXOffs.iCurPtr++ ] + pYOffs[ rYOffs.iCurPtr++ ] * iStride;

					if ( iRef < 0 || iRef > iRefEnd )
						return TFALSE;

					Copy8( pBlock, pPrev + iRef, iStride );

					TUtil::MemClear( aiBlock, sizeof( aiBlock ) );
					ReadResidue( aiBlock, rBits.Read( 7 ) );
					Add8( pBlock, iStride, aiBlock );
					break;
				}
				case BLOCK_INTRA:
				{
					TUtil::MemClear( aiBlock, sizeof( aiBlock ) );
					aiBlock[ 0 ] = pIntraDC[ rIntraDC.iCurPtr++ ];

					const TINT iQuant = ReadDctCoeffs( aiBlock );

					if ( iQuant < 0 )
						return TFALSE;

					Unquantize( aiBlock, s_aBinkIntraQuant, iQuant );
					IdctPut( pBlock, iStride, aiBlock, m_iCoefCount );
					break;
				}
				case BLOCK_FILL:
					Fill8( pBlock, iStride, pColors[ rColors.iCurPtr++ ] );
					break;
				case BLOCK_INTER:
				{
					const TINT iRef = iOffset + pXOffs[ rXOffs.iCurPtr++ ] + pYOffs[ rYOffs.iCurPtr++ ] * iStride;

					if ( iRef < 0 || iRef > iRefEnd )
						return TFALSE;

					Copy8( pBlock, pPrev + iRef, iStride );

					TUtil::MemClear( aiBlock, sizeof( aiBlock ) );
					aiBlock[ 0 ] = pInterDC[ rInterDC.iCurPtr++ ];

					const TINT iQuant = ReadDctCoeffs( aiBlock );

					if ( iQuant < 0 )
						return TFALSE;

					Unquantize( aiBlock, s_aBinkInterQuant, iQuant );
					IdctInPlace( aiBlock );
					Add8( pBlock, iStride, aiBlock );
					break;
				}
				case BLOCK_PATTERN:
				{
					const TUINT8 uiColor0 = pColors[ rColors.iCurPtr++ ];
					const TUINT8 uiColor1 = pColors[ rColors.iCurPtr++ ];

					for ( TINT i = 0; i < 8; i++ )
					{
						TINT    iBits = pPatterns[ rPatterns.iCurPtr++ ];
						TUINT8* pRow  = pBlock + i * iStride;

						for ( TINT j = 0; j < 8; j++, iBits >>= 1 )
							pRow[ j ] = ( iBits & 1 ) ? uiColor1 : uiColor0;
					}

					break;
				}
				case BLOCK_RAW:
				{
					for ( TINT i = 0; i < 8; i++ )
						memcpy( pBlock + i * iStride, pColors + rColors.iCurPtr + i * 8, 8 );

					rColors.iCurPtr += 64;
					break;
				}
				default:
					return TFALSE;
			}
		}
	}

	if ( rBits.IsOver() )
		return TFALSE;

	rBits.Align32();
	return TTRUE;
}

//-----------------------------------------------------------------------------
// Conversion
//-----------------------------------------------------------------------------

TINT ABinkVideo::CopyToSurface( TUINT8* a_pDst, TINT a_iPitch, TUINT32 a_uiHeight, TUINT32 a_uiX, TUINT32 a_uiY, TUINT32 a_uiFormat ) const
{
	const TUINT32 uiFormat = a_uiFormat & 15;

	if ( uiFormat < BINKSURFACE32 || uiFormat > BINKSURFACE32RA )
		return 0;

	const TINT iRows = TMath::Min( m_iHeight, TINT( a_uiHeight ) - TINT( a_uiY ) );

	if ( iRows <= 0 )
		return 0;

	const ColorTables& rTables = s_aColors[ m_iVersion >= VERSION_K ? 1 : 0 ];

	const TINT* pTY = rTables.aY;
	const TINT* pRV = rTables.aRV;
	const TINT* pGU = rTables.aGU;
	const TINT* pGV = rTables.aGV;
	const TINT* pBU = rTables.aBU;

	const TUINT8* pClamp = s_aClamp + 512;

	const TUINT8* pPlaneY = m_apPlanes[ PLANE_Y ];
	const TUINT8* pPlaneU = m_apPlanes[ PLANE_U ];
	const TUINT8* pPlaneV = m_apPlanes[ PLANE_V ];
	const TUINT8* pPlaneA = m_apPlanes[ PLANE_ALPHA ];

	const TINT iStrideY = m_aiStrides[ PLANE_Y ];
	const TINT iStrideC = m_aiStrides[ PLANE_U ];

	const TBOOL bReverse = uiFormat == BINKSURFACE32R || uiFormat == BINKSURFACE32RA;
	const TBOOL bAlpha   = pPlaneA && ( uiFormat == BINKSURFACE32A || uiFormat == BINKSURFACE32RA );

	// Where red and blue go in a little endian pixel
	const TINT iShiftR = bReverse ? 0 : 16;
	const TINT iShiftB = bReverse ? 16 : 0;

	const TINT  iPairs = m_iWidth >> 1;
	const TBOOL bOdd   = ( m_iWidth & 1 ) != 0;

	for ( TINT iRow = 0; iRow < iRows; iRow++ )
	{
		const TUINT8* pY = pPlaneY + iRow * iStrideY;
		const TUINT8* pU = pPlaneU + ( iRow >> 1 ) * iStrideC;
		const TUINT8* pV = pPlaneV + ( iRow >> 1 ) * iStrideC;

		TUINT8* pOut = a_pDst + ( a_uiY + iRow ) * a_iPitch + a_uiX * 4;

		for ( TINT i = 0; i < iPairs + ( bOdd ? 1 : 0 ); i++ )
		{
			const TINT iU = pU[ i ];
			const TINT iV = pV[ i ];
			const TINT iR = pRV[ iV ];
			const TINT iG = pGU[ iU ] + pGV[ iV ];
			const TINT iB = pBU[ iU ];

			TINT    iLuma   = pTY[ pY[ 0 ] ];
			TUINT32 uiPixel = 0xFF000000u | ( TUINT32( pClamp[ ( iLuma + iR ) >> 16 ] ) << iShiftR ) | ( TUINT32( pClamp[ ( iLuma + iG ) >> 16 ] ) << 8 ) | ( TUINT32( pClamp[ ( iLuma + iB ) >> 16 ] ) << iShiftB );

			memcpy( pOut, &uiPixel, 4 );

			if ( i < iPairs )
			{
				iLuma   = pTY[ pY[ 1 ] ];
				uiPixel = 0xFF000000u | ( TUINT32( pClamp[ ( iLuma + iR ) >> 16 ] ) << iShiftR ) | ( TUINT32( pClamp[ ( iLuma + iG ) >> 16 ] ) << 8 ) | ( TUINT32( pClamp[ ( iLuma + iB ) >> 16 ] ) << iShiftB );

				memcpy( pOut + 4, &uiPixel, 4 );
			}

			pY += 2;
			pOut += 8;
		}

		if ( bAlpha )
		{
			const TUINT8* pA      = pPlaneA + iRow * iStrideY;
			TUINT8*       pRowOut = a_pDst + ( a_uiY + iRow ) * a_iPitch + a_uiX * 4;

			for ( TINT i = 0; i < m_iWidth; i++ )
				pRowOut[ i * 4 + 3 ] = pA[ i ];
		}
	}

	return iRows;
}
