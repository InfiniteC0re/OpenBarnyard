#include "pch.h"

#include <fmod/fmod.h>

#include <File/TFile.h>

#include "Platform/WASM/Movie/AMovieVoice_WASM.h"

#include <emscripten.h>
#include <pthread.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// There's no FMOD 3 for the web: this plays the game's FSB3 banks with the same calls. A thread mixes into
// a ring buffer the page's AudioWorklet reads, and the time keeps going when the page can't play sound yet.

static constexpr TINT   MIX_RATE     = 44100;
static constexpr TINT   MIX_CHUNK    = 512;
static constexpr TINT   RING_FRAMES  = 8192;
static constexpr TINT   MAX_CHANNELS = 64;
static constexpr TINT   MAX_VOICES   = 4;
static constexpr TINT   BLOCK_FRAMES = 64;
static constexpr TINT   BLOCK_BYTES  = 0x24;
static constexpr TUINT8 FSB_BASICHEADERS = 2;

static TFLOAT       s_afRing[ RING_FRAMES * 2 ];
static volatile int s_iRingWrite;

extern "C" EMSCRIPTEN_KEEPALIVE float* WASM_GetAudioRing()
{
	return s_afRing;
}

extern "C" EMSCRIPTEN_KEEPALIVE volatile int* WASM_GetAudioWrite()
{
	return &s_iRingWrite;
}

struct SampleInfo
{
	TCHAR   szName[ 32 ];
	TUINT32 uiLength;
	TUINT32 uiCompressed;
	TUINT32 uiLoopStart;
	TUINT32 uiLoopEnd;
	TUINT32 uiMode;
	TINT    iFrequency;
	TINT    iVolume;
	TINT    iPan;
	TINT    iPriority;
	TINT    iChannels;
	TFLOAT  fMinDistance;
	TFLOAT  fMaxDistance;
	TUINT32 uiOffset;
};

struct FMUSIC_MODULE;

struct FSOUND_SAMPLE
{
	SampleInfo     oInfo;
	TUINT32        uiMode;
	FMUSIC_MODULE* pOwner;
	const TUINT8*  pData;
	TINT16*        pDecoded; // interleaved, on the first play
};

struct FMUSIC_MODULE
{
	TUINT8*        pBytes;
	TINT           iNumSamples;
	FSOUND_SAMPLE* pSamples;
};

struct FSOUND_STREAM
{
	TCHAR       szPath[ 256 ];
	TUINT8*     pMemory;
	TUINT32     uiBase;
	SampleInfo* pSubs;
	TINT        iNumSubs;
	TINT        iSub;
	TUINT32     uiMode;
	TINT        iChannel;
	TINT        iGeneration;
	TUINT32     uiTimeMs;
	TFile*      pFile;
	TINT        iCachedBlock;
	TINT16      aiBlock[ BLOCK_FRAMES * 4 ];

	// What FSOUND_GetCurrentSample answers for a stream: captions are found by the name of the line
	FSOUND_SAMPLE oCurrentSample;
};

struct Channel
{
	TINT           iGeneration;
	FSOUND_SAMPLE* pSample;
	FSOUND_STREAM* pStream;
	const SampleInfo* pInfo;
	TBOOL          bReserved;
	TBOOL          bPaused;
	TBOOL          b3D;
	TBOOL          bBackwards;
	TUINT32        uiLoop;
	TINT           iNumVoices;
	TINT           aiVolumes[ MAX_VOICES ];
	TINT           aiPans[ MAX_VOICES ];
	TINT           iFrequency;
	TINT           iPriority;
	double         dPosition;
	TFLOAT         afPosition[ 3 ];
	TFLOAT         fMinDistance;
	TFLOAT         fMaxDistance;
};

static Channel         s_aChannels[ MAX_CHANNELS ];
static TINT            s_iNextGeneration = 1;
static pthread_mutex_t s_oMutex          = PTHREAD_MUTEX_INITIALIZER;
static pthread_t       s_oMixThread;
static TBOOL           s_bMixing;
static TFLOAT          s_afListener[ 3 ];
static TFLOAT          s_afRight[ 3 ] = { 1.0f, 0.0f, 0.0f };
static TUINT8          s_aOutputHandle[ 16 ];

class ScopedAudioLock
{
public:
	ScopedAudioLock() { pthread_mutex_lock( &s_oMutex ); }
	~ScopedAudioLock() { pthread_mutex_unlock( &s_oMutex ); }
};

//-----------------------------------------------------------------------------
// FSB3
//-----------------------------------------------------------------------------

static TUINT32 Read32( const TUINT8* a_pData ) { TUINT32 uiValue; memcpy( &uiValue, a_pData, 4 ); return uiValue; }
static TUINT16 Read16( const TUINT8* a_pData ) { TUINT16 uiValue; memcpy( &uiValue, a_pData, 2 ); return uiValue; }

// The sample headers, the offsets are from the start of the bank
static TINT ParseFSB( const TUINT8* a_pData, TUINT32 a_uiSize, SampleInfo** a_ppOutInfos )
{
	if ( a_uiSize < 24 || memcmp( a_pData, "FSB3", 4 ) != 0 )
		return 0;

	const TINT    iNumSamples = TINT( Read32( a_pData + 4 ) );
	const TUINT32 uiHeaders   = Read32( a_pData + 8 );
	const TUINT32 uiBankMode  = Read32( a_pData + 20 );

	SampleInfo* pInfos = TSTATICCAST( SampleInfo, TMalloc( sizeof( SampleInfo ) * iNumSamples ) );
	TUINT32     uiPos  = 24;

	for ( TINT i = 0; i < iNumSamples; i++ )
	{
		SampleInfo& rInfo = pInfos[ i ];

		if ( i > 0 && ( uiBankMode & FSB_BASICHEADERS ) )
		{
			rInfo              = pInfos[ 0 ];
			rInfo.uiLength     = Read32( a_pData + uiPos );
			rInfo.uiCompressed = Read32( a_pData + uiPos + 4 );
			uiPos += 8;
			continue;
		}

		const TUINT8* pHeader = a_pData + uiPos;
		const TUINT16 uiSize  = Read16( pHeader );

		TUtil::MemCopy( rInfo.szName, pHeader + 2, 30 );
		rInfo.szName[ 30 ] = '\0';
		rInfo.uiLength     = Read32( pHeader + 32 );
		rInfo.uiCompressed = Read32( pHeader + 36 );
		rInfo.uiLoopStart  = Read32( pHeader + 40 );
		rInfo.uiLoopEnd    = Read32( pHeader + 44 );
		rInfo.uiMode       = Read32( pHeader + 48 );
		rInfo.iFrequency   = TINT( Read32( pHeader + 52 ) );
		rInfo.iVolume      = Read16( pHeader + 56 );
		rInfo.iPan         = TINT16( Read16( pHeader + 58 ) );
		rInfo.iPriority    = Read16( pHeader + 60 );
		rInfo.iChannels    = TMath::Max( TINT( Read16( pHeader + 62 ) ), 1 );

		TUINT32 uiMin = 0, uiMax = 0;

		if ( uiSize >= 72 )
		{
			uiMin = Read32( pHeader + 64 );
			uiMax = Read32( pHeader + 68 );
			memcpy( &rInfo.fMinDistance, &uiMin, 4 );
			memcpy( &rInfo.fMaxDistance, &uiMax, 4 );
		}
		else
		{
			rInfo.fMinDistance = 1.0f;
			rInfo.fMaxDistance = 1000000.0f;
		}

		uiPos += uiSize;
	}

	TUINT32 uiOffset = 24 + uiHeaders;

	for ( TINT i = 0; i < iNumSamples; i++ )
	{
		pInfos[ i ].uiOffset = uiOffset;
		uiOffset += pInfos[ i ].uiCompressed;
	}

	*a_ppOutInfos = pInfos;
	return iNumSamples;
}

