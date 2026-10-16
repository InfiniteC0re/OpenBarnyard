#include "pch.h"
#include "AQuestManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/AMultiplexStream.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/ASimPersistData.h"
#include "AQuest.h"
#include "AQuestElement.h"
#include "AGameEventQuestElement.h"
#include "ALoadScreen.h"
#include "ACheatActivator.h"
#include "GUI/AGUI2.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "GameInterface/ASimState.h"
#include "HUD/ASimHudGroup.h"
#include "HUD/AHudManager.h"
#include "HUD/AItemCountHudElement.h"
#include "HUD/ATimeLeftHudElement.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AWeatherManager.h"
#include "FX/AGameTimeFXManager.h"
#include "SoundBank/ui.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "Tasks/ARootTask.h"
#include "Units/AAnimalPopulationManager.h"
#include "Units/ANPCBehaviourManager.h"
#include "Units/APlayerControlComponent.h"
#include "Units/AUnit.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitType.h"
#include "Units/AInteractionQuestEvent.h"
#include "Units/AInteractionEventManager.h"

#include <File/TTRB.h>
#include <GUI/T2GUIFontManager.h>
#include <T2Locale/T2Locale.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TString8.h>
#include "Helpers/AReplayManager.h"
#include "MiniGame/ABarnExploreMinigameState.h"
#include "Units/AScriptedSequenceActivity.h"
#include "GameInterface/AHackedQuickSave.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005cbe30, CreateObject 005cf160, CreateObjectInPlace 005cf1c0
TDEFINE_CLASS( AQuestManager );

TPSTRING8_DECLARE( music );
TPSTRING8_DECLARE( ASleepCycleState );
TPSTRING8_DECLARE( ABarnRewardState );
TPSTRING8_DECLARE( ABarnExploreMinigameState );
TPSTRING8_DECLARE( ASimState );

// Text the quest givers say when the time to do their quest is running out
// 0x0072ce40
static const TINT s_aLateTexts[ AQuestManager::NUM_LATE_TEXTS ] = { 3209, 3210, 3211, 3212, 3213, 3214, 3215, 3216, 3217, 3218 };

// Scripted sequence played when the story reaches a tier (absolute tier - 1)
struct TierSequence
{
#ifdef BARNYARD_LATEST
	// eDayPhase: the time of the day doesn't change
	static constexpr TINT DAYPHASE_KEEP = 6;
#endif

	TINT iSequence;
	TINT iTier;
#ifdef BARNYARD_LATEST
	// Wii: where the player starts and the time of the day once the sequence is over
	const TCHAR* szStartGoal; // a TPString8* in the Wii table, TNULL keeps the start goal
	TINT         iVISGroup;   // NOTE: not used
	TBOOL        bBarn;       // the sequence returns to ABarnExploreMinigameState instead of ASimState
	TINT         eDayPhase;   // AGameTimeManager::DAYPHASE or DAYPHASE_KEEP
#endif
};

#ifdef BARNYARD_LATEST

// 0x804f5c38 (Wii)
// NOTE: "near_barn_entrance" is an area of the park2 network, not a goal: AInteriorNetwork::FindGoal fails
// (asserts in debug) and the player is placed at goal 0
static const TierSequence s_aTierSequences[] = {
	{ 1, 11, TNULL, -1, TFALSE, TierSequence::DAYPHASE_KEEP },
	{ 2, 16, "near_barn_entrance", 0, TFALSE, TierSequence::DAYPHASE_KEEP },
	{ 3, 33, "barn_entrance", -1, TTRUE, AGameTimeManager::DAYPHASE_NIGHT },
	{ 4, 34, "eddie_hill", 28, TFALSE, AGameTimeManager::DAYPHASE_MORNING },
	{ AScriptedSequenceActivity::SEQUENCE_SS06_STORYMERGED, 36, "near_barn_entrance", 0, TFALSE, TierSequence::DAYPHASE_KEEP }, // SS06_story on PC
	{ 6, 45, "otis_beady", 2, TFALSE, AGameTimeManager::DAYPHASE_NIGHT },
	{ 8, 49, "near_barn_entrance", 0, TFALSE, AGameTimeManager::DAYPHASE_DAWN },
	{ 9, 51, "hidentwink05", 0, TFALSE, AGameTimeManager::DAYPHASE_DAWN },
};

#else

// 0x0072ce00
static const TierSequence s_aTierSequences[] = {
	{ 1, 11 },
	{ 2, 16 },
	{ 3, 33 },
	{ 4, 34 },
	{ 5, 36 },
	{ 6, 45 },
	{ 8, 49 },
	{ 9, 51 },
};

#endif // BARNYARD_LATEST

// 0x0077539c - 0x007753a8
static const TCHAR* s_szBarnDetailingQuest = "Root (Night) Barn Detailing";
static const TCHAR* s_szFlowersForBenQuest = "Daisy Flowers For Ben";
static const TCHAR* s_szRoundUpChicksQuest = "Twink Round Up Chicks";
static const TCHAR* s_szCoyoteTippingQuest = "Miles Coyote Tipping";

static TFLOAT s_fBannerDelay = 0.5f; // 0x007753ac

static TBOOL         s_bSleepCycle                  = TFALSE; // 0x0078c094, the time of the day changed during ASleepCycleState
static TINT          s_iLateText                    = 0;      // 0x0078c098
static TFLOAT        s_fTimeSinceLateText           = 0.0f;   // 0x0078c09c
static TBOOL         s_bSavedSecondaryLauncherInput = TFALSE; // 0x0078c0a0
static TBOOL         s_bSavedLauncherInput          = TFALSE; // 0x0078c0a1
static TBOOL         s_bGameTimeWasStopped          = TFALSE; // 0x0078c0d8
static const TWCHAR* s_wszQuestFailed               = TNULL;  // 0x0078c0dc

#ifdef BARNYARD_LATEST
// Wii: set while the time of the day is changed on purpose (the birthday cake sequence, some minigames), see OnDateChange
static TBOOL s_bForcedDayPhaseChange = TFALSE; // 0x806b507d (Wii)
// Wii: the accepted quest isn't abandoned by the day phase change. Set together with s_bForcedDayPhaseChange by the Wii
// minigame code (FUN_8018867c, AMiniGameManager) that is not recovered yet
static TBOOL s_bKeepQuestOnDayPhaseChange = TFALSE; // 0x806b507e (Wii)

// Wii: set once "Ben Tomato Launcher" is done, AQuestManager::OnUpdate starts the tomato launcher sequence (SSUL_Main)
static TBOOL s_bStartTomatoLauncherSequence = TFALSE; // 0x806b5088 (Wii)

// Data/Quests/tier_data_JP.trb (Wii): the quests of act 1 tiers 2-5, in the order of the file
const AQuestManager::LatestAct1Quest AQuestManager::ms_aLatestAct1Quests[ LATEST_ACT1_NUM_QUESTS ] = {
	{ "Ben Welcome", 2, 2 },
	{ "Pip Barnyard Secrets", 2, 3 },
	{ "Otis Pest Squirting", 3, 4 },
	{ "Pig Apple Pie", 3, 2 },
	{ "Miles Target Squirting 1", 4, 5 },
	{ "Duke Farmer Alert", 4, 2 },
	{ "Pig Mud Jumpers", 5, 5 },
	{ "Hanna Missing Eggs", 5, 5 },
};

// NOTE: not in the original. Set by ApplyLatestAct1Order once the act 1 quests are in the order of the latest version
// (it stays TFALSE if the tier data doesn't have the expected quests, then the PC order is kept)
static TBOOL s_bLatestAct1Order = TFALSE;

// NOTE: not in the original. Act 1 tiers 2-5 in the PC order (their completed masks aren't used), the order of the saves
static AQuestManager::QuestTier s_aPCAct1Tiers[ AQuestManager::LATEST_ACT1_NUM_TIERS ];

#  ifdef BARNYARD_COMMUNITY_PATCH
// NOTE: the PC data has none of the Wii scripted sequences and ASSeqState crashes on a missing one,
// so they're only played once the converted Wii files are added to the game data.
// a_szTerrain: the Wii terrain the sequence plays in (Data/Terrain/<terrain>/<terrain>.trb), which must be there too
static TBOOL IsScriptedSequenceAvailable( const TCHAR* a_szName, const TCHAR* a_szTerrain = TNULL )
{
	TString8 strFileName;
	strFileName.Format( "Data/ScriptedSequence/%s.tso", a_szName );

	TFile* pFile = TFile::Create( strFileName );

	if ( pFile && a_szTerrain )
	{
		pFile->Destroy();
		strFileName.Format( "Data/Terrain/%s/%s.trb", a_szTerrain, a_szTerrain );
		pFile = TFile::Create( strFileName );
	}

	if ( !pFile )
	{
		TWARN( "AQuestManager: %s is missing, the Wii scripted sequence %s is skipped\n", strFileName.GetString(), a_szName );
		return TFALSE;
	}

	pFile->Destroy();
	return TTRUE;
}
#  endif // BARNYARD_COMMUNITY_PATCH
#endif   // BARNYARD_LATEST

// $Barnyard: FUNCTION 005ce380
AQuestManager::AQuestManager()
    : m_vecQuests( GetGlobalAllocator(), 32, 8 )
    , m_iNextInstanceId( 1 )
    , m_iTier( 1 )
    , m_iAct( 1 )
    , m_iNumTiers( 0 )
    , m_iQuestMusicCue( -1 )
    , m_iWeatherCue( -1 )
    , m_bEnabled( TTRUE )
    , m_bTimerPaused( TFALSE )
    , m_bMissionMusicPlaying( TFALSE )
    , m_uiOfferedQuests( 0 )
    , m_bGameComplete( TFALSE )
    , m_pSimHudGroup( TNULL )
    , m_eBannerState( BANNERSTATE_NONE )
    , m_bBannerComplete( TFALSE )
    , m_bBannerDayQuest( TFALSE )
    , m_bBannerGameEvent( TFALSE )
    , m_fBannerTime( 0.0f )
    , m_bPlayCompleteMusic( TFALSE )
    , m_bPlayFailedMusic( TFALSE )
{
	for ( TINT i = 0; i < MAX_NUM_TIERS; i++ )
	{
		m_aTiers[ i ].uiCompletedMask = 0;
		m_aTiers[ i ].uiNumQuests     = 0;
	}

	TUtil::MemClear( m_auiNumTiers, 3 );

	m_pSequenceListener = new SequenceListener();
}

