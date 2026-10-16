#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>
#include <Toshi/TSystem.h>
#include <Toshi/TPString8.h>
#include <File/TTRB.h>

//-----------------------------------------------------------------------------
// Purpose: Plays the music of the main game and of some minigames: FMOD streams
// whose sub channels are layers of the same music (Data/Sound/multiplexstreams.trb,
// "MultiplexStream" entries). The volume of each layer can be changed while the
// stream plays (see SetVolume), AMusicManager fades them in and out.
// Only one stream is open at a time.
// NOTE: DAT_0078c3f4 is the instance.
//
// Size 0x414, created by AGameSystemManager.
//-----------------------------------------------------------------------------
class AMultiplexStream
    : public Toshi::TSingleton<AMultiplexStream>
{
public:
#ifdef BARNYARD_LATEST
	// Room for the streams the Wii data adds (Data/Sound/MultiplexStreams_Wii.trb, see LoadWiiStreams)
	static constexpr TINT MAX_NUM_STREAMS = 32;
#else
	static constexpr TINT MAX_NUM_STREAMS = 17;
#endif

	// NOTE: same states as AVOStream::STATE
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_NONE,
		STATE_OPENING,        // the stream is being opened
		STATE_PLAY_REQUESTED, // unused
		STATE_PLAYING,
		STATE_STOP_REQUESTED, // stops once the stream is open
		STATE_CLOSE_REQUESTED,
		STATE_READY,
		STATE_CLOSED,
	};

	// A "MultiplexStream" of multiplexstreams.trb
	struct Stream
	{
		Stream();
		~Stream();

		Toshi::TPString8  strName;           // 0x00, "Name"
		Toshi::TPString8  strFSB;            // 0x04, "FSB"
		Toshi::TPString8* aCategories;       // 0x08, "ChannelCategories": sound category of each channel
		TFLOAT*           afMaxVolumes;      // 0x0c, "ChannelMaxVolumes"
		TFLOAT*           afVolumes;         // 0x10, current volume of each channel
		TINT*             aiPans;            // 0x14, "ChannelPan"
		TINT              iNumChannels;      // 0x18
		TBOOL             bLoop;             // 0x1c, "Loop"
		TFLOAT            fVolume;           // 0x20, volume of the whole stream
		TINT              iStreamBufferSize; // 0x24, "StreamBufferSize"
	};

public:
	AMultiplexStream();
	~AMultiplexStream();

	// Updates the state of the stream (opening, stopping, closing)
	void Update( TFLOAT a_fDeltaTime );

	// Opens the stream called a_szName (closes the open one if it's another stream)
	void Open( const TCHAR* a_szName );

	// Plays the open stream, every channel starts with the volume a_fVolume
	TBOOL Play( TFLOAT a_fVolume );

	// Stops the stream (it stays open)
	void Stop();

	// Releases the channels of the stream, a_bImmediately closes the stream now instead of in Update
	void Close( TBOOL a_bImmediately );

	// Sets the volume of the channel a_iChannel (-1: the whole stream), a_bStore keeps it as the current volume
	void SetVolume( TFLOAT a_fVolume, TINT a_iChannel, TBOOL a_bStore );

	// Returns the current volume of the channel a_iChannel (-1: the whole stream)
	TFLOAT GetVolume( TINT a_iChannel );

	// Pauses the stream (calls are counted)
	void Pause( TBOOL a_bPause );

	STATE GetState() const { return m_eState; }
	TINT  GetNumSubChannels() const { return m_iNumSubChannels; }

#ifdef BARNYARD_LATEST
	// Is there a stream of that name (the Wii-only ones need the converted Wii data)
	TBOOL HasStream( const TCHAR* a_szName ) const;

	// Is the open stream the one of that name
	TBOOL IsCurrentStream( const Toshi::TPString8& a_rcName ) const;

	// Is the stream's channel still playing (FALSE once a stream that doesn't loop has ended)
	TBOOL IsChannelPlaying() const;
#endif

	// $Barnyard: FUNCTION 005d3c50
	static TBOOL IsPaused() { return ms_iPauseCount > 0; }

private:
	static TBOOL __stdcall OnSystemPause( AMultiplexStream* a_pStream, Toshi::TSystemManager* a_pSystemManager, TBOOL* a_pPaused );

private:
	// 0x0078c3f0
	static TINT ms_iPauseCount;

private:
#ifdef BARNYARD_LATEST
	void LoadWiiStreams( const TCHAR* a_szFileName );
#endif

	void*                                                            m_pStream;                     // 0x000, FSOUND_STREAM
	STATE                                                            m_eState;                      // 0x004
	TINT                                                             m_iChannel;                    // 0x008
	TINT                                                             m_iNumSubChannels;             // 0x00c
	Toshi::TTRB                                                      m_oTRB;                        // 0x010
	TINT                                                             m_iNumStreams;                 // 0x150
	TINT                                                             m_iCurrentStream;              // 0x154
	Stream                                                           m_aStreams[ MAX_NUM_STREAMS ]; // 0x158
	Toshi::TListener<Toshi::TSystemManager, TBOOL, AMultiplexStream> m_oPauseListener;              // 0x400
};