static const TINT16 s_aImaSteps[ 89 ] = {
	7, 8, 9, 10, 11, 12, 13, 14, 16, 17, 19, 21, 23, 25, 28, 31, 34, 37, 41, 45, 50, 55, 60, 66, 73, 80, 88, 97, 107, 118, 130, 143,
	157, 173, 190, 209, 230, 253, 279, 307, 337, 371, 408, 449, 494, 544, 598, 658, 724, 796, 876, 963, 1060, 1166, 1282, 1411, 1552,
	1707, 1878, 2066, 2272, 2499, 2749, 3024, 3327, 3660, 4026, 4428, 4871, 5358, 5894, 6484, 7132, 7845, 8630, 9493, 10442, 11487,
	12635, 13899, 15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794, 32767
};

static const TINT8 s_aImaIndex[ 8 ] = { -1, -1, -1, -1, 2, 4, 6, 8 };

// One block of Xbox style IMA ADPCM: multichannel samples keep their headers split and their data in 2 byte groups
static void DecodeImaBlock( const SampleInfo& a_rInfo, const TUINT8* a_pBlock, TINT16* a_pOut )
{
	const TINT  iChannels = a_rInfo.iChannels;
	const TBOOL bMulti    = ( a_rInfo.uiMode & FSOUND_MULTICHANNEL ) != 0 || iChannels > 2;
	const TUINT8* pData   = a_pBlock + 4 * iChannels;

	for ( TINT c = 0; c < iChannels; c++ )
	{
		const TINT iHeader = bMulti ? 2 * c : 4 * c;
		const TINT iStep   = bMulti ? 2 * iChannels + 2 * c : iHeader + 2;

		TINT iPrediction = TINT16( Read16( a_pBlock + iHeader ) );
		TINT iIndex      = TMath::Min( TINT( a_pBlock[ iStep ] ), 88 );

		a_pOut[ c ] = TINT16( iPrediction );

		for ( TINT i = 1; i < BLOCK_FRAMES; i++ )
		{
			const TINT k = i - 1;

			const TINT iAt = bMulti ?
			    ( k >> 2 ) * 2 * iChannels + 2 * c + ( ( k & 3 ) >> 1 ) :
			    ( k >> 3 ) * 4 * iChannels + 4 * c + ( ( k & 7 ) >> 1 );

			const TINT iNibble = ( pData[ iAt ] >> ( ( k & 1 ) << 2 ) ) & 15;
			const TINT iSize   = s_aImaSteps[ iIndex ];

			TINT iDiff = iSize >> 3;
			if ( iNibble & 1 ) iDiff += iSize >> 2;
			if ( iNibble & 2 ) iDiff += iSize >> 1;
			if ( iNibble & 4 ) iDiff += iSize;

			iPrediction = ( iNibble & 8 ) ? TMath::Max( iPrediction - iDiff, -32768 ) : TMath::Min( iPrediction + iDiff, 32767 );
			iIndex      = TMath::Max( 0, TMath::Min( 88, iIndex + s_aImaIndex[ iNibble & 7 ] ) );

			a_pOut[ i * iChannels + c ] = TINT16( iPrediction );
		}
	}
}

static void GetLoopRange( const SampleInfo& a_rInfo, double& a_rdStart, double& a_rdEnd )
{
	const TUINT32 uiEnd = ( a_rInfo.uiLoopEnd > a_rInfo.uiLoopStart && a_rInfo.uiLoopEnd < a_rInfo.uiLength ) ? a_rInfo.uiLoopEnd + 1 : a_rInfo.uiLength;

	a_rdStart = TMath::Min( a_rInfo.uiLoopStart, uiEnd > 0 ? uiEnd - 1 : 0 );
	a_rdEnd   = uiEnd;
}

static void DecodeSample( FSOUND_SAMPLE* a_pSample )
{
	const SampleInfo& rInfo     = a_pSample->oInfo;
	const TINT        iChannels = rInfo.iChannels;

	a_pSample->pDecoded = TSTATICCAST( TINT16, TMalloc( rInfo.uiLength * iChannels * sizeof( TINT16 ) + BLOCK_FRAMES * iChannels * sizeof( TINT16 ) ) );

	if ( rInfo.uiMode & FSOUND_IMAADPCM )
	{
		const TUINT32 uiBlockBytes = BLOCK_BYTES * iChannels;
		const TUINT32 uiNumBlocks  = rInfo.uiCompressed / uiBlockBytes;

		for ( TUINT32 b = 0; b < uiNumBlocks && b * BLOCK_FRAMES < rInfo.uiLength; b++ )
			DecodeImaBlock( rInfo, a_pSample->pData + b * uiBlockBytes, a_pSample->pDecoded + b * BLOCK_FRAMES * iChannels );
	}
	else if ( rInfo.uiMode & FSOUND_8BITS )
	{
		const TBOOL bUnsigned = ( rInfo.uiMode & FSOUND_UNSIGNED ) != 0;

		for ( TUINT32 i = 0; i < rInfo.uiLength * iChannels; i++ )
			a_pSample->pDecoded[ i ] = bUnsigned ? TINT16( ( TINT( a_pSample->pData[ i ] ) - 128 ) << 8 ) : TINT16( TINT8( a_pSample->pData[ i ] ) << 8 );
	}
	else
	{
		TUtil::MemCopy( a_pSample->pDecoded, a_pSample->pData, TMath::Min( rInfo.uiCompressed, rInfo.uiLength * iChannels * 2 ) );
	}
}

//-----------------------------------------------------------------------------
// Mixing
//-----------------------------------------------------------------------------

static TBOOL IsMultichannel( const SampleInfo& a_rInfo )
{
	return ( a_rInfo.uiMode & FSOUND_MULTICHANNEL ) != 0 || a_rInfo.iChannels > 2;
}

