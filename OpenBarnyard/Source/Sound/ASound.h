#pragma once
#include "ASoundLegacy.h"
#include "ARandom.h"
#include "Memory/AMemory.h"

#include <Toshi/TBitArray.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

class AWaveBank;

class ASound
{
public:
	friend class ASoundManager;

	struct Sample
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
	ASound();

	TINT GetFirstWaveForTrack( TINT a_iTrack ) const;

	TINT GetRandomSample( TINT a_iWaveIndexMin, TINT a_iWaveIndexMax );

	TINT GetRandomSampleWeighted( TINT a_iTrackIndex, TINT a_iWaveIndexMin, TINT a_iWaveIndexMax );

private:
	TINT                           m_iFlags;
	TINT                           m_iId;
	TINT                           m_uiCategoryIndex;
	TUINT8                         m_ui8Priority;
	TFLOAT                         m_fMinDist;
	Toshi::T2DynamicVector<Sample> m_vecSamples;
	Toshi::T2DynamicVector<TINT>   m_vecTracks;
	Toshi::TBitArray               m_TrackLoop;
	Toshi::T2Vector<TINT, 15>      m_vecLoopStarts;
};
