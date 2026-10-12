#pragma once
#include <Toshi/TSingleton.h>
#include <Toshi/TEvent.h>
#include <Toshi/TSystem.h>
#include <File/TTRB.h>
#include <Math/TVector3.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// Purpose: Plays the voice overs: a FMOD stream of an FSB file whose sub streams
// are the lines ("SS.fsb" of the cutscenes, "QuestVO.fsb" of the quests, which
// also has sets of lines in Data/Sound/QuestVO.trb). Another stream can be pushed
// on top of the current one while it's open (see Open).
// NOTE: retail calls it "VOStream" (Wii and PS2 builds), DAT_0078c430 is the instance.
//
// Size 0x288, created by AGameSystemManager.
//-----------------------------------------------------------------------------
class AVOStream
    : public Toshi::TSingleton<AVOStream>
{
public:
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_NONE,
		STATE_OPENING,        // the stream is being opened
		STATE_PLAY_REQUESTED, // plays the sub stream once the stream is open
		STATE_PLAYING,
		STATE_STOP_REQUESTED, // stops once the stream is open
		STATE_CLOSE_REQUESTED,
		STATE_READY,
		STATE_CLOSED,
	};

	// A stream and its state
	struct Stream
	{
		void* pStream;       // 0x00, FSOUND_STREAM
		STATE eState;        // 0x04
		TINT  iChannel;      // 0x08
		TCHAR szName[ 128 ]; // 0x0c, the name of the FSB file in lower case

		// $Barnyard: FUNCTION 005d5990
		Stream& operator=( const Stream& a_rcOther )
		{
			pStream  = a_rcOther.pStream;
			eState   = a_rcOther.eState;
			iChannel = a_rcOther.iChannel;
			Toshi::T2String8::Copy( szName, a_rcOther.szName, sizeof( szName ) );
			return *this;
		}
	};

	// A set of lines of QuestVO.trb, the lines are played one after another
	struct VOSet
	{
		using FLAGS = TUINT8;
		enum FLAGS_ : FLAGS
		{
			FLAGS_NONE = 0,
			FLAGS_3D   = BITFLAG( 1 ),
		};

		FLAGS        uiFlags;      // 0x00
		TUINT8       uiNumIds;     // 0x01
		TUINT16      uiCurrentId;  // 0x02
		TUINT16      auiIds[ 12 ]; // 0x04, sub streams of QuestVO.fsb
		TFLOAT       fMinDistance; // 0x1c
		const TCHAR* szName;       // 0x20
	};

public:
	AVOStream();
	~AVOStream();

	// Updates the state of the stream
	void Update( TFLOAT a_fDeltaTime );

	// Opens the FSB file a_szName (the language's file for "questvo" and "data/sound/fsb/ss.fsb").
	// a_bWait waits until it's open, a_bPush keeps the current stream to restore it on Close.
	void Open( const TCHAR* a_szName, TBOOL a_bWait, TBOOL a_bPush );

	// Closes the stream (a_bImmediately or on the next update) and restores the pushed one
	void Close( TBOOL a_bImmediately );

	// Plays the sub stream a_iSubStream once the stream is open
	void Play( TINT a_iSubStream, TBOOL a_b3D );

	// Plays the sub stream a_iSubStream at a_rcPosition (3D)
	void PlayAtPosition( TINT a_iSubStream, const Toshi::TVector4& a_rcPosition );

	// Plays the next line of the set a_szName at a_rcPosition
	void PlaySet( const TCHAR* a_szName, const Toshi::TVector4& a_rcPosition );

	// Plays the next line of the set "<a_szName>_<a_szSuffix>" at a_rcPosition
	void PlaySet( const TCHAR* a_szName, const TCHAR* a_szSuffix, const Toshi::TVector4& a_rcPosition );

	// Stops the line once the stream is open
	void Stop();

	// TTRUE while the line plays
	TBOOL IsPlaying();

	// TTRUE if a line waits for the stream to play
	TBOOL IsPlayRequested();

	// The length of the line in milliseconds (0 if it doesn't play)
	TINT GetLengthMs();

	// The name of the line (TNULL if it doesn't play)
	const TCHAR* GetLineName();

	// The time of the line in milliseconds (0 if it doesn't play)
	TINT GetTime();

private:
	// Returns the set called a_szName of QuestVO.trb
	VOSet* FindSet( const TCHAR* a_szName, TINT a_iUnused );

	// Plays the next line of a_pSet
	void PlayFromSet( VOSet* a_pSet );

	// Starts playing the requested sub stream
	void StartPlaying();

	static TBOOL __stdcall OnPause( AVOStream* a_pVOStream, Toshi::TSystemManager* a_pSystemManager, TBOOL* a_pPaused );

private:
	Stream                                                    m_oCurrent;       // 0x00
	Stream                                                    m_oPushed;        // 0x8c
	Toshi::TListener<Toshi::TSystemManager, TBOOL, AVOStream> m_oPauseListener; // 0x118
	Toshi::TTRB                                               m_oQuestVOTRB;    // 0x12c
	VOSet*                                                    m_pSets;          // 0x26c, "QuestVOSets" of QuestVO.trb
	TINT                                                      m_iNumSets;       // 0x270, "NumQuestVOSets" of QuestVO.trb
	Toshi::TVector3                                           m_vPosition;      // 0x274, FMOD space (z is negated)
	VOSet*                                                    m_pPlayingSet;    // 0x280
	TBOOL                                                     m_b3D;            // 0x284
};