static const TINT16* GetStreamFrame( FSOUND_STREAM* a_pStream, const SampleInfo& a_rInfo, TINT a_iFrame )
{
	const TINT iChannels = a_rInfo.iChannels;
	const TINT iBlock    = a_iFrame / BLOCK_FRAMES;

	if ( iChannels > MAX_VOICES )
		return TNULL;

	if ( iBlock != a_pStream->iCachedBlock )
	{
		const TBOOL   bCompressed  = ( a_rInfo.uiMode & FSOUND_IMAADPCM ) != 0;
		const TUINT32 uiBlockBytes = bCompressed ? BLOCK_BYTES * iChannels : BLOCK_FRAMES * iChannels * sizeof( TINT16 );
		const TUINT32 uiOffset     = a_pStream->uiBase + a_rInfo.uiOffset + iBlock * uiBlockBytes;

		TUINT8  aCompressed[ BLOCK_BYTES * MAX_VOICES ];
		TUINT8* pBlock = bCompressed ? aCompressed : TREINTERPRETCAST( TUINT8*, a_pStream->aiBlock );

		if ( a_pStream->pMemory )
		{
			TUtil::MemCopy( pBlock, a_pStream->pMemory + uiOffset, uiBlockBytes );
		}
		else
		{
			if ( !a_pStream->pFile )
				a_pStream->pFile = TFile::Create( a_pStream->szPath );

			if ( !a_pStream->pFile )
				return TNULL;

			a_pStream->pFile->Seek( uiOffset, TSEEK_SET );
			a_pStream->pFile->Read( pBlock, uiBlockBytes );
		}

		if ( bCompressed )
			DecodeImaBlock( a_rInfo, aCompressed, a_pStream->aiBlock );

		a_pStream->iCachedBlock = iBlock;
	}

	return a_pStream->aiBlock + ( a_iFrame % BLOCK_FRAMES ) * iChannels;
}

static void MixChannel( Channel& a_rChannel, TFLOAT* a_pOut, TINT a_iFrames )
{
	const SampleInfo& rInfo = *a_rChannel.pInfo;

	if ( a_rChannel.pSample && !a_rChannel.pSample->pDecoded )
		DecodeSample( a_rChannel.pSample );

	TFLOAT fGain = a_rChannel.aiVolumes[ 0 ] / 255.0f;
	TFLOAT fPan  = a_rChannel.aiPans[ 0 ] < 0 ? 0.5f : a_rChannel.aiPans[ 0 ] / 255.0f;

	if ( a_rChannel.b3D )
	{
		const TFLOAT fX = a_rChannel.afPosition[ 0 ] - s_afListener[ 0 ];
		const TFLOAT fY = a_rChannel.afPosition[ 1 ] - s_afListener[ 1 ];
		const TFLOAT fZ = a_rChannel.afPosition[ 2 ] - s_afListener[ 2 ];

		const TFLOAT fDistance = sqrtf( fX * fX + fY * fY + fZ * fZ );

		if ( fDistance > a_rChannel.fMinDistance )
			fGain *= a_rChannel.fMinDistance / fDistance;

		if ( fDistance > 0.001f )
			fPan = 0.5f + 0.5f * ( fX * s_afRight[ 0 ] + fY * s_afRight[ 1 ] + fZ * s_afRight[ 2 ] ) / fDistance;
	}

	const TFLOAT fLeftGain  = fGain * sqrtf( 1.0f - fPan );
	const TFLOAT fRightGain = fGain * sqrtf( fPan );
	const double dStep      = double( a_rChannel.iFrequency ) / MIX_RATE;
	const TINT   iChannels  = rInfo.iChannels;

	// Multichannel sounds are one mono voice per channel, each with its own volume and pan
	TFLOAT afVoiceLeft[ MAX_VOICES ];
	TFLOAT afVoiceRight[ MAX_VOICES ];

	for ( TINT i = 0; i < a_rChannel.iNumVoices; i++ )
	{
		const TFLOAT fVoiceGain = a_rChannel.aiVolumes[ i ] / 255.0f;
		const TFLOAT fVoicePan  = a_rChannel.aiPans[ i ] < 0 ? 0.5f : a_rChannel.aiPans[ i ] / 255.0f;

		afVoiceLeft[ i ]  = fVoiceGain * sqrtf( 1.0f - fVoicePan );
		afVoiceRight[ i ] = fVoiceGain * sqrtf( fVoicePan );
	}

	double dLoopStart, dLoopEnd;
	GetLoopRange( rInfo, dLoopStart, dLoopEnd );

	const double dLength = rInfo.uiLength;

	for ( TINT i = 0; i < a_iFrames; i++ )
	{
		const TINT iFrame = TINT( a_rChannel.dPosition );

		if ( iFrame >= TINT( dLength ) || iFrame < 0 )
			break;

		const TINT16* pFrame = a_rChannel.pSample ? a_rChannel.pSample->pDecoded + iFrame * iChannels : GetStreamFrame( a_rChannel.pStream, rInfo, iFrame );

		if ( pFrame )
		{
			TFLOAT afFrame[ MAX_VOICES ];
			const TINT iUsed = TMath::Min( iChannels, MAX_VOICES );

			for ( TINT k = 0; k < iUsed; k++ )
				afFrame[ k ] = pFrame[ k ] / 32768.0f;

			// Most sounds are 11 to 22 kHz, without this they come out harsh. A looping sound continues at its start
			TINT iNext = iFrame + 1;

			if ( a_rChannel.uiLoop && iNext >= TINT( dLoopEnd ) )
				iNext = TINT( dLoopStart );

			if ( iNext < TINT( dLength ) )
			{
				const TINT16* pNext  = a_rChannel.pSample ? a_rChannel.pSample->pDecoded + iNext * iChannels : GetStreamFrame( a_rChannel.pStream, rInfo, iNext );
				const TFLOAT  fBlend = TFLOAT( a_rChannel.dPosition - iFrame );

				if ( pNext )
				{
					for ( TINT k = 0; k < iUsed; k++ )
						afFrame[ k ] += ( pNext[ k ] / 32768.0f - afFrame[ k ] ) * fBlend;
				}
			}

			if ( a_rChannel.iNumVoices > 1 )
			{
				for ( TINT k = 0; k < a_rChannel.iNumVoices; k++ )
				{
					a_pOut[ i * 2 + 0 ] += afFrame[ k ] * afVoiceLeft[ k ];
					a_pOut[ i * 2 + 1 ] += afFrame[ k ] * afVoiceRight[ k ];
				}
			}
			else
			{
				a_pOut[ i * 2 + 0 ] += afFrame[ 0 ] * fLeftGain;
				a_pOut[ i * 2 + 1 ] += afFrame[ iChannels > 1 ? 1 : 0 ] * fRightGain;
			}
		}

		if ( a_rChannel.bBackwards )
		{
			a_rChannel.dPosition -= dStep;

			if ( a_rChannel.dPosition < dLoopStart )
			{
				a_rChannel.dPosition  = dLoopStart;
				a_rChannel.bBackwards = TFALSE;
			}
		}
		else
		{
			a_rChannel.dPosition += dStep;

			if ( a_rChannel.uiLoop && a_rChannel.dPosition >= dLoopEnd && dLoopEnd > dLoopStart )
			{
				if ( a_rChannel.uiLoop & FSOUND_LOOP_BIDI )
				{
					a_rChannel.dPosition  = dLoopEnd - 1.0;
					a_rChannel.bBackwards = TTRUE;
				}
				else
				{
					a_rChannel.dPosition = dLoopStart + fmod( a_rChannel.dPosition - dLoopStart, dLoopEnd - dLoopStart );
				}
			}
		}
	}
}

