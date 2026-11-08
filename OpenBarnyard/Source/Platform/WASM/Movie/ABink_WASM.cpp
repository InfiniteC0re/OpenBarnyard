#include "pch.h"

#include "ABinkAudio_WASM.h"
#include "ABinkVideo_WASM.h"
#include "AMovieVoice_WASM.h"

#include <File/TFile.h>

#include <bink.h>
#include <emscripten.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Frames are read and decoded on the thread that calls the API, the sound of a frame with its picture. It takes about
// a millisecond of audio work per second of movie, and every file has over 0.7 s of sound ahead of the picture, so a
// slow download doesn't reach the speakers.

static constexpr TUINT32 HEADER_SIZE   = 44;
static constexpr TUINT32 MAX_TRACKS    = 256;
static constexpr TUINT32 MAX_FRAMES    = 1000000;
static constexpr TUINT32 READ_PADDING  = 16;
static constexpr TINT    MAX_CATCH_UP  = 8;
static constexpr TINT    NO_SOUND_RATE = 44100;

struct ABink : public BINK
{
	TFile*     pFile;
	TUINT32*   pOffsets;
	TUINT8*    pFrame;
	void*      pMemory;
	ABinkVideo oVideo;
	ABinkAudio oAudio;
	TBOOL      bHasAudio;
	TUINT32    uiFlags;
	TUINT32    uiNumTracks;
	TUINT32    uiFilePosition;
	TUINT32    uiDecoded; // frame number of the planes
	TINT       iClockRate;
	TBOOL      bStarted;
	TBOOL      bLooping;
	TBOOL      bPaused;
	TDOUBLE    dWallStart; // the wall clock is only for when there's no mixer
	TDOUBLE    dWallPaused;
};

static BINKMEMALLOC s_fnAlloc;
static BINKMEMFREE  s_fnFree;

void BinkSetMemory( BINKMEMALLOC a_fnAlloc, BINKMEMFREE a_fnFree )
{
	s_fnAlloc = a_fnAlloc;
	s_fnFree  = a_fnFree;
}

static void* BinkAllocate( TUINT32 a_uiSize )
{
	return s_fnAlloc ? s_fnAlloc( a_uiSize ) : TMalloc( a_uiSize );
}

static void BinkDeallocate( void* a_pMemory )
{
	if ( s_fnFree )
		s_fnFree( a_pMemory );
	else
		TFree( a_pMemory );
}

static TUINT32 ReadHeader32( const TUINT8* a_pData )
{
	TUINT32 uiValue;
	TUtil::MemCopy( &uiValue, a_pData, 4 );
	return uiValue;
}

static void WriteMovieSound( const TFLOAT* a_pSamples, TINT a_iChannels, TINT a_iFrames, void* a_pUser )
{
	AMovieVoice_Write( a_pSamples, a_iChannels, a_iFrames );
}