// $Barnyard: FUNCTION 005cf200
// $Barnyard: FUNCTION 005cf340
AQuestManager::~AQuestManager()
{
	m_oBannerText.RemoveSelf();
	ClearAssignedQuests();

	for ( TINT i = 0; i < m_vecQuests.Size(); i++ )
	{
		if ( m_vecQuests[ i ] )
			delete m_vecQuests[ i ];
	}

	m_vecQuests.Clear();
	m_oDateListener.Disconnect();

	if ( m_pSequenceListener )
	{
		m_pSequenceListener->Disconnect();
		delete m_pSequenceListener;
	}

	m_pSimHudGroup = TNULL;
	m_oUnkListener.Disconnect();
}

// $Barnyard: FUNCTION 005cef80
TBOOL AQuestManager::Create()
{
	TTRB oTRB;

	if ( oTRB.Load( "Data/QuestLst.trb" ) != TTRB::ERROR_OK )
		return TFALSE;

	g_oLoadScreen.Update( 1.0f, TTRUE );

	const PBProperties* pMain = TSTATICCAST( const PBProperties, oTRB.GetSymbolAddress( "Main" ) );
	oTRB.DeleteSymbolTable();

	const PBPropertyValue* pFiles = pMain->GetProperty( "files" );

	if ( pFiles && pFiles->GetArray() )
	{
		const PBPropertyValueArray* pFileArray = pFiles->GetArray();
		TINT                        iNumFiles  = pFileArray->GetSize();

		for ( TINT i = 0; i < iNumFiles; i++ )
		{
			LoadQuests( pFileArray->GetValue( i )->GetString() );
			g_oLoadScreen.Update( 1.0f, TTRUE );
		}
	}

	oTRB.Close();
	LoadTierData();

	m_oDateListener.Connect( *AGameTimeManager::GetSingleton()->GetDateChangeEmitter(), this, OnDateChangeCallback, 0 );
	m_pSequenceListener->Connect( this, OnSequenceEventCallback );

	// The banner
	m_oBannerText.RemoveSelf();
	m_oBannerText.Create( T2GUIFontManager::FindFont( "Rekord26" ), 50.0f );
	m_oBannerText.SetAttachment( T2GUIATTACHMENT_MIDDLECENTER, T2GUIATTACHMENT_MIDDLECENTER );
	AGUI2::GetRootElement()->AddChildTail( m_oBannerText );
	m_oBannerText.SetAlpha( 0.0f );

	m_eBannerState    = BANNERSTATE_NONE;
	m_bBannerComplete = TFALSE;

	return TTask::Create();
}

// $Barnyard: FUNCTION 005ccc40
TBOOL AQuestManager::Reset()
{
	m_iNextInstanceId = 1;
	m_iTier           = 1;
	m_iAct            = 1;
	ClearAssignedQuests();

	APlayerProfileManager::GetSingleton()->GetMainProfile()->ReleaseAllItems();

	m_uiOfferedQuests      = 0;
	m_bGameComplete        = TFALSE;
	m_bTimerPaused         = TFALSE;
	m_bMissionMusicPlaying = TFALSE;
	m_pSimHudGroup         = TNULL;
	m_bPlayCompleteMusic   = TFALSE;
	m_bPlayFailedMusic     = TFALSE;
	m_iQuestMusicCue       = -1;

	if ( m_iWeatherCue != -1 )
	{
		if ( !AGameLoader::IsMainGameLoaded() && ASoundManager::GetSingleton()->IsCueUsed( m_iWeatherCue ) )
			ASoundManager::GetSingleton()->StopCue( m_iWeatherCue );

		m_iWeatherCue = -1;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005cc020
void AQuestManager::OnActivate()
{
	m_oBannerText.Show();
}

// $Barnyard: FUNCTION 005cc030
void AQuestManager::OnDeactivate()
{
	m_oBannerText.Hide();
}

// $Barnyard: FUNCTION 005ce520
TBOOL AQuestManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnabled )
		return TTRUE;

	UpdateBanner( a_fDeltaTime );

	// The music of a completed/failed quest is over
	if ( m_iQuestMusicCue != -1 && !ASoundManager::GetSingleton()->IsCueUsed( m_iQuestMusicCue ) )
	{
		if ( !AGameStateController::GetSingleton()->GetCurrentState()->IsA( &TGetClass( ASSeqState ) ) )
		{
			m_bMissionMusicPlaying = m_bTimerPaused == TTRUE;

			if ( AGameLoader::IsBarnExploreMiniGame() )
			{
				AMultiplexStream::GetSingleton()->Pause( TFALSE );
			}
			else
			{
				ASoundManager::GetSingleton()->UnloadWaveBankSamples( TPS8( music ) );

				AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
			}

			if ( m_iWeatherCue != -1 )
			{
				ASoundManager::GetSingleton()->FadeOutCue( m_iWeatherCue, 2.0f, TTRUE, 3.0f );
				m_iWeatherCue = -1;
			}

			m_iQuestMusicCue = -1;
		}
	}

	// Nothing to do in the current tier
	if ( GetCurrentQuestTier()->uiNumQuests == 0 )
		AdvanceQuestTier( 1 );

	// Run the assigned quests
	TBOOL                                   bQuestCompleted = TFALSE;
	T2Vector<TINT, MAX_NUM_ASSIGNED_QUESTS> vecRunningQuests;

	for ( auto it = m_vecAssignedQuests.Begin(); m_vecAssignedQuests.Size() != 0 && it != m_vecAssignedQuests.End(); )
	{
		AQuestInstance* pInstance = *it;
		pInstance->Update( a_fDeltaTime );

		if ( pInstance->GetState() == AQuestInstance::STATE_COMPLETE )
		{
			RemoveQuest( pInstance, TTRUE, TTRUE );
			bQuestCompleted = TTRUE;
			PlayQuestCompleteMusic();
			m_eBannerState = BANNERSTATE_COMPLETE;
		}
		else if ( pInstance->GetState() == AQuestInstance::STATE_FAILED )
		{
			AQuest* pQuest = pInstance->GetQuest();

			// Minigame quests can be retried
			TBOOL bIsGameEvent = TFALSE;

			for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
			{
				if ( pQuest->GetElement( i )->GetClass() == &TGetClass( AGameEventQuestElement ) )
					bIsGameEvent = TTRUE;
			}

			APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

			if ( bIsGameEvent )
			{
				pProfile->AddUsedItem( pQuest->GetTitle(), pQuest->GetQuestGiver(), pQuest->GetTitleId(), TFALSE );
			}
			else
			{
				if ( !s_wszQuestFailed )
					s_wszQuestFailed = T2Locale::GetSingleton()->GetString( 235 );

				pProfile->AddUsedItem( s_wszQuestFailed, pQuest->GetQuestGiver(), 235, TTRUE );
			}

			if ( pInstance->GetQuest() )
			{
				m_bBannerDayQuest  = ( pInstance->GetQuest()->GetFlags() & AQuest::FLAGS_DAY ) == AQuest::FLAGS_DAY;
				m_bBannerGameEvent = bIsGameEvent;
				m_eBannerState     = BANNERSTATE_FAILED;
			}

			RemoveQuest( pInstance, TFALSE, TTRUE );
			PlayQuestFailedMusic();
		}
		else
		{
			vecRunningQuests.PushBack( pInstance->GetQuestIndex() );
			++it;
		}
	}

	// Let the quest givers tell the player when the time to do their quest is running out
	TFLOAT fTimeLeft = -1.0f;
	s_fTimeSinceLateText += a_fDeltaTime;

	if ( AGameLoader::IsMainGameLoaded() && !AGameLoader::IsBarnExploreMiniGame() && !AGameLoader::IsReturningToFrontEnd() )
	{
		AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
		QuestTier*        pTier            = GetCurrentQuestTier();
		TFLOAT            fDayTime         = pGameTimeManager->GetDayTime();

		for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
		{
			TINT    iQuest = pTier->auiQuests[ i ];
			AQuest* pQuest = m_vecQuests[ iQuest ];

			if ( pQuest->GetName() == TPS8D( "DUMMY QUEST" ) )
				break;

			TBOOL bAssigned = m_vecAssignedQuests.Size() > 0 && m_vecAssignedQuests[ 0 ]->GetQuest() == pQuest;

			TFLOAT fTimeLimit = TMath::Max( pQuest->GetTimeLimit(), 5.0f );

			if ( bAssigned )
				fTimeLimit = 0.0f;

			// The time the quest has to be started before
			TFLOAT fStartBefore = 0.0f;

			if ( pQuest->GetFlags() & AQuest::FLAGS_DAY )
				fStartBefore = AGameTimeManager::GetDayPhaseStartTime( AGameTimeManager::DAYPHASE_NIGHT ) - fTimeLimit;
			else if ( pQuest->GetFlags() & AQuest::FLAGS_NIGHT )
				fStartBefore = AGameTimeManager::GetDayPhaseStartTime( AGameTimeManager::DAYPHASE_DAWN ) - fTimeLimit;

			if ( fStartBefore >= 1200.0f )
				fStartBefore = 0.0f;

			if ( bAssigned )
			{
				TFLOAT fElapsed = fDayTime - m_vecAssignedQuests[ 0 ]->GetTime();

				if ( AGameTimeManager::DAY_START_SECONDS * AGameTimeManager::REAL_TIME_TO_GAME_TIME > fDayTime )
					fElapsed += 1200.0f;

				if ( pQuest->GetFlags() & AQuest::FLAGS_DAY )
					fTimeLeft = fStartBefore - fElapsed;
				else if ( pQuest->GetFlags() & AQuest::FLAGS_NIGHT )
					fTimeLeft = ( 1200.0f - fElapsed ) + fStartBefore;
			}

			if ( !s_bSleepCycle )
			{
				// The quest can't be done at this time of the day
				if ( ( pQuest->GetFlags() & ( AQuest::FLAGS_DAY | AQuest::FLAGS_NIGHT ) ) == ( AQuest::FLAGS_DAY | AQuest::FLAGS_NIGHT ) ||
				     ( ( pQuest->GetFlags() & AQuest::FLAGS_DAY ) && pGameTimeManager->IsItNight() ) ||
				     ( ( pQuest->GetFlags() & AQuest::FLAGS_NIGHT ) && AGameTimeManager::GetDayPhaseStartTime( AGameTimeManager::DAYPHASE_MORNING ) < fDayTime ) )
				{
					continue;
				}
			}

			TUINT8 uiBit = TUINT8( 1 << i );

			TBOOL bLate = vecRunningQuests.Find( iQuest ) == vecRunningQuests.End() &&
			    ( pTier->uiCompletedMask & uiBit ) == 0 &&
			    fStartBefore < fDayTime &&
			    ( uiBit & m_uiOfferedQuests ) == 0;

			if ( !bLate )
				continue;

			m_uiOfferedQuests |= uiBit;

			AQuest::FLAGS uiSpawnFlags = pGameTimeManager->IsItNight() ? AQuest::FLAGS_NIGHT : AQuest::FLAGS_DAY;

			if ( s_bSleepCycle )
			{
				if ( pGameTimeManager->IsItNight() && ( pQuest->GetFlags() & AQuest::FLAGS_DAY ) )
					uiSpawnFlags = AQuest::FLAGS_DAY;
				else if ( !pGameTimeManager->IsItNight() && ( pQuest->GetFlags() & AQuest::FLAGS_NIGHT ) )
					uiSpawnFlags = AQuest::FLAGS_NIGHT;
			}

			AAnimalPopulationManager::GetSingleton()->ClearQuestTier( GetCurAbsQuestTier(), pQuest->GetName().GetString(), uiSpawnFlags );

			if ( pQuest->GetFlags() & AQuest::FLAGS_NIGHT )
				continue;

			// Don't tell it twice
			APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
			TBOOL              bTold    = TFALSE;

			for ( TINT k = 1; k <= TINT( pProfile->GetNumUsedItems() ); k++ )
			{
				APlayerSimProfile::Item* pItem = pProfile->GetUsedItem( k );

				if ( pItem && pItem->m_strName == pQuest->GetQuestGiver() )
				{
					for ( TINT n = 0; n < NUM_LATE_TEXTS; n++ )
					{
						if ( pItem->m_iTextId == s_aLateTexts[ n ] )
						{
							bTold = TTRUE;
							break;
						}
					}
				}

				if ( bTold )
					break;
			}

			if ( bTold )
				continue;

			pProfile->ReleaseItem( pQuest->GetQuestGiver() );

			if ( ++s_iLateText >= NUM_LATE_TEXTS )
				s_iLateText = 0;

			const TWCHAR* wszText  = T2Locale::GetSingleton()->GetString( s_aLateTexts[ s_iLateText ] );
			TBOOL         bShowHud = !s_bSleepCycle && s_fTimeSinceLateText > 10.0f && m_vecAssignedQuests.Size() == 0;

			pProfile->AddUsedItem( wszText, pQuest->GetQuestGiver(), s_aLateTexts[ s_iLateText ], bShowHud );

			if ( bShowHud )
				s_fTimeSinceLateText = 0.0f;
		}

		s_bSleepCycle = TFALSE;

#ifdef BARNYARD_LATEST
		// Wii: the tomato launcher sequence starts once the player isn't busy with an interaction any more
		if ( s_bStartTomatoLauncherSequence )
		{
			ASimState* pSimState   = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );
			AUnit*     pPlayerUnit = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

			if ( pPlayerUnit && !pPlayerUnit->HasInteraction() && AGameLoader::IsMainGameLoaded() )
			{
				pSimState->StartTomatoLauncherSequence();
				s_bStartTomatoLauncherSequence = TFALSE;
			}
		}
#endif // BARNYARD_LATEST
	}

	// The time left of the assigned quest
	if ( m_bTimerPaused )
	{
		if ( m_pSimHudGroup )
			m_pSimHudGroup->GetTimeLeftElement()->GetTimer().StopCue();
	}
	else if ( !AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASleepCycleState ) ) )
	{
		if ( m_vecAssignedQuests.Size() <= 0 || fTimeLeft <= 0.0f )
		{
			HideTimeLeft();
		}
		else
		{
			AQuestInstance* pInstance = m_vecAssignedQuests[ 0 ];

			if ( !m_pSimHudGroup )
			{
				ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );
				m_pSimHudGroup       = pSimState->GetSimHudGroup();
				m_pSimHudGroup->SetShowTimer( TTRUE );
				m_pSimHudGroup->GetTimeLeftElement()->Setup( fTimeLeft );
				m_pSimHudGroup->GetTimeLeftElement()->SetTime( fTimeLeft );
			}

			m_pSimHudGroup->GetTimeLeftElement()->SetTime( fTimeLeft - pInstance->GetTime() );

			if ( fTimeLeft - pInstance->GetTime() < 0.0f )
			{
				m_pSimHudGroup->SetShowTimer( TFALSE );
				m_pSimHudGroup->GetTimeLeftElement()->GetTimer().Reset();
			}
		}
	}

	if ( bQuestCompleted == TTRUE )
	{
		ANPCBehaviourManager::GetSingleton()->RequestScheduleUpdate();

		AReplayManager::GetSingleton()->UpdateReplays();
	}

	// Play the music of the quest once it's been loaded
	if ( !ARootTask::GetSingleton()->IsPaused() && ( m_bPlayCompleteMusic == TTRUE || m_bPlayFailedMusic == TTRUE ) )
	{
		// Wait for the stream of the main game to be closed and for the sim to be done with its sequences
		ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

		if ( AMultiplexStream::GetSingleton()->GetState() == AMultiplexStream::STATE_CLOSED && !pSimState->IsRestartingMusic() )
		{
			ASoundManager::GetSingleton()->LoadWaveBankSamples( TPS8( music ), 0, -1 );
			ASoundManager::GetSingleton()->LoadSoundBankSamples( TPS8( music ) );
			m_iQuestMusicCue     = ASoundManager::GetSingleton()->PlayCue( ( m_bPlayCompleteMusic == TTRUE ) ? 2 : 3 );
			m_bPlayCompleteMusic = TFALSE;
			m_bPlayFailedMusic   = TFALSE;
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005cce20
TBOOL AQuestManager::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "QuestManager" );
	a_pBuffer->WriteInt( m_iNextInstanceId );

	TINT iNumAssigned = m_vecAssignedQuests.Size();
	a_pBuffer->WriteInt( iNumAssigned );

	for ( TINT i = 0; i < iNumAssigned; i++ )
		m_vecAssignedQuests[ i ]->Save( a_pBuffer );

#ifdef BARNYARD_LATEST
	// NOTE: saves keep the PC order of act 1 (see SaveCompletedQuests), the saved tier is the first PC tier with a quest left
	TINT iSavedTier = m_iTier;

	if ( s_bLatestAct1Order && m_iAct == 1 && m_iTier >= LATEST_ACT1_FIRST_TIER && m_iTier <= LATEST_ACT1_LAST_TIER )
	{
		QuestTier aPCTiers[ LATEST_ACT1_NUM_TIERS ];

		for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
		{
			aPCTiers[ i ]                 = s_aPCAct1Tiers[ i ];
			aPCTiers[ i ].uiCompletedMask = GetAct1CompletedMask( s_aPCAct1Tiers[ i ], TNULL );
		}

		iSavedTier = GetFirstIncompleteAct1Tier( aPCTiers );
	}

	a_pBuffer->WriteInt( iSavedTier );
#else
	a_pBuffer->WriteInt( m_iTier );
#endif
	a_pBuffer->WriteInt( m_iAct );
	a_pBuffer->WriteInt( m_iNumTiers );
	a_pBuffer->WriteBool( m_bGameComplete );

	// Don't show the "new quest" banner again
	if ( TUINT( m_eBannerState ) < BANNERSTATE_COMPLETE )
	{
		m_eBannerState    = BANNERSTATE_NONE;
		m_bBannerComplete = TTRUE;
	}

	a_pBuffer->WriteInt( m_eBannerState );
	a_pBuffer->WriteBool( m_bBannerComplete );
	SaveCompletedQuests( a_pBuffer );

	return TTRUE;
}