static TBOOL IsChannelPlaying( const Channel& a_rChannel )
{
	if ( !a_rChannel.pSample && !a_rChannel.pStream )
		return TFALSE;

	return a_rChannel.uiLoop != 0 || a_rChannel.dPosition < a_rChannel.pInfo->uiLength;
}

//-----------------------------------------------------------------------------
// Movie sound
//-----------------------------------------------------------------------------

// Holds the sound the movies have ahead of their picture (at most 0.8 s in the game's files) with room to spare
static constexpr TUINT32 MOVIE_RING_FRAMES = 1 << 16;

static TINT16 s_aiMovieRing[ MOVIE_RING_FRAMES * 2 ];

struct MovieVoice
{
	TBOOL   bActive;
	TBOOL   bDraining;
	double  dPosition;
	double  dStep;
	TUINT32 uiWritten;
	TUINT32 uiPosition;
	TINT    iVolume;
	TINT    iSoundOn;
	TINT    iPaused;
};

static MovieVoice s_oMovieVoice = { TFALSE, TFALSE, 0.0, 1.0, 0, 0, 32768, 1, 1 };

TBOOL AMovieVoice_IsMixerRunning()
{
	return s_bMixing;
}

void AMovieVoice_Start( TINT a_iSampleRate )
{
	ScopedAudioLock oLock;

	s_oMovieVoice.bActive   = TTRUE;
	s_oMovieVoice.bDraining = TFALSE;
	s_oMovieVoice.dPosition = 0.0;
	s_oMovieVoice.dStep     = double( a_iSampleRate ) / MIX_RATE;
	s_oMovieVoice.iPaused   = 1;
	s_oMovieVoice.iSoundOn  = 1;
	s_oMovieVoice.iVolume   = 32768;

	__atomic_store_n( &s_oMovieVoice.uiWritten, 0, __ATOMIC_RELEASE );
	__atomic_store_n( &s_oMovieVoice.uiPosition, 0, __ATOMIC_RELEASE );
}

void AMovieVoice_Stop( TBOOL a_bPlayRemaining )
{
	ScopedAudioLock oLock;

	s_oMovieVoice.bDraining = a_bPlayRemaining;

	if ( a_bPlayRemaining )
		s_oMovieVoice.iPaused = 0;
	else
		s_oMovieVoice.bActive = TFALSE;
}

void AMovieVoice_Write( const TFLOAT* a_pSamples, TINT a_iChannels, TINT a_iFrames )
{
	const TUINT32 uiWritten = s_oMovieVoice.uiWritten;
	const TINT    iRight    = a_iChannels > 1 ? 1 : 0;

	for ( TINT i = 0; i < a_iFrames; i++ )
	{
		const TUINT32 uiAt = ( uiWritten + i ) & ( MOVIE_RING_FRAMES - 1 );

		const TFLOAT fLeft  = a_pSamples[ i * a_iChannels ] * 32767.0f;
		const TFLOAT fRight = a_pSamples[ i * a_iChannels + iRight ] * 32767.0f;

		s_aiMovieRing[ uiAt * 2 + 0 ] = TINT16( fLeft > 32767.0f ? 32767 : ( fLeft < -32768.0f ? -32768 : TINT( fLeft ) ) );
		s_aiMovieRing[ uiAt * 2 + 1 ] = TINT16( fRight > 32767.0f ? 32767 : ( fRight < -32768.0f ? -32768 : TINT( fRight ) ) );
	}

	__atomic_store_n( &s_oMovieVoice.uiWritten, uiWritten + a_iFrames, __ATOMIC_RELEASE );
}

void AMovieVoice_SetPaused( TBOOL a_bPaused )
{
	__atomic_store_n( &s_oMovieVoice.iPaused, a_bPaused ? 1 : 0, __ATOMIC_RELAXED );
}

void AMovieVoice_SetVolume( TINT a_iVolume )
{
	__atomic_store_n( &s_oMovieVoice.iVolume, a_iVolume, __ATOMIC_RELAXED );
}

void AMovieVoice_SetSoundOn( TBOOL a_bOn )
{
	__atomic_store_n( &s_oMovieVoice.iSoundOn, a_bOn ? 1 : 0, __ATOMIC_RELAXED );
}

TUINT32 AMovieVoice_GetPosition()
{
	return __atomic_load_n( &s_oMovieVoice.uiPosition, __ATOMIC_ACQUIRE );
}

// Called with the audio lock held
static void MixMovieVoice( TFLOAT* a_pOut, TINT a_iFrames )
{
	if ( !s_oMovieVoice.bActive || __atomic_load_n( &s_oMovieVoice.iPaused, __ATOMIC_RELAXED ) )
		return;

	const TUINT32 uiWritten = __atomic_load_n( &s_oMovieVoice.uiWritten, __ATOMIC_ACQUIRE );
	const TBOOL   bSoundOn  = __atomic_load_n( &s_oMovieVoice.iSoundOn, __ATOMIC_RELAXED ) != 0;
	const TFLOAT  fGain     = bSoundOn ? __atomic_load_n( &s_oMovieVoice.iVolume, __ATOMIC_RELAXED ) / ( 32768.0f * 32768.0f ) : 0.0f;
	const double  dStep     = s_oMovieVoice.dStep;
	double        dPosition = s_oMovieVoice.dPosition;

	for ( TINT i = 0; i < a_iFrames; i++ )
	{
		const TUINT32 uiFrame = TUINT32( dPosition );

		if ( uiFrame < uiWritten )
		{
			const TUINT32 uiNext = uiFrame + 1 < uiWritten ? uiFrame + 1 : uiFrame;
			const TINT16* pA     = s_aiMovieRing + ( uiFrame & ( MOVIE_RING_FRAMES - 1 ) ) * 2;
			const TINT16* pB     = s_aiMovieRing + ( uiNext & ( MOVIE_RING_FRAMES - 1 ) ) * 2;
			const TFLOAT  fT     = TFLOAT( dPosition - uiFrame );

			a_pOut[ i * 2 + 0 ] += ( pA[ 0 ] + ( pB[ 0 ] - pA[ 0 ] ) * fT ) * fGain;
			a_pOut[ i * 2 + 1 ] += ( pA[ 1 ] + ( pB[ 1 ] - pA[ 1 ] ) * fT ) * fGain;
		}

		dPosition += dStep;
	}

	s_oMovieVoice.dPosition = dPosition;
	__atomic_store_n( &s_oMovieVoice.uiPosition, TUINT32( dPosition ), __ATOMIC_RELEASE );

	if ( s_oMovieVoice.bDraining && TUINT32( dPosition ) >= uiWritten )
		s_oMovieVoice.bActive = TFALSE;
}

