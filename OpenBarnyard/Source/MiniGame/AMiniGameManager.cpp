#include "pch.h"
#include "AMiniGameManager.h"
#include "Player/APlayerManager.h"
#include "GameInterface/AGameState.h"

#include <Toshi/TClass.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AMiniGameManager );

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
    : m_iNumVisibleMiniGames( 0 )
{
	TIMPLEMENT();

	RegisterMiniGame( "AChickenFireMinigameState", "AChickenFireMinigameState", "Chicken_Launch", 0x20a, 0x20b, 8, TPS8( bkg_Chicken_Launch ), TFALSE, 1, 4, 7, 2, 0 )
	    .AddVariant( 0x2d1, 0x334, 0x335 )
	    .AddVariant( 0x2d2, 0x334, 0x335 )
	    .AddVariant( 0x2d3, 0x334, 0x335 )
	    .AddVariant( 0x2d4, 0x334, 0x335 );

	RegisterMiniGame( "AGolfMiniGameState", "AGolfMinigameState", "Golf", 0x1fb, 0x1fc, 8, TPS8( bkg_Gopher_Golf ), TFALSE, 1, 4, 7, 1, 0 )
	    .AddVariant( 0x2d6, 0x336, 0x337 )
	    .AddVariant( 0x2d7, 0x336, 0x337 )
	    .AddVariant( 0x2d8, 0x336, 0x337 )
	    .AddVariant( 0x2d9, 0x336, 0x337 )
	    .AddVariant( 0x2da, 0x34e, 0x34f )
	    .AddVariant( 0x2db, 0x34e, 0x34f );

	RegisterMiniGame( "AStatuesMiniGameState", "AStatuesMiniGameState", "Mailman", 0x215, 0x216, 0, TPS8( bkg_Mailman ), TFALSE, 1, 1, 7, 2, 0 )
	    .AddVariant( 0x2e9, 0x322, 0x323 )
	    .AddVariant( 0x2ea, 0x322, 0x323 );

	RegisterMiniGame( "ACarChaseMiniGameState", "ACarChaseMiniGameState", "Joy_Ride", 0x28f, 0x290, 7, TPS8( bkg_Joy_Ride ), TFALSE, 1, 4, 3, 1, 2 )
	    .AddVariant( 0x2e2, 0x33a, 0x33b )
	    .AddVariant( 0x2e3, 0x33a, 0x33b )
	    .AddVariant( 0x2e4, 0x33a, 0x33b )
	    .AddVariant( 0x2e5, 0x33a, 0x33b )
	    .AddVariant( 0x2e6, 0x33c, 0x33d )
	    .AddVariant( 0x2e7, 0x33e, 0x33f )
	    .AddVariant( 0x2e8, 0x340, 0x341 );

	RegisterMiniGame( "AGateCrashMiniGameState", "AGateCrashMinigameState", "Mud", 0x1f9, 0x1fa, 3, TPS8( bkg_Mud_Jump ), TTRUE, 4, 4, 7, 1, 0 )
	    .AddVariant( 0x2cf, 0x31e, 0x31f )
	    .AddVariant( 0x2d0, 0x31e, 0x31f );

	RegisterMiniGame( "ABarnYardPoolMiniGameState", "ABarnYardPoolMiniGameState", "Pool", 0x263, 0x264, 0, TPS8( bkg_Barn_Pool ), TTRUE, 2, 4, -1, 3, 0 )
	    .AddVariant( 0x2dc, 0x324, 0x325 )
	    .AddVariant( 0x2dd, 0x324, 0x325 )
	    .AddVariant( 0x2de, 0x324, 0x325 )
	    .AddVariant( 0x2df, 0x324, 0x325 )
	    .AddVariant( 0x2e0, 0x324, 0x325 )
	    .AddVariant( 0x2e1, 0x324, 0x325 );

	RegisterMiniGame( "ABikeRaceMicroGame", "ABikeRaceMicroGame", "Bike_Race", 0x213, 0x214, 0, TPS8( bkg_Bike_Race ), TFALSE, 1, 1, 7, 1, 1 )
	    .AddVariant( 0x2ed, 0x326, 0x327 )
	    .AddVariant( 0x2ee, 0x326, 0x327 )
	    .AddVariant( 0x2ef, 0x326, 0x327 )
	    .AddVariant( 0x2f0, 0x326, 0x327 )
	    .AddVariant( 0x2f1, 0x326, 0x327 )
	    .AddVariant( 0x2f2, 0x326, 0x327 );

	RegisterMiniGame( "AChasingChicksMiniGame", "AChasingChicksMiniGame", "Chicks", 0x265, 0x266, 0, TPS8( bkg_Chasing_Chicks ), TFALSE, 1, 1, 0, 1, 1 )
	    .AddVariant( 0x2cc, 0x32a, 0x32b )
	    .AddVariant( 0x2cd, 0x32a, 0x32b );

	RegisterMiniGame( "AVeggiePatchDefenderGame", "AVeggiePatchDefenderGame", "Veggie", 0x2a6, 0x2a7, 0, TPS8( bkg_Veggie ), TFALSE, 1, 1, 5, 1, 0 )
	    .AddVariant( 0x2fd, 0x328, 0x329 )
	    .AddVariant( 0x2fe, 0x328, 0x329 )
	    .AddVariant( 0x2ff, 0x328, 0x329 );

	RegisterMiniGame( "ACowTippingMiniGameState", "ACowTippingMiniGameState", "Cow_Tipping", 0x257, 0x258, 7, TPS8( bkg_Cow_Tipping ), TFALSE, 1, 4, 7, 1, 0 )
	    .AddVariant( 0x24f, 0x344, 0x345 )
	    .AddVariant( 0x250, 0x346, 0x347 )
	    .AddVariant( 0x251, 0x352, 0x353, "AFinalMissionMiniGameState" );

	RegisterMiniGame( "ATapperMiniGameState", "ATapperMiniGameState", "BarnTender", 0x259, 0x25a, 7, TPS8( bkg_Barntender ), TFALSE, 1, 4, -1, 1, 0 )
	    .AddVariant( 0x25f, 0x348, 0x349 )
	    .AddVariant( 0x260, 0x348, 0x349 )
	    .AddVariant( 0x261, 0x348, 0x349 )
	    .AddVariant( 0x262, 0x348, 0x349 );

	RegisterMiniGame( "ADartMiniGame", "ADartMiniGame", "Darts", 0x305, 0x306, 0, TPS8( bkg_Darts ), TTRUE, 2, 2, -1, 1, 0 )
	    .AddVariant( 0x310, 0x34c, 0x34d );

	RegisterMiniGame( "AWhackARaccMiniGameState", "AWhackARaccMiniGameState", "Whack", 0x2a8, 0x2a9, 7, TPS8( bkg_Whack_a_Rack ), TFALSE, 1, 4, -1, 1, 0 )
	    .AddVariant( -1, 0x34a, 0x34b );

	RegisterMiniGame( "ATeaseMrsBeadyGame", "AStatuesMiniGameState", "Beady", 0x217, 0x218, 0, TPS8( bkg_Beady ), TFALSE, 1, 1, -1, 0, 0 )
	    .AddVariant( 0x2eb, 0x350, 0x351 )
	    .AddVariant( 0x2ec, 0x350, 0x351 );

	RegisterHiddenMiniGame( "AHoneyCollectingGame", "AHoneyCollectingGame", "Honey", 0x21b, 0x21c, 0, TPS8( bkg_Honey ), 7, 0 )
	    .AddVariant( -1, 0x32c, 0x32d );

	AMiniGame& rSharpSquirt  = RegisterHiddenMiniGame( "APrecisionSquirt", "APrecisionSquirt", "Sharp_Squirt", 0x26b, 0x26c, 0, TPS8( bkg_Sharp_Squirter ), 0, 1 );
	rSharpSquirt.field6_0x18 = 1;
	rSharpSquirt.AddVariant( 0x2f3, 0x320, 0x321 )
	    .AddVariant( 0x2f4, 0x320, 0x321 )
	    .AddVariant( 0x2f5, 0x320, 0x321 )
	    .AddVariant( 0x2f6, 0x320, 0x321 )
	    .AddVariant( 0x2f7, 0x320, 0x321 );

	RegisterHiddenMiniGame( "AChickenCoopDefender", "AChickenCoopDefender", "Coop_Defender", 0x268, 0x26a, 0, TPS8( bkg_Chicken_Coop_Defender ), 6, 1 )
	    .AddVariant( 0x243, 0x32e, 0x32f )
	    .AddVariant( 0x242, 0x332, 0x333 )
	    .AddVariant( 0x269, 0x330, 0x331 );
}

