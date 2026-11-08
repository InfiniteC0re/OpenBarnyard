#include "pch.h"
#include "ABinkAudio_WASM.h"

#include <math.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TUINT32 AUDIO_STEREO = 0x2000;
static constexpr TUINT32 AUDIO_USEDCT = 0x1000;
static constexpr TDOUBLE AUDIO_PI     = 3.14159265358979323846;

static const TUINT16 s_aCriticalFreqs[ 25 ] = { 100, 200, 300, 400, 510, 630, 770, 920, 1080, 1270, 1480, 1720, 2000, 2320, 2700, 3150, 3700, 4400, 5300, 6400, 7700, 9500, 12000, 15500, 24500 };
static const TUINT8  s_aRleLengths[ 16 ]    = { 2, 3, 4, 5, 6, 8, 9, 10, 11, 12, 13, 14, 15, 16, 32, 64 };

static TINT GetFrameLength( TINT a_iSampleRate, TUINT32 a_uiFlags )
{
	TINT iBits = a_iSampleRate < 22050 ? 9 : ( a_iSampleRate < 44100 ? 10 : 11 );

	// RDFT: the channels are interleaved in one transform
	if ( !( a_uiFlags & AUDIO_USEDCT ) && ( a_uiFlags & AUDIO_STEREO ) )
		iBits += 1;

	return 1 << iBits;
}

TUINT32 ABinkAudio::GetMemorySize( TINT a_iSampleRate, TUINT32 a_uiFlags )
{
	const TUINT32 uiN    = GetFrameLength( a_iSampleRate, a_uiFlags );
	const TUINT32 uiHalf = uiN / 2;

	// Same order as Create
	return sizeof( TFLOAT ) * ( ( uiN + 2 ) + uiN * 2 + ( uiN >> 4 ) * 2 + uiN * 2 + ( uiHalf + 1 ) * 2 + uiHalf * 2 + uiHalf / 2 * 2 + uiHalf * 2 + uiHalf * 2 ) + sizeof( TUINT16 ) * uiHalf + 16;
}

void ABinkAudio::Create( void* a_pMemory, TINT a_iSampleRate, TUINT32 a_uiFlags )
{
	m_bUseDct      = ( a_uiFlags & AUDIO_USEDCT ) != 0;
	m_iOutChannels = ( a_uiFlags & AUDIO_STEREO ) ? 2 : 1;
	m_iChannels    = m_bUseDct ? m_iOutChannels : 1;
	m_bFirst       = TTRUE;

	const TINT iRate = m_bUseDct ? a_iSampleRate : a_iSampleRate * m_iOutChannels;
	const TINT iN    = GetFrameLength( a_iSampleRate, a_uiFlags );
	const TINT iHalf = iN / 2;

	m_iFrameLength  = iN;
	m_iOverlap      = iN >> 4;
	m_iBlockSamples = iN - m_iOverlap;

	const TDOUBLE dRoot = m_bUseDct ? iN / ( sqrt( TDOUBLE( iN ) ) * 32768.0 ) : 2.0 / ( sqrt( TDOUBLE( iN ) ) * 32768.0 );

	m_fRoot = TFLOAT( dRoot );

	for ( TINT i = 0; i < 96; i++ )
		m_afQuantTable[ i ] = TFLOAT( exp( i * 0.15289164787221953823 ) * dRoot );

	const TINT iHalfRate = ( iRate + 1 ) >> 1;
	TINT       iNumBands = 1;

	while ( iNumBands < 25 && iHalfRate > s_aCriticalFreqs[ iNumBands - 1 ] )
		iNumBands++;

	m_iNumBands    = iNumBands;
	m_aiBands[ 0 ] = 2;

	for ( TINT i = 1; i < iNumBands; i++ )
		m_aiBands[ i ] = TINT( TINT64( s_aCriticalFreqs[ i - 1 ] ) * iN / iHalfRate ) & ~1;

	m_aiBands[ iNumBands ] = iN;

	TFLOAT* pMemory = TSTATICCAST( TFLOAT, a_pMemory );

	TUtil::MemClear( a_pMemory, GetMemorySize( a_iSampleRate, a_uiFlags ) );

	m_pCoeffs = pMemory;
	pMemory += iN + 2;

	for ( TINT i = 0; i < 2; i++ )
	{
		m_apOut[ i ] = pMemory;
		pMemory += iN;
	}

	for ( TINT i = 0; i < 2; i++ )
	{
		m_apPrevious[ i ] = pMemory;
		pMemory += m_iOverlap;
	}

	m_pInterleaved = pMemory;
	pMemory += iN * 2;

	m_pSpectrumRe = pMemory;
	pMemory += iHalf + 1;
	m_pSpectrumIm = pMemory;
	pMemory += iHalf + 1;

	m_pFftRe = pMemory;
	pMemory += iHalf;
	m_pFftIm = pMemory;
	pMemory += iHalf;

	m_pFftCos = pMemory;
	pMemory += iHalf / 2;
	m_pFftSin = pMemory;
	pMemory += iHalf / 2;

	m_pUnpackCos = pMemory;
	pMemory += iHalf;
	m_pUnpackSin = pMemory;
	pMemory += iHalf;

	m_pDctCos = pMemory;
	pMemory += iHalf;
	m_pDctSin = pMemory;
	pMemory += iHalf;

	m_pReverse = TREINTERPRETCAST( TUINT16*, pMemory );

	const TINT iBits = 31 - __builtin_clz( iHalf );

	for ( TINT i = 0; i < iHalf; i++ )
	{
		TINT iReversed = 0;

		for ( TINT b = 0; b < iBits; b++ )
			iReversed |= ( ( i >> b ) & 1 ) << ( iBits - 1 - b );

		m_pReverse[ i ] = TUINT16( iReversed );

		m_pUnpackCos[ i ] = TFLOAT( cos( 2.0 * AUDIO_PI * i / iN ) );
		m_pUnpackSin[ i ] = TFLOAT( sin( 2.0 * AUDIO_PI * i / iN ) );

		// Only needed up to the middle of the spectrum, the rest comes from the symmetry
		m_pDctCos[ i ] = TFLOAT( cos( AUDIO_PI * i / ( 2 * iN ) ) );
		m_pDctSin[ i ] = TFLOAT( sin( AUDIO_PI * i / ( 2 * iN ) ) );
	}

	for ( TINT i = 0; i < iHalf / 2; i++ )
	{
		m_pFftCos[ i ] = TFLOAT( cos( 2.0 * AUDIO_PI * i / iHalf ) );
		m_pFftSin[ i ] = TFLOAT( sin( 2.0 * AUDIO_PI * i / iHalf ) );
	}
}