static void* MixThread( void* )
{
	double dLast = emscripten_get_now();
	double dDebt = 0.0;

	for ( ;; )
	{
		const double dNow = emscripten_get_now();
		dDebt += ( dNow - dLast ) * MIX_RATE / 1000.0;
		dLast = dNow;

		// The sounds finish on time even if nothing is listening
		while ( dDebt >= MIX_CHUNK )
		{
			dDebt -= MIX_CHUNK;

			TFLOAT afMix[ MIX_CHUNK * 2 ];
			TUtil::MemClear( afMix, sizeof( afMix ) );

			pthread_mutex_lock( &s_oMutex );

			for ( Channel& rChannel : s_aChannels )
			{
				if ( !rChannel.bPaused && IsChannelPlaying( rChannel ) )
					MixChannel( rChannel, afMix, MIX_CHUNK );
			}

			MixMovieVoice( afMix, MIX_CHUNK );

			pthread_mutex_unlock( &s_oMutex );

			TINT iWrite = __atomic_load_n( &s_iRingWrite, __ATOMIC_RELAXED );

			for ( TINT i = 0; i < MIX_CHUNK; i++ )
			{
				const TINT iAt = ( iWrite + i ) & ( RING_FRAMES - 1 );

				s_afRing[ iAt * 2 + 0 ] = TMath::Max( TMath::Min( afMix[ i * 2 + 0 ], 1.0f ), -1.0f );
				s_afRing[ iAt * 2 + 1 ] = TMath::Max( TMath::Min( afMix[ i * 2 + 1 ], 1.0f ), -1.0f );
			}

			__atomic_store_n( &s_iRingWrite, ( iWrite + MIX_CHUNK ) & 0x7FFFFFFF, __ATOMIC_RELEASE );

			// Too much catching up means the page was away
			if ( dDebt > RING_FRAMES )
				dDebt = 0.0;
		}

		usleep( 3000 );
	}

	return TNULL;
}

//-----------------------------------------------------------------------------
// Channels
//-----------------------------------------------------------------------------

static TINT MakeHandle( const Channel& a_rChannel )
{
	return TINT( &a_rChannel - s_aChannels ) | ( a_rChannel.iGeneration << 12 );
}

// Same as FMOD: a handle is the whole channel, a plain number from FSOUND_GetSubChannel is one voice of it
static Channel* GetChannel( TINT a_iHandle, TINT* a_piOutVoice = TNULL )
{
	if ( a_piOutVoice )
		*a_piOutVoice = -1;

	if ( a_iHandle < 0 )
		return TNULL;

	TINT       iSlot       = a_iHandle & 0xFFF;
	const TINT iGeneration = a_iHandle >> 12;

	if ( iGeneration == 0 )
	{
		TINT iVoice = 0;

		if ( iSlot >= MAX_CHANNELS )
		{
			iVoice = ( iSlot - MAX_CHANNELS ) % MAX_VOICES;
			iSlot  = ( iSlot - MAX_CHANNELS ) / MAX_VOICES;
		}

		if ( iSlot >= MAX_CHANNELS )
			return TNULL;

		if ( a_piOutVoice )
			*a_piOutVoice = iVoice;

		return &s_aChannels[ iSlot ];
	}

	if ( iSlot >= MAX_CHANNELS || s_aChannels[ iSlot ].iGeneration != iGeneration )
		return TNULL;

	return &s_aChannels[ iSlot ];
}

template <class Func>
static signed char ForChannels( TINT a_iHandle, Func a_fnFunc )
{
	ScopedAudioLock oLock;

	if ( a_iHandle == FSOUND_ALL )
	{
		for ( Channel& rChannel : s_aChannels )
		{
			if ( rChannel.pSample || rChannel.pStream )
				a_fnFunc( rChannel );
		}

		return 1;
	}

	Channel* pChannel = GetChannel( a_iHandle );

	if ( !pChannel )
		return 0;

	a_fnFunc( *pChannel );
	return 1;
}

static void SetVoiceValue( Channel& a_rChannel, TINT a_iVoice, TINT a_iValue, TINT( Channel::*a_pValues )[ MAX_VOICES ] )
{
	if ( a_iVoice >= 0 )
	{
		if ( a_iVoice < a_rChannel.iNumVoices )
			( a_rChannel.*a_pValues )[ a_iVoice ] = a_iValue;

		return;
	}

	for ( TINT i = 0; i < a_rChannel.iNumVoices; i++ )
		( a_rChannel.*a_pValues )[ i ] = a_iValue;
}

static signed char SetVoiceValues( TINT a_iHandle, TINT a_iValue, TINT( Channel::*a_pValues )[ MAX_VOICES ] )
{
	ScopedAudioLock oLock;

	if ( a_iHandle == FSOUND_ALL )
	{
		for ( Channel& rChannel : s_aChannels )
			SetVoiceValue( rChannel, -1, a_iValue, a_pValues );

		return 1;
	}

	TINT     iVoice;
	Channel* pChannel = GetChannel( a_iHandle, &iVoice );

	if ( !pChannel )
		return 0;

	SetVoiceValue( *pChannel, iVoice, a_iValue, a_pValues );
	return 1;
}

static void SetupVoices( Channel& a_rChannel, const SampleInfo& a_rInfo, TINT a_iVolume )
{
	a_rChannel.iNumVoices = IsMultichannel( a_rInfo ) ? TMath::Min( a_rInfo.iChannels, MAX_VOICES ) : 1;

	for ( TINT i = 0; i < a_rChannel.iNumVoices; i++ )
	{
		a_rChannel.aiVolumes[ i ] = a_iVolume;
		a_rChannel.aiPans[ i ]    = a_rChannel.iNumVoices > 1 ? ( ( i & 1 ) ? 255 : 0 ) : a_rInfo.iPan;
	}
}

