#include "pch.h"
#include "AMiniGameManager.h"
#include "Player/APlayerManager.h"
#include "GameInterface/AGameState.h"
#include "GUI/AGUISlideshow.h"
#include "Helpers/AGameLoader.h"
#include "Cameras/ACameraManager.h"
#include "Quests/AQuestManager.h"
#include "GameInterface/AGameStateController.h"
#include "Input/AInputHandler.h"
#include "FSM/AFunctionTable.h"
#include "FSM/AMethodPointer.h"
#include "Player/APlayerSet.h"
#include "Units/AUnit.h"

#include <Toshi/TClass.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 0046b2a0, CreateObjectInPlace 0046b300
TDEFINE_CLASS_INIT( AMiniGameManager, 1, 0 );

AFunctionTable AMiniGameManager::ms_oFunctionTable( &TGetClass( AMiniGameManager ), TNULL );

// The natives of the table are called on the manager
// $Barnyard: FUNCTION 00469190
static TObject* __stdcall AMiniGameManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AMiniGameManager::GetSingleton();
}

// $Barnyard: FUNCTION 00469a50
void AMiniGameManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AMiniGameManagerFunctionFallback ) );

	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiniGameManager, const TPString8&>::Create( "SetNextMinigame", &AMiniGameManager::SetNextMinigame ) );
	ms_oFunctionTable.AddFunction( AMethodPointer2Args<AMiniGameManager, const TPString8&, TINT>::Create( "SetNextMinigameVariant", &AMiniGameManager::SetNextMinigameVariant ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AMiniGameManager, const TPString8&>::Create( "SetNextMinigameNoFade", &AMiniGameManager::SetNextMinigameNoFade ) );
}

// $Barnyard: FUNCTION 00469130
void AMiniGameManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

// $Barnyard: FUNCTION 0046b1a0
AFunctionTable* AMiniGameManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 004699a0
void AMiniGameManager::ClearNextMiniGame()
{
	m_strNextMiniGameName = TPString8();
	m_bHasSimResults      = TFALSE;
	m_iNextMiniGameIndex  = -1;
}

// $Barnyard: FUNCTION 004698b0
void AMiniGameManager::SetNextMinigame( const TPString8& a_rcName )
{
	SetNextMinigameVariant( a_rcName, 1 );
}

// $Barnyard: FUNCTION 00469830
void AMiniGameManager::SetNextMinigameVariant( const TPString8& a_rcName, TINT a_iVariant )
{
	m_strNextMiniGameName = a_rcName;
	m_iMiniGameVariant    = a_iVariant - 1;

	AGameStateController::GetSingleton()->ShowOverlay( TTRUE, TTRUE, 1.0f, TFALSE );
	ms_bHasFade = TTRUE;
}

// $Barnyard: FUNCTION 00469890
void AMiniGameManager::SetNextMinigameNoFade( const TPString8& a_rcName )
{
	SetNextMinigameVariant( a_rcName, 1 );
	ms_bHasFade = TFALSE;
}

TPSTRING8_DECLARE( bkg_Chicken_Launch );
TPSTRING8_DECLARE( bkg_Gopher_Golf );
TPSTRING8_DECLARE( bkg_Mailman );
TPSTRING8_DECLARE( bkg_Joy_Ride );
TPSTRING8_DECLARE( bkg_Mud_Jump );
TPSTRING8_DECLARE( bkg_Barn_Pool );
TPSTRING8_DECLARE( bkg_Bike_Race );
TPSTRING8_DECLARE( bkg_Chasing_Chicks );
TPSTRING8_DECLARE( bkg_Veggie );
TPSTRING8_DECLARE( bkg_Cow_Tipping );
TPSTRING8_DECLARE( bkg_Barntender );
TPSTRING8_DECLARE( bkg_Darts );
TPSTRING8_DECLARE( bkg_Whack_a_Rack );
TPSTRING8_DECLARE( bkg_Beady );
TPSTRING8_DECLARE( bkg_Honey );
TPSTRING8_DECLARE( bkg_Sharp_Squirter );
TPSTRING8_DECLARE( bkg_Chicken_Coop_Defender );