HBINK BinkOpen( const char* a_szName, U32 a_uiFlags )
{
	TFile* pFile = TFile::Create( a_szName );

	if ( !pFile )
		return TNULL;

	TUINT8 aHeader[ HEADER_SIZE + 4 ];

	if ( pFile->Read( aHeader, sizeof( aHeader ) ) != sizeof( aHeader ) || aHeader[ 0 ] != 'B' || aHeader[ 1 ] != 'I' || aHeader[ 2 ] != 'K' )
	{
		pFile->Destroy();
		return TNULL;
	}

	const TCHAR   cRevision   = TCHAR( aHeader[ 3 ] );
	const TUINT32 uiFileSize  = ReadHeader32( aHeader + 4 ) + 8;
	const TUINT32 uiFrames    = ReadHeader32( aHeader + 8 );
	const TUINT32 uiWidth     = ReadHeader32( aHeader + 20 );
	const TUINT32 uiHeight    = ReadHeader32( aHeader + 24 );
	const TUINT32 uiRate      = ReadHeader32( aHeader + 28 );
	const TUINT32 uiRateDiv   = ReadHeader32( aHeader + 32 );
	const TUINT32 uiFlags     = ReadHeader32( aHeader + 36 );
	const TUINT32 uiNumTracks = ReadHeader32( aHeader + 40 );

	// Revision b, the oldest, has a different block layer
	if ( cRevision < 'c' || cRevision > 'k' || uiNumTracks > MAX_TRACKS || uiFrames == 0 || uiFrames > MAX_FRAMES || !uiWidth || !uiHeight || !uiRate )
	{
		TERROR( "BinkOpen: %s isn't a supported Bink file\n", a_szName );
		pFile->Destroy();
		return TNULL;
	}

	// Maximum decoded sizes, sample rates and flags, ids, then the frame offsets
	TUINT32 uiPosition = HEADER_SIZE + ( cRevision == 'k' ? 4 : 0 ) + uiNumTracks * 4;
	TUINT32 uiTrack    = 0;

	if ( uiNumTracks > 0 )
	{
		pFile->Seek( uiPosition, TSEEK_SET );
		pFile->Read( &uiTrack, 4 );
	}

	uiPosition += uiNumTracks * 8;

	const TINT    iSampleRate = TINT( uiTrack & 0xFFFF );
	const TUINT32 uiAudio     = uiTrack >> 16;

	const TUINT32 uiOffsetsSize = ( uiFrames + 1 ) * 4;
	TUINT32*      pOffsets      = TSTATICCAST( TUINT32, BinkAllocate( uiOffsetsSize ) );

	pFile->Seek( uiPosition, TSEEK_SET );

	if ( pFile->Read( pOffsets, uiOffsetsSize ) != uiOffsetsSize )
	{
		BinkDeallocate( pOffsets );
		pFile->Destroy();
		return TNULL;
	}

	const TUINT32 uiRealFileSize = TUINT32( pFile->GetSize() );
	TUINT32       uiLargest      = 0;

	pOffsets[ uiFrames ] = TMath::Min( uiFileSize, uiRealFileSize );

	for ( TUINT32 i = 0; i < uiFrames; i++ )
	{
		pOffsets[ i ] &= ~1u; // bit 0 is keyframe

		const TUINT32 uiNext = i + 1 < uiFrames ? pOffsets[ i + 1 ] & ~1u : pOffsets[ uiFrames ];

		if ( uiNext <= pOffsets[ i ] )
		{
			TERROR( "BinkOpen: %s has a broken frame table\n", a_szName );
			BinkDeallocate( pOffsets );
			pFile->Destroy();
			return TNULL;
		}

		uiLargest = TMath::Max( uiLargest, uiNext - pOffsets[ i ] );
	}

	const TBOOL   bHasAudio  = uiNumTracks > 0 && iSampleRate > 0;
	const TUINT32 uiFrameMem = uiLargest + READ_PADDING;
	const TUINT32 uiVideoMem = ( ABinkVideo::GetMemorySize( uiWidth, uiHeight, uiFlags ) + 15 ) & ~15u;
	const TUINT32 uiAudioMem = bHasAudio ? ABinkAudio::GetMemorySize( iSampleRate, uiAudio ) : 0;

	TUINT8* pMemory = TSTATICCAST( TUINT8, BinkAllocate( uiVideoMem + uiAudioMem + uiFrameMem ) );
	ABink*  pBink   = TSTATICCAST( ABink, BinkAllocate( sizeof( ABink ) ) );

	TUtil::MemClear( pBink, sizeof( ABink ) );

	pBink->Width        = uiWidth;
	pBink->Height       = uiHeight;
	pBink->Frames       = uiFrames;
	pBink->FrameNum     = 1;
	pBink->LastFrameNum = 0;
	pBink->FrameRate    = uiRate;
	pBink->FrameRateDiv = uiRateDiv ? uiRateDiv : 1;
	pBink->ReadError    = 0;

	pBink->pFile          = pFile;
	pBink->pOffsets       = pOffsets;
	pBink->pMemory        = pMemory;
	pBink->bHasAudio      = bHasAudio;
	pBink->uiFlags        = a_uiFlags;
	pBink->uiNumTracks    = uiNumTracks;
	pBink->uiFilePosition = TUINT32( -1 );
	pBink->iClockRate     = bHasAudio ? iSampleRate : NO_SOUND_RATE;

	pBink->oVideo.Create( pMemory, uiWidth, uiHeight, cRevision, uiFlags );

	if ( bHasAudio )
		pBink->oAudio.Create( pMemory + uiVideoMem, iSampleRate, uiAudio );

	pBink->pFrame = pMemory + uiVideoMem + uiAudioMem;
	TUtil::MemClear( pBink->pFrame, uiFrameMem );

	AMovieVoice_Start( pBink->iClockRate );

	return pBink;
}