void ABinkAudio::Decode( const TUINT8* a_pData, TUINT32 a_uiSize, t_fnOutput a_fnOutput, void* a_pUser )
{
	if ( a_uiSize < 4 )
		return;

	m_oBits.Init( a_pData, a_uiSize );
	m_oBits.Skip( 32 ); // decoded size in bytes

	while ( m_oBits.GetLeft() > 0 )
	{
		if ( !DecodeBlock() )
			break;

		if ( m_iChannels == 1 )
		{
			a_fnOutput( m_apOut[ 0 ], m_iOutChannels, m_iBlockSamples / m_iOutChannels, a_pUser );
		}
		else
		{
			for ( TINT i = 0; i < m_iBlockSamples; i++ )
			{
				m_pInterleaved[ i * 2 + 0 ] = m_apOut[ 0 ][ i ];
				m_pInterleaved[ i * 2 + 1 ] = m_apOut[ 1 ][ i ];
			}

			a_fnOutput( m_pInterleaved, 2, m_iBlockSamples, a_pUser );
		}

		m_oBits.Align32();
	}
}

TFLOAT ABinkAudio::GetFloat()
{
	const TINT iPower = m_oBits.Read( 5 );
	TFLOAT     fValue = TFLOAT( ldexp( TDOUBLE( m_oBits.Read( 23 ) ), iPower - 23 ) );

	return m_oBits.ReadBit() ? -fValue : fValue;
}

