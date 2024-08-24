#pragma once
#include "ASoundLegacy.h"
#include "Memory/AMemory.h"

#include <Toshi/TBitArray.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AWaveBank;

class ASound
{
public:
	friend class ASoundManager;

	struct Wave
	{
		AWaveBank* pWaveBank            = TNULL;
		TINT       iId                  = -1;
		TFLOAT     fVolume              = 1.0f;
		TFLOAT     fMinVolumeMultiplier = 0.0f;
		TFLOAT     fMaxVolumeMultiplier = 0.0f;
		TFLOAT     fPitch               = 1.0f;
		TFLOAT     fMinPitch            = 0.0f;
		TFLOAT     fMaxPitch            = 0.0f;
		TINT       iFrequency           = 44100;
		TFLOAT     fStart               = 0.0f;
		TFLOAT     fVarDelay            = 0.0f;
		TINT       iWeight              = 50;
	};

public:
	ASound() :
	    m_iFlags( 0 ),
	    m_iId( 0 ),
	    m_uiCategoryIndex( 0 ),
	    m_ui8Priority( 255 ),
	    m_fMinDist( 1.0f ),
	    m_vecWaves( AMemory::GetAllocator( AMemory::POOL_Sound ), 1, 1 ),
	    m_vecTracks( AMemory::GetAllocator( AMemory::POOL_Sound ), 1, 1 )
	{
	}

	TINT GetFirstWaveForTrack( TINT a_iTrack ) const
	{
		TINT iFirstWaveIndex = 0;
		for ( TINT iTrack = 0; iTrack < a_iTrack; iTrack++ )
			iFirstWaveIndex += m_vecTracks[ iTrack ];

		return iFirstWaveIndex;
	}

private:
	TINT                         m_iFlags;
	TINT                         m_iId;
	TINT                         m_uiCategoryIndex;
	TUINT8                       m_ui8Priority;
	TFLOAT                       m_fMinDist;
	Toshi::T2DynamicVector<Wave> m_vecWaves;
	Toshi::T2DynamicVector<TINT> m_vecTracks;
	Toshi::TBitArray             m_TrackLoop;
	Toshi::T2Vector<TINT, 15>    m_vecLoopStarts;
};
