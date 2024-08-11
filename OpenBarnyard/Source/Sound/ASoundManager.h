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

	struct Category : public Toshi::T2DList<Category>::Node
	{

	};

	struct S4 : public Toshi::T2DList<S4>::Node
	{

	};

	struct CameraData
	{
		Toshi::TVector4 Position = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Velocity = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Forward = Toshi::TVector4::VEC_ZERO;
		Toshi::TVector4 Up = Toshi::TVector4::VEC_ZERO;
	};

	enum SOUNDEVENT : TUINT
	{
		SOUNDEVENT_PlayAudio,
		// ...
		SOUNDEVENT_NUMOF,
	};

	struct EventParameters
	{
		static constexpr TSIZE MAX_NUM_PARAMS = 3;
		TFLOAT aParams[ MAX_NUM_PARAMS ];

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
	};

	struct PlayingSound :
		public Toshi::T2DList<PlayingSound>::Node
	{

	};

	class SoundEvent :
		public Toshi::T2DList<SoundEvent>::Node
	{
	public:
		SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, PlayingSound* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex );
		SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, TFLOAT a_fCustomParam1, PlayingSound* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex );
		SoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fStartTime, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, const EventParameters& a_rcCustomParams, PlayingSound* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex );
		
	public:
		SOUNDEVENT eEventType;
		TFLOAT fStartTime;
		TUINT uiFlags;
		Cue* pCue;
		ASoundAdvanced::Wave* pWave;
		PlayingSound* pPlayingSound;
		EventParameters oParameters;
		TINT iTrackIndex;
	};

	// Sorts sound events in the SoundEventList by the start time (from 0 to n)
	struct SoundEventSortResults
	{
		TINT operator()( const SoundEvent& a_rcVal1, const SoundEvent& a_rcVal2 ) const
		{
			TFLOAT fStartTime1 = a_rcVal1.fStartTime;
			TFLOAT fStartTime2 = a_rcVal2.fStartTime;

			TBOOL bIsNan1 = Toshi::TMath::IsNaN( fStartTime1 );
			TBOOL bIsNan2 = Toshi::TMath::IsNaN( fStartTime1 );
			TBOOL bAnyNan = bIsNan1 || bIsNan2;

			if ( bAnyNan || fStartTime1 == fStartTime2 || fStartTime1 < fStartTime2 )
			{
				if ( ( bAnyNan || fStartTime1 < fStartTime2 ) != bAnyNan )
					return -1;
			}

			return 0;
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

		ListContainer* operator->() const { return m_pEventList; }
		
	private:
		ListContainer* m_pEventList;
	};

	// Sorts lists of the events by the amount of stored events (from 0 to n)
	struct SoundEventListSortResults
	{
		TINT operator()( const SoundEventList& a_rcVal1, const SoundEventList& a_rcVal2 ) const
		{
			if ( a_rcVal1->Size() == 0 )
			{
				// The list we are adding is empty
				return ( a_rcVal2->Size() == 0 ) ? 0 : -1;
			}

			if ( a_rcVal2->Size() > 0 )
			{
				// Both lists aren't empty so compare by their first events
				return SoundEventSortResults()( *a_rcVal1->Begin(), *a_rcVal2->Begin() );
			}

			return 1;
		}
	};

	struct Cue : public Toshi::T2DList<Cue>::Node
	{
		Cue();
		~Cue();

		TBOOL bUsed; // ?
		TFLOAT fStartTime;
		ASoundAdvanced* pSoundAdvanced;
		TFLOAT fStartTime2;
		Toshi::TVector4 vecPosition;
		TFLOAT fVolume;
		TFLOAT fFrequency;
		TINT m_iNumPlayingSounds;
		// T2DList PlayingSounds
		SoundEventList EventList;
		// T2DList[2] m_aSomeLists
		Toshi::T2Vector<TINT, 15> m_vecLoopStarts;
	};

	struct EventHandler
	{
		using Callback_t = void ( ASoundManager::* )( SoundEvent* a_pSoundEvent );

		Callback_t fnCallback;
		TUINT Unused;
	};

	using PauseListener = Toshi::TListener<Toshi::TSystemManager, TBOOL, ASoundManager>;

	static constexpr const TCHAR* SOUNDS_BASE_DIRECTORY = "Data/Sound/";
	static constexpr TUINT MAX_NUM_CATEGORIES = 16;

	friend AWaveBank;

public:
	ASoundManager();
	~ASoundManager();

	virtual TBOOL Reset() override;
	virtual TBOOL OnCreate() override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void OnDestroy() override;

	void PauseAllSound( TBOOL a_bPaused );

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
	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, PlayingSound* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, TFLOAT a_fCustomParam1, PlayingSound* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex );

	// Allocates new SoundEvent and adds it to the specified cue
	SoundEvent* CreateSoundEvent( SOUNDEVENT a_eEventType, TFLOAT a_fDelay, Cue* a_pCue, ASoundAdvanced::Wave* a_pWave, const EventParameters& a_rcCustomParams, PlayingSound* a_pPlayingSound, TUINT a_uiFlags, TINT a_iTrackIndex );

	// Adds event to the cue and queues the cue to be played
	void AddEventToCue( Cue* a_pCue, SoundEvent* a_pSoundEvent );