// $Barnyard: FUNCTION 0046a020
AMiniGameManager::AMiniGameManager()
    : m_iNextMiniGameIndex( -1 )
    , m_bHasSimResults( TFALSE )
    , m_bQuitRequested( TFALSE )
    , m_bReturnRequested( TFALSE )
    , m_iMiniGameVariant( 0 )
    , m_iNumVisibleMiniGames( 0 )
{
	RegisterMiniGame( "AChickenFireMinigameState", "AChickenFireMinigameState", "Chicken_Launch", 0x20a, 0x20b, 8, TPS8( bkg_Chicken_Launch ), TFALSE, 1, 4, 7, 2, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2d1, 0x334, 0x335 )
	    .AddVariant( 0x2d2, 0x334, 0x335 )
	    .AddVariant( 0x2d3, 0x334, 0x335 )
	    .AddVariant( 0x2d4, 0x334, 0x335 );

	RegisterMiniGame( "AGolfMiniGameState", "AGolfMinigameState", "Golf", 0x1fb, 0x1fc, 8, TPS8( bkg_Gopher_Golf ), TFALSE, 1, 4, 7, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2d6, 0x336, 0x337 )
	    .AddVariant( 0x2d7, 0x336, 0x337 )
	    .AddVariant( 0x2d8, 0x336, 0x337 )
	    .AddVariant( 0x2d9, 0x336, 0x337 )
	    .AddVariant( 0x2da, 0x34e, 0x34f )
	    .AddVariant( 0x2db, 0x34e, 0x34f );

	RegisterMiniGame( "AStatuesMiniGameState", "AStatuesMiniGameState", "Mailman", 0x215, 0x216, 0, TPS8( bkg_Mailman ), TFALSE, 1, 1, 7, 2, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2e9, 0x322, 0x323 )
	    .AddVariant( 0x2ea, 0x322, 0x323 );

	RegisterMiniGame( "ACarChaseMiniGameState", "ACarChaseMiniGameState", "Joy_Ride", 0x28f, 0x290, 7, TPS8( bkg_Joy_Ride ), TFALSE, 1, 4, 3, 1, AMiniGame::LOADFLAGS_REGROWTH )
	    .AddVariant( 0x2e2, 0x33a, 0x33b )
	    .AddVariant( 0x2e3, 0x33a, 0x33b )
	    .AddVariant( 0x2e4, 0x33a, 0x33b )
	    .AddVariant( 0x2e5, 0x33a, 0x33b )
	    .AddVariant( 0x2e6, 0x33c, 0x33d )
	    .AddVariant( 0x2e7, 0x33e, 0x33f )
	    .AddVariant( 0x2e8, 0x340, 0x341 );

	RegisterMiniGame( "AGateCrashMiniGameState", "AGateCrashMinigameState", "Mud", 0x1f9, 0x1fa, 3, TPS8( bkg_Mud_Jump ), TTRUE, 4, 4, 7, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2cf, 0x31e, 0x31f )
	    .AddVariant( 0x2d0, 0x31e, 0x31f );

	RegisterMiniGame( "ABarnYardPoolMiniGameState", "ABarnYardPoolMiniGameState", "Pool", 0x263, 0x264, 0, TPS8( bkg_Barn_Pool ), TTRUE, 2, 4, -1, 3, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2dc, 0x324, 0x325 )
	    .AddVariant( 0x2dd, 0x324, 0x325 )
	    .AddVariant( 0x2de, 0x324, 0x325 )
	    .AddVariant( 0x2df, 0x324, 0x325 )
	    .AddVariant( 0x2e0, 0x324, 0x325 )
	    .AddVariant( 0x2e1, 0x324, 0x325 );

	RegisterMiniGame( "ABikeRaceMicroGame", "ABikeRaceMicroGame", "Bike_Race", 0x213, 0x214, 0, TPS8( bkg_Bike_Race ), TFALSE, 1, 1, 7, 1, AMiniGame::LOADFLAGS_GATES )
	    .AddVariant( 0x2ed, 0x326, 0x327 )
	    .AddVariant( 0x2ee, 0x326, 0x327 )
	    .AddVariant( 0x2ef, 0x326, 0x327 )
	    .AddVariant( 0x2f0, 0x326, 0x327 )
	    .AddVariant( 0x2f1, 0x326, 0x327 )
	    .AddVariant( 0x2f2, 0x326, 0x327 );

	RegisterMiniGame( "AChasingChicksMiniGame", "AChasingChicksMiniGame", "Chicks", 0x265, 0x266, 0, TPS8( bkg_Chasing_Chicks ), TFALSE, 1, 1, 0, 1, AMiniGame::LOADFLAGS_GATES )
	    .AddVariant( 0x2cc, 0x32a, 0x32b )
	    .AddVariant( 0x2cd, 0x32a, 0x32b );

	RegisterMiniGame( "AVeggiePatchDefenderGame", "AVeggiePatchDefenderGame", "Veggie", 0x2a6, 0x2a7, 0, TPS8( bkg_Veggie ), TFALSE, 1, 1, 5, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2fd, 0x328, 0x329 )
	    .AddVariant( 0x2fe, 0x328, 0x329 )
	    .AddVariant( 0x2ff, 0x328, 0x329 );

	RegisterMiniGame( "ACowTippingMiniGameState", "ACowTippingMiniGameState", "Cow_Tipping", 0x257, 0x258, 7, TPS8( bkg_Cow_Tipping ), TFALSE, 1, 4, 7, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x24f, 0x344, 0x345 )
	    .AddVariant( 0x250, 0x346, 0x347 )
	    .AddVariant( 0x251, 0x352, 0x353, "AFinalMissionMiniGameState" );

	RegisterMiniGame( "ATapperMiniGameState", "ATapperMiniGameState", "BarnTender", 0x259, 0x25a, 7, TPS8( bkg_Barntender ), TFALSE, 1, 4, -1, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x25f, 0x348, 0x349 )
	    .AddVariant( 0x260, 0x348, 0x349 )
	    .AddVariant( 0x261, 0x348, 0x349 )
	    .AddVariant( 0x262, 0x348, 0x349 );

	RegisterMiniGame( "ADartMiniGame", "ADartMiniGame", "Darts", 0x305, 0x306, 0, TPS8( bkg_Darts ), TTRUE, 2, 2, -1, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x310, 0x34c, 0x34d );

	RegisterMiniGame( "AWhackARaccMiniGameState", "AWhackARaccMiniGameState", "Whack", 0x2a8, 0x2a9, 7, TPS8( bkg_Whack_a_Rack ), TFALSE, 1, 4, -1, 1, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( -1, 0x34a, 0x34b );

	RegisterMiniGame( "ATeaseMrsBeadyGame", "AStatuesMiniGameState", "Beady", 0x217, 0x218, 0, TPS8( bkg_Beady ), TFALSE, 1, 1, -1, 0, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( 0x2eb, 0x350, 0x351 )
	    .AddVariant( 0x2ec, 0x350, 0x351 );

	RegisterHiddenMiniGame( "AHoneyCollectingGame", "AHoneyCollectingGame", "Honey", 0x21b, 0x21c, 0, TPS8( bkg_Honey ), 7, AMiniGame::LOADFLAGS_NONE )
	    .AddVariant( -1, 0x32c, 0x32d );

	AMiniGame& rSharpSquirt  = RegisterHiddenMiniGame( "APrecisionSquirt", "APrecisionSquirt", "Sharp_Squirt", 0x26b, 0x26c, 0, TPS8( bkg_Sharp_Squirter ), 0, AMiniGame::LOADFLAGS_GATES );
	rSharpSquirt.field6_0x18 = 1;
	rSharpSquirt.AddVariant( 0x2f3, 0x320, 0x321 )
	    .AddVariant( 0x2f4, 0x320, 0x321 )
	    .AddVariant( 0x2f5, 0x320, 0x321 )
	    .AddVariant( 0x2f6, 0x320, 0x321 )
	    .AddVariant( 0x2f7, 0x320, 0x321 );

	RegisterHiddenMiniGame( "AChickenCoopDefender", "AChickenCoopDefender", "Coop_Defender", 0x268, 0x26a, 0, TPS8( bkg_Chicken_Coop_Defender ), 6, AMiniGame::LOADFLAGS_GATES )
	    .AddVariant( 0x243, 0x32e, 0x32f )
	    .AddVariant( 0x242, 0x332, 0x333 )
	    .AddVariant( 0x269, 0x330, 0x331 );

#ifdef BARNYARD_LATEST
	// The XUI load screens of the Wii build (the Coyote Showdown one is picked by the loaders)
	// NOTE: the Wii build sets them right after registering each minigame (AMiniGame::SetXUILoadScreen)
	static constexpr const TCHAR* s_aXUILoadScreens[][ 2 ] = {
		{ "AChickenFireMinigameState", "ChickenLaunch_LoadScreen" },
		{ "AGolfMiniGameState", "Gophergolf_LoadScreen" },
		{ "AStatuesMiniGameState", "TeaseTheMailman_LoadScreen" },
		{ "ACarChaseMiniGameState", "JoyRide_LoadScreen" },
		{ "AGateCrashMiniGameState", "MudJumpers_LoadScreen" },
		{ "ABarnYardPoolMiniGameState", "BarnyardPool_LoadScreen" },
		{ "ABikeRaceMicroGame", "Bike_Race_LoadScreen" },
		{ "AChasingChicksMiniGame", "ChasingChicks_LoadScreen" },
		{ "AVeggiePatchDefenderGame", "VegetablePatchDefender_LoadScreen" },
		{ "ACowTippingMiniGameState", "CowTipping_LoadScreen" },
		{ "ATapperMiniGameState", "MilkBar_LoadScreen" },
		{ "ADartMiniGame", "BarnyardDarts_LoadScreen" },
		{ "AWhackARaccMiniGameState", "WhackARac_LoadScreen" },
		{ "ATeaseMrsBeadyGame", "Teasemrsbeady_LoadScreen" },
		{ "AHoneyCollectingGame", "HoneyCollecting_LoadScreen" },
		{ "APrecisionSquirt", "SharpSquirter_LoadScreen" },
		{ "AChickenCoopDefender", "ChickenCoopDefender_LoadScreen" },
	};

	for ( TINT i = 0; i < TARRAYSIZE( s_aXUILoadScreens ); i++ )
	{
		TINT iMiniGame = FindMiniGame( TPString8( s_aXUILoadScreens[ i ][ 0 ] ) );

		if ( iMiniGame != -1 )
			m_vecMiniGames[ iMiniGame ].SetXUILoadScreen( TPString8( s_aXUILoadScreens[ i ][ 1 ] ) );
	}
#endif // BARNYARD_LATEST
}

// $Barnyard: FUNCTION 0046b1b0
// $Barnyard: FUNCTION 0046b340
AMiniGameManager::~AMiniGameManager()
{
}

// 0x00773818
static const TCHAR* s_aSimStates[] = { "ASimState", "ABarnExploreMinigameState" };

// $Barnyard: FUNCTION 00469ac0
TBOOL AMiniGameManager::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bQuitRequested == TTRUE )
	{
		// The player has quit from the start menu
		AGameLoader::LeaveMiniGame( TFALSE );
		m_bQuitRequested = TFALSE;
	}

	if ( m_bReturnRequested == TTRUE )
	{
		ACameraManager::GetSingleton()->DetachCameraHelpers();
		ACameraManager::GetSingleton()->ResetHelperTargets();
		AGameLoader::ReturnToFrontEnd( TNULL, TTRUE, TTRUE );
		m_bReturnRequested = TFALSE;
	}

	// Let the quests know the results once the sim is back
	if ( m_bHasSimResults )
	{
		TPString8 strState( AGameStateController::GetSingleton()->GetCurrentState()->GetClass()->GetName() );

		for ( TINT i = 0; i < TARRAYSIZE( s_aSimStates ); i++ )
		{
			if ( TStringManager::String8Compare( strState.GetString(), s_aSimStates[ i ] ) == 0 )
			{
				AQuestManager::GetSingleton()->OnMiniGameResult( &m_oLastResults );
				m_bHasSimResults = TFALSE;
			}
		}
	}

	// Start the minigame requested by the sim (see AAnimalConversationState and the quests)
	if ( !m_strNextMiniGameName.IsEmpty() )
	{
		if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->HasInteraction() )
		{
			// Wait until the player is done with the interaction
			AGameStateController::GetSingleton()->ShowOverlay( TTRUE, TTRUE, 1.0f, TFALSE );
		}
		else
		{
			TINT iMiniGame = FindMiniGame( m_strNextMiniGameName );

			// The main player plays it alone
			APlayerManager::GetSingleton()->GetPlayerSet( 1 )->Reset();
			APlayerManager::GetSingleton()->GetPlayerSet( 1 )->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 );

			TPString8 strClassName;
			GetStateClassName( strClassName, iMiniGame );
			AQuestManager::GetSingleton()->OnMiniGameStart( strClassName );

			MakeValidMiniGamePlayerSet( iMiniGame );
			APlayerManager::GetSingleton()->GetPlayerSet( 1 )->FinishUpAddingAIs();

			AGameStateController::GetSingleton()->ShowOverlay( TFALSE, TTRUE, 1.0f, TFALSE );
			AGameLoader::SetTransitionToMiniGame( m_strNextMiniGameName, TTRUE );

			m_strNextMiniGameName = TPString8();
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00469d30
AMiniGame& AMiniGameManager::RegisterMiniGame( const TCHAR* a_szMiniGameName, const TCHAR* a_szClassStateName, const TCHAR* a_szIconName, TINT a_eLocaleTitle, TINT a_eLocaleDescription, TINT a_eStartHudKind, const Toshi::TPString8& a_strLoadingScreenName, TBOOL a_bUnk2, TINT a_iMinNumPlayers, TINT a_iMaxNumPlayers, TINT a_eInstancesType, TINT a_iOrderId, AMiniGame::LOADFLAGS a_eLoadFlags )
{
	AMiniGame& rMiniGame = m_vecMiniGames.PushBack();

	rMiniGame.m_strMiniGameName        = a_szMiniGameName;
	rMiniGame.m_eLocaleTitle           = a_eLocaleTitle;
	rMiniGame.m_eLocaleDescription     = a_eLocaleDescription;
	rMiniGame.m_pGameStateClass        = TClass::Find( a_szClassStateName, &TGetClass( AGameState ) );
	rMiniGame.m_iMinNumPlayers         = a_iMinNumPlayers;
	rMiniGame.m_iMaxNumPlayers         = a_iMaxNumPlayers;
	rMiniGame.field7_0x1c              = a_bUnk2;
	rMiniGame.m_pLoadingScreenName     = &a_strLoadingScreenName;
	rMiniGame.m_szIconName             = a_szIconName;
	rMiniGame.m_eInstancesType         = a_eInstancesType;
	rMiniGame.m_iDefaultVariant        = a_iOrderId - 1;
	rMiniGame.m_iNumSelectableVariants = 0;
	rMiniGame.m_eStartHudKind          = a_eStartHudKind;
	rMiniGame.m_eLoadFlags             = a_eLoadFlags;

	if ( rMiniGame.m_iDefaultVariant != -1 )
		m_iNumVisibleMiniGames += 1;

	return rMiniGame;
}

// $Barnyard: FUNCTION 00469e60
AMiniGame& AMiniGameManager::RegisterHiddenMiniGame( const TCHAR* a_szMiniGameName, const TCHAR* a_szClassStateName, const TCHAR* a_szIconName, TINT a_eLocaleTitle, TINT a_eLocaleDescription, TINT a_eStartHudKind, const Toshi::TPString8& a_strLoadingScreenName, TINT a_eInstancesType, AMiniGame::LOADFLAGS a_eLoadFlags )
{
	AMiniGame& rMiniGame = m_vecMiniGames.PushBack();

	rMiniGame.m_strMiniGameName        = a_szMiniGameName;
	rMiniGame.m_eLocaleTitle           = a_eLocaleTitle;
	rMiniGame.m_eLocaleDescription     = a_eLocaleDescription;
	rMiniGame.m_pGameStateClass        = TClass::Find( a_szClassStateName, &TGetClass( AGameState ) );
	rMiniGame.m_pLoadingScreenName     = &a_strLoadingScreenName;
	rMiniGame.m_iMinNumPlayers         = 1;
	rMiniGame.m_iMaxNumPlayers         = 1;
	rMiniGame.field6_0x18              = 1;
	rMiniGame.m_szIconName             = a_szIconName;
	rMiniGame.m_iDefaultVariant        = -1;
	rMiniGame.m_eLoadFlags             = a_eLoadFlags;
	rMiniGame.m_eInstancesType         = a_eInstancesType;
	rMiniGame.field7_0x1c              = TFALSE;
	rMiniGame.m_iNumSelectableVariants = 0;
	rMiniGame.m_eStartHudKind          = a_eStartHudKind;

	return rMiniGame;
}

// $Barnyard: FUNCTION 00469340
void AMiniGameManager::MakeValidMiniGamePlayerSet( TINT a_iMiniGame )
{
	APlayerManager* pPlyrMgr = APlayerManager::GetSingleton();
	APlayerSet*     pPlyrSet = pPlyrMgr->GetPlayerSet( 1 );

	// Add AIs until it's enough players in the set
	const TINT iMiniGameNumPlayers = m_vecMiniGames[ a_iMiniGame ].m_iMinNumPlayers;
	while ( pPlyrSet->GetTotalPlayerCount() < iMiniGameNumPlayers )
		pPlyrSet->AddAIPlayer();

	pPlyrSet->MakeTeamsFair();
}

// $Barnyard: FUNCTION 004692c0
AMiniGame* AMiniGameManager::GetMiniGame( TINT a_iIndex )
{
	TASSERT( a_iIndex < m_vecMiniGames.Size() );
	return &m_vecMiniGames[ a_iIndex ];
}

// $Barnyard: FUNCTION 004692e0
AMiniGame* AMiniGameManager::GetVisibleMiniGame( TINT a_iIndex )
{
	TINT iIndex = 0;

	T2_FOREACH( m_vecMiniGames, it )
	{
		if ( !it->IsPlaceholder() )
		{
			if ( iIndex == a_iIndex )
				return it;

			iIndex += 1;
		}
	}

	// NOTE: retail falls back to the start of the element array (the first mini game), not TNULL.
	// Callers in AFrontEndMiniGameState2 dereference the result without a check.
	return m_vecMiniGames.Data();
}

// $Barnyard: FUNCTION 00469390
TINT AMiniGameManager::GetIndexForAntic( TINT a_iIndex )
{
	TINT iIndex = 0;

	T2_FOREACH( m_vecMiniGames, it )
	{
		if ( !it->IsPlaceholder() )
		{
			if ( iIndex == a_iIndex )
				return it.Index();

			iIndex += 1;
		}
	}

	return 0;
}

// $Barnyard: FUNCTION 00469240
const TCHAR* AMiniGameManager::GetLibraryName( TINT a_iMiniGame )
{
	AMiniGame& rMiniGame = m_vecMiniGames[ a_iMiniGame ];

	if ( m_iMiniGameVariant >= 0 && m_iMiniGameVariant < rMiniGame.m_iNumStateNames )
	{
		const TCHAR* szAlternativeName = rMiniGame.m_aszAlternativeStateNames[ m_iMiniGameVariant ];

		if ( szAlternativeName )
			return szAlternativeName;
	}

	return rMiniGame.m_strMiniGameName.GetString8().GetString();
}

// $Barnyard: FUNCTION 004698f0
TPString8& AMiniGameManager::GetStateClassName( TPString8& a_rOutName, TINT a_iMiniGame )
{
	a_rOutName = TPString8( m_vecMiniGames[ a_iMiniGame ].m_pGameStateClass->GetName() );
	return a_rOutName;
}

// $Barnyard: FUNCTION 00469930
TPString8& AMiniGameManager::GetLoadingScreenName( TPString8& a_rOutName, TINT a_iMiniGame )
{
	const TPString8* pLoadingScreenName = m_vecMiniGames[ a_iMiniGame ].m_pLoadingScreenName;

	if ( !pLoadingScreenName )
	{
		a_rOutName = TPString8();
		return a_rOutName;
	}

	AGUISlideshow::LocaliseBackgroundFileName( a_rOutName, *pLoadingScreenName );
	return a_rOutName;
}

#ifdef BARNYARD_LATEST

// $BarnyardWiiJP: FUNCTION 8018927c
const TPString8& AMiniGameManager::GetXUILoadScreenName( TINT a_iMiniGame )
{
	static TPString8 s_strEmpty;

	if ( a_iMiniGame == -1 )
		return s_strEmpty;

	return m_vecMiniGames[ a_iMiniGame ].GetXUILoadScreen();
}

#endif // BARNYARD_LATEST

// $Barnyard: FUNCTION 00469970
void AMiniGameManager::StoreResults( AMiniGameState::S1* a_pResults )
{
	CopyResults( m_oLastResults, *a_pResults );

	if ( AGameLoader::IsStoryMode() )
		m_bHasSimResults = TTRUE;
}

// $Barnyard: FUNCTION 00469630
void AMiniGameManager::CopyResults( AMiniGameState::S1& a_rDest, AMiniGameState::S1& a_rSource )
{
	for ( TINT i = 0; i < AMiniGameState::MAX_PLAYERS; i++ )
	{
		AMiniGameState::PlayerInfo& rDest   = a_rDest.oPlayerList[ i ];
		AMiniGameState::PlayerInfo& rSource = a_rSource.oPlayerList[ i ];

		rDest.bUnk1 = rSource.bUnk1;
		rDest.iUnk2 = rSource.iUnk2;
		rDest.iUnk3 = rSource.iUnk3;

		rDest.vecRows.Clear();
		T2_FOREACH( rSource.vecRows, it )
		{
			rDest.vecRows.PushBack( *it );
		}

		rDest.wszTotalName = rSource.wszTotalName;
		rDest.iTotal       = rSource.iTotal;
	}

	a_rDest.iResult          = a_rSource.iResult;
	a_rDest.iMiniGameIndex   = a_rSource.iMiniGameIndex;
	a_rDest.iMiniGameVariant = a_rSource.iMiniGameVariant;
	a_rDest.strUnk           = a_rSource.strUnk;
	a_rDest.bFlag2           = a_rSource.bFlag2;
	a_rDest.bWon             = a_rSource.bWon;
	a_rDest.bFlag5           = a_rSource.bFlag5;
}

// $Barnyard: FUNCTION 004691a0
TINT AMiniGameManager::FindMiniGame( const TPString8& a_rcName )
{
	for ( TINT i = 0; i < m_vecMiniGames.Size(); i++ )
	{
		if ( TStringManager::String8CompareNoCase( m_vecMiniGames[ i ].GetName().GetString(), a_rcName.GetString() ) == 0 )
			return i;
	}

	return -1;
}

// $Barnyard: FUNCTION 00469210
TINT AMiniGameManager::FindMiniGameByTitle( TINT a_eLocaleTitle )
{
	for ( TINT i = 0; i < m_vecMiniGames.Size(); i++ )
	{
		if ( m_vecMiniGames[ i ].GetLocaleTitle() == a_eLocaleTitle )
			return i;
	}

	return -1;
}
