#include "pch.h"
#include "APrimeDebugQuest.h"

#ifdef PRIMEDEBUG_DEV
#  include <Console/PCFunc.h>
#  include <Console/PCVar.h>
#  include <Menu/PDebugMenu.h>
#  include <Util/PString.h>

#  include "Helpers/AGameLoader.h"
#  include "Quests/AQuestManager.h"
#  include "Quests/AQuestInstance.h"
#  include "Quests/AQuest.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

static constexpr TINT MAX_STORY_TEXT = 96;

static TINT  s_iStoryAct  = 1;
static TINT  s_iStoryTier = 1;
static PCVar s_oStoryActVar;
static PCVar s_oStoryTierVar;

static TINT  s_iStoryCurAct;
static TINT  s_iStoryCurTier;
static TINT  s_iStoryCurAbsTier;
static TCHAR s_szStoryQuest[ MAX_STORY_TEXT ];
static TCHAR s_aszStoryTierQuests[ AQuestManager::MAX_NUM_TIER_QUESTS ][ MAX_STORY_TEXT ];
static PCVar s_oStoryCurActVar;
static PCVar s_oStoryCurTierVar;
static PCVar s_oStoryCurAbsTierVar;
static PCVar s_oStoryQuestVar;
static PCVar s_aStoryTierQuestVars[ AQuestManager::MAX_NUM_TIER_QUESTS ];

static const TCHAR* s_aStoryTierQuestVarNames[ AQuestManager::MAX_NUM_TIER_QUESTS ] = {
	"story.info.tier_quest1",
	"story.info.tier_quest2",
	"story.info.tier_quest3",
};

static PCFunc s_oStoryGotoFunc;
static PCFunc s_oStoryAdvanceFunc;
static PCFunc s_oStoryResetFunc;

static void OnStoryTargetChanged( PCVar* a_pVar )
{
	s_iStoryAct  = TMath::Min( TMath::Max( s_iStoryAct, 1 ), 3 );
	s_iStoryTier = TMath::Min( TMath::Max( s_iStoryTier, 1 ), AGameLoader::Dev_GetNumTiersInAct( s_iStoryAct ) );
}

static TBOOL StoryGotoCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	if ( a_iArgc != 1 && a_iArgc != 3 )
		return TFALSE;

	if ( a_iArgc == 3 )
	{
		s_iStoryAct  = atoi( a_ppArgv[ 1 ] );
		s_iStoryTier = atoi( a_ppArgv[ 2 ] );
	}

	OnStoryTargetChanged( TNULL );
	AGameLoader::Dev_RequestStory( s_iStoryAct, s_iStoryTier );
	return TTRUE;
}

static TBOOL StoryAdvanceCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	AGameLoader::Dev_GetAdvancedTier( atoi( a_ppArgv[ 1 ] ), s_iStoryAct, s_iStoryTier );
	OnStoryTargetChanged( TNULL );
	AGameLoader::Dev_RequestStory( s_iStoryAct, s_iStoryTier );
	return TTRUE;
}

static TBOOL StoryResetCommand( TINT a_iArgc, TCHAR** a_ppArgv, PCFunc* a_pFunc )
{
	s_iStoryAct  = 1;
	s_iStoryTier = 1;
	AGameLoader::Dev_RequestStory( 0, 0 );
	return TTRUE;
}