// $Barnyard: FUNCTION 005ccf00
TBOOL AQuestManager::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "QuestManager" );
	a_pBuffer->ReadInt( m_iNextInstanceId );
	LoadAssignedQuests( a_pBuffer, m_vecAssignedQuests, TFALSE );
	a_pBuffer->ReadInt( m_iTier );
	a_pBuffer->ReadInt( m_iAct );
	a_pBuffer->ReadInt( m_iNumTiers );
	a_pBuffer->ReadBool( m_bGameComplete );
	a_pBuffer->ReadInt( m_eBannerState );
	a_pBuffer->ReadBool( m_bBannerComplete );
	LoadCompletedQuests( a_pBuffer );

#ifdef BARNYARD_LATEST
	// NOTE: saves have the PC order of act 1 (LoadCompletedQuests converts the completed quests), the story continues at
	// the first tier of the latest order with a quest left. A save of this version comes back exactly as it was saved.
	if ( s_bLatestAct1Order && m_iAct == 1 && m_iTier >= LATEST_ACT1_FIRST_TIER && m_iTier <= LATEST_ACT1_LAST_TIER )
	{
		TINT iTier = GetFirstIncompleteAct1Tier( &m_aTiers[ LATEST_ACT1_FIRST_TIER - 1 ] );

		if ( iTier != m_iTier )
			TINFO( "AQuestManager: the saved act 1 tier %d (PC order) is tier %d of the latest order\n", m_iTier, iTier );

		m_iTier = iTier;
	}
#endif

	ACheatActivator::GetSingleton()->UpdateQuestTier();
	ACheatActivator::GetSingleton()->UpdateTierRequirements();

	return TTRUE;
}

