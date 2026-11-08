#pragma once

// The part of the Bink SDK the game uses, ABink_WASM decodes the movies
typedef unsigned int U32;
typedef int          S32;

#define RADLINK
#define BinkOpenWaveOut 0
#define BinkOpenDirectSound 0
#define BinkSetSoundSystem( SYSTEM, PARAM )
#define BinkSetSoundTrack( COUNT, TRACKS )
#define RADEXPLINK
#define PTR4

struct BINK
{
	U32 Width;
	U32 Height;
	U32 Frames;
	U32 FrameNum;
	U32 LastFrameNum;
	U32 FrameRate;
	U32 FrameRateDiv;
	U32 ReadError;
};

typedef BINK* HBINK;

#define BINKSURFACE32     3
#define BINKSURFACE32R    4
#define BINKSURFACE32A    5
#define BINKSURFACE32RA   6
#define BINKSURFACE565    10
#define BINKSURFACE5551   8
#define BINKSURFACE4444   9
#define BINKCOPYALL       0x80000000L
#define BINKPRELOADALL    0x00002000L
#define BINKIOSIZE        0x01000000L
#define BINKNOTHREADEDIO  0x00800000L
#define BINKNOSKIP        0x00080000L

typedef void* ( *BINKMEMALLOC )( U32 );
typedef void ( *BINKMEMFREE )( void* );

void   BinkSetMemory( BINKMEMALLOC a_fnAlloc, BINKMEMFREE a_fnFree );
HBINK  BinkOpen( const char* a_szName, U32 a_uiFlags );
void   BinkClose( HBINK a_hBink );
S32    BinkWait( HBINK a_hBink );
S32    BinkDoFrame( HBINK a_hBink );
void   BinkNextFrame( HBINK a_hBink );
S32    BinkCopyToBuffer( HBINK a_hBink, void* a_pDst, S32 a_iPitch, U32 a_uiHeight, U32 a_uiX, U32 a_uiY, U32 a_uiFlags );
S32    BinkPause( HBINK a_hBink, S32 a_iPause );
void   BinkSetVolume( HBINK a_hBink, U32 a_uiTrack, S32 a_iVolume );
void   BinkSetSoundOnOff( HBINK a_hBink, S32 a_iOn );
void*  BinkLogoAddress();
const char* BinkGetError();
S32    BinkSoundUseDirectSound( void* a_pDirectSound );