void BinkClose( HBINK a_hBink )
{
	if ( !a_hBink )
		return;

	ABink* pBink = TSTATICCAST( ABink, a_hBink );

	// The sound of a movie that ran to its end goes on for a moment, a skipped one stops
	AMovieVoice_Stop( pBink->LastFrameNum == pBink->Frames );

	pBink->pFile->Destroy();

	BinkDeallocate( pBink->pOffsets );
	BinkDeallocate( pBink->pMemory );
	BinkDeallocate( pBink );
}

static TUINT32 GetClock( const ABink* a_pBink )
{
	if ( AMovieVoice_IsMixerRunning() )
		return AMovieVoice_GetPosition();

	const TDOUBLE dNow = a_pBink->bPaused ? a_pBink->dWallPaused : emscripten_get_now();
	return TUINT32( ( dNow - a_pBink->dWallStart ) * a_pBink->iClockRate / 1000.0 );
}

// In frames of the clock, when a frame's picture has to be on screen
static TUINT32 GetFrameTime( const ABink* a_pBink, TUINT32 a_uiFrame )
{
	return TUINT32( TUINT64( a_uiFrame - 1 ) * a_pBink->FrameRateDiv * a_pBink->iClockRate / a_pBink->FrameRate );
}

S32 BinkWait( HBINK a_hBink )
{
	ABink* pBink = TSTATICCAST( ABink, a_hBink );

	if ( pBink->bPaused )
		return 1;

	if ( !pBink->bStarted || pBink->bLooping )
		return 0;

	return GetClock( pBink ) < GetFrameTime( pBink, pBink->FrameNum ) ? 1 : 0;
}

static void DecodeCurrentFrame( ABink* a_pBink )
{
	const TUINT32 uiFrame    = a_pBink->FrameNum;
	const TUINT32 uiPosition = a_pBink->pOffsets[ uiFrame - 1 ];
	const TUINT32 uiSize     = a_pBink->pOffsets[ uiFrame ] - uiPosition;

	if ( a_pBink->uiFilePosition != uiPosition )
		a_pBink->pFile->Seek( uiPosition, TSEEK_SET );

	a_pBink->uiFilePosition = uiPosition + uiSize;

	if ( a_pBink->pFile->Read( a_pBink->pFrame, uiSize ) != uiSize )
	{
		a_pBink->ReadError      = 1;
		a_pBink->uiFilePosition = TUINT32( -1 );
		return;
	}

	TUtil::MemClear( a_pBink->pFrame + uiSize, READ_PADDING );

	// Per track a size and that many bytes, then the picture. The first track plays, like in Bink.
	const TUINT8* pData = a_pBink->pFrame;
	const TUINT8* pEnd  = pData + uiSize;

	for ( TUINT32 i = 0; i < a_pBink->uiNumTracks; i++ )
	{
		if ( pData + 4 > pEnd )
			return;

		const TUINT32 uiPacket = ReadHeader32( pData );
		pData += 4;

		if ( uiPacket > TUINT32( pEnd - pData ) )
			return;

		if ( i == 0 && a_pBink->bHasAudio && uiPacket >= 4 )
			a_pBink->oAudio.Decode( pData, uiPacket, WriteMovieSound, TNULL );

		pData += uiPacket;
	}

	a_pBink->oVideo.Decode( pData, TUINT32( pEnd - pData ) );
}