// $Barnyard: FUNCTION 005cc890
TBOOL AQuestManager::LoadAssignedQuests( AHackedQuickSave::Buffer* a_pBuffer, T2Vector<AQuestInstance*, MAX_NUM_ASSIGNED_QUESTS>& a_rQuests, TBOOL a_bSpawnUnits )
{
	TINT iNumQuests = 0;
	a_pBuffer->ReadInt( iNumQuests );

	for ( TINT i = 0; i < iNumQuests; i++ )
	{
		AQuestInstance* pInstance = m_oInstancePool.NewObject();
		pInstance->Load( a_pBuffer, a_bSpawnUnits );
		a_rQuests.PushBack( pInstance );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005cc900
TBOOL AQuestManager::SaveCompletedQuests( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < m_iNumTiers; i++ )
	{
#ifdef BARNYARD_LATEST
		// NOTE: act 1 is saved in the PC order, so PC retail and this version read the same quests as completed
		if ( s_bLatestAct1Order && i + 1 >= LATEST_ACT1_FIRST_TIER && i + 1 <= LATEST_ACT1_LAST_TIER )
		{
			a_pBuffer->WriteUInt8( GetAct1CompletedMask( s_aPCAct1Tiers[ i + 1 - LATEST_ACT1_FIRST_TIER ], TNULL ) );
			continue;
		}
#endif

		a_pBuffer->WriteUInt8( m_aTiers[ i ].uiCompletedMask );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005cc950
TBOOL AQuestManager::LoadCompletedQuests( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < m_iNumTiers; i++ )
	{
		TUINT8 uiMask;
		a_pBuffer->ReadUInt8( uiMask );
		m_aTiers[ i ].uiCompletedMask = uiMask;
	}

#ifdef BARNYARD_LATEST
	// NOTE: act 1 is saved in the PC order (see SaveCompletedQuests)
	if ( s_bLatestAct1Order )
	{
		QuestTier aPCTiers[ LATEST_ACT1_NUM_TIERS ];

		for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
		{
			aPCTiers[ i ]                 = s_aPCAct1Tiers[ i ];
			aPCTiers[ i ].uiCompletedMask = m_aTiers[ LATEST_ACT1_FIRST_TIER - 1 + i ].uiCompletedMask;
		}

		for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
		{
			QuestTier& rTier      = m_aTiers[ LATEST_ACT1_FIRST_TIER - 1 + i ];
			rTier.uiCompletedMask = GetAct1CompletedMask( rTier, aPCTiers );
		}
	}
#endif

	return TTRUE;
}

// $Barnyard: FUNCTION 005cc630
void AQuestManager::LoadQuests( const TCHAR* a_szFileName )
{
	TString8 strFilePath;
	strFilePath.Format( "Data/Quests/%s.trb", a_szFileName );

	TTRB oTRB;

	if ( oTRB.Load( strFilePath ) != TTRB::ERROR_OK )
		return;

	const PBProperties* pMain = TSTATICCAST( const PBProperties, oTRB.GetSymbolAddress( "Main" ) );
	oTRB.DeleteSymbolTable();

	TINT iNumProperties = pMain->GetPropertyCount();

	for ( TINT i = 0; i < iNumProperties; i++ )
	{
		g_oLoadScreen.Update( 1.0f, TTRUE );

		const PBProperties::PBProperty* pProperty = pMain->GetPropertyByIndex( i );

		// "quest"
		if ( pProperty->GetName().GetString()[ 0 ] == 'q' )
		{
			AQuest* pQuest = new AQuest();

			if ( pQuest->Create( pProperty->GetValue()->GetProperties() ) )
				m_vecQuests.PushBack( pQuest );
			else
				delete pQuest;
		}
	}
}

// $Barnyard: FUNCTION 005cd450
void AQuestManager::LoadTierData()
{
	TTRB oTRB;

	if ( oTRB.Load( "Data/Quests/tier_data.trb" ) != TTRB::ERROR_OK )
		return;

	const PBProperties* pMain = TSTATICCAST( const PBProperties, oTRB.GetSymbolAddress( "Main" ) );
	oTRB.DeleteSymbolTable();

	TPString8 strQuestKey( "quest" );
	TPString8 strActKey( "act" );
	TPString8 strTierKey( "tier" );
	TPString8 strQuest;

	TINT iNumProperties = pMain->GetPropertyCount();

	// The number of tiers of every act
	for ( TINT i = 0; i < iNumProperties; i++ )
	{
		g_oLoadScreen.Update( 1.0f, TTRUE );

		const PBProperties::PBProperty* pProperty = pMain->GetPropertyByIndex( i );

		// "tier_quest"
		if ( pProperty->GetName().GetString()[ 0 ] == 't' )
		{
			const PBProperties* pTierQuest = pProperty->GetValue()->GetProperties();

			TINT iAct;
			TINT iTier;
			pTierQuest->GetOptionalPropertyValue( iAct, strActKey );
			pTierQuest->GetOptionalPropertyValue( iTier, strTierKey );

			if ( m_auiNumTiers[ iAct - 1 ] < iTier )
				m_auiNumTiers[ iAct - 1 ] = TUINT8( iTier );
		}
	}

	m_iNumTiers += m_auiNumTiers[ 0 ];
	m_iNumTiers += m_auiNumTiers[ 1 ];
	m_iNumTiers += m_auiNumTiers[ 2 ];

	// The quests of every tier
	for ( TINT i = 0; i < iNumProperties; i++ )
	{
		g_oLoadScreen.Update( 1.0f, TTRUE );

		const PBProperties::PBProperty* pProperty = pMain->GetPropertyByIndex( i );

		if ( pProperty->GetName().GetString()[ 0 ] == 't' )
		{
			const PBProperties* pTierQuest = pProperty->GetValue()->GetProperties();

			TINT iAct;
			TINT iTier;
			pTierQuest->GetOptionalPropertyValue( iAct, strActKey );
			pTierQuest->GetOptionalPropertyValue( iTier, strTierKey );
			pTierQuest->GetOptionalPropertyValue( strQuest, strQuestKey );

			QuestTier* pTier = &m_aTiers[ GetAbsQuestTier( iAct, iTier ) - 1 ];

			for ( TINT k = 0; k < m_vecQuests.Size(); k++ )
			{
				g_oLoadScreen.Update( 1.0f, TTRUE );

				if ( m_vecQuests[ k ]->GetName() == strQuest )
				{
					pTier->auiQuests[ pTier->uiNumQuests++ ] = TUINT8( k );
					break;
				}
			}
		}
	}

#ifdef BARNYARD_LATEST
	// NOTE: the latest version (Wii) loads Data/Quests/tier_data_JP.trb instead, which changes the order of act 1.
	// The PC tier data is loaded here and put in that order.
	ApplyLatestAct1Order();
#endif
}

#ifdef BARNYARD_LATEST

TBOOL AQuestManager::IsLatestAct1Order()
{
	return s_bLatestAct1Order;
}

TINT AQuestManager::GetLatestAct1Tier( const TCHAR* a_szQuest, TINT a_iPCTier )
{
	if ( !s_bLatestAct1Order )
		return a_iPCTier;

	for ( TINT i = 0; i < LATEST_ACT1_NUM_QUESTS; i++ )
	{
		if ( ms_aLatestAct1Quests[ i ].iPCTier == a_iPCTier && TStringManager::String8CompareNoCase( ms_aLatestAct1Quests[ i ].szQuest, a_szQuest ) == 0 )
			return ms_aLatestAct1Quests[ i ].iTier;
	}

	return a_iPCTier;
}

void AQuestManager::ApplyLatestAct1Order()
{
	s_bLatestAct1Order = TFALSE;

	if ( m_auiNumTiers[ 0 ] < LATEST_ACT1_LAST_TIER )
		return;

	QuestTier* pTiers = &m_aTiers[ GetAbsQuestTier( 1, LATEST_ACT1_FIRST_TIER ) - 1 ];

	// The PC tiers must have exactly the quests of the table, otherwise (other tier data) the PC order is kept
	TINT iNumQuests = 0;

	for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
	{
		for ( TINT k = 0; k < pTiers[ i ].uiNumQuests; k++ )
		{
			TBOOL bFound = TFALSE;

			for ( TINT n = 0; n < LATEST_ACT1_NUM_QUESTS && !bFound; n++ )
			{
				bFound = ms_aLatestAct1Quests[ n ].iPCTier == i + LATEST_ACT1_FIRST_TIER &&
				    TStringManager::String8CompareNoCase( ms_aLatestAct1Quests[ n ].szQuest, m_vecQuests[ pTiers[ i ].auiQuests[ k ] ]->GetName().GetString() ) == 0;
			}

			if ( !bFound )
			{
				TWARN( "AQuestManager: the act 1 tier %d has the unexpected quest '%s', the act 1 order of the latest version isn't used\n", i + LATEST_ACT1_FIRST_TIER, m_vecQuests[ pTiers[ i ].auiQuests[ k ] ]->GetName().GetString() );
				return;
			}

			iNumQuests++;
		}
	}

	if ( iNumQuests != LATEST_ACT1_NUM_QUESTS )
	{
		TWARN( "AQuestManager: act 1 tiers %d-%d have %d quests instead of %d, the act 1 order of the latest version isn't used\n", LATEST_ACT1_FIRST_TIER, LATEST_ACT1_LAST_TIER, iNumQuests, LATEST_ACT1_NUM_QUESTS );
		return;
	}

	// Keep the PC order for the saves
	for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
	{
		s_aPCAct1Tiers[ i ]                 = pTiers[ i ];
		s_aPCAct1Tiers[ i ].uiCompletedMask = 0;
		pTiers[ i ].uiNumQuests             = 0;
		pTiers[ i ].uiCompletedMask         = 0;
	}

	// The tiers of the latest version
	for ( TINT n = 0; n < LATEST_ACT1_NUM_QUESTS; n++ )
	{
		const LatestAct1Quest& rcQuest = ms_aLatestAct1Quests[ n ];
		QuestTier&             rTier   = pTiers[ rcQuest.iTier - LATEST_ACT1_FIRST_TIER ];

		for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
		{
			for ( TINT k = 0; k < s_aPCAct1Tiers[ i ].uiNumQuests; k++ )
			{
				TUINT8 uiQuest = s_aPCAct1Tiers[ i ].auiQuests[ k ];

				if ( TStringManager::String8CompareNoCase( m_vecQuests[ uiQuest ]->GetName().GetString(), rcQuest.szQuest ) == 0 )
					rTier.auiQuests[ rTier.uiNumQuests++ ] = uiQuest;
			}
		}
	}

	s_bLatestAct1Order = TTRUE;
	TINFO( "AQuestManager: act 1 tiers %d-%d use the order of the latest version\n", LATEST_ACT1_FIRST_TIER, LATEST_ACT1_LAST_TIER );
}

TUINT8 AQuestManager::GetAct1CompletedMask( const QuestTier& a_rcTier, const QuestTier* a_pCompletedTiers ) const
{
	const QuestTier* pFrom  = a_pCompletedTiers ? a_pCompletedTiers : &m_aTiers[ LATEST_ACT1_FIRST_TIER - 1 ];
	TUINT8           uiMask = 0;

	for ( TINT k = 0; k < a_rcTier.uiNumQuests; k++ )
	{
		for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
		{
			for ( TINT n = 0; n < pFrom[ i ].uiNumQuests; n++ )
			{
				if ( pFrom[ i ].auiQuests[ n ] == a_rcTier.auiQuests[ k ] && ( pFrom[ i ].uiCompletedMask & ( 1 << n ) ) )
					uiMask |= TUINT8( 1 << k );
			}
		}
	}

	return uiMask;
}

TINT AQuestManager::GetFirstIncompleteAct1Tier( const QuestTier* a_pTiers )
{
	for ( TINT i = 0; i < LATEST_ACT1_NUM_TIERS; i++ )
	{
		if ( !IsTierComplete( a_pTiers[ i ] ) )
			return i + LATEST_ACT1_FIRST_TIER;
	}

	return LATEST_ACT1_LAST_TIER + 1;
}

TBOOL AQuestManager::IsTierComplete( const QuestTier& a_rcTier )
{
	return a_rcTier.uiCompletedMask == TUINT8( ( 1 << a_rcTier.uiNumQuests ) - 1 );
}

#endif // BARNYARD_LATEST

// $Barnyard: FUNCTION 005cc130
void AQuestManager::ClearAssignedQuests()
{
	for ( TINT i = 0; i < m_vecAssignedQuests.Size(); i++ )
	{
		AQuestInstance* pInstance = m_vecAssignedQuests[ i ];
		pInstance->Reset();
		m_oInstancePool.DeleteObject( pInstance );
	}

	for ( TINT i = 0; i < m_vecAssignedQuests.Size(); i++ )
		m_vecAssignedQuests[ i ] = TNULL;

	m_vecAssignedQuests.Clear();

	for ( TINT i = 0; i < MAX_NUM_TIERS; i++ )
	{
		// NOTE: retail clears the mask once for every quest of the tier
		if ( m_aTiers[ i ].uiNumQuests != 0 )
			m_aTiers[ i ].uiCompletedMask = 0;
	}
}

// $Barnyard: FUNCTION 005cbf90
TINT AQuestManager::GetAbsQuestTier( TINT a_iAct, TINT a_iTier ) const
{
	TINT iNumPrevious = 0;

	for ( TINT i = 0; i < a_iAct - 1; i++ )
		iNumPrevious += m_auiNumTiers[ i ];

	return a_iTier + iNumPrevious;
}

// $Barnyard: FUNCTION 005cbfd0
TINT AQuestManager::GetNumTiersInAct( TINT a_iAct ) const
{
	return m_auiNumTiers[ a_iAct ];
}

// $Barnyard: FUNCTION 005ccac0
AQuestManager::QuestTier* AQuestManager::GetCurrentQuestTier()
{
	return &m_aTiers[ GetCurAbsQuestTier() - 1 ];
}

// $Barnyard: FUNCTION 005cca00
AQuestManager::QuestTier* AQuestManager::FindQuestTier( TINT a_iQuest, TINT* a_pOutAct, TINT* a_pOutTier )
{
	TINT iTier = 0;
	TINT iAct  = 1;

	for ( TINT i = 0; i < m_iNumTiers; i++ )
	{
		QuestTier* pTier = &m_aTiers[ i ];

		if ( ++iTier > m_auiNumTiers[ iAct - 1 ] )
		{
			iTier = 1;
			iAct++;
		}

		for ( TINT k = 0; k < pTier->uiNumQuests; k++ )
		{
			if ( pTier->auiQuests[ k ] == a_iQuest )
			{
				if ( a_pOutAct )
					*a_pOutAct = iAct;

				if ( a_pOutTier )
					*a_pOutTier = iTier;

				return pTier;
			}
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005ccfb0
AQuestManager::QuestTier* AQuestManager::FindQuestTier( const TCHAR* a_szQuest, TINT* a_pOutAct, TINT* a_pOutTier )
{
	TINT iQuest = -1;

	for ( TINT i = 0; i < m_vecQuests.Size(); i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_vecQuests[ i ]->GetName().GetString(), a_szQuest ) == 0 )
		{
			iQuest = i;
			break;
		}
	}

	return FindQuestTier( iQuest, a_pOutAct, a_pOutTier );
}

// $Barnyard: FUNCTION 005cd020
AQuestManager::QuestTier* AQuestManager::FindQuestTier( AQuestInstance* a_pInstance, TINT* a_pOutAct, TINT* a_pOutTier )
{
	return FindQuestTier( a_pInstance->GetQuestIndex(), a_pOutAct, a_pOutTier );
}

// The state the story goes back to after a scripted sequence (inlined in SetQuestTier)
static const TClass* GetStoryStateClass()
{
	if ( AGameLoader::IsBarnExploreMiniGame() )
		return &TGetClass( ABarnExploreMinigameState );

	return &TGetClass( ASimState );
}

// $Barnyard: FUNCTION 005cda60
// $BarnyardWiiJP: FUNCTION 80361b78
void AQuestManager::SetQuestTier( TINT a_iAct, TINT a_iTier, TBOOL a_bSkipCompleted )
{
	APlayerProgress::GetSingleton()->UnlockQuestTierFlags( a_iAct, a_iTier );

	if ( a_iAct > 3 )
	{
		// The end of the story
		if ( !m_bGameComplete )
		{
			// SS11, the ending
			AGameLoader::SetTransitionToScriptedSequence( 10 );
			AGameLoader::SetPrevGameStateClass( GetStoryStateClass() );
			m_bGameComplete = TTRUE;
		}

		return;
	}

	RemoveTierQuestOffers( GetCurAbsQuestTier() );

	TINT iAbsTier = GetAbsQuestTier( a_iAct, a_iTier );
	m_iTier       = a_iTier;
	m_iAct        = a_iAct;

	AAnimalPopulationManager::GetSingleton()->SetQuestTier( iAbsTier, a_bSkipCompleted, 0 );

	ASimPersistData::GetSingleton()->SetAct( m_iAct );
	ACheatActivator::GetSingleton()->UpdateQuestTier();

	OfferTierQuests( iAbsTier, AGameTimeManager::GetSingleton()->IsItNight() ? AQuest::FLAGS_NIGHT : AQuest::FLAGS_NONE );

#ifdef BARNYARD_LATEST
	// Wii: once Daisy's birthday cake is done (tier 27 starts), the farmer comes home to the party
	ASimState* pSimState = TSTATICCAST( ASimState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASimState ) ) );

#  ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the party only happens if the converted Wii files are in the game data
	if ( pSimState && GetCurAbsQuestTier() == 27 && !IsScriptedSequenceAvailable( "SSBFF_Main" ) )
		pSimState = TNULL;
#  endif // BARNYARD_COMMUNITY_PATCH

	if ( GetCurAbsQuestTier() == 27 && pSimState )
	{
		pSimState->StartBirthdayCakeSequence();

		// The party is in the evening
		s_bForcedDayPhaseChange = TTRUE;
		AGameTimeManager::GetSingleton()->SetTime( 21, 0, 0, TFALSE, TTRUE );
		s_bForcedDayPhaseChange = TFALSE;
	}

	// Play the scripted sequence of the tier
	for ( TINT i = 0; i < TARRAYSIZE( s_aTierSequences ); i++ )
	{
		if ( s_aTierSequences[ i ].iTier == GetCurAbsQuestTier() - 1 )
		{
			const TierSequence& rcSequence = s_aTierSequences[ i ];
			TINT                iSequence  = rcSequence.iSequence;

#  ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: without the converted Wii files, the PC version of the sequence is played (SS06_story)
			if ( iSequence == AScriptedSequenceActivity::SEQUENCE_SS06_STORYMERGED && !IsScriptedSequenceAvailable( "SS06_storyMerged", "ss06merged" ) )
				iSequence = 5;
#  endif // BARNYARD_COMMUNITY_PATCH

			AGameLoader::SetTransitionToScriptedSequence( iSequence );
			AGameLoader::SetPrevGameStateClass( rcSequence.bBarn ? &TGetClass( ABarnExploreMinigameState ) : &TGetClass( ASimState ) );

			// Where the player is once the sequence is over
			if ( rcSequence.szStartGoal )
				AGameLoader::SetPlayerStartGoal( TPString8( rcSequence.szStartGoal ) );

			// The time of the day the story continues at, the next day if it's earlier than now
			AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();

			if ( rcSequence.eDayPhase < pGameTimeManager->GetCurrentDayPhase() )
				pGameTimeManager->SkipToNextDay();

			if ( rcSequence.eDayPhase != TierSequence::DAYPHASE_KEEP )
			{
				s_bForcedDayPhaseChange = TTRUE;

				// The barn library of the night is loaded, the quick save mustn't forget it (see AAssetLoader::DestroyAssetPackOfLibrary)
				if ( AGameLoader::IsBarnExploreMiniGame() && pGameTimeManager->GetCurrentDayPhase() == AGameTimeManager::DAYPHASE_NIGHT )
					AHackedQuickSave::SetSavedDayPhase( AHackedQuickSave::SAVEDDAYPHASE_NIGHT_BARN );

				pGameTimeManager->ChangePhaseOfDay( AGameTimeManager::DAYPHASE( rcSequence.eDayPhase ), TTRUE, TTRUE );
				s_bForcedDayPhaseChange = TFALSE;
			}

			break;
		}
	}
#else
	// Play the scripted sequence of the tier
	for ( TINT i = 0; i < TARRAYSIZE( s_aTierSequences ); i++ )
	{
		if ( s_aTierSequences[ i ].iTier == GetCurAbsQuestTier() - 1 )
		{
			AGameLoader::SetTransitionToScriptedSequence( s_aTierSequences[ i ].iSequence );
			AGameLoader::SetPrevGameStateClass( GetStoryStateClass() );
			break;
		}
	}
#endif // BARNYARD_LATEST
}

// $Barnyard: FUNCTION 005ce0d0
void AQuestManager::AdvanceQuestTier( TINT a_iNumTiers )
{
	if ( m_bGameComplete )
		return;

	for ( TINT i = a_iNumTiers; i > 0; i-- )
	{
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: SetQuestTier removes the quest offers of the current tier, but the tier is already advanced
		// when it's called from here, so the offers of the previous tier stayed. Skipping several tiers at once
		// left more offers than APlayerSimProfile has items for, which corrupted its lists
		if ( m_bGameComplete || m_iAct > 3 )
			break;

		RemoveTierQuestOffers( GetCurAbsQuestTier() );
#endif // BARNYARD_COMMUNITY_PATCH

		if ( ++m_iTier > m_auiNumTiers[ m_iAct - 1 ] )
		{
			m_iTier = 1;
			m_iAct++;

			ASimPersistData::GetSingleton()->SetAct( m_iAct );
		}

		SetQuestTier( m_iAct, m_iTier, TFALSE );
	}

#ifdef BARNYARD_LATEST
	// NOTE: not in the original. A PC retail save can have completed quests of a later act 1 tier of the latest order (see
	// Load); a tier with nothing left to do is skipped, the story would stop there otherwise
	if ( s_bLatestAct1Order && !m_bGameComplete && m_iAct == 1 && m_iTier >= LATEST_ACT1_FIRST_TIER && m_iTier <= LATEST_ACT1_LAST_TIER && IsTierComplete( *GetCurrentQuestTier() ) )
		AdvanceQuestTier( 1 );
#endif
}

// $Barnyard: FUNCTION 005cd790
void AQuestManager::OfferTierQuests( TINT a_iAbsTier, AQuest::FLAGS a_uiFlags )
{
	QuestTier*         pTier    = &m_aTiers[ a_iAbsTier - 1 ];
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
	{
		AQuest* pQuest = m_vecQuests[ pTier->auiQuests[ i ] ];

		if ( ( pTier->uiCompletedMask & ( 1 << i ) ) == 0 &&
		     !FindAssignedQuest( pQuest->GetName(), 1 ) &&
		     ( a_uiFlags == 0 || ( a_uiFlags & pQuest->GetFlags() ) == a_uiFlags ) )
		{
			pProfile->AddPendingItem( pQuest->GetTitle(), pQuest->GetQuestGiver(), pQuest->GetTitleId() );
		}
	}
}

// $Barnyard: FUNCTION 005cd910
void AQuestManager::RemoveTierQuestOffers( TINT a_iAbsTier )
{
	QuestTier*         pTier    = &m_aTiers[ a_iAbsTier - 1 ];
	APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

	for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
	{
		AQuest* pQuest = m_vecQuests[ pTier->auiQuests[ i ] ];

		if ( ( pTier->uiCompletedMask & ( 1 << i ) ) == 0 && !FindAssignedQuest( pQuest->GetName(), 1 ) )
			pProfile->ReleaseItem( pQuest->GetQuestGiver() );
	}
}

// $Barnyard: FUNCTION 005cd250
AQuestInstance* AQuestManager::AssignQuest( AUnit* a_pQuestGiver, TPString8 a_strName )
{
	TINT iQuest = 0;

	for ( ; iQuest < m_vecQuests.Size(); iQuest++ )
	{
		if ( m_vecQuests[ iQuest ]->GetName() == a_strName )
			break;
	}

	if ( iQuest >= m_vecQuests.Size() )
	{
		AReplayManager::GetSingleton()->UpdateReplays();
		return TNULL;
	}

	AQuestInstance* pInstance = m_oInstancePool.NewObject();
	pInstance->Create( iQuest, m_iNextInstanceId++, a_pQuestGiver );
	m_vecAssignedQuests.PushBack( pInstance );

	AAnimalPopulationManager::GetSingleton()->ResetExclamationView( TFALSE );

	// The minigames have their own banner
	AQuest* pQuest       = pInstance->GetQuest();
	TBOOL   bIsGameEvent = TFALSE;

	for ( TINT i = 0; i < pQuest->GetNumElements(); i++ )
	{
		if ( !pInstance->IsElementDone( i ) && pQuest->GetElement( i )->IsA( &TGetClass( AGameEventQuestElement ) ) )
		{
			bIsGameEvent = TTRUE;
			break;
		}
	}

	if ( !bIsGameEvent )
	{
		m_eBannerState    = BANNERSTATE_NEWQUEST;
		m_bBannerComplete = TFALSE;
	}

	AReplayManager::GetSingleton()->UpdateReplays();

	return pInstance;
}

// $Barnyard: FUNCTION 005ce150
// $BarnyardWiiJP: FUNCTION 80361138
void AQuestManager::RemoveQuest( AQuestInstance* a_pInstance, TBOOL a_bCompleted, TBOOL a_bAdvance )
{
	a_pInstance->RemoveMarkers();

	// NOTE: inlined HideTimeLeft (005cbfe0)
	HideTimeLeft();

	AAnimalPopulationManager::GetSingleton()->ResetExclamationView( TTRUE );
	m_vecAssignedQuests.Erase( m_vecAssignedQuests.Find( a_pInstance ) );

	if ( a_bCompleted )
	{
		// Lets the units know about the quest
		AInteractionQuestEvent* pEvent = TSTATICCAST( AInteractionQuestEvent, AInteractionEventManager::GetSingleton()->NewActiveEvent( &TGetClass( AInteractionQuestEvent ) ) );
		pEvent->SetQuest( a_pInstance->GetQuest() );
		pEvent->SetCompleted( a_bCompleted );

		if ( AUnit* pQuestGiver = a_pInstance->GetQuestGiver() )
			pEvent->SetUnit( pQuestGiver );
		else
			pEvent->SetUnitRef( a_pInstance->GetQuestGiverRef() );
	}

	TINT       iAct  = 0;
	TINT       iTier = 0;
	QuestTier* pTier = FindQuestTier( a_pInstance->GetQuestIndex(), &iAct, &iTier );

	if ( a_bAdvance == TTRUE )
	{
#ifdef BARNYARD_LATEST
		// NOTE: not in the original. A quest of a PC retail save can be in a later act 1 tier of the latest order than the
		// current one (see Load), the story stays at the current tier then
		if ( s_bLatestAct1Order && iAct == 1 && m_iAct == 1 && m_iTier >= LATEST_ACT1_FIRST_TIER && iTier > m_iTier && iTier <= LATEST_ACT1_LAST_TIER )
			iTier = m_iTier;
#endif

		m_iAct  = iAct;
		m_iTier = iTier;

		ASimPersistData::GetSingleton()->SetAct( iAct );

		if ( pTier && a_bCompleted )
			AAnimalPopulationManager::GetSingleton()->ClearQuestTier( GetAbsQuestTier( m_iAct, m_iTier ), a_pInstance->GetQuest()->GetName().GetString(), 0 );
	}

	if ( a_bCompleted && pTier )
	{
		TBOOL bTierComplete = TTRUE;

		for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
		{
			if ( pTier->auiQuests[ i ] == a_pInstance->GetQuestIndex() )
				pTier->uiCompletedMask |= 1 << i;

			bTierComplete = bTierComplete && ( pTier->uiCompletedMask & ( 1 << i ) ) != 0;
		}

#ifdef BARNYARD_LATEST
		// NOTE: only once the current tier is complete (the quest's tier can be a later one, see above)
		if ( bTierComplete && a_bAdvance && ( pTier == GetCurrentQuestTier() || IsTierComplete( *GetCurrentQuestTier() ) ) )
			AdvanceQuestTier( 1 );
#else
		if ( bTierComplete && a_bAdvance )
			AdvanceQuestTier( 1 );
#endif
	}

#ifdef BARNYARD_LATEST
	// Wii: Ben's tomato launcher is ready, the screen goes black until the tomato launcher sequence (SSUL_Main) starts (see OnUpdate)
	if ( a_bCompleted && a_pInstance->GetQuest()->GetName() == TPS8D( "Ben Tomato Launcher" ) )
	{
#  ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: only if the converted Wii files are in the game data
		if ( IsScriptedSequenceAvailable( "SSUL_Main" ) )
#  endif
		{
			s_bStartTomatoLauncherSequence = TTRUE;
			AGameStateController::GetSingleton()->ShowOverlay( TTRUE, TFALSE, 1.0f, TFALSE, "ASimState" );
		}
	}
#endif // BARNYARD_LATEST

	a_pInstance->Reset();
	m_oInstancePool.DeleteObject( a_pInstance );

	AReplayManager::GetSingleton()->UpdateReplays();
}

// $Barnyard: FUNCTION 005cd030
void AQuestManager::AbandonQuest()
{
	if ( m_vecAssignedQuests.Size() <= 0 )
		return;

	AQuestInstance* pInstance = m_vecAssignedQuests[ 0 ];
	AAnimalPopulationManager::GetSingleton()->ClearQuestTier( GetCurAbsQuestTier(), pInstance->GetQuest()->GetName().GetString(), 0 );

	// NOTE: retail doesn't check the element, it isn't always created in this repo yet
	AItemCountHudElement* pItemCount = AHudManager::GetSingleton()->FindElement<AItemCountHudElement>();

	if ( pItemCount )
		pItemCount->SetVisibleForce( TFALSE );

	for ( TINT i = 0; i < m_vecAssignedQuests.Size(); i++ )
		m_vecAssignedQuests[ i ]->RemoveMarkers();

	// Don't offer it again
	QuestTier* pTier = GetCurrentQuestTier();

	for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
	{
		if ( pInstance->GetQuestIndex() == pTier->auiQuests[ i ] )
		{
			m_uiOfferedQuests |= 1 << i;
			break;
		}
	}

	pInstance->SetElementsState( AQuestInstance::ELEMENTSTATE_FAILEDFORGOOD );
}

// $Barnyard: FUNCTION 005ccdb0
TBOOL AQuestManager::IsQuestComplete( const TPString8& a_rcName, TINT a_iUnused ) const
{
	for ( TINT i = 0; i < MAX_NUM_TIERS; i++ )
	{
		const QuestTier& rTier = m_aTiers[ i ];

		for ( TINT k = 0; k < rTier.uiNumQuests; k++ )
		{
			if ( m_vecQuests[ rTier.auiQuests[ k ] ]->GetName() == a_rcName )
				return ( rTier.uiCompletedMask & ( 1 << k ) ) != 0;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005cd160
TBOOL AQuestManager::IsQuestUnavailable( AQuest* a_pQuest )
{
	AGameTimeManager* pGameTimeManager = AGameTimeManager::GetSingleton();
	QuestTier*        pTier            = GetCurrentQuestTier();
	TINT              iIndex           = -1;

	for ( TINT i = 0; i < pTier->uiNumQuests; i++ )
	{
		if ( m_vecQuests[ pTier->auiQuests[ i ] ] == a_pQuest )
		{
			iIndex = i;

			if ( pTier->uiCompletedMask & ( 1 << i ) )
				return TTRUE;

			break;
		}
	}

	// NOTE: retail shifts by -1 when the quest isn't in the tier, which tests bit 0 on x86
	if ( ( m_uiOfferedQuests & TUINT8( 1u << ( iIndex & 31 ) ) ) == 0 )
	{
		return ( ( ( a_pQuest->GetFlags() & AQuest::FLAGS_DAY ) && pGameTimeManager->IsItNight() ) ||
		         ( ( a_pQuest->GetFlags() & AQuest::FLAGS_NIGHT ) && !pGameTimeManager->IsItNight() ) ) &&
		    ( a_pQuest->GetFlags() & ( AQuest::FLAGS_DAY | AQuest::FLAGS_NIGHT ) ) != ( AQuest::FLAGS_DAY | AQuest::FLAGS_NIGHT );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005cc800
AQuestInstance* AQuestManager::GetAssignedQuest( TINT a_iIndex ) const
{
	if ( a_iIndex >= m_vecAssignedQuests.Size() )
		return TNULL;

	return m_vecAssignedQuests[ a_iIndex ];
}

// $Barnyard: FUNCTION 005cc820
AQuestInstance* AQuestManager::FindAssignedQuest( const TPString8& a_rcName, TINT a_iOccurrence ) const
{
	TINT iNumFound = 0;

	for ( TINT i = 0; i < m_vecAssignedQuests.Size(); i++ )
	{
		// NOTE: retail uses TPString8::Compare (006bc110), which compares the pooled pointers too
		if ( m_vecAssignedQuests[ i ]->GetQuest()->GetName() == a_rcName )
		{
			iNumFound++;

			if ( iNumFound == a_iOccurrence )
				return m_vecAssignedQuests[ i ];
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 005cc9a0
void AQuestManager::AddElementMarkers()
{
	for ( TINT i = 0; i < m_vecAssignedQuests.Size(); i++ )
	{
		AQuestInstance* pInstance = m_vecAssignedQuests[ i ];

		for ( TINT k = 0; k < pInstance->GetQuest()->GetNumElements(); k++ )
			pInstance->GetQuest()->GetElement( k )->GetLocator().AddMarker();
	}
}

// $Barnyard: FUNCTION 005cbfe0
void AQuestManager::HideTimeLeft()
{
	if ( m_pSimHudGroup )
	{
		m_pSimHudGroup->SetShowTimer( TFALSE );
		m_pSimHudGroup->GetTimeLeftElement()->GetTimer().Reset();
		m_pSimHudGroup = TNULL;
	}
}

// $Barnyard: FUNCTION 005ccb00
void AQuestManager::PlayQuestCompleteMusic()
{
	if ( m_iQuestMusicCue != -1 )
		return;

	if ( AGameLoader::IsBarnExploreMiniGame() )
	{
		AMultiplexStream::GetSingleton()->Pause( TTRUE );
		m_iQuestMusicCue = ASoundManager::GetSingleton()->PlayCue( 2 );
	}
	else
	{
		AMusicManager::GetSingleton()->SetEnabled( TFALSE, TFALSE );
		m_bPlayCompleteMusic = TTRUE;
	}

	if ( !ARootTask::GetSingleton()->IsPaused() )
	{
		// The rain gets louder
		if ( AWeatherManager::GetSingleton()->IsBadWeather() )
			m_iWeatherCue = ASoundManager::GetSingleton()->PlayCue( soundbank::UI_RAIN );
	}
}

// $Barnyard: FUNCTION 005ccb90
void AQuestManager::PlayQuestFailedMusic()
{
	if ( m_iQuestMusicCue != -1 )
		return;

	if ( AGameLoader::IsBarnExploreMiniGame() )
	{
		AMultiplexStream::GetSingleton()->Pause( TTRUE );
		m_iQuestMusicCue = ASoundManager::GetSingleton()->PlayCue( 3 );
	}
	else
	{
		AMusicManager::GetSingleton()->SetEnabled( TFALSE, TFALSE );
		m_bPlayFailedMusic = TTRUE;
	}

	if ( !ARootTask::GetSingleton()->IsPaused() )
	{
		// The rain gets louder
		if ( AWeatherManager::GetSingleton()->IsBadWeather() )
			m_iWeatherCue = ASoundManager::GetSingleton()->PlayCue( soundbank::UI_RAIN );
	}
}

// $Barnyard: FUNCTION 005cccf0
TBOOL AQuestManager::OnMiniGameResult( AMiniGameState::S1* a_pResults )
{
	for ( auto it = m_vecAssignedQuests.Begin(); it != m_vecAssignedQuests.End(); it++ )
		( *it )->OnMiniGameResult( a_pResults );

	return TTRUE;
}

// $Barnyard: FUNCTION 005ccd50
TBOOL AQuestManager::OnMiniGameStart( const TPString8& a_rcName )
{
	for ( auto it = m_vecAssignedQuests.Begin(); it != m_vecAssignedQuests.End(); it++ )
		( *it )->OnMiniGameStart( a_rcName );

	return TTRUE;
}

// $Barnyard: FUNCTION 005cbe80
TBOOL AQuestManager::IsBannerShown() const
{
	return m_eBannerState != BANNERSTATE_NONE;
}

// $Barnyard: FUNCTION 005cbf50
void AQuestManager::SetBannerState( BANNERSTATE a_eState )
{
	m_eBannerState = a_eState;

	if ( a_eState == BANNERSTATE_NEWQUEST || a_eState == BANNERSTATE_NONE )
		m_bBannerComplete = TFALSE;
}

// $Barnyard: FUNCTION 005cbf70
void AQuestManager::SetSavedLauncherInput( TBOOL a_bEnabled )
{
	s_bSavedSecondaryLauncherInput = a_bEnabled;
	s_bSavedLauncherInput          = a_bEnabled;
}

// $Barnyard: FUNCTION 005cbe40
const TCHAR* AQuestManager::GetBarnDetailingQuest()
{
	return s_szBarnDetailingQuest;
}

// $Barnyard: FUNCTION 005cbe50
const TCHAR* AQuestManager::GetFlowersForBenQuest()
{
	return s_szFlowersForBenQuest;
}

// $Barnyard: FUNCTION 005cbe60
const TCHAR* AQuestManager::GetRoundUpChicksQuest()
{
	return s_szRoundUpChicksQuest;
}

// $Barnyard: FUNCTION 005cbe70
const TCHAR* AQuestManager::GetCoyoteTippingQuest()
{
	return s_szCoyoteTippingQuest;
}

// NOTE: inlined into UpdateBanner (005cc1e0)
void AQuestManager::SetBannerText( const TWCHAR* a_wszText )
{
	m_oBannerText.SetTextAndFitWidth( a_wszText );
}

// $Barnyard: FUNCTION 005cbe90
void AQuestManager::SetBannerFade( TFLOAT a_fTime, TFLOAT a_fFadeIn, TFLOAT a_fHold, TFLOAT a_fFadeOut )
{
	if ( a_fTime <= 0.0f )
	{
		m_oBannerText.SetAlpha( 0.0f );
		return;
	}

	TFLOAT fHoldEnd = a_fFadeIn + a_fHold;

	if ( fHoldEnd + a_fFadeOut <= a_fTime )
	{
		m_oBannerText.SetAlpha( 0.0f );
		return;
	}

	if ( a_fTime >= a_fFadeIn && a_fTime <= fHoldEnd )
	{
		m_oBannerText.SetAlpha( 1.0f );
		return;
	}

	if ( a_fTime < a_fFadeIn )
		m_oBannerText.SetAlpha( a_fTime / a_fFadeIn );
	else
		m_oBannerText.SetAlpha( 1.0f - ( a_fTime - fHoldEnd ) / a_fFadeOut );
}

// $Barnyard: FUNCTION 005cc1e0
void AQuestManager::UpdateBanner( TFLOAT a_fDeltaTime )
{
	if ( m_eBannerState == BANNERSTATE_NONE || m_bTimerPaused ||
	     AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABarnRewardState ) ) || AGameLoader::IsInMiniGame() )
	{
		m_oBannerText.SetAlpha( 0.0f );
		s_fBannerDelay = 0.5f;
		return;
	}

	if ( s_fBannerDelay > 0.0f )
	{
		m_oBannerText.SetAlpha( 0.0f );
		s_fBannerDelay -= a_fDeltaTime;
		return;
	}

	if ( !m_oBannerText.IsVisible() )
		return;

	m_fBannerTime += a_fDeltaTime;
	TFLOAT fTime = m_fBannerTime;

	switch ( m_eBannerState )
	{
		case BANNERSTATE_NEWQUEST:
			SetBannerText( T2Locale::GetSingleton()->GetString( 3362 ) );
			m_oBannerText.SetAlpha( 0.0f );
			m_fBannerTime  = 0.0f;
			m_eBannerState = BANNERSTATE_NEWQUEST_SHOW;
			break;
		case BANNERSTATE_NEWQUEST_SHOW:
			SetBannerFade( fTime, 0.75f, 0.75f, 0.75f );

			if ( m_fBannerTime >= 2.25f )
			{
				m_fBannerTime     = 0.0f;
				m_bBannerComplete = TTRUE;
				m_eBannerState    = BANNERSTATE_NONE;
			}

			break;
		case BANNERSTATE_COMPLETE:
			m_oBannerText.SetAlpha( 0.0f );

			if ( m_bBannerComplete )
			{
				SetBannerText( T2Locale::GetSingleton()->GetString( 3369 ) );
				m_fBannerTime     = 0.0f;
				m_bBannerComplete = TFALSE;
				m_eBannerState    = BANNERSTATE_COMPLETE_SHOW;
			}
			else
			{
				m_bBannerComplete = TFALSE;
				m_eBannerState    = BANNERSTATE_NONE;
			}

			break;
		case BANNERSTATE_COMPLETE_SHOW:
		case BANNERSTATE_FINAL_SHOW:
			SetBannerFade( fTime, 0.75f, 0.75f, 0.75f );

			if ( m_fBannerTime >= 2.25f )
			{
				m_fBannerTime  = 0.0f;
				m_eBannerState = BANNERSTATE_NONE;
			}

			break;
		case BANNERSTATE_FAILED:
			m_oBannerText.SetAlpha( 0.0f );

			if ( m_bBannerGameEvent )
			{
				SetBannerText( T2Locale::GetSingleton()->GetString( 3364 ) );
				m_fBannerTime      = 0.0f;
				m_bBannerGameEvent = TFALSE;
				m_bBannerComplete  = TFALSE;
				m_eBannerState     = BANNERSTATE_FINAL_SHOW;
			}
			else if ( m_bBannerComplete )
			{
				SetBannerText( T2Locale::GetSingleton()->GetString( 3366 ) );
				m_fBannerTime      = 0.0f;
				m_bBannerGameEvent = TFALSE;
				m_bBannerComplete  = TFALSE;
				m_eBannerState     = BANNERSTATE_FAILED_SHOW;
			}
			else
			{
				m_bBannerGameEvent = TFALSE;
				m_bBannerComplete  = TFALSE;
				m_eBannerState     = BANNERSTATE_NONE;
			}

			break;
		case BANNERSTATE_FAILED_SHOW:
			SetBannerFade( fTime, 0.75f, 0.75f, 0.75f );

			if ( m_fBannerTime >= 2.25f )
			{
				m_fBannerTime  = 0.0f;
				m_eBannerState = BANNERSTATE_FINAL_SHOW;
				SetBannerText( T2Locale::GetSingleton()->GetString( 3364 ) );
			}

			break;
		case BANNERSTATE_FAILED_RETRY_SHOW:
			SetBannerFade( fTime, 0.75f, 0.75f, 0.75f );

			if ( m_fBannerTime >= 2.25f )
			{
				m_fBannerTime  = 0.0f;
				m_eBannerState = BANNERSTATE_FINAL_SHOW;
				SetBannerText( T2Locale::GetSingleton()->GetString( 3368 ) );

				if ( m_bBannerDayQuest )
					SetBannerText( T2Locale::GetSingleton()->GetString( 3367 ) );
			}

			break;
	}
}

// $Barnyard: FUNCTION 005cdc00
// $BarnyardWiiJP: FUNCTION 80361e5c
TBOOL AQuestManager::OnDateChange( AGameTimeManager::DateChangeEvent* a_pEvent )
{
	if ( AGameLoader::IsChangingTerrain() || !AGameLoader::IsMainGameLoaded() || AGameLoader::IsReturningToFrontEnd() || !m_bEnabled )
		return TTRUE;

	if ( a_pEvent->m_eUpdateType != AGameTimeManager::DATECHANGETYPE_DAYPHASE )
		return TTRUE;

	s_bSleepCycle = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASleepCycleState ) ) != TNULL;
	OnUpdate( 0.0f );

	AGameTimeManager::DAYPHASE eDayPhase = a_pEvent->m_eCurrDayPhase;

#ifdef BARNYARD_LATEST
	// Wii: a forced change that wraps around to an earlier phase of the day counts as a new morning
	if ( s_bForcedDayPhaseChange )
	{
		AGameTimeManager::DAYPHASE ePrevDayPhase = a_pEvent->m_ePrevDayPhase;

		if ( ( ePrevDayPhase == AGameTimeManager::DAYPHASE_NIGHT || eDayPhase != AGameTimeManager::DAYPHASE_NIGHT ) && eDayPhase < ePrevDayPhase )
			eDayPhase = AGameTimeManager::DAYPHASE_DAWN;
	}
#endif // BARNYARD_LATEST

	if ( eDayPhase == AGameTimeManager::DAYPHASE_DAWN )
	{
		// The day quests
		m_uiOfferedQuests = 0;
#ifdef BARNYARD_LATEST
		if ( !s_bKeepQuestOnDayPhaseChange )
#endif
			AbandonQuest();

		TINT iAbsTier = GetCurAbsQuestTier();
		AAnimalPopulationManager::GetSingleton()->Populate( AAnimalPopulationManager::SPAWNFLAGS_DAY | AAnimalPopulationManager::SPAWNFLAGS_UNK2, TTRUE );
		AAnimalPopulationManager::GetSingleton()->SetQuestTier( iAbsTier, TTRUE, AAnimalPopulationManager::SPAWNFLAGS_DAY );
		RemoveTierQuestOffers( iAbsTier );
		OfferTierQuests( iAbsTier, AQuest::FLAGS_NONE );
	}
	else if ( eDayPhase == AGameTimeManager::DAYPHASE_NIGHT )
	{
		// The night quests
		m_uiOfferedQuests = 0;
#ifdef BARNYARD_LATEST
		if ( !s_bKeepQuestOnDayPhaseChange )
#endif
			AbandonQuest();

		TINT iAbsTier = GetCurAbsQuestTier();
		AAnimalPopulationManager::GetSingleton()->Populate( AAnimalPopulationManager::SPAWNFLAGS_NIGHT | AAnimalPopulationManager::SPAWNFLAGS_UNK2, TTRUE );
		AAnimalPopulationManager::GetSingleton()->SetQuestTier( iAbsTier, TTRUE, AAnimalPopulationManager::SPAWNFLAGS_NIGHT );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 005ce340
TBOOL __stdcall AQuestManager::OnDateChangeCallback( AQuestManager* a_pCaller, AGameTimeManager* a_pOwner, AGameTimeManager::DateChangeEvent* a_pEvent )
{
	return a_pCaller->OnDateChange( a_pEvent );
}

// $Barnyard: FUNCTION 005cdd70
void AQuestManager::OnSequenceEvent( const AInteractionManagerEvent& a_rcEvent )
{
	if ( !AGameLoader::IsMainGameLoaded() && !AGameLoader::IsBarnExploreMiniGame() )
		return;

	if ( AGameLoader::IsReturningToFrontEnd() == TTRUE )
		return;

	AInteractionManagerEvent::TYPE eType = a_rcEvent.GetType();

	if ( ( eType & AInteractionManagerEvent::TYPE_SEQUENCE ) == 0 )
		return;

	// Only the quest sequences
	if ( a_rcEvent.GetKind() != AInteractionManagerEvent::KIND_MISSION_CONVERSATION && a_rcEvent.GetKind() != AInteractionManagerEvent::KIND_MISSION_OBJECTIVE )
		return;

	if ( eType & AInteractionManagerEvent::TYPE_STARTED )
	{
		// A quest sequence starts
		AUnit*                   pPlayer        = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
		APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );

		// NOTE: retail saves the flag at 0x32 and restores it to 0x31
		s_bSavedSecondaryLauncherInput = pPlayerControl->GetSecondaryLauncherInput();
		s_bSavedLauncherInput          = pPlayerControl->GetUnk32();
		pPlayerControl->SetSecondaryLauncherInput( TFALSE );
		pPlayerControl->SetLauncherInput( TFALSE );

		AUnitPlayer* pUnitPlayer = TSTATICCAST( AUnitPlayer, pPlayer );

		if ( pUnitPlayer->GetActiveLauncher() && pUnitPlayer->GetActiveLauncher() == pUnitPlayer->GetSecondaryLauncher() )
			pUnitPlayer->SelectLauncher( TFALSE );

		m_bTimerPaused         = TTRUE;
		m_bMissionMusicPlaying = m_iQuestMusicCue == -1 && !m_bPlayCompleteMusic && !m_bPlayFailedMusic;

		AInteractionSequence* pSequence = a_rcEvent.GetSequence();

		if ( pSequence )
		{
			// Not with the bull
			for ( TINT i = 0; i < pSequence->GetComponents().Size(); i++ )
			{
				AUnit* pUnit = pSequence->GetUnit( i );

				if ( pUnit && pUnit->GetType() &&
				     TStringManager::String8Compare( pUnit->GetType()->GetNameString(), "nb_bull" ) == 0 )
				{
					m_bMissionMusicPlaying = TFALSE;
					break;
				}
			}
		}

		if ( AGameLoader::IsMainGameLoaded() == TTRUE )
		{
			s_bGameTimeWasStopped = AGameTimeManager::GetSingleton()->IsUpdateStopped();

			if ( !s_bGameTimeWasStopped )
			{
				AGameTimeManager::GetSingleton()->PushUpdateState();

				AGameTimeFXManager::GetSingleton()->Pause( TTRUE );
			}

			return;
		}

		if ( AGameLoader::IsBarnExploreMiniGame() && AGameTimeManager::GetSingleton()->IsItNight() )
		{
			// The music of the barn is turned down during the sequence
			ABarnExploreMinigameState* pBarnState = TSTATICCAST( ABarnExploreMinigameState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) ) );
			const TINT                 iFloor     = ABarnExploreMinigameState::ms_iFloor;

			if ( iFloor != ABarnExploreMinigameState::FLOOR_LOFT )
			{
				if ( iFloor > 0 && iFloor <= 2 && ABarnExploreMinigameState::ms_bWurlitzerOn )
					pBarnState->SetMusicVolume( 1, 0.3f );
			}
			else if ( ABarnExploreMinigameState::ms_bRadioOn )
			{
				pBarnState->SetMusicVolume( 0, 0.3f );
			}
		}
	}
	else if ( eType & AInteractionManagerEvent::TYPE_ENDED )
	{
		// A quest sequence ends
		AUnit*                   pPlayer        = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );
		APlayerControlComponent* pPlayerControl = TSTATICCAST( APlayerControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) );
		pPlayerControl->SetSecondaryLauncherInput( s_bSavedSecondaryLauncherInput );
		pPlayerControl->SetLauncherInput( s_bSavedLauncherInput );

		m_bTimerPaused         = TFALSE;
		m_bMissionMusicPlaying = TFALSE;

		if ( AGameLoader::IsMainGameLoaded() == TTRUE && !s_bGameTimeWasStopped )
		{
			AGameTimeManager::GetSingleton()->PopUpdateState();

			AGameTimeFXManager::GetSingleton()->Resume();
			return;
		}

		if ( AGameLoader::IsBarnExploreMiniGame() && AGameTimeManager::GetSingleton()->IsItNight() )
		{
			// The music of the barn is back
			ABarnExploreMinigameState* pBarnState = TSTATICCAST( ABarnExploreMinigameState, AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) ) );
			const TINT                 iFloor     = ABarnExploreMinigameState::ms_iFloor;

			if ( iFloor == ABarnExploreMinigameState::FLOOR_LOFT )
			{
				if ( ABarnExploreMinigameState::ms_bRadioOn )
					pBarnState->SetMusicVolume( 0, 1.0f );
			}
			else if ( iFloor > 0 && iFloor <= 2 && ABarnExploreMinigameState::ms_bWurlitzerOn )
			{
				pBarnState->SetMusicVolume( 1, 1.0f );
			}
		}
	}
}

// $Barnyard: FUNCTION 005ce360
void AQuestManager::OnSequenceEventCallback( void* a_pCaller, const AInteractionManagerEvent& a_rcEvent )
{
	TSTATICCAST( AQuestManager, a_pCaller )->OnSequenceEvent( a_rcEvent );
}
