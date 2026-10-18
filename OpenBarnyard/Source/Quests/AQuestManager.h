#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Toshi/TEvent.h>
#include <Toshi/TGlobalEmitter.h>
#include <Toshi/T2ObjectPool.h>
#include <Toshi/T2Vector.h>
#include <ToshiTools/T2DynamicVector.h>

#include "AQuest.h"
#include "AQuestInstance.h"
#include "GUI/AGUI2TextBox.h"
#include "Helpers/AGameTimeManager.h"
#include "MiniGame/AMiniGameState.h"
#include "Units/AInteractionManager.h"

class AQuest;
class AUnit;
class ASimHudGroup;

//-----------------------------------------------------------------------------
// Loads the quests (Data/QuestLst.trb, Data/Quests/*.trb) and the story
// progression (Data/Quests/tier_data.trb), assigns the quests, runs them and
// moves the story to the next tier once the quests of a tier are complete.
// The story is split into acts (1-3) made of tiers (up to 3 quests each).
// Also shows the "new quest" / "quest complete" / "quest failed" banner and
// plays the quest music.
//-----------------------------------------------------------------------------
class MODAPI AQuestManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AQuestManager>
{
public:
	TDECLARE_CLASS( AQuestManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_ASSIGNED_QUESTS = 1;
	static constexpr TINT MAX_NUM_TIERS           = 60;
	static constexpr TINT MAX_NUM_TIER_QUESTS     = 3;
	static constexpr TINT NUM_ACTS                = 4; // only 3 are used, the 4th one ends the game
	static constexpr TINT NUM_LATE_TEXTS          = 10;

	// The quests of a tier of the story
	struct QuestTier
	{
		TUINT8 auiQuests[ MAX_NUM_TIER_QUESTS ]; // 0x00, indices into m_vecQuests
		TUINT8 uiCompletedMask;                  // 0x03, a bit for every completed quest of auiQuests
		TUINT8 uiNumQuests;                      // 0x04
	};

	// State of the quest banner (m_oBannerText)
	using BANNERSTATE = TINT;
	enum BANNERSTATE_ : BANNERSTATE
	{
		BANNERSTATE_NEWQUEST          = 0, // a quest has been assigned
		BANNERSTATE_NEWQUEST_SHOW     = 1,
		BANNERSTATE_COMPLETE          = 4, // a quest has been completed
		BANNERSTATE_COMPLETE_SHOW     = 5,
		BANNERSTATE_FAILED            = 8, // a quest has failed
		BANNERSTATE_FAILED_SHOW       = 9,
		BANNERSTATE_FAILED_RETRY_SHOW = 10, // NOTE: never set
		BANNERSTATE_FINAL_SHOW        = 11,
		BANNERSTATE_NONE              = 14,
	};

	using InstancePool     = Toshi::T2ObjectPool<AQuestInstance, 1, 64, 4>;
	using DateListener     = Toshi::TListener<AGameTimeManager, AGameTimeManager::DateChangeEvent, AQuestManager>;
	using SequenceListener = Toshi::TGlobalListener<AQuestManager, AInteractionManagerEvent>;

public:
	AQuestManager();
	~AQuestManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Create() OVERRIDE;                        // +0x0c
	virtual TBOOL Reset() OVERRIDE;                         // +0x14
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x1c
	virtual void  OnActivate() OVERRIDE;                    // +0x30
	virtual void  OnDeactivate() OVERRIDE;                  // +0x34

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	virtual TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer ); // +0x38
	virtual TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer ); // +0x3c

	// Number of the tier a_iTier of the (1-based) act a_iAct counted over all the acts (1-based)
	TINT GetAbsQuestTier( TINT a_iAct, TINT a_iTier ) const;

	// GetAbsQuestTier of the current act and tier
	// $Barnyard: FUNCTION 0041ce30
	TINT GetCurAbsQuestTier() const { return GetAbsQuestTier( m_iAct, m_iTier ); }

	// Number of tiers of the (0-based) act a_iAct
	TINT GetNumTiersInAct( TINT a_iAct ) const;

	// The quest group of the current tier
	QuestTier* GetCurrentQuestTier();

	// The tier of the quest a_iQuest (index into m_vecQuests), a_pOutAct and a_pOutTier receive its act and tier
	QuestTier* FindQuestTier( TINT a_iQuest, TINT* a_pOutAct, TINT* a_pOutTier );
	QuestTier* FindQuestTier( const TCHAR* a_szQuest, TINT* a_pOutAct, TINT* a_pOutTier );
	QuestTier* FindQuestTier( AQuestInstance* a_pInstance, TINT* a_pOutAct, TINT* a_pOutTier );

	// Moves the story to the tier a_iTier of the act a_iAct
	void SetQuestTier( TINT a_iAct, TINT a_iTier, TBOOL a_bSkipCompleted );

	// Moves the story a_iNumTiers tiers forward
	void AdvanceQuestTier( TINT a_iNumTiers );

	// Assigns the quest called a_strName, given by a_pQuestGiver (can be TNULL)
	AQuestInstance* AssignQuest( AUnit* a_pQuestGiver, Toshi::TPString8 a_strName );

	// Ends the quest, a_bAdvance updates the current tier and moves to the next one once the tier is complete
	void RemoveQuest( AQuestInstance* a_pInstance, TBOOL a_bCompleted, TBOOL a_bAdvance );

	// Fails the assigned quest
	void AbandonQuest();

	// TRUE if the quest called a_rcName has been completed.
	TBOOL IsQuestComplete( const Toshi::TPString8& a_rcName, TINT a_iUnused ) const;

	// TRUE if a_pQuest of the current tier is complete, has been offered already or can't be done at this time of the day
	TBOOL IsQuestUnavailable( AQuest* a_pQuest );

	// The a_iIndex-th assigned quest, TNULL if there is none.
	AQuestInstance* GetAssignedQuest( TINT a_iIndex ) const;

	// The a_iOccurrence-th (1-based) assigned quest called a_rcName, TNULL if there is none.
	AQuestInstance* FindAssignedQuest( const Toshi::TPString8& a_rcName, TINT a_iOccurrence ) const;

	// Adds the markers of the elements of the assigned quests to ASimMiniMap
	void AddElementMarkers();

	// Hides the time left of a timed quest
	void HideTimeLeft();

	// Plays the music of a completed quest
	void PlayQuestCompleteMusic();

	// Plays the music of a failed quest
	void PlayQuestFailedMusic();

	// Lets the assigned quests know the results of a minigame (called by AMiniGameManager)
	TBOOL OnMiniGameResult( AMiniGameState::S1* a_pResults );

	// Lets the assigned quests set up the players of the minigame a_rcName (called by AMiniGameManager)
	TBOOL OnMiniGameStart( const Toshi::TPString8& a_rcName );

	// TRUE while the banner shows something
	TBOOL IsBannerShown() const;

	void SetBannerState( BANNERSTATE a_eState );

	// Stores the launcher input flags of APlayerControlComponent restored once a quest sequence is over
	static void SetSavedLauncherInput( TBOOL a_bEnabled );

	// Names of the quests the game checks for
	static const TCHAR* GetBarnDetailingQuest();
	static const TCHAR* GetFlowersForBenQuest();
	static const TCHAR* GetRoundUpChicksQuest();
	static const TCHAR* GetCoyoteTippingQuest();