TBOOL ABinkAudio::DecodeBlock()
{
	ABinkBits& rBits = m_oBits;

	const TINT iN      = m_iFrameLength;
	TFLOAT*    pCoeffs = m_pCoeffs;

	if ( m_bUseDct )
		rBits.Skip( 2 );

	for ( TINT iChannel = 0; iChannel < m_iChannels; iChannel++ )
	{
		if ( rBits.GetLeft() < 58 )
			return TFALSE;

		pCoeffs[ 0 ] = GetFloat() * m_fRoot;
		pCoeffs[ 1 ] = GetFloat() * m_fRoot;

		if ( rBits.GetLeft() < m_iNumBands * 8 )
			return TFALSE;

		for ( TINT i = 0; i < m_iNumBands; i++ )
			m_afQuant[ i ] = m_afQuantTable[ TMath::Min( TINT( rBits.Read( 8 ) ), 95 ) ];

		TINT   iBand  = 0;
		TFLOAT fQuant = m_afQuant[ 0 ];

		for ( TINT i = 2; i < iN; )
		{
			TINT iEnd = rBits.ReadBit() ? i + s_aRleLengths[ rBits.Read( 4 ) ] * 8 : i + 8;

			if ( iEnd > iN )
				iEnd = iN;

			const TINT iWidth = rBits.Read( 4 );

			if ( iWidth == 0 )
			{
				TUtil::MemClear( pCoeffs + i, ( iEnd - i ) * sizeof( TFLOAT ) );
				i = iEnd;

				while ( m_aiBands[ iBand ] < i )
					fQuant = m_afQuant[ iBand++ ];
			}
			else
			{
				for ( ; i < iEnd; i++ )
				{
					if ( m_aiBands[ iBand ] == i )
						fQuant = m_afQuant[ iBand++ ];

					const TINT iCoef = rBits.Read( iWidth );

					if ( iCoef )
						pCoeffs[ i ] = rBits.ReadBit() ? -fQuant * iCoef : fQuant * iCoef;
					else
						pCoeffs[ i ] = 0.0f;
				}
			}
		}

		if ( rBits.IsOver() )
			return TFALSE;

		if ( m_bUseDct )
			InverseDct( m_apOut[ iChannel ] );
		else
			InverseRdft( m_apOut[ iChannel ] );
	}

	// Cross-fade the start with the end of the previous block
	const TINT   iChannels = m_iChannels;
	const TINT   iOverlap  = m_iOverlap;
	const TINT   iCount    = iOverlap * iChannels;
	const TFLOAT fInverse  = 1.0f / iCount;

	for ( TINT iChannel = 0; iChannel < iChannels; iChannel++ )
	{
		TFLOAT* pOut      = m_apOut[ iChannel ];
		TFLOAT* pPrevious = m_apPrevious[ iChannel ];

		if ( !m_bFirst )
		{
			for ( TINT i = 0, j = iChannel; i < iOverlap; i++, j += iChannels )
				pOut[ i ] = ( pPrevious[ i ] * ( iCount - j ) + pOut[ i ] * j ) * fInverse;
		}

		TUtil::MemCopy( pPrevious, pOut + iN - iOverlap, iOverlap * sizeof( TFLOAT ) );
	}

	m_bFirst = TFALSE;
	return TTRUE;
}

// out[n] = 2/N sum( c[k] cos( pi k (n + 1/2) / N ) ), with the DC term counting whole. As a real inverse FFT of N
// points of (c[k] - i c[N-k]) e^(i pi k / 2N): that gives even n from the start and odd n backwards from the end.
void ABinkAudio::InverseDct( TFLOAT* a_pOut )
{
	const TINT    iN      = m_iFrameLength;
	const TINT    iHalf   = iN / 2;
	const TFLOAT* pCoeffs = m_pCoeffs;

	m_pSpectrumRe[ 0 ] = pCoeffs[ 0 ] * 2.0f;
	m_pSpectrumIm[ 0 ] = 0.0f;

	for ( TINT k = 1; k < iHalf; k++ )
	{
		const TFLOAT fA = pCoeffs[ k ];
		const TFLOAT fB = -pCoeffs[ iN - k ];

		m_pSpectrumRe[ k ] = fA * m_pDctCos[ k ] - fB * m_pDctSin[ k ];
		m_pSpectrumIm[ k ] = fA * m_pDctSin[ k ] + fB * m_pDctCos[ k ];
	}

	// (1 - i) c e^(i pi / 4)
	m_pSpectrumRe[ iHalf ] = pCoeffs[ iHalf ] * 1.41421356237309504880f;
	m_pSpectrumIm[ iHalf ] = 0.0f;

	// The output of the real transform is in the FFT arrays until it's put in order
	InverseRealFft( TNULL, 1.0f );

	const TFLOAT  fScale = 1.0f / iN;
	const TFLOAT* pRe    = m_pFftRe;
	const TFLOAT* pIm    = m_pFftIm;

	for ( TINT m = 0; m < iHalf; m++ )
	{
		// y[ 2m ] = re, y[ 2m + 1 ] = im; y[ j ] goes to out[ 2j ], or out[ 2N - 1 - 2j ] in the second half
		const TINT iEven = 2 * m;
		const TINT iOdd  = 2 * m + 1;

		a_pOut[ iEven < iHalf ? 2 * iEven : 2 * iN - 1 - 2 * iEven ] = pRe[ m ] * fScale;
		a_pOut[ iOdd < iHalf ? 2 * iOdd : 2 * iN - 1 - 2 * iOdd ]    = pIm[ m ] * fScale;
	}
}

