#pragma once
#include "AWaveBank.h"
#include "ASoundBank.h"
#include "FSM/AFunctionTable.h"

#include <Toshi/TPString8.h>
#include <Toshi/TTask.h>
#include <Toshi/TSystem.h>
#include <Toshi/T2Map.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2SortedList.h>
#include <Toshi/T2ObjectPool.h>

#include <File/TFile.h>

#include <Math/TVector4.h>

#define ASOUNDMANAGER_MAX_NUM_CUE 128

class PBPropertyValue;

class MODAPI ASoundManager
    : public Toshi::TTask
    , public Toshi::TSingleton<ASoundManager>
{
public:
	TDECLARE_CLASS( ASoundManager, Toshi::TTask );

	// Result of FSOUND_Stream_GetOpenState / FMUSIC_GetOpenState
	using OPENSTATE = TINT;
	enum OPENSTATE_ : OPENSTATE
	{
		OPENSTATE_READY      = 0,
		OPENSTATE_INVALID    = -1,
		OPENSTATE_OPENING    = -2,
		OPENSTATE_FAILED     = -3,
		OPENSTATE_CONNECTING = -4, // internet streams only
		OPENSTATE_BUFFERING  = -5, // internet streams only
	};

	class Cue;
	struct ChannelRef;

	// Slot of a legacy (ASoundLegacy) sound, see PlayStreamedVO
	struct S2 : public Toshi::T2DList<S2>::Node
	{
		// $Barnyard: FUNCTION 005d67c0
		S2()
		{
			pSound        = TNULL;
			pChannelRef   = TNULL;
			bStreamQueued = TFALSE;
			fStartTime    = -1.0f;
			fVolume       = -1.0f;
			fVolumeScale  = 1.0f;
		}

		// Stops the sound and frees the slot
		void Reset();

		TFLOAT        fStartTime;    // 0x08
		ASoundLegacy* pSound;        // 0x0C
		TFLOAT        fVolume;       // 0x10
		TFLOAT        fVolumeScale;  // 0x14
		ChannelRef*   pChannelRef;   // 0x18
		TBOOL         bStreamQueued; // 0x1C
	};

	// Volume fade of a cue (see FadeCue, FadeOutCue and UpdateFades), 0x24 bytes
	struct S4 : public Toshi::T2DList<S4>::Node
	{
		TINT   iCueIndex;     // 0x08
		TFLOAT fTime;         // 0x0c, time since the fade started
		TFLOAT fUpdateTimer;  // 0x10, time since the volume was last set
		TFLOAT fDuration;     // 0x14
		TFLOAT fStartVolume;  // 0x18
		TFLOAT fEndVolume;    // 0x1c
		TBOOL  bStopWhenDone; // 0x20
	};

	struct MemoryStats
	{
		TUINT uiCurrentAlloced; // 0x0
		TUINT uiMaxAlloced;     // 0x4
		TUINT uiUnknown;        // 0x8, always 0
	};

	struct CameraData
	{
		// $Barnyard: FUNCTION 005d60f0
		CameraData()
		    : Position( Toshi::TVector4::VEC_ZERO )
		    , Velocity( Toshi::TVector4::VEC_ZERO )
		    , Forward( Toshi::TVector4::VEC_POSZ )
		    , Up( Toshi::TVector4::VEC_NEGY )
		{}

		Toshi::TVector4 Position;
		Toshi::TVector4 Velocity;
		Toshi::TVector4 Forward;
		Toshi::TVector4 Up;
	};

	enum SOUNDEVENT : TUINT
	{
		SOUNDEVENT_PlayAudio,
		SOUNDEVENT_PlayStream,
		SOUNDEVENT_StopAudio,
		SOUNDEVENT_UpdateChannelVolume,
		SOUNDEVENT_UpdateChannelFrequency,
		SOUNDEVENT_UNUSED1,
		SOUNDEVENT_UpdateChannelPosition,
		SOUNDEVENT_UNUSED2,
		SOUNDEVENT_UNUSED3,
		SOUNDEVENT_NUMOF,
	};

	struct EventParameters
	{
	public:
		static constexpr TSIZE MAX_NUM_PARAMS = 3;

	public:
		TFLOAT& operator[]( TUINT a_uiIndex )
		{
			TASSERT( a_uiIndex < MAX_NUM_PARAMS );
			return aParams[ a_uiIndex ];
		}

		TFLOAT operator[]( TUINT a_uiIndex ) const
		{
			TASSERT( a_uiIndex < MAX_NUM_PARAMS );
			return aParams[ a_uiIndex ];
		}

	public:
		TFLOAT aParams[ MAX_NUM_PARAMS ];
	};

	// NOTE: in the original ChannelRef and ChannelRefLegacy are the same structure, split here for
	// readability. It doesn't affect compatibility since ChannelRefLegacy is never created by the game.

	struct ChannelRef : public Toshi::T2DList<ChannelRef>::Node
	{
	public:
		TINT            iFMODChannelHandle = -1;
		ASound::Sample* pSample            = TNULL;
		ASound::Sample* pLegacySample      = TNULL; // NOTE: for legacy sounds it points at ASoundLegacy::m_pWaveBank, so only pWaveBank and iId are valid
		ASound::FLAGS   iFlags             = ASound::FLAGS_NONE;
		TFLOAT          fVolume            = 1.0f;
		TINT            iFrequency         = 44100;
	};

	struct TDEPRECATED ChannelRefLegacy : public Toshi::T2DList<ChannelRefLegacy>::Node
	{
	public:
		using Callback_t = void ( * )( TINT a_iType, TINT a_iCueIndex, void* a_pUserData );

	public:
		Callback_t fnCallback = TNULL;
		void*      pUserData  = TNULL;
	};

	struct StreamRef : public Toshi::T2DList<StreamRef>::Node
	{
	public:
		ChannelRef* pChannelRef = TNULL;
		Cue*        pCue        = TNULL;
		void*       pLegacySlot = TNULL;
	};

	class SoundEvent : public Toshi::T2DList<SoundEvent>::Node
	{
	public:
		// NOTE: also inlined into T2DynamicObjectPool<SoundEvent>::NewObject (005d6a70), the parameters aren't initialised
		// $Barnyard: FUNCTION 005d6790
		SoundEvent()
		    : eEventType( SOUNDEVENT_PlayAudio ), fStartTime( 0.0f ), iFlags( 0 ), pCue( TNULL ), pSample( TNULL ), pChannel( TNULL ), iTrackIndex( -1 )
		{}

	public:
		SOUNDEVENT      eEventType;  // 0x08
		TFLOAT          fStartTime;  // 0x0C
		ASound::FLAGS   iFlags;      // 0x10
		Cue*            pCue;        // 0x14
		ASound::Sample* pSample;     // 0x18
		ChannelRef*     pChannel;    // 0x1C
		EventParameters oParameters; // 0x20
		TINT            iTrackIndex; // 0x2C
	};

	// Sorts sound events in the SoundEventList by the start time (from 0 to n)
	struct SoundEventSortResults
	{
		TINT operator()( const SoundEvent& a_rcVal1, const SoundEvent& a_rcVal2 ) const
		{
			TFLOAT fStartTime1 = a_rcVal1.fStartTime;
			TFLOAT fStartTime2 = a_rcVal2.fStartTime;

			if ( fStartTime1 > fStartTime2 )
				return 1;

			if ( fStartTime1 >= fStartTime2 )
				return 0;

			return -1;
		}
	};

	//-----------------------------------------------------------------------------
	// Purpose: contains SoundEvents in sorted order, can be used in T2DList
	//-----------------------------------------------------------------------------
	using SoundEventList = Toshi::T2DListNodeWrapper<Toshi::T2SortedList<SoundEvent, Toshi::T2DList<SoundEvent>, SoundEventSortResults>>;

	// Sorts lists of the events by the amount of stored events (from 0 to n)
	struct SoundEventListSortResults
	{
		// $Barnyard: FUNCTION 005d7170
		TINT operator()( const SoundEventList& a_rcVal1, const SoundEventList& a_rcVal2 ) const
		{
			if ( a_rcVal1->Size() <= 0 )
				return ( a_rcVal2->Size() == 0 ) ? 0 : -1;

			if ( a_rcVal2->Size() <= 0 )
				return 1;

			return SoundEventSortResults()( *a_rcVal1->Begin(), *a_rcVal2->Begin() );
		}
	};

	//-----------------------------------------------------------------------------
	// Purpose: Cue is an only playable unit that can be created by ASoundManager.
	// It contains all the channels (sounds) that are played by ASound.
	//-----------------------------------------------------------------------------
	class Cue : public Toshi::T2DList<Cue>::Node
	{
	public:
		Cue();
		~Cue();

		void Reset();

		TBOOL HasEventOfType( SOUNDEVENT a_eEventType ) const;

		TBOOL IsUsed() const { return fStartTime > 0.0f; }

	public:
		TBOOL                            bUsed;
		TFLOAT                           fStartTime;
		ASound*                          pSound;
		TFLOAT                           fStartTime2;
		Toshi::TVector4                  vecPosition;
		TFLOAT                           fVolume;
		TFLOAT                           fFrequency;
		TINT                             iNumChannelRefs;
		Toshi::T2DList<ChannelRef>       oChannelRefs;
		SoundEventList                   oEventList;
		Toshi::T2DList<ChannelRefLegacy> aChannelRefsLegacy[ 2 ];
		Toshi::T2Vector<TINT, 15>        vecLoopStarts;
	};

	//-----------------------------------------------------------------------------
	// Purpose: stores common settings for specified groups of sounds.
	//-----------------------------------------------------------------------------
	struct Category
	{
	public:
		// NOTE: retail leaves fPrevVolumeMultiplier uninitialised
		// $Barnyard: FUNCTION 005d64a0
		Category() {}

		TBOOL               bPausable             = TTRUE;
		TBOOL               bFlag2                = TTRUE;
		TBOOL               bFlag3                = TFALSE;
		TFLOAT              fVolumeMultiplier     = 1.0f;
		TFLOAT              fPrevVolumeMultiplier = 1.0f; // set by SetCategoryVolume
		Toshi::T2DList<Cue> PlayingCues;
	};

	//-----------------------------------------------------------------------------
	// Purpose: stores list of event handlers that can be executed by the event
	// loop in ASoundManager::Update
	//-----------------------------------------------------------------------------
	struct SoundEventManager
	{
	private:
		struct EventHandler
		{
		public:
			using Callback_t = void ( ASoundManager::* )( SoundEvent* a_pSoundEvent );

		public:
			Callback_t fnCallback = TNULL;
			TUINT      Unused     = 0;
		};

	public:
		SoundEventManager()  = default;
		~SoundEventManager() = default;

		void SetEventHandler( SOUNDEVENT a_eEventType, EventHandler::Callback_t a_fnHandler );
		void ExecuteEvent( SOUNDEVENT a_eEventType, ASoundManager* a_pSoundManager, SoundEvent* a_pEvent );

	private:
		EventHandler m_aEventHandlers[ SOUNDEVENT_NUMOF ];
	};

	using PauseListener = Toshi::TListener<Toshi::TSystemManager, TBOOL, ASoundManager>;

	static constexpr const TCHAR* SOUNDS_BASE_DIRECTORY = "Data/Sound/";
	static constexpr TUINT        MAX_NUM_CATEGORIES    = 16;

	friend AWaveBank;

public:
	ASoundManager();
	~ASoundManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x38

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Immediately plays cue and returns it's index. Use PlayCueEx for more settings.
	TINT PlayCue( ASoundId a_iSound );

	// Plays cue and returns it's index
	TINT PlayCueEx( ASoundId a_iSound, TFLOAT a_fVolume, TBOOL a_bFlag, TFLOAT a_fDelay, TINT a_iTrack );

	// Moves cue and all of it's playing channels to the specified position
	// $Barnyard: FUNCTION 005da0c0
	void SetCuePosition( TINT a_iCueIndex, const Toshi::TVector4& a_rcPosition, TFLOAT a_fDelay )
	{
		if ( a_iCueIndex == -1 )
			return;

		Cue* pCue = &m_aCues[ a_iCueIndex ];

		if ( pCue->IsUsed() )
		{
			pCue->vecPosition = a_rcPosition;

			T2_FOREACH( pCue->oChannelRefs, channel )
			{
				if ( channel->iFMODChannelHandle == -1 )
					continue;

				EventParameters oParams;
				oParams[ 0 ] = a_rcPosition.x;
				oParams[ 1 ] = a_rcPosition.y;
				oParams[ 2 ] = -a_rcPosition.z;

				CreateSoundEvent( SOUNDEVENT_UpdateChannelPosition, a_fDelay, pCue, channel->pSample, oParams, channel, 0, -1 );
			}
		}
	}

	// Immediately plays cue at the specified position and returns it's index
	// $Barnyard: FUNCTION 005dbbf0
	// $Barnyard: FUNCTION 005daf30
	void SetCuePosition( TINT a_iCueIndex, const Toshi::TVector4& a_rcPosition ) { SetCuePosition( a_iCueIndex, a_rcPosition, 0.0f ); }

	TINT PlayCueAtPosition( ASoundId a_iSound, const Toshi::TVector4& a_rcPosition )
	{
		TINT iCueIndex = PlayCueEx( a_iSound, 1.0f, TFALSE, 0.0f, -1 );

		if ( iCueIndex == -1 )
			return -1;

		SetCuePosition( iCueIndex, a_rcPosition, 0.0f );
		return iCueIndex;
	}

	// Same as PlayCueEx, but also moves the cue to the specified position
	// $Barnyard: FUNCTION 005dae00
	TINT PlayCueAtPositionEx( ASoundId a_iSound, const Toshi::TVector4& a_rcPosition, TFLOAT a_fVolume, TBOOL a_bFlag, TFLOAT a_fDelay, TINT a_iTrack )
	{
		TINT iCueIndex = PlayCueEx( a_iSound, a_fVolume, a_bFlag, a_fDelay, a_iTrack );

		if ( iCueIndex == -1 )
			return -1;

		SetCuePosition( iCueIndex, a_rcPosition, a_fDelay );
		return iCueIndex;
	}

	// Changes the frequency multiplier of the cue and of all it's playing channels
	void SetCueFrequency( TINT a_iCueIndex, TFLOAT a_fFrequency, TFLOAT a_fDelay );

	// $Barnyard: FUNCTION 005dae60
	void SetCueVolume( TINT a_iCueIndex, TFLOAT a_fVolume ) { SetCueVolume( a_iCueIndex, a_fVolume, 0.0f ); }

	// $Barnyard: FUNCTION 005daf10
	void SetCueFrequency( TINT a_iCueIndex, TFLOAT a_fFrequency ) { SetCueFrequency( a_iCueIndex, a_fFrequency, 0.0f ); }

	// Moves the cue to the position of a_pUnit
	void SetCuePositionToUnit( TINT a_iCueIndex, class AUnit* a_pUnit );

	// Plays the cue at the position of a_pUnit
	TINT PlayCueAtUnit( ASoundId a_iSound, class AUnit* a_pUnit );

	// Immediately stops cue and all it's sounds
	void StopCue( TINT& a_rCueIndex );

	// Calls a_fnCallback( a_iType, cue, a_pUserData ) when the cue starts playing (a_iType 0) or is reset (a_iType 1).
	// Does nothing if the cue isn't playing.
	void AddCueCallback( TINT a_iCueIndex, TINT a_iType, ChannelRefLegacy::Callback_t a_fnCallback, void* a_pUserData );
	void RemoveCueCallback( TINT a_iCueIndex, TINT a_iType, ChannelRefLegacy::Callback_t a_fnCallback );

	// Asynchronously stop cue and all it's sounds
	void StopCueAsync( TINT& a_rCueIndex, TFLOAT a_fDelay );

	// Allows to control pause state of sounds
	void PauseAllCues( TBOOL a_bPause );
	void PauseCue( TINT a_iCueIndex, TBOOL a_bPause );

	// Returns index of an available cue that can be used to play sound
	TINT GetAvailableCueIndex();

	// Returns TTRUE if cue is playing
	TBOOL IsCuePlaying( TINT a_iCueIndex );

	// Returns TTRUE if the cue slot is in use (TFALSE for -1)
	// $Barnyard: FUNCTION 005d6560
	TBOOL IsCueUsed( TINT a_iCueIndex ) const
	{
		if ( a_iCueIndex == -1 )
			return TFALSE;

		return m_aCues[ a_iCueIndex ].bUsed;
	}

	// Returns the volume of the cue (0 if it isn't playing)
	// $Barnyard: FUNCTION 005d6520
	TFLOAT GetCueVolume( TINT a_iCueIndex ) const
	{
		if ( a_iCueIndex != -1 && m_aCues[ a_iCueIndex ].fStartTime > 0.0f )
			return m_aCues[ a_iCueIndex ].fVolume;

		return 0.0f;
	}

	// Returns the id of the sound the cue plays or ASOUNDID_INVALID
	// $Barnyard: FUNCTION 005d64f0
	ASoundId GetCueSoundId( TINT a_iCueIndex ) const
	{
		if ( a_iCueIndex != -1 )
		{
			ASound* pSound = m_aCues[ a_iCueIndex ].pSound;

			if ( pSound )
				return pSound->GetId();
		}

		return ASOUNDID_INVALID;
	}

	// Returns the first playing cue of the sound a_iSound (cue 0 isn't checked) or -1
	// $Barnyard: FUNCTION 005d6650
	TINT FindPlayingCue( ASoundId a_iSound ) const
	{
		for ( TINT i = 1; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
		{
			const Cue& rCue = m_aCues[ i ];

			if ( rCue.fStartTime > 0.0f && rCue.pSound && rCue.pSound->GetId() == a_iSound )
				return i;
		}

		return -1;
	}

	// Returns TTRUE if the cue is used and its sound isn't waiting for the stream to open
	TBOOL IsCueStreamReady( TINT a_iCueIndex ) const;

	// Returns TTRUE if the stream cue is still in use, being faded or has a playing channel
	TBOOL IsStreamCueActive( TINT a_iCueIndex );

	// Returns TTRUE if a volume fade of the cue is running
	TBOOL IsCueFading( TINT a_iCueIndex );

	// Returns the time (ms) of the stream played by the cue or -1 if it isn't a playing stream
	TINT GetStreamCueTime( TINT a_iCueIndex );

	// Sets the time (ms) of the stream played by the cue
	void SetStreamCueTime( TINT a_iCueIndex, TINT a_iTime );

	// Queues the stream of the legacy sound a_iSound (VO category) in a legacy slot (m_aS2) as a 3D sound,
	// returns the index of the slot or -1 if it can't be played
	TINT PlayStreamedVO( ASoundId a_iSound );

	// TRUE if the stream of the legacy slot a_iSlot has been queued
	TBOOL IsStreamedVOQueued( TINT a_iSlot ) const;

	// Sets the 3D position of the channel of the legacy slot a_iSlot
	void SetStreamedVOPosition( TINT a_iSlot, const Toshi::TVector4& a_rcPosition );

	// Sets the volume of every cue
	void SetAllCuesVolume( TFLOAT a_fVolume );

	// Sets the volume of every cue to the volume it's supposed to have (after SetAllCuesVolume)
	// $Barnyard: FUNCTION 005dae80
	void RestoreAllCuesVolume()
	{
		for ( TINT i = 0; i < ASOUNDMANAGER_MAX_NUM_CUE; i++ )
			SetCueVolume( i, GetCueVolume( i ), 0.0f );
	}

	// $Barnyard: FUNCTION 005d61b0
	const CameraData& GetCameraData() const { return m_oCameraData; }

	// Cancels all queued events of specified type
	void CancelCueEvents( Cue* a_pCue, SOUNDEVENT a_eEventType );

	// Sets the volume of every channel of the cue after a_fDelay (clamped to 0-1)
	void SetCueVolume( TINT a_iCueIndex, TFLOAT a_fVolume, TFLOAT a_fDelay );

	// Fades the volume of the cue from its current volume to a_fVolume in a_fDuration seconds
	void FadeCue( TINT a_iCueIndex, TFLOAT a_fDuration, TFLOAT a_fVolume, TFLOAT a_fUnused );

	// Fades the volume of the cue out in a_fDuration seconds, a_bStop stops the cue once it's silent
	void FadeOutCue( TINT a_iCueIndex, TFLOAT a_fDuration, TBOOL a_bStop, TFLOAT a_fUnused );

	// Sets global volume for all cues
	void SetGlobalVolume( TFLOAT a_flVolume );

	// Sets the volume multiplier of the category called a_rcName and updates its playing cues
	void SetCategoryVolume( const Toshi::TPString8& a_rcName, TFLOAT a_fVolume, TFLOAT a_fDelay );

	// Sets the volume multiplier of the category and updates its playing cues
	void SetCategoryVolume( TINT a_iCategory, TFLOAT a_fVolume, TFLOAT a_fDelay );

	// Returns the volume multiplier of the category called a_rcName (0 if there is none)
	TFLOAT GetCategoryVolume( const Toshi::TPString8& a_rcName );

	//-----------------------------------------------------------------------------
	// Wavebanks
	// ----------------------------------------------------------------------------
	// They are storing information about different packs of files (usually .fsb
	// if FMOD is being used) and their samples. Samples store technical info about
	// flags, frequency and length making sure the sounds from soundbanks can get
	// this information when neccessary.
	//-----------------------------------------------------------------------------

	// Returns pointer to a loaded wavebank or TNULL if not found
	AWaveBank* FindWaveBank( const Toshi::TPString8& a_rcName );

	// Loads info about all wavebanks from a specified PProperties file
	TBOOL LoadWaveBanksInfo( const TCHAR* a_szFileName );

	// Loads all samples (f.e. from fsb file) of a specified bank
	TBOOL LoadWaveBankSamples( const Toshi::TPString8& a_rcName, AWaveBank::LOADFLAGS a_eLoadFlags, TINT a_iBufferSize );

	// Unloads all samples of a specified bank
	void UnloadWaveBankSamples( const Toshi::TPString8& a_rcName );

	//-----------------------------------------------------------------------------
	// Soundbanks
	// ----------------------------------------------------------------------------
	// Unlike wavebanks, soundbanks store info about how each sound should be
	// played by the engine. Such info includes volume, pitch, various
	// randomizations and much much more...
	//-----------------------------------------------------------------------------
	// The sounds can be stored either with ASound or ASoundEx class.
	// ASound class doesn't store any advanced settings for the sound.
	// ASoundEx class allows sounds to have different volume, pitch and more.
	//-----------------------------------------------------------------------------

	// Returns pointer to a loaded soundbank or TNULL if not found
	ASoundBank* FindSoundBank( const Toshi::TPString8& a_rcName );

	// Loads soundbank if it's not loaded yet.
	// If a_bSimpleSound is TFALSE, sounds are created from ASoundEx allowing many settings to be set from the properties file.
	// Returns TTRUE if was (now) loaded.
	TBOOL LoadSoundBank( const Toshi::TPString8& a_rcName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately );

	// Initialises sounds of the sound bank
	void LoadSoundBankSamples( const Toshi::TPString8& a_rcName );

	// Destroys the sound bank and unloads the wave banks used by its sounds.
	// a_bSimpleSound has to match the value the bank was loaded with.
	void UnloadSoundBank( const Toshi::TPString8& a_rcName, TBOOL a_bSimpleSound );

	// Loads every sound bank named in the string array a_pBankList (the "soundbanks" property of a library).
	// Player voice banks (am_m/am_f) are skipped unless they match the main player's gender.
	void LoadSoundBanksFromProperty( const PBPropertyValue* a_pBankList );

	// Unloads every sound bank named in the string array a_pBankList
	void UnloadSoundBanksFromProperty( const PBPropertyValue* a_pBankList );

private:
	void CreatePlaySoundEvent( Cue* a_pCue, TINT a_iTrackIndex, TINT a_iFirstWaveIndex, TINT a_iLastWaveIndex, TINT a_iFlags, TFLOAT a_fDelay1, TFLOAT a_fDelay2 );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pSample, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pSample, TFLOAT a_fCustomParam1, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pSample, const EventParameters& a_rcCustomParams, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	// Adds event to the cue and queues the cue to be played
	void AddEventToCue( Cue* a_pCue, SoundEvent* a_pSoundEvent );

	// Moves the fades of the cue back to the free list
	void RemoveCueFades( TINT a_iCueIndex );

	// Updates the volume of the faded cues
	void UpdateFades( TFLOAT a_fDeltaTime );

	// Updates stream that has some unknown value (probably ASoundActivity). Returns TTRUE if stream is over
	TBOOL UpdateStreamActivity( StreamRef* a_pStream );

	// Updates stream that has Cue. Returns TTRUE if stream is over
	TBOOL UpdateStreamCue( StreamRef* a_pStream );

	// Immediately stops every playing cue and stream
	void StopAllCues();

private:
	//-----------------------------------------------------------------------------
	// Event handlers
	//-----------------------------------------------------------------------------
	void EventHandler_PlaySound( SoundEvent* a_pEvent );
	void EventHandler_PlayStream( SoundEvent* a_pEvent );
	void EventHandler_StopAudio( SoundEvent* a_pEvent );
	void EventHandler_UpdateChannelVolume( SoundEvent* a_pEvent );
	void EventHandler_UpdateChannelFrequency( SoundEvent* a_pEvent );
	void EventHandler_UpdatePosition( SoundEvent* a_pEvent );

private:
	TBOOL Initialise();

public:
	// Fills a_pStats with the FMOD memory usage
	void GetMemoryStats( MemoryStats* a_pStats );

private:
	// Listener of the system pause emitter, pauses or resumes every cue
	static TBOOL __stdcall OnSystemPause( ASoundManager* a_pSoundManager, Toshi::TSystemManager* a_pSystemManager, TBOOL* a_pPaused );

	AWaveBank* LoadWaveBankFromAsset( const Toshi::TString8& a_strName, TUINT32 a_uiForcedFlags );
	AWaveBank* AllocateWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strLibrary, const Toshi::TPString8& a_strType, const Toshi::TPString8& a_strPath );

	TBOOL LoadSoundBankImpl( const TCHAR* a_szName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately );

private:
	static TUINT GetCueIndex( Cue* a_pCue );

public:
	// Whether ALoadScreen should be updated while loading audio stream or not
	inline static TBOOL ms_bShouldUpdateLoadingScreen = TFALSE;

private:
	inline static Toshi::TFileSystem*                                                      ms_pFileSystem;
	inline static Toshi::T2Map<Toshi::TPString8, AWaveBank*, Toshi::TPString8::Comparator> ms_WaveBanks;
	inline static TINT                                                                     ms_iNumPauses;

	// FSM natives (Play, Play3DFromUnit, Stop, SetVolume, SetPaused) (DAT_0078c480)
	static AFunctionTable ms_oFunctionTable;

private:
	Toshi::T2DynamicObjectPool<SoundEvent>                              m_SoundEventPool;                     // 0x20
	Toshi::T2DynamicObjectPool<StreamRef>                               m_StreamRefPool;                      // 0x38
	Toshi::T2DynamicObjectPool<ChannelRef>                              m_ChannelRefPool;                     // 0x50
	Toshi::T2DynamicObjectPool<ChannelRefLegacy>                        m_ChannelRefLegacyPool;               // 0x68
	Toshi::T2Map<Toshi::TPString8, TSIZE, Toshi::TPString8::Comparator> m_CategoryIndices;                    // 0x80
	Toshi::T2Map<TINT, ASound*>                                         m_SoundIdToSound;                     // 0x98
	Toshi::T2Map<TINT, ASoundLegacy*>                                   m_SoundIdToSoundLegacy;               // 0xB0
	CameraData                                                          m_oCameraData;                        // 0xC8
	Cue                                                                 m_aCues[ ASOUNDMANAGER_MAX_NUM_CUE ]; // 0x108
	S2                                                                  m_aS2[ 8 ];                           // 0x4D08
	Toshi::T2DList<StreamRef>                                           m_StreamRefs;                         // 0x4E08
	// ...
	Category                                                                                       m_aCategories[ MAX_NUM_CATEGORIES ]; // 0x4E10
	TINT                                                                                           m_iLastAvailableSoundExSlot;         // 0x4F50
	Toshi::T2SortedList<SoundEventList, Toshi::T2DList<SoundEventList>, SoundEventListSortResults> m_QueuedSortedEventLists;            // 0x4F54
	TFLOAT                                                                                         m_fCurrentTime;                      // 0x4F5C
	SoundEventManager                                                                              m_oEventManager;                     // 0x4F60
	TBOOL                                                                                          m_bMuted;                            // 0x4FA8
	TBOOL                                                                                          m_bUseMinHardwareChannels;           // 0x4FA9
	TINT                                                                                           m_iMinHWChannels;                    // 0x4FAC
	TINT                                                                                           m_iNumChannels;                      // 0x4FB0
	TINT                                                                                           m_iGlobalFrequency;                  // 0x4FB4
	PauseListener                                                                                  m_PauseListener;                     // 0x4FB8
	S4*                                                                                            m_pS4;                               // 0x4FCC
	Toshi::T2DList<S4>                                                                             m_FreeListS4;                        // 0x4FD0
	Toshi::T2DList<S4>                                                                             m_UsedListS4;                        // 0x4FD8
	Toshi::T2DList<ASoundBank>                                                                     m_SoundBanks;                        // 0x4FE0
	Toshi::T2DList<S2>                                                                             m_FreeListS2;                        // 0x4FE8
	Toshi::T2DList<S2>                                                                             m_UsedListS2;                        // 0x4FF0
	                                                                                                                                    // ...
};
