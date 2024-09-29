#include "pch.h"
#include "ASound.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_USING

ASound::ASound()
    : m_iFlags( 0 ), m_iId( 0 ), m_uiCategoryIndex( 0 ), m_ui8Priority( 255 ), m_fMinDist( 1.0f ), m_vecSamples( AMemory::GetAllocator( AMemory::POOL_Sound ), 1, 1 ), m_vecTracks( AMemory::GetAllocator( AMemory::POOL_Sound ), 1, 1 )
{
}

TINT ASound::GetFirstWaveForTrack( TINT a_iTrack ) const
{
	TINT iFirstWaveIndex = 0;
	for ( TINT iTrack = 0; iTrack < a_iTrack; iTrack++ )
		iFirstWaveIndex += m_vecTracks[ iTrack ];

	return iFirstWaveIndex;
}

TINT ASound::GetRandomSample( TINT a_iWaveIndexMin, TINT a_iWaveIndexMax )
{
	return a_iWaveIndexMin + ARandom::GetSingleton()->m_oRandom.GetInt( a_iWaveIndexMax - a_iWaveIndexMin );
}

TINT ASound::GetRandomSampleWeighted( TINT a_iTrackIndex, TINT a_iWaveIndexMin, TINT a_iWaveIndexMax )
{
	TASSERT( a_iTrackIndex != -1 );
	TASSERT( a_iWaveIndexMax - a_iWaveIndexMin > 0 );

	TINT  iNumWaves      = a_iWaveIndexMax - a_iWaveIndexMin;
	TINT  iTrackNumWaves = m_vecTracks[ a_iTrackIndex ];
	TINT* pWeights       = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) TINT[ iTrackNumWaves ];

	TINT iTotalWeight = 0;
	for ( TINT i = 0; i < a_iWaveIndexMax - a_iWaveIndexMin; i++ )
	{
		pWeights[ i ] = m_vecSamples[ i + a_iWaveIndexMin ].iWeight;
		iTotalWeight += pWeights[ i ];
	}

	TINT* pWeightedSamples = new ( AMemory::GetMemBlock( AMemory::POOL_Sound ) ) TINT[ iTotalWeight ];
	for ( TINT i = 0, j = 0; i < a_iWaveIndexMax - a_iWaveIndexMin; i++ )
	{
		// Copy sample index N (weight) times to the array
		for ( TINT k = 0; k < pWeights[ i ]; k++ )
		{
			TASSERT( j < iTotalWeight );
			pWeightedSamples[ j++ ] = a_iWaveIndexMin + i;
		}
	}

	TINT iSelectedSample = ARandom::GetSingleton()->m_oRandom.GetInt( iTotalWeight );

	delete[] pWeights;
	delete[] pWeightedSamples;

	return iSelectedSample;
}
