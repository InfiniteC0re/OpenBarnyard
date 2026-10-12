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

	// The "flags" of the sound bank, also copied to the play events and the channels
	using FLAGS = TINT32;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE           = 0,
		FLAGS_PAUSED         = BITFLAG( 0 ), // channel: paused (PauseCue), cue: starts paused (PlayCueEx)
		FLAGS_LOOP           = BITFLAG( 1 ), // FSOUND_LOOP_NORMAL, set for the looping tracks
		FLAGS_3D             = BITFLAG( 2 ), // FSOUND_HW3D, the samples are 3D
		FLAGS_UNK3           = BITFLAG( 3 ), // plays a single random sample of the track
		FLAGS_STREAM         = BITFLAG( 4 ),
		FLAGS_STARTED        = BITFLAG( 5 ), // the FMOD channel has been started
		FLAGS_STREAM_OPENING = BITFLAG( 6 ), // waiting for FSOUND_Stream_GetOpenState
	};

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
	~ASound();

	TINT GetFirstWaveForTrack( TINT a_iTrack ) const;

	TINT GetRandomSample( TINT a_iWaveIndexMin, TINT a_iWaveIndexMax );

	TINT GetRandomSampleWeighted( TINT a_iTrackIndex, TINT a_iWaveIndexMin, TINT a_iWaveIndexMax );

	Toshi::T2DynamicVector<Sample>& GetSamples() { return m_vecSamples; }

	TINT GetId() const { return m_iId; }

private:
	FLAGS                          m_iFlags;
	TINT                           m_iId;
	TINT                           m_uiCategoryIndex;
	TUINT8                         m_ui8Priority;
	TFLOAT                         m_fMinDist;
	Toshi::T2DynamicVector<Sample> m_vecSamples;
	Toshi::T2DynamicVector<TINT>   m_vecTracks;
	Toshi::TBitArray               m_TrackLoop;
	Toshi::T2Vector<TINT, 15>      m_vecLoopStarts;
};