#ifdef BARNYARD_LATEST
	// Latest version (JP Wii, Data/Quests/tier_data_JP.trb): the quests of act 1 tiers 2-5 are in another order
	// (Pip first, then Otis and the Pig, then Miles and Duke). LoadTierData puts the quests of the PC tier_data.trb in
	// that order (see ms_aLatestAct1Quests). Saves keep the PC order, see SaveCompletedQuests and LoadCompletedQuests.
	static constexpr TINT LATEST_ACT1_FIRST_TIER = 2;
	static constexpr TINT LATEST_ACT1_LAST_TIER  = 5;
	static constexpr TINT LATEST_ACT1_NUM_TIERS  = LATEST_ACT1_LAST_TIER - LATEST_ACT1_FIRST_TIER + 1;
	static constexpr TINT LATEST_ACT1_NUM_QUESTS = 8;

	struct LatestAct1Quest
	{
		const TCHAR* szQuest;
		TINT         iTier;   // act 1 tier of the latest version
		TINT         iPCTier; // act 1 tier of PC retail
	};

	// The quests of act 1 tiers 2-5 in the order of tier_data_JP.trb
	static const LatestAct1Quest ms_aLatestAct1Quests[ LATEST_ACT1_NUM_QUESTS ];

	// TRUE once LoadTierData has put the act 1 quests in the order of the latest version
	static TBOOL IsLatestAct1Order();

	// The act 1 tier of the quest a_szQuest in the latest version, a_iPCTier if the order doesn't change it
	static TINT GetLatestAct1Tier( const TCHAR* a_szQuest, TINT a_iPCTier );