S32 BinkDoFrame( HBINK a_hBink )
{
	ABink* pBink = TSTATICCAST( ABink, a_hBink );

	if ( pBink->uiDecoded == pBink->FrameNum )
		return 0;

	if ( pBink->bLooping )
	{
		pBink->bLooping = TFALSE;
		pBink->bStarted = TFALSE;

		if ( pBink->bHasAudio )
			pBink->oAudio.Reset();

		AMovieVoice_Start( pBink->iClockRate );
	}

	// Too slow for real time: frames that are already over are decoded, since the next ones need them, but the
	// caller only has to convert and upload the latest
	if ( pBink->bStarted && !( pBink->uiFlags & BINKNOSKIP ) )
	{
		const TUINT32 uiClock = GetClock( pBink );

		for ( TINT i = 0; i < MAX_CATCH_UP && pBink->FrameNum < pBink->Frames && GetFrameTime( pBink, pBink->FrameNum + 1 ) <= uiClock; i++ )
		{
			DecodeCurrentFrame( pBink );
			pBink->FrameNum++;
		}
	}

	DecodeCurrentFrame( pBink );

	pBink->uiDecoded    = pBink->FrameNum;
	pBink->LastFrameNum = pBink->FrameNum;

	if ( !pBink->bStarted )
	{
		pBink->bStarted   = TTRUE;
		pBink->dWallStart = emscripten_get_now();

		AMovieVoice_SetPaused( pBink->bPaused );
	}

	return 0;
}

void BinkNextFrame( HBINK a_hBink )
{
	ABink* pBink = TSTATICCAST( ABink, a_hBink );

	if ( pBink->FrameNum < pBink->Frames )
	{
		pBink->FrameNum++;
		return;
	}

	// Wraps around, the game stops the movie here and the end of the sound has to keep playing
	pBink->FrameNum  = 1;
	pBink->uiDecoded = 0;
	pBink->bLooping  = TTRUE;
}

S32 BinkCopyToBuffer( HBINK a_hBink, void* a_pDst, S32 a_iPitch, U32 a_uiHeight, U32 a_uiX, U32 a_uiY, U32 a_uiFlags )
{
	ABink* pBink = TSTATICCAST( ABink, a_hBink );

	if ( !a_pDst )
		return -1;

	pBink->oVideo.CopyToSurface( TSTATICCAST( TUINT8, a_pDst ), a_iPitch, a_uiHeight, a_uiX, a_uiY, a_uiFlags );
	return 0;
}

S32 BinkPause( HBINK a_hBink, S32 a_iPause )
{
	ABink*      pBink   = TSTATICCAST( ABink, a_hBink );
	const TBOOL bPaused = a_iPause != 0;

	if ( bPaused != pBink->bPaused )
	{
		if ( bPaused )
			pBink->dWallPaused = emscripten_get_now();
		else
			pBink->dWallStart += emscripten_get_now() - pBink->dWallPaused;

		pBink->bPaused = bPaused;

		if ( pBink->bStarted )
			AMovieVoice_SetPaused( bPaused );
	}

	return bPaused ? 1 : 0;
}

void BinkSetVolume( HBINK a_hBink, U32 a_uiTrack, S32 a_iVolume )
{
	AMovieVoice_SetVolume( a_iVolume );
}

void BinkSetSoundOnOff( HBINK a_hBink, S32 a_iOn )
{
	AMovieVoice_SetSoundOn( a_iOn != 0 );
}