static Channel* StartChannel( TINT a_iWanted )
{
	Channel* pChannel = TNULL;

	if ( a_iWanted < 0 )
	{
		for ( Channel& rChannel : s_aChannels )
		{
			if ( !rChannel.bReserved && !IsChannelPlaying( rChannel ) )
			{
				pChannel = &rChannel;
				break;
			}
		}
	}
	else
	{
		pChannel = &s_aChannels[ a_iWanted & 0xFFF ];
	}

	if ( !pChannel )
		return TNULL;

	const TBOOL bReserved = pChannel->bReserved;
	TUtil::MemClear( pChannel, sizeof( Channel ) );

	pChannel->bReserved    = bReserved;
	pChannel->iGeneration  = ( s_iNextGeneration++ & 0x7FFFF ) | 1;
	pChannel->fMinDistance = 1.0f;
	pChannel->fMaxDistance = 1000000000.0f;

	return pChannel;
}

//-----------------------------------------------------------------------------
// FSOUND
//-----------------------------------------------------------------------------

extern "C"
{
	signed char F_API FSOUND_Init( int mixrate, int maxsoftwarechannels, unsigned int flags )
	{
		if ( !s_bMixing )
		{
			s_bMixing = TTRUE;
			pthread_create( &s_oMixThread, TNULL, MixThread, TNULL );
		}

		return 1;
	}

	void F_API FSOUND_Close()
	{
		ScopedAudioLock oLock;

		for ( Channel& rChannel : s_aChannels )
		{
			rChannel.pSample = TNULL;
			rChannel.pStream = TNULL;
		}
	}

	void F_API FSOUND_Update() {}
	signed char F_API FSOUND_SetOutput( int outputtype ) { return 1; }
	signed char F_API FSOUND_SetBufferSize( int len_ms ) { return 1; }
	signed char F_API FSOUND_SetMinHardwareChannels( int min ) { return 1; }
	int F_API FSOUND_GetMaxChannels() { return MAX_CHANNELS; }
	int F_API FSOUND_GetOutput() { return FSOUND_OUTPUT_DSOUND; }
	void* F_API FSOUND_GetOutputHandle() { return s_aOutputHandle; }

	int F_API FSOUND_GetNumSubChannels( int channel )
	{
		ScopedAudioLock oLock;

		Channel* pChannel = GetChannel( channel );
		return pChannel ? pChannel->iNumVoices : 0;
	}

	int F_API FSOUND_GetSubChannel( int channel, int subchannel )
	{
		ScopedAudioLock oLock;

		Channel* pChannel = GetChannel( channel );

		if ( !pChannel || subchannel < 0 || subchannel >= pChannel->iNumVoices )
			return -1;

		const TINT iSlot = TINT( pChannel - s_aChannels );
		return subchannel == 0 ? iSlot : MAX_CHANNELS + iSlot * MAX_VOICES + subchannel;
	}

	signed char F_API FSOUND_GetNumHWChannels( int* num2d, int* num3d, int* total )
	{
		if ( num2d ) *num2d = 0;
		if ( num3d ) *num3d = 0;
		if ( total ) *total = 0;
		return 1;
	}

	void F_API FSOUND_GetMemoryStats( unsigned int* currentalloced, unsigned int* maxalloced )
	{
		if ( currentalloced ) *currentalloced = 1 << 20;
		if ( maxalloced ) *maxalloced = 1 << 20;
	}

	void F_API FSOUND_3D_SetDistanceFactor( float scale ) {}
	void F_API FSOUND_3D_SetDopplerFactor( float scale ) {}
	void F_API FSOUND_3D_SetRolloffFactor( float scale ) {}

	void F_API FSOUND_3D_Listener_SetAttributes( const float* pos, const float* vel, float fx, float fy, float fz, float tx, float ty, float tz )
	{
		ScopedAudioLock oLock;

		if ( pos )
			TUtil::MemCopy( s_afListener, pos, sizeof( s_afListener ) );

		// Up x forward, FMOD's space is left handed by default
		s_afRight[ 0 ] = ty * fz - tz * fy;
		s_afRight[ 1 ] = tz * fx - tx * fz;
		s_afRight[ 2 ] = tx * fy - ty * fx;
	}

	signed char F_API FSOUND_3D_SetAttributes( int channel, const float* pos, const float* vel )
	{
		return ForChannels( channel, [ & ]( Channel& a_rChannel ) {
			if ( pos ) TUtil::MemCopy( a_rChannel.afPosition, pos, sizeof( a_rChannel.afPosition ) );
		} );
	}

	signed char F_API FSOUND_3D_SetMinMaxDistance( int channel, float min, float max )
	{
		return ForChannels( channel, [ & ]( Channel& a_rChannel ) {
			a_rChannel.fMinDistance = min;
			a_rChannel.fMaxDistance = max;
		} );
	}

	int F_API FSOUND_PlaySoundEx( int channel, FSOUND_SAMPLE* sptr, FSOUND_DSPUNIT* dsp, signed char startpaused )
	{
		if ( !sptr )
			return -1;

		ScopedAudioLock oLock;

		Channel* pChannel = StartChannel( channel );

		if ( !pChannel )
			return -1;

		const SampleInfo& rInfo = sptr->oInfo;

		pChannel->pSample      = sptr;
		pChannel->pInfo        = &rInfo;
		pChannel->uiLoop       = sptr->uiMode & ( FSOUND_LOOP_NORMAL | FSOUND_LOOP_BIDI );
		pChannel->iFrequency   = rInfo.iFrequency;
		pChannel->iPriority    = rInfo.iPriority;
		pChannel->b3D          = ( sptr->uiMode & FSOUND_HW3D ) != 0;
		pChannel->fMinDistance = rInfo.fMinDistance;
		pChannel->fMaxDistance = rInfo.fMaxDistance;
		pChannel->bPaused      = startpaused != 0;

		SetupVoices( *pChannel, rInfo, rInfo.iVolume );

		// The world starts many copies of an ambient loop at once. Sound cards start them a little apart, here they
		// would add up in phase and sound far too loud
		if ( pChannel->uiLoop && rInfo.uiLength > 0 )
		{
			const TUINT32 uiSeed = TUINT32( pChannel->iGeneration ) * 2654435761u;
			pChannel->dPosition  = ( uiSeed >> 8 ) % rInfo.uiLength;
		}

		return MakeHandle( *pChannel );
	}

	signed char F_API FSOUND_StopSound( int channel )
	{
		return ForChannels( channel, []( Channel& a_rChannel ) {
			a_rChannel.pSample = TNULL;
			a_rChannel.pStream = TNULL;
		} );
	}

	signed char F_API FSOUND_SetPaused( int channel, signed char paused )
	{
		return ForChannels( channel, [ & ]( Channel& a_rChannel ) { a_rChannel.bPaused = paused != 0; } );
	}

	signed char F_API FSOUND_SetVolume( int channel, int vol )
	{
		return SetVoiceValues( channel, TMath::Max( TMath::Min( vol, 255 ), 0 ), &Channel::aiVolumes );
	}

	signed char F_API FSOUND_SetPan( int channel, int pan )
	{
		return SetVoiceValues( channel, pan == -1 ? -1 : TMath::Max( TMath::Min( pan, 255 ), 0 ), &Channel::aiPans );
	}

	signed char F_API FSOUND_SetFrequency( int channel, int freq )
	{
		return ForChannels( channel, [ & ]( Channel& a_rChannel ) { a_rChannel.iFrequency = freq; } );
	}

	signed char F_API FSOUND_SetPriority( int channel, int priority )
	{
		return ForChannels( channel, [ & ]( Channel& a_rChannel ) { a_rChannel.iPriority = priority; } );
	}

	signed char F_API FSOUND_SetReserved( int channel, signed char reserved )
	{
		ScopedAudioLock oLock;

		TINT     iVoice;
		Channel* pChannel = GetChannel( channel, &iVoice );

		if ( !pChannel )
			return 0;

		// The other voices don't take a channel of their own here
		if ( iVoice <= 0 )
			pChannel->bReserved = reserved != 0;

		return 1;
	}

	signed char F_API FSOUND_GetReserved( int channel )
	{
		ScopedAudioLock oLock;

		Channel* pChannel = GetChannel( channel );
		return pChannel && pChannel->bReserved ? 1 : 0;
	}

	signed char F_API FSOUND_IsPlaying( int channel )
	{
		ScopedAudioLock oLock;

		Channel* pChannel = GetChannel( channel );
		return pChannel && IsChannelPlaying( *pChannel ) ? 1 : 0;
	}

	FSOUND_SAMPLE* F_API FSOUND_GetCurrentSample( int channel )
	{
		ScopedAudioLock oLock;

		Channel* pChannel = GetChannel( channel );

		if ( !pChannel )
			return TNULL;

		if ( pChannel->pStream )
		{
			FSOUND_SAMPLE& rSample = pChannel->pStream->oCurrentSample;

			rSample.oInfo  = *pChannel->pInfo;
			rSample.uiMode = pChannel->pInfo->uiMode;

			return &rSample;
		}

		return pChannel->pSample;
	}

	signed char F_API FSOUND_Sample_SetMode( FSOUND_SAMPLE* sptr, unsigned int mode )
	{
		if ( !sptr )
			return 0;

		sptr->uiMode = ( sptr->uiMode & ~7u ) | ( mode & 7u );
		return 1;
	}

	unsigned int F_API FSOUND_Sample_GetMode( FSOUND_SAMPLE* sptr )
	{
		return sptr ? sptr->uiMode : 0;
	}

	const char* F_API FSOUND_Sample_GetName( FSOUND_SAMPLE* sptr )
	{
		return sptr ? sptr->oInfo.szName : TNULL;
	}

	signed char F_API FSOUND_Sample_GetDefaults( FSOUND_SAMPLE* sptr, int* deffreq, int* defvol, int* defpan, int* defpri )
	{
		if ( !sptr )
			return 0;

		if ( deffreq ) *deffreq = sptr->oInfo.iFrequency;
		if ( defvol ) *defvol = sptr->oInfo.iVolume;
		if ( defpan ) *defpan = sptr->oInfo.iPan;
		if ( defpri ) *defpri = sptr->oInfo.iPriority;
		return 1;
	}

	FMUSIC_MODULE* F_API FMUSIC_LoadSongEx( const char* name_or_data, int offset, int length, unsigned int mode, const int* samplelist, int samplelistnum )
	{
		TUINT8* pBytes = TNULL;
		TUINT32 uiSize = 0;

		if ( mode & FSOUND_LOADMEMORY )
		{
			uiSize = length;
			pBytes = TSTATICCAST( TUINT8, TMalloc( uiSize ) );
			TUtil::MemCopy( pBytes, name_or_data, uiSize );
		}
		else
		{
			TFile* pFile = TFile::Create( name_or_data );

			if ( !pFile )
				return TNULL;

			uiSize = length ? length : pFile->GetSize() - offset;
			pBytes = TSTATICCAST( TUINT8, TMalloc( uiSize ) );

			pFile->Seek( offset, TSEEK_SET );
			pFile->Read( pBytes, uiSize );
			pFile->Destroy();
		}

		SampleInfo* pInfos  = TNULL;
		const TINT  iNumber = ParseFSB( pBytes, uiSize, &pInfos );

		if ( iNumber == 0 )
		{
			TFree( pBytes );
			return TNULL;
		}

		FMUSIC_MODULE* pSong = new FMUSIC_MODULE;
		pSong->pBytes        = pBytes;
		pSong->iNumSamples   = iNumber;
		pSong->pSamples      = new FSOUND_SAMPLE[ iNumber ];

		for ( TINT i = 0; i < iNumber; i++ )
		{
			FSOUND_SAMPLE& rSample = pSong->pSamples[ i ];

			rSample.oInfo    = pInfos[ i ];
			rSample.uiMode   = pInfos[ i ].uiMode;
			rSample.pOwner   = pSong;
			rSample.pData    = pBytes + pInfos[ i ].uiOffset;
			rSample.pDecoded = TNULL;
		}

		TFree( pInfos );
		return pSong;
	}

	signed char F_API FMUSIC_FreeSong( FMUSIC_MODULE* mod )
	{
		if ( !mod )
			return 0;

		ScopedAudioLock oLock;

		for ( Channel& rChannel : s_aChannels )
		{
			if ( rChannel.pSample && rChannel.pSample->pOwner == mod )
				rChannel.pSample = TNULL;
		}

		for ( TINT i = 0; i < mod->iNumSamples; i++ )
		{
			if ( mod->pSamples[ i ].pDecoded )
				TFree( mod->pSamples[ i ].pDecoded );
		}

		delete[] mod->pSamples;
		TFree( mod->pBytes );
		delete mod;

		return 1;
	}

	int F_API FMUSIC_GetOpenState( FMUSIC_MODULE* mod ) { return mod ? 0 : -1; }
	int F_API FMUSIC_GetNumSamples( FMUSIC_MODULE* mod ) { return mod ? mod->iNumSamples : 0; }

	FSOUND_SAMPLE* F_API FMUSIC_GetSample( FMUSIC_MODULE* mod, int sampno )
	{
		return mod && sampno >= 0 && sampno < mod->iNumSamples ? &mod->pSamples[ sampno ] : TNULL;
	}

	FSOUND_STREAM* F_API FSOUND_Stream_Open( const char* name_or_data, unsigned int mode, int offset, int length )
	{
		FSOUND_STREAM* pStream = new FSOUND_STREAM;
		TUtil::MemClear( pStream, sizeof( FSOUND_STREAM ) );

		pStream->uiMode       = mode;
		pStream->uiBase       = offset;
		pStream->iCachedBlock = -1;
		pStream->iChannel     = -1;

		TUINT8* pHeader = TNULL;
		TUINT32 uiSize  = 0;

		if ( mode & FSOUND_LOADMEMORY )
		{
			uiSize           = length;
			pStream->pMemory = TSTATICCAST( TUINT8, TMalloc( uiSize ) );
			TUtil::MemCopy( pStream->pMemory, name_or_data, uiSize );
			pHeader = pStream->pMemory;
		}
		else
		{
			T2String8::Copy( pStream->szPath, name_or_data, sizeof( pStream->szPath ) - 1 );

			TFile* pFile = TFile::Create( name_or_data );

			if ( !pFile )
			{
				delete pStream;
				return TNULL;
			}

			uiSize  = TMath::Min( TUINT32( length ? length : pFile->GetSize() - offset ), TUINT32( 1024 * 1024 ) );
			pHeader = TSTATICCAST( TUINT8, TMalloc( uiSize ) );

			pFile->Seek( offset, TSEEK_SET );
			pFile->Read( pHeader, uiSize );
			pFile->Destroy();
		}

		pStream->iNumSubs = ParseFSB( pHeader, uiSize, &pStream->pSubs );

		if ( !pStream->pMemory )
			TFree( pHeader );

		if ( pStream->iNumSubs == 0 )
		{
			delete pStream;
			return TNULL;
		}

		return pStream;
	}

	static void StopStreamChannel( FSOUND_STREAM* a_pStream )
	{
		if ( a_pStream->iChannel >= 0 )
		{
			Channel& rChannel = s_aChannels[ a_pStream->iChannel ];

			if ( rChannel.pStream == a_pStream )
				rChannel.pStream = TNULL;

			a_pStream->iChannel = -1;
		}
	}

	static Channel* GetStreamChannel( FSOUND_STREAM* a_pStream )
	{
		if ( a_pStream->iChannel >= 0 && s_aChannels[ a_pStream->iChannel ].pStream == a_pStream )
			return &s_aChannels[ a_pStream->iChannel ];

		return TNULL;
	}

	int F_API FSOUND_Stream_PlayEx( int channel, FSOUND_STREAM* stream, FSOUND_DSPUNIT* dsp, signed char startpaused )
	{
		if ( !stream )
			return -1;

		ScopedAudioLock oLock;

		StopStreamChannel( stream );

		Channel* pChannel = StartChannel( channel );

		if ( !pChannel )
			return -1;

		const SampleInfo& rInfo = stream->pSubs[ stream->iSub ];

		pChannel->pStream      = stream;
		pChannel->pInfo        = &rInfo;
		pChannel->uiLoop       = stream->uiMode & ( FSOUND_LOOP_NORMAL | FSOUND_LOOP_BIDI );
		pChannel->iFrequency   = rInfo.iFrequency;
		pChannel->b3D          = ( stream->uiMode & FSOUND_HW3D ) != 0;
		pChannel->fMinDistance = rInfo.fMinDistance;
		pChannel->fMaxDistance = rInfo.fMaxDistance;
		pChannel->bPaused      = startpaused != 0;
		pChannel->dPosition    = stream->uiTimeMs * double( rInfo.iFrequency ) / 1000.0;

		SetupVoices( *pChannel, rInfo, 255 );

		stream->iChannel = TINT( pChannel - s_aChannels );
		return MakeHandle( *pChannel );
	}

	signed char F_API FSOUND_Stream_Stop( FSOUND_STREAM* stream )
	{
		if ( !stream )
			return 0;

		ScopedAudioLock oLock;

		StopStreamChannel( stream );
		stream->uiTimeMs = 0;

		return 1;
	}

	signed char F_API FSOUND_Stream_Close( FSOUND_STREAM* stream )
	{
		if ( !stream )
			return 0;

		ScopedAudioLock oLock;

		StopStreamChannel( stream );

		if ( stream->pFile )
			stream->pFile->Destroy();

		if ( stream->pMemory )
			TFree( stream->pMemory );

		TFree( stream->pSubs );
		delete stream;

		return 1;
	}

	signed char F_API FSOUND_Stream_SetMode( FSOUND_STREAM* stream, unsigned int mode )
	{
		if ( !stream )
			return 0;

		ScopedAudioLock oLock;

		stream->uiMode = mode;

		// The game sets it after playing the stream paused
		if ( Channel* pChannel = GetStreamChannel( stream ) )
		{
			pChannel->uiLoop = mode & ( FSOUND_LOOP_NORMAL | FSOUND_LOOP_BIDI );
			pChannel->b3D    = ( mode & FSOUND_HW3D ) != 0;
		}

		return 1;
	}

	unsigned int F_API FSOUND_Stream_GetMode( FSOUND_STREAM* stream ) { return stream ? stream->uiMode : 0; }
	signed char F_API FSOUND_Stream_SetBufferSize( int ms ) { return 1; }
	int F_API FSOUND_Stream_GetOpenState( FSOUND_STREAM* stream ) { return stream ? 0 : -3; }
	int F_API FSOUND_Stream_GetNumSubStreams( FSOUND_STREAM* stream ) { return stream ? stream->iNumSubs : 0; }

	int F_API FSOUND_Stream_GetLengthMs( FSOUND_STREAM* stream )
	{
		if ( !stream )
			return 0;

		const SampleInfo& rInfo = stream->pSubs[ stream->iSub ];
		return TINT( 1000.0 * rInfo.uiLength / rInfo.iFrequency );
	}

	int F_API FSOUND_Stream_GetTime( FSOUND_STREAM* stream )
	{
		if ( !stream )
			return 0;

		ScopedAudioLock oLock;

		if ( Channel* pChannel = GetStreamChannel( stream ) )
			return TINT( TMath::Min( pChannel->dPosition, double( pChannel->pInfo->uiLength ) ) * 1000.0 / pChannel->pInfo->iFrequency );

		return stream->uiTimeMs;
	}

	signed char F_API FSOUND_Stream_SetTime( FSOUND_STREAM* stream, int ms )
	{
		if ( !stream )
			return 0;

		ScopedAudioLock oLock;

		stream->uiTimeMs = ms;

		if ( Channel* pChannel = GetStreamChannel( stream ) )
			pChannel->dPosition = ms * double( pChannel->pInfo->iFrequency ) / 1000.0;

		return 1;
	}

	signed char F_API FSOUND_Stream_SetSubStream( FSOUND_STREAM* stream, int index )
	{
		if ( !stream || index < 0 || index >= stream->iNumSubs )
			return 0;

		ScopedAudioLock oLock;

		stream->iSub         = index;
		stream->uiTimeMs     = 0;
		stream->iCachedBlock = -1;

		return 1;
	}
}