#endif // BARNYARD_LATEST

	AQuest* GetQuest( TINT a_iIndex ) const
	{
		return m_vecQuests[ a_iIndex ];
	}
	TINT       GetNumQuests() const { return m_vecQuests.Size(); }
	TINT       GetNumAssignedQuests() const { return m_vecAssignedQuests.Size(); }
	TINT       GetAct() const { return m_iAct; }
	TINT       GetTier() const { return m_iTier; }
	TINT       GetNumTiers() const { return m_iNumTiers; }
	QuestTier* GetQuestTier( TINT a_iAbsTier ) { return &m_aTiers[ a_iAbsTier - 1 ]; }
	TBOOL      IsTimerPaused() const { return m_bTimerPaused; }
	TBOOL      IsMissionMusicPlaying() const { return m_bMissionMusicPlaying; }
	TINT       GetQuestMusicCue() const { return m_iQuestMusicCue; }
	TBOOL      IsGameComplete() const { return m_bGameComplete; }
	TUINT8     GetOfferedQuests() const { return m_uiOfferedQuests; }
	void       SetOfferedQuests( TUINT8 a_uiMask ) { m_uiOfferedQuests = a_uiMask; }
	void       SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

private:
	// Reads Data/Quests/a_szFileName.trb
	void LoadQuests( const TCHAR* a_szFileName );

	// Reads Data/Quests/tier_data.trb
	void LoadTierData();

	// Ends every assigned quest and forgets the completed ones
	void ClearAssignedQuests();

	// Reads the assigned quests of a quick save
	TBOOL LoadAssignedQuests( AHackedQuickSave::Buffer* a_pBuffer, Toshi::T2Vector<AQuestInstance*, MAX_NUM_ASSIGNED_QUESTS>& a_rQuests, TBOOL a_bSpawnUnits );

	TBOOL SaveCompletedQuests( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL LoadCompletedQuests( AHackedQuickSave::Buffer* a_pBuffer );

#ifdef BARNYARD_LATEST
	// Puts the quests of act 1 tiers 2-5 in the order of the latest version (called by LoadTierData)
	void ApplyLatestAct1Order();

	// The completed mask of a_rcTier: the bits of its quests that are completed in m_aTiers (a_pCompletedTiers TNULL)
	// or in a_pCompletedTiers (act 1 tiers 2-5 of the other order)
	TUINT8 GetAct1CompletedMask( const QuestTier& a_rcTier, const QuestTier* a_pCompletedTiers ) const;

	// The first act 1 tier between 2 and 5 of a_pTiers (tiers 2-5) that has a quest left, LATEST_ACT1_LAST_TIER + 1 if none
	static TINT GetFirstIncompleteAct1Tier( const QuestTier* a_pTiers );

	// TRUE if every quest of a_rcTier is completed
	static TBOOL IsTierComplete( const QuestTier& a_rcTier );
#endif // BARNYARD_LATEST

	// Lets the quest givers of the tier a_iAbsTier offer their quests (AQuest::FLAGS a_uiFlags, 0 for all of them)
	void OfferTierQuests( TINT a_iAbsTier, AQuest::FLAGS a_uiFlags );

	// Removes the offers of the quests of the tier a_iAbsTier
	void RemoveTierQuestOffers( TINT a_iAbsTier );

	void UpdateBanner( TFLOAT a_fDeltaTime );

	// Fades the banner in, keeps it and fades it out
	void SetBannerFade( TFLOAT a_fTime, TFLOAT a_fFadeIn, TFLOAT a_fHold, TFLOAT a_fFadeOut );

	// Shows a_wszText on the banner
	void SetBannerText( const TWCHAR* a_wszText );

	// Changes the spawns of the quest givers once the time of the day changes
	TBOOL OnDateChange( AGameTimeManager::DateChangeEvent* a_pEvent );

	// Pauses the quest timers and the game time during the quest sequences
	void OnSequenceEvent( const AInteractionManagerEvent& a_rcEvent );

	static TBOOL __stdcall OnDateChangeCallback( AQuestManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent );
	static void OnSequenceEventCallback( void* a_pCaller, const AInteractionManagerEvent& a_rcEvent );

private:
	InstancePool                                              m_oInstancePool;           // 0x20
	Toshi::T2DynamicVector<AQuest*>                           m_vecQuests;               // 0x68
	Toshi::T2Vector<AQuestInstance*, MAX_NUM_ASSIGNED_QUESTS> m_vecAssignedQuests;       // 0x7c
	TINT                                                      m_iNextInstanceId;         // 0x84
	QuestTier                                                 m_aTiers[ MAX_NUM_TIERS ]; // 0x88
	TUINT8                                                    m_auiNumTiers[ NUM_ACTS ]; // 0x1b4, tiers of every act
	TINT                                                      m_iTier;                   // 0x1b8, 1-based
	TINT                                                      m_iAct;                    // 0x1bc, 1-based
	TINT                                                      m_iNumTiers;               // 0x1c0
	TINT                                                      m_iQuestMusicCue;          // 0x1c4
	TINT                                                      m_iWeatherCue;             // 0x1c8
	TBOOL                                                     m_bEnabled;                // 0x1cc
	DateListener                                              m_oDateListener;           // 0x1d0
	Toshi::TGenericListener                                   m_oUnkListener;            // 0x1e4, never connected
	SequenceListener*                                         m_pSequenceListener;       // 0x1f8
	TBOOL                                                     m_bTimerPaused;            // 0x1fc, a quest sequence is running
	TBOOL                                                     m_bMissionMusicPlaying;    // 0x1fd, AMusicManager plays a mission stream
	TUINT8                                                    m_uiOfferedQuests;         // 0x1fe, a bit for every quest of the current tier that has been offered
	AGUI2TextBox                                              m_oBannerText;             // 0x200
	TBOOL                                                     m_bGameComplete;           // 0x278, the story reached the 4th act
	ASimHudGroup*                                             m_pSimHudGroup;            // 0x27c, of ASimState while the time left of a timed quest is shown
	BANNERSTATE                                               m_eBannerState;            // 0x280
	TBOOL                                                     m_bBannerComplete;         // 0x284, show the "quest complete" text next
	TBOOL                                                     m_bBannerDayQuest;         // 0x285
	TBOOL                                                     m_bBannerGameEvent;        // 0x286
	TFLOAT                                                    m_fBannerTime;             // 0x288
	TBOOL                                                     m_bPlayCompleteMusic;      // 0x28c, waits for the music to be loaded
	TBOOL                                                     m_bPlayFailedMusic;        // 0x28d, waits for the music to be loaded
};

TSTATICASSERT( sizeof( AQuestManager::QuestTier ) == 0x5 );
TSTATICASSERT( sizeof( AQuestManager ) == 0x290 );