AMiniGameManager::~AMiniGameManager()
{
}

// $Barnyard: FUNCTION 00469d30
AMiniGame& AMiniGameManager::RegisterMiniGame( const TCHAR* a_szMiniGameName, const TCHAR* a_szClassStateName, const TCHAR* a_szIconName, TINT a_eLocaleTitle, TINT a_eLocaleDescription, TINT a_iUnk1, const Toshi::TPString8& a_strLoadingScreenName, TBOOL a_bUnk2, TINT a_iMinNumPlayers, TINT a_iMaxNumPlayers, TINT a_iUnk5, TINT a_iOrderId, TINT a_iUnk7 )
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
	rMiniGame.m_eInstancesType         = a_iUnk5;
	rMiniGame.m_iDefaultVariant        = a_iOrderId - 1;
	rMiniGame.m_iNumSelectableVariants = 0;
	rMiniGame.field20_0xfc             = a_iUnk1;
	rMiniGame.field21_0x100            = a_iUnk7;

	if ( rMiniGame.m_iDefaultVariant != -1 )
		m_iNumVisibleMiniGames += 1;

	return rMiniGame;
}

// $Barnyard: FUNCTION 00469e60
AMiniGame& AMiniGameManager::RegisterHiddenMiniGame( const TCHAR* a_szMiniGameName, const TCHAR* a_szClassStateName, const TCHAR* a_szIconName, TINT a_eLocaleTitle, TINT a_eLocaleDescription, TINT a_iUnk1, const Toshi::TPString8& a_strLoadingScreenName, TINT a_iUnk2, TINT a_iUnk3 )
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
	rMiniGame.field21_0x100            = a_iUnk3;
	rMiniGame.m_eInstancesType         = a_iUnk2;
	rMiniGame.field7_0x1c              = TFALSE;
	rMiniGame.m_iNumSelectableVariants = 0;
	rMiniGame.field20_0xfc             = a_iUnk1;

	return rMiniGame;
}

// $Barnyard: FUNCTION 00469340
void AMiniGameManager::MakeValidMiniGamePlayerSet( TINT a_iMiniGame )
{
	APlayerManager* pPlyrMgr = APlayerManager::GetSingleton();
	APlayerSet*     pPlyrSet = pPlyrMgr->GetPlayerSet( 1 );

	// Add AIs until it's enough players in the set
	const TINT iMiniGameNumPlayers = m_vecMiniGames[ a_iMiniGame ].m_iMinNumPlayers;
	while ( iMiniGameNumPlayers < pPlyrSet->GetTotalNumPlayers() )
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

	return TNULL;
}

// $Barnyard: FUNCTION 00469390
TINT AMiniGameManager::GetHiddenMiniGameIndex( TINT a_iIndex )
{
	TINT iIndex = 0;

	T2_FOREACH( m_vecMiniGames, it )
	{
		if ( it->IsPlaceholder() )
		{
			if ( iIndex == a_iIndex )
				return it.Index();

			iIndex += 1;
		}
	}

	return 0;
}