void APrimeDebugQuest::Register()
{
	s_oStoryActVar.CreateIntRefRange( "story.act", "The act story.goto goes to", PCVar::FLAGS_VALIDATERANGE, &s_iStoryAct, OnStoryTargetChanged, TNULL, 1, 3, 1 );
	s_oStoryTierVar.CreateIntRefRange( "story.tier", "The tier of story.act story.goto goes to (16, 20 and 18 tiers)", PCVar::FLAGS_VALIDATERANGE, &s_iStoryTier, OnStoryTargetChanged, TNULL, 1, 20, 1 );
	s_oStoryCurActVar.CreateIntRef( "story.info.act", "The current act (0 before the story has started)", PCVar::FLAGS_READONLY, &s_iStoryCurAct, TNULL, TNULL );
	s_oStoryCurTierVar.CreateIntRef( "story.info.tier", "The current tier of the act", PCVar::FLAGS_READONLY, &s_iStoryCurTier, TNULL, TNULL );
	s_oStoryCurAbsTierVar.CreateIntRef( "story.info.abs_tier", "The current tier counted over all the acts", PCVar::FLAGS_READONLY, &s_iStoryCurAbsTier, TNULL, TNULL );

	// Read-only strings keep their value in these buffers
	PCVarValue oNoString;
	memset( &oNoString, 0, sizeof( oNoString ) );
	s_oStoryQuestVar.Create( "story.info.quest", "The assigned quest", PCVar::FLAGS_READONLY, PCVar::TYPE_STRING, &oNoString, s_szStoryQuest, TNULL, TNULL );

	for ( TINT i = 0; i < AQuestManager::MAX_NUM_TIER_QUESTS; i++ )
		s_aStoryTierQuestVars[ i ].Create( s_aStoryTierQuestVarNames[ i ], "A quest of the current tier", PCVar::FLAGS_READONLY, PCVar::TYPE_STRING, &oNoString, s_aszStoryTierQuests[ i ], TNULL, TNULL );

	s_oStoryGotoFunc.Create( "story.goto", "Starts a new game at the act and tier (story.act and story.tier without arguments), the quests of the earlier tiers completed. A game going on returns to the frontend first.", "story.goto {act tier}", StoryGotoCommand, PCFunc::ANY_NUM_ARGS, TNULL );
	s_oStoryAdvanceFunc.Create( "story.advance", "Starts a new game at the tier n tiers after the first one (like story.goto).", "story.advance <n>", StoryAdvanceCommand, 1, TNULL );
	s_oStoryResetFunc.Create( "story.reset", "Starts a new game at act 1 tier 1 (the dev save file, never the game's save).", "story.reset", StoryResetCommand, 0, TNULL );

	PDebugMenu::AddSubmenu( PDebugMenu::ROOT, "Quests" );
	PDebugMenu::AddVar( "./Quests", "story.act" );
	PDebugMenu::AddVar( "./Quests", "story.tier" );
	PDebugMenu::AddCommandLine( "./Quests", "Go to act/tier", "story.goto" );
	PDebugMenu::AddCommandLine( "./Quests", "Reset story", "story.reset" );
	PDebugMenu::AddSubmenu( "./Quests", "Current" );
	PDebugMenu::AddVar( "./Quests/Current", "story.info.*" );
}

void APrimeDebugQuest::Unregister()
{
	s_oStoryResetFunc.Remove();
	s_oStoryAdvanceFunc.Remove();
	s_oStoryGotoFunc.Remove();

	for ( TINT i = 0; i < AQuestManager::MAX_NUM_TIER_QUESTS; i++ )
		s_aStoryTierQuestVars[ i ].Remove();

	s_oStoryQuestVar.Remove();
	s_oStoryCurAbsTierVar.Remove();
	s_oStoryCurTierVar.Remove();
	s_oStoryCurActVar.Remove();
	s_oStoryTierVar.Remove();
	s_oStoryActVar.Remove();
}

void APrimeDebugQuest::Update( TFLOAT a_fDeltaTime )
{
	s_iStoryCurAct     = 0;
	s_iStoryCurTier    = 0;
	s_iStoryCurAbsTier = 0;
	PString::Copy( s_szStoryQuest, sizeof( s_szStoryQuest ), "-" );

	for ( TINT i = 0; i < AQuestManager::MAX_NUM_TIER_QUESTS; i++ )
		PString::Copy( s_aszStoryTierQuests[ i ], sizeof( s_aszStoryTierQuests[ i ] ), "-" );

	if ( !AQuestManager::IsSingletonCreated() )
		return;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( pQuestManager->GetAct() < 1 || pQuestManager->GetTier() < 1 || pQuestManager->GetNumTiers() < 1 )
		return;

	s_iStoryCurAct     = pQuestManager->GetAct();
	s_iStoryCurTier    = pQuestManager->GetTier();
	s_iStoryCurAbsTier = pQuestManager->GetCurAbsQuestTier();

	if ( s_iStoryCurAbsTier < 1 || s_iStoryCurAbsTier > pQuestManager->GetNumTiers() )
		return;

	if ( AQuestInstance* pInstance = pQuestManager->GetAssignedQuest( 0 ) )
		PString::Copy( s_szStoryQuest, sizeof( s_szStoryQuest ), pInstance->GetQuest()->GetName().GetString() );

	AQuestManager::QuestTier* pTier = pQuestManager->GetCurrentQuestTier();

	for ( TINT i = 0; i < pTier->uiNumQuests && i < AQuestManager::MAX_NUM_TIER_QUESTS; i++ )
	{
		AQuest* pQuest = pQuestManager->GetQuest( pTier->auiQuests[ i ] );
		PString::Format( s_aszStoryTierQuests[ i ], sizeof( s_aszStoryTierQuests[ i ] ), "%s%s", pQuest->GetName().GetString(), ( pTier->uiCompletedMask & ( 1 << i ) ) ? " (done)" : "" );
	}
}

#endif // PRIMEDEBUG_DEV
