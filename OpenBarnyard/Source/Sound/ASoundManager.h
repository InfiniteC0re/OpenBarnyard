#pragma once
#include "AWaveBank.h"
#include "ASoundBank.h"

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

#undef PlaySound
#define ASOUNDMANAGER_MAX_NUM_CUE 128

class ASoundManager :
    public Toshi::TTask,
    public Toshi::TSingleton<ASoundManager>
{
public:
	TDECLARE_CLASS( ASoundManager, Toshi::TTask );

	struct Cue;

	struct S2 : public Toshi::T2DList<S2>::Node
	{
	};

	struct S4 : public Toshi::T2DList<S4>::Node
	{
	};

	struct CameraData
	{
		Toshi::TVector4 Position = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Velocity = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Forward  = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Up       = Toshi::TVector4::VEC_ZERO;
	};

	enum SOUNDEVENT : TUINT
	{
		SOUNDEVENT_PlayAudio,
		SOUNDEVENT_PlayStream,
		// ...
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

	// NOTE: originally ChannelRef and ChannelRefLegacy are the same structure
	// but it's a fucking mess to maintain and understand code if they're merged
	// so I split them in two. It doesn't affect anything including compatibility
	// since ChannelRefLegacy is not even created by the game.

	struct ChannelRef :
	    public Toshi::T2DList<ChannelRef>::Node
	{
	public:
		TINT          iFMODChannelHandle = -1;
		ASound::Sample* pWave              = TNULL;
		TINT          Unknown            = 0;
		TINT         iFlags       = 0;
		TFLOAT        fVolume            = 1.0f;
		TINT          iFrequency         = 44100;
	};

	struct TDEPRECATED ChannelRefLegacy :
	    public Toshi::T2DList<ChannelRefLegacy>::Node
	{
	public:
		using Callback_t = void ( * )( TINT a_iType, TINT a_iCueIndex, void* a_pUserData );

	public:
		Callback_t fnCallback = TNULL;
		void*      pUserData  = TNULL;
	};

	struct StreamRef :
	    public Toshi::T2DList<StreamRef>::Node
	{
	public:
		ChannelRef* pChannelRef;
		Cue*        pCue;
		void*       pUnknown;
	};

	class SoundEvent :
	    public Toshi::T2DList<SoundEvent>::Node
	{
	public:
		SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASound::Sample* a_pWave, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );
		SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASound::Sample* a_pWave, TFLOAT a_fCustomParam1, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );
		SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASound::Sample* a_pWave, const EventParameters& a_rcCustomParams, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	public:
		SOUNDEVENT      eEventType;
		TFLOAT          fStartTime;
		TINT           iFlags;
		Cue*            pCue;
		ASound::Sample*   pSample;
		ChannelRef*     pChannel;
		EventParameters oParameters;
		TINT            iTrackIndex;
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

	// Contains SoundEvents in a sorted way being a T2DList node
	class SoundEventList : public Toshi::T2DList<SoundEventList>::Node
	{
	public:
		using ListContainer = Toshi::T2SortedList<SoundEvent, Toshi::T2DList<SoundEvent>, SoundEventSortResults>;

	public:
		SoundEventList();
		~SoundEventList();

		ListContainer* EventList() const { return m_pEventList; }
		ListContainer* operator->() const { return m_pEventList; }

	private:
		ListContainer* m_pEventList;
	};

	// Sorts lists of the events by the amount of stored events (from 0 to n)
	struct SoundEventListSortResults
	{
		TINT operator()( const SoundEventList& a_rcVal1, const SoundEventList& a_rcVal2 ) const
		{
			if ( a_rcVal1->Size() <= 0 )
				return ( a_rcVal2->Size() == 0 ) ? 0 : -1;

			if ( a_rcVal2->Size() <= 0 )
				return 1;

			return SoundEventSortResults()( *a_rcVal1->Begin(), *a_rcVal2->Begin() );
		}
	};

	struct Cue : public Toshi::T2DList<Cue>::Node
	{
	public:
		Cue();
		~Cue();

		void Reset();

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

	struct Category
	{
	public:
		TBOOL  bFlag1            = TTRUE;
		TBOOL  bFlag2            = TTRUE;
		TBOOL  bFlag3            = TFALSE;
		TFLOAT fVolumeMultiplier = 1.0f;
		// ...
		Toshi::T2DList<Cue> PlayingCues;
	};

	struct EventHandler
	{
	public:
		using Callback_t = void ( ASoundManager::* )( SoundEvent* a_pSoundEvent );

	public:
		Callback_t fnCallback;
		TUINT      Unused;
	};

	struct SoundEventManager
	{
	public:
		SoundEventManager();
		~SoundEventManager();

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

	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnDestroy() override;

	TINT PlaySound( ASoundWaveId a_iSound );
	TINT PlaySoundEx( ASoundWaveId a_iSound, TFLOAT a_fVolume, TBOOL a_bFlag, TFLOAT a_fStartDelay, TINT a_iTrack );

	void PauseAllSound( TBOOL a_bPaused );

	TINT  GetAvailableCueIndex();
	TBOOL IsCuePlaying( TINT a_iCueIndex );

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

private:
	void CreatePlaySoundEvent( Cue* a_pCue, TINT a_iTrackIndex, TINT a_iFirstWaveIndex, TINT a_iLastWaveIndex, TINT a_iFlags, TFLOAT a_fDelay1, TFLOAT a_fDelay2 );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pWave, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pWave, TFLOAT a_fCustomParam1, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASound::Sample* a_pWave, const EventParameters& a_rcCustomParams, ChannelRef* a_pChannel, TINT a_iFlags, TINT a_iTrackIndex );

	// Adds event to the cue and queues the cue to be played
	void AddEventToCue( Cue* a_pCue, SoundEvent* a_pSoundEvent );

	// Updates stream that has some unknown value (probably ASoundActivity). Returns TTRUE if stream is over
	TBOOL UpdateStreamActivity( StreamRef* a_pStream );

	// Updates stream that has Cue. Returns TTRUE if stream is over
	TBOOL UpdateStreamCue( StreamRef* a_pStream );

private:
	//-----------------------------------------------------------------------------
	// Event handlers
	//-----------------------------------------------------------------------------
	void EventHandler_PlaySound( SoundEvent* a_pEvent );
	void EventHandler_PlayStream( SoundEvent* a_pEvent );

private:
	TBOOL Initialise();

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
	inline static TINT                                                                     ms_iPaused;

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
	Category                                                                                       m_aCategories[ MAX_NUM_CATEGORIES ];  // 0x4E10
	TINT                                                                                           m_iLastAvailableSoundExSlot;          // 0x4F50
	Toshi::T2SortedList<SoundEventList, Toshi::T2DList<SoundEventList>, SoundEventListSortResults> m_QueuedSortedEventLists;             // 0x4F54
	TFLOAT                                                                                         m_fCurrentTime;                       // 0x4F5C
	SoundEventManager                                                                              m_oEventManager;                      // 0x4F60
	TBOOL                                                                                          m_bMuted;                             // 0x4FA8
	TBOOL                                                                                          m_bUseMinHardwareChannels;            // 0x4FA9
	TINT                                                                                           m_iMinHWChannels;                     // 0x4FAC
	TINT                                                                                           m_iNumChannels;                       // 0x4FB0
	TINT                                                                                           m_iGlobalFrequency;                   // 0x4FB4
	PauseListener                                                                                  m_PauseListener;                      // 0x4FB8
	S4*                                                                                            m_pS4;                                // 0x4FCC
	Toshi::T2DList<S4>                                                                             m_FreeListS4;                         // 0x4FD0
	Toshi::T2DList<S4>                                                                             m_UnkList1;                           // 0x4FD8
	Toshi::T2DList<ASoundBank>                                                                     m_SoundBanks;                         // 0x4FE0
	Toshi::T2DList<S2>                                                                             m_FreeListS2;                         // 0x4FE8
	                                                                                                                                     // ...
};
