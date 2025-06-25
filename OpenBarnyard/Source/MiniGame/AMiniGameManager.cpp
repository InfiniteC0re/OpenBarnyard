#include "pch.h"
#include "AMiniGameManager.h"
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
	    .AddVariant( 0x2db, 0x34e, 0x34f )
	    .AddVariant( 0x2e9, 0x322, 0x323 )
	    .AddVariant( 0x2ea, 0x322, 0x323 );

	RegisterMiniGame( "AStatuesMiniGameState", "AStatuesMiniGameState", "Mailman", 0x215, 0x216, 0, TPS8( bkg_Mailman ), TFALSE, 1, 1, 7, 2, 0 )
	    .AddVariant( 0x2e9, 0x322, 0x323 )
	    .AddVariant( 0x2ea, 0x322, 0x323 );

	RegisterMiniGame( "ACarChaseMiniGameState", "ACarChaseMiniGameState", "Joy_Ride", 0x28f, 0x290, 7, TPS8( bkg_Joy_Ride ), TFALSE, 1, 4, 3, 1, 2 )
	    .AddVariant( 0x2e2, 0x33a, 0x33b )
	    .AddVariant( 0x2e4, 0x33a, 0x33b )
	    .AddVariant( 0x2e5, 0x33a, 0x33b )
	    .AddVariant( 0x2e6, 0x33c, 0x33d )
	    .AddVariant( 0x2e7, 0x33e, 0x33f )
	    .AddVariant( 0x2e8, 0x340, 0x341 );

	RegisterMiniGame( "AGateCrashMiniGameState", "AGateCrashMinigameState", "Mud", 0x1f9, 0x1fa, 3, TPS8( bkg_Mud_Jump ), TTRUE, 4, 4, 7, 1, 0 )
	    .AddVariant( 0x2cf, 0x31e, 0x31f )
	    .AddVariant( 0x2d0, 0x31e, 0x31f );

	// TODO: Register other minigames
}

AMiniGameManager::~AMiniGameManager()
{
}

// $Barnyard: FUNCTION 00469d30
AMiniGame& AMiniGameManager::RegisterMiniGame( const TCHAR* a_szMiniGameName, const TCHAR* a_szClassStateName, const TCHAR* a_szIconName, TINT a_eLocaleTitle, TINT a_eLocaleDescription, TINT a_iUnk1, const Toshi::TPString8& a_strLoadingScreenName, TBOOL a_bUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5, TINT a_iOrderId, TINT a_iUnk7 )
{
	AMiniGame& rMiniGame = m_vecMiniGames.PushBack();

	rMiniGame.m_eLocaleTitle       = a_eLocaleTitle;
	rMiniGame.m_eLocaleDescription = a_eLocaleDescription;
	rMiniGame.m_pGameStateClass    = TClass::Find( a_szClassStateName, &TGetClass( AGameState ) );
	rMiniGame.field4_0x10          = a_iUnk3;
	rMiniGame.field7_0x1c          = a_bUnk2;
	rMiniGame.field5_0x14          = a_iUnk4;
	rMiniGame.m_pLoadingScreenName = &a_strLoadingScreenName;
	rMiniGame.m_szIconName         = a_szIconName;
	rMiniGame.field13_0x28         = a_iUnk5;
	rMiniGame.m_iAvailabilityFlag  = a_iOrderId - 1;
	rMiniGame.m_iNumUnknown2       = 0;
	rMiniGame.field20_0xfc         = a_iUnk1;
	rMiniGame.field21_0x100        = a_iUnk7;

	if ( rMiniGame.m_iAvailabilityFlag != -1 )
		m_iNumVisibleMiniGames += 1;

	return rMiniGame;
}

// $Barnyard: FUNCTION 00469e60
AMiniGame& AMiniGameManager::RegisterHiddenMiniGame( const TCHAR* a_szMiniGameName, const TCHAR* a_szClassStateName, const TCHAR* a_szIconName, TINT a_eLocaleTitle, TINT a_eLocaleDescription, TINT a_iUnk1, const Toshi::TPString8& a_strLoadingScreenName, TINT a_iUnk2, TINT a_iUnk3 )
{
	AMiniGame& rMiniGame = m_vecMiniGames.PushBack();

	rMiniGame.m_eLocaleTitle       = a_eLocaleTitle;
	rMiniGame.m_eLocaleDescription = a_eLocaleDescription;
	rMiniGame.m_pGameStateClass    = TClass::Find( a_szClassStateName, &TGetClass( AGameState ) );
	rMiniGame.m_pLoadingScreenName = &a_strLoadingScreenName;
	rMiniGame.field4_0x10          = 1;
	rMiniGame.field5_0x14          = 1;
	rMiniGame.field6_0x18          = 1;
	rMiniGame.m_szIconName         = a_szIconName;
	rMiniGame.m_iAvailabilityFlag  = -1;
	rMiniGame.field21_0x100        = a_iUnk3;
	rMiniGame.field13_0x28         = a_iUnk2;
	rMiniGame.field7_0x1c          = TFALSE;
	rMiniGame.m_iNumUnknown2       = 0;
	rMiniGame.field20_0xfc         = a_iUnk1;

	return rMiniGame;
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
		if ( it->m_iAvailabilityFlag != -1 )
		{
			if ( iIndex == a_iIndex )
				return it;

			iIndex += 1;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00469390
AMiniGame* AMiniGameManager::GetHiddenMiniGame( TINT a_iIndex )
{
	TINT iIndex = 0;

	T2_FOREACH( m_vecMiniGames, it )
	{
		if ( it->m_iAvailabilityFlag == -1 )
		{
			if ( iIndex == a_iIndex )
				return it;

			iIndex += 1;
		}
	}

	return TNULL;
}