// The packed spectrum is c[0] = DC, c[1] = Nyquist, then re/im pairs, stored as the conjugate
void ABinkAudio::InverseRdft( TFLOAT* a_pOut )
{
	const TINT    iHalf   = m_iFrameLength / 2;
	const TFLOAT* pCoeffs = m_pCoeffs;

	m_pSpectrumRe[ 0 ]     = pCoeffs[ 0 ];
	m_pSpectrumIm[ 0 ]     = 0.0f;
	m_pSpectrumRe[ iHalf ] = pCoeffs[ 1 ];
	m_pSpectrumIm[ iHalf ] = 0.0f;

	for ( TINT k = 1; k < iHalf; k++ )
	{
		m_pSpectrumRe[ k ] = pCoeffs[ 2 * k ];
		m_pSpectrumIm[ k ] = -pCoeffs[ 2 * k + 1 ];
	}

	InverseRealFft( a_pOut, 0.5f );
}

// Hermitian spectrum of N points -> N real samples, through a complex FFT of N/2. Without an output the samples stay
// in the FFT arrays, as y[ 2m ] + i y[ 2m + 1 ].
void ABinkAudio::InverseRealFft( TFLOAT* a_pOut, TFLOAT a_fScale )
{
	const TINT iHalf = m_iFrameLength / 2;

	const TFLOAT* pSpecRe = m_pSpectrumRe;
	const TFLOAT* pSpecIm = m_pSpectrumIm;
	TFLOAT*       pRe     = m_pFftRe;
	TFLOAT*       pIm     = m_pFftIm;

	for ( TINT k = 0; k < iHalf; k++ )
	{
		const TFLOAT fAr = pSpecRe[ k ];
		const TFLOAT fAi = pSpecIm[ k ];
		const TFLOAT fBr = pSpecRe[ iHalf - k ];
		const TFLOAT fBi = -pSpecIm[ iHalf - k ];

		const TFLOAT fEr = fAr + fBr;
		const TFLOAT fEi = fAi + fBi;
		const TFLOAT fDr = fAr - fBr;
		const TFLOAT fDi = fAi - fBi;
		const TFLOAT fOr = fDr * m_pUnpackCos[ k ] - fDi * m_pUnpackSin[ k ];
		const TFLOAT fOi = fDr * m_pUnpackSin[ k ] + fDi * m_pUnpackCos[ k ];

		const TINT iAt = m_pReverse[ k ];

		pRe[ iAt ] = fEr - fOi;
		pIm[ iAt ] = fEi + fOr;
	}

	InverseFft();

	if ( a_pOut )
	{
		for ( TINT m = 0; m < iHalf; m++ )
		{
			a_pOut[ 2 * m + 0 ] = pRe[ m ] * a_fScale;
			a_pOut[ 2 * m + 1 ] = pIm[ m ] * a_fScale;
		}
	}
}

// Radix 2, e^(+i), not normalised. The input is already in bit reversed order.
void ABinkAudio::InverseFft()
{
	const TINT iSize = m_iFrameLength / 2;

	TFLOAT*       pRe  = m_pFftRe;
	TFLOAT*       pIm  = m_pFftIm;
	const TFLOAT* pCos = m_pFftCos;
	const TFLOAT* pSin = m_pFftSin;

	for ( TINT iSpan = 2; iSpan <= iSize; iSpan <<= 1 )
	{
		const TINT iSpanHalf = iSpan >> 1;
		const TINT iStep     = iSize / iSpan;

		for ( TINT iStart = 0; iStart < iSize; iStart += iSpan )
		{
			for ( TINT k = 0, t = 0; k < iSpanHalf; k++, t += iStep )
			{
				const TINT iA = iStart + k;
				const TINT iB = iA + iSpanHalf;

				const TFLOAT fWr = pCos[ t ];
				const TFLOAT fWi = pSin[ t ];
				const TFLOAT fXr = pRe[ iB ] * fWr - pIm[ iB ] * fWi;
				const TFLOAT fXi = pRe[ iB ] * fWi + pIm[ iB ] * fWr;

				pRe[ iB ] = pRe[ iA ] - fXr;
				pIm[ iB ] = pIm[ iA ] - fXi;
				pRe[ iA ] += fXr;
				pIm[ iA ] += fXi;
			}
		}
	}
}