public:
	// Whether ALoadScreen should be updated while loading audio stream or not
	inline static TBOOL ms_bShouldUpdateLoadingScreen = TFALSE;

private:
	TBOOL Initialise();

	AWaveBank* LoadWaveBankFromAsset( const Toshi::TString8& a_strName, TUINT32 a_uiForcedFlags );
	AWaveBank* AllocateWaveBank( const Toshi::TPString8& a_strBank, const Toshi::TPString8& a_strLibrary, const Toshi::TPString8& a_strType, const Toshi::TPString8& a_strPath );

	TBOOL LoadSoundBankImpl( const TCHAR* a_szName, TBOOL a_bSimpleSound, TBOOL a_bLoadImmediately );

private:
	inline static Toshi::TFileSystem* ms_pFileSystem;
	inline static Toshi::T2Map<Toshi::TPString8, AWaveBank*, Toshi::TPString8::Comparator> ms_WaveBanks;

private:
	Toshi::T2DynamicObjectPool<SoundEvent> m_SoundEventPool;                                                           // 0x20
	Toshi::T2Map<Toshi::TPString8, TSIZE, Toshi::TPString8::Comparator> m_CategoryIndices;                             // 0x80
	Toshi::T2Map<TINT, ASoundAdvanced*> m_SoundIdToSoundEx;                                                            // 0x80
	Toshi::T2Map<TINT, ASound*> m_SoundIdToSound;                                                                      // 0x80
	CameraData m_CameraData;                                                                                           // 0xC8
	Cue m_aCues[ 128 ];                                                                                                // 0x108
	S2 m_aS2[ 8 ];                                                                                                     // 0x4D08
	// ...
	Category m_aCategories[ MAX_NUM_CATEGORIES ];                                                                      // 0x4E10
	TINT m_iLastAvailableSoundExSlot;                                                                                  // 0x4F50
	Toshi::T2SortedList<SoundEventList, Toshi::T2DList<SoundEventList>, SoundEventListSortResults> m_QueuedEventLists; // 0x4F54
	TFLOAT m_fCurrentTime;                                                                                             // 0x4F5C
	EventHandler m_aEventHandlers[ SOUNDEVENT_NUMOF ];                                                                 // 0x4F60
	TBOOL m_bMuted;                                                                                                    // 0x4FA8
	TBOOL m_bUseMinHardwareChannels;                                                                                   // 0x4FA9
	TINT m_iMinHWChannels;                                                                                             // 0x4FAC
	TINT m_iNumChannels;                                                                                               // 0x4FB0
	TINT m_iGlobalFrequency;                                                                                           // 0x4FB4
	PauseListener m_PauseListener;                                                                                     // 0x4FB8
	S4* m_pS4;                                                                                                         // 0x4FCC
	Toshi::T2DList<S4> m_FreeListS4;                                                                                   // 0x4FD0
	Toshi::T2DList<S4> m_UnkList1;                                                                                     // 0x4FD8
	Toshi::T2DList<ASoundBank> m_SoundBanks;                                                                           // 0x4FE0
	Toshi::T2DList<S2> m_FreeListS2;                                                                                   // 0x4FE8
	// ...
};
