#include "pch.h"
#include "AReplayManager.h"
#include "AHighScoreManager.h"
#include "FSM/AMethodPointer.h"
#include "Animation/AModel.h"
#include "Animation/AModelRepos.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimatableObjectType.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "GameInterface/AReplayVariantSelectState.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"
#include "MiniGame/AMiniGameManager.h"
#include "MiniGame/AMiniGame.h"
#include "MiniGame/AGopherShopInterface.h"
#include "Quests/AQuestManager.h"
#include "Terrain/ATerrainInterface.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"

#include <Toshi/TStringManager.h>
#include <Toshi/TSceneObject.h>
#include <Render/TRenderInterface.h>
#include <Render/TRenderContext.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( ACoinPickupMicroGame );
TPSTRING8_DECLARE( AHoneyCollectingGame );
TPSTRING8_DECLARE( ASleepCycleState );
TPSTRING8_DECLARE( barnpool_replay );
TPSTRING8_DECLARE( barntender_replay );
TPSTRING8_DECLARE( barnyard_squirt_replay );
TPSTRING8_DECLARE( beady_car_joyride );
TPSTRING8_DECLARE( beady_squirt_replay );
TPSTRING8_DECLARE( benshill_squirt_replay );
TPSTRING8_DECLARE( chase_chick_replay );
TPSTRING8_DECLARE( chase_chick_two_replay );
TPSTRING8_DECLARE( chick_coop_replay );
TPSTRING8_DECLARE( chickenlaunch_four_replay );
TPSTRING8_DECLARE( chickenlaunch_one_replay );
TPSTRING8_DECLARE( chickenlaunch_two_replay );
TPSTRING8_DECLARE( cowtipping_replay );
TPSTRING8_DECLARE( darts_replay );
TPSTRING8_DECLARE( freddy_dream_replay );
TPSTRING8_DECLARE( golf_backnine_replay );
TPSTRING8_DECLARE( golf_nineholes_replay );
TPSTRING8_DECLARE( golf_squirt_replay );
TPSTRING8_DECLARE( honey_collecting_replay );
TPSTRING8_DECLARE( honey_collecting_replay2 );
TPSTRING8_DECLARE( izzy_precision_golf );
TPSTRING8_DECLARE( mud_jumpers_replay );
TPSTRING8_DECLARE( precision_back_nine );
TPSTRING8_DECLARE( star );
TPSTRING8_DECLARE( star_completed );
TPSTRING8_DECLARE( star_unavailable );
TPSTRING8_DECLARE( tease_beady_replay );
TPSTRING8_DECLARE( tease_mailman_replay );
TPSTRING8_DECLARE( veggie_patch_replay );
TPSTRING8_DECLARE( whack_replay );

// NOTE: GetClass 00546d00, CreateObject 0054b160, CreateObjectInPlace 0054b1c0
TDEFINE_CLASS_INIT( AReplayManager, 1, 0 );

// DAT_00783dc8
AFunctionTable AReplayManager::ms_oFunctionTable( &TGetClass( AReplayManager ), TNULL );

// The current act and tier of the story
static TINT GetStoryAct() { return AQuestManager::GetSingleton()->GetAct(); }
static TINT GetStoryTier() { return AQuestManager::GetSingleton()->GetTier(); }

// TTRUE if the story is past a_iAct/a_iTier
static TBOOL IsStoryPast( TINT a_iAct, TINT a_iTier )
{
	TINT iAct = GetStoryAct();
	return iAct > a_iAct || ( iAct == a_iAct && GetStoryTier() > a_iTier );
}

static TBOOL IsItNight()
{
	return AGameTimeManager::GetSingleton()->IsItNight();
}

// The golf replays aren't available while the gopher trap tier is played
// $Barnyard: FUNCTION 005470a0
static TBOOL IsNotInGopherTrapTier()
{
	TINT iAct  = GetStoryAct();
	TINT iTier = GetStoryTier();

	TINT iTrapAct, iTrapTier;
	AQuestManager::GetSingleton()->FindQuestTier( "Otis Gopher Trap", &iTrapAct, &iTrapTier );

	return iAct != iTrapAct || iTrapTier != iTier;
}

// $Barnyard: FUNCTION 00547010
static void RenderStarModel( AModelInstance* a_pModel )
{
	TSceneObject* pSceneObject = a_pModel->GetSceneObject();
	pSceneObject->EnableSkeletonUpdate();
	pSceneObject->RenderIfVisible();
	pSceneObject->DisableSkeletonUpdate();
}

// Spins a_pModel by a_fAngle
// $Barnyard: FUNCTION 00546d50
static void RotateStarModel( TFLOAT a_fAngle, AModelInstance* a_pModel )
{
	TMatrix44 matRotation;
	matRotation.Identity();
	matRotation.RotateZ( a_fAngle );

	TMatrix44 matTransform;
	a_pModel->GetLocalTransform( matTransform );

	TMatrix44 matLocal = matTransform;
	matTransform.Multiply( matLocal, matRotation );
	a_pModel->SetTransform( matTransform );
}

// FSM fallback: the natives are called on the singleton
// $Barnyard: FUNCTION 00547090
static TObject* __stdcall AReplayManagerFunctionFallback( AFunctionTable* a_pTable, const AFunctionCaller* a_pCaller )
{
	return AReplayManager::GetSingletonSafe();
}

// $Barnyard: FUNCTION 0054b200
void AReplayManager::InitialiseClass()
{
	ms_oFunctionTable.Register( reinterpret_cast<void*>( &AReplayManagerFunctionFallback ) );
	ms_oFunctionTable.AddFunction( AMethodPointer1Args<AReplayManager, TPString8>::Create( "ReplayMiniGame", &AReplayManager::ReplayMiniGame ) );
}

// $Barnyard: FUNCTION 00546e90
void AReplayManager::DeinitialiseClass()
{
	ms_oFunctionTable.Unregister();
}

//-----------------------------------------------------------------------------
// AReplayableGame
//-----------------------------------------------------------------------------

// NOTE: inlined in retail (AReplayManager::AReplayManager)
AReplayableGame::AReplayableGame()
    : m_iMiniGame( -1 )
    , m_iAct( -1 )
    , m_iTier( -1 )
    , m_iHighScoreRecord( -1 )
    , m_iMapIcon( -1 )
    , m_eLocation( 0 )
    , m_eStarState( STARSTATE_NONE )
    , m_pStar( TNULL )
    , m_pStarUnavailable( TNULL )
    , m_pStarCompleted( TNULL )
    , m_pEffect( TNULL )
    , m_pNext( TNULL )
    , m_bDisabled( TFALSE )
    , m_iNight( 0 )
{
	for ( TINT i = 0; i < MAX_NUM_VARIANTS; i++ )
		m_abVariants[ i ] = TFALSE;
}

// $Barnyard: FUNCTION 005481c0
// $Barnyard: FUNCTION 00548480
// $Barnyard: FUNCTION 0054a020
// $Barnyard: FUNCTION 0054a040
AReplayableGame::~AReplayableGame()
{
}

// $Barnyard: FUNCTION 00548280
void AReplayableGame::Create( TPString8 a_strLocator, const TPString8& a_rcMiniGame, LOCATION a_eLocation, TPString8 a_strName, TPString8 a_strLocator2 )
{
	Reset();

	// The locator is the name if there's none
	TPString8 strName = a_strName;

	if ( strName.IsEmpty() && strName != a_strLocator )
		strName = a_strLocator;

	m_strName     = strName;
	m_strLocator  = a_strLocator;
	m_strLocator2 = a_strLocator2;
	m_iMiniGame   = AMiniGameManager::GetSingleton()->FindMiniGame( a_rcMiniGame );
	m_strMiniGame = a_rcMiniGame;
	m_eLocation   = a_eLocation;
}

// $Barnyard: FUNCTION 005470f0
TBOOL AReplayableGame::IsTierReached() const
{
	return IsStoryPast( m_iAct, m_iTier );
}

// $Barnyard: FUNCTION 00547120
void AReplayableGame::GetStarPosition( TVector4& a_rOutPosition ) const
{
	TVector4 vecPosition = TVector4::VEC_ZERO;

	if ( m_pStar )
	{
		m_pStar->GetPosition( vecPosition );
		a_rOutPosition = vecPosition;
	}
	else
	{
		a_rOutPosition = TVector4::VEC_ZERO;
	}
}

// $Barnyard: FUNCTION 00546d10
TINT AReplayableGame::GetNumVariants() const
{
	TINT iNumVariants = 0;

	for ( TINT i = 0; i < MAX_NUM_VARIANTS; i++ )
	{
		if ( m_abVariants[ i ] == TTRUE )
			iNumVariants++;
	}

	return iNumVariants;
}

// $Barnyard: FUNCTION 00546ea0
TBOOL AReplayableGame::HasMultipleVariants() const
{
	TINT iNumVariants = 0;

	for ( TINT i = 0; i < MAX_NUM_VARIANTS; i++ )
		iNumVariants += m_abVariants[ i ];

	return iNumVariants > 1;
}

// NOTE: inlined in retail, clears m_abVariants
static void ClearVariants( TBOOL* a_pVariants )
{
	for ( TINT i = 0; i < AReplayableGame::MAX_NUM_VARIANTS; i++ )
		a_pVariants[ i ] = TFALSE;
}

// $Barnyard: FUNCTION 005476e0
TBOOL AGenericReplayGame::IsAvailable()
{
	if ( ( IsItNight() && m_iNight == 1 ) || ( !IsItNight() && m_iNight == 0 ) )
	{
		if ( IsTierReached() )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005471b0
TBOOL AReplayVeggiePatch::IsAvailable()
{
	ClearVariants( m_abVariants );

	TINT iAct  = GetStoryAct();
	TINT iTier = GetStoryTier();

	if ( IsItNight() )
		return TFALSE;

	TINT iAct1, iTier1, iAct2, iTier2, iAct3, iTier3;
	AQuestManager::GetSingleton()->FindQuestTier( "Ben Veggie Patch 1", &iAct1, &iTier1 );
	AQuestManager::GetSingleton()->FindQuestTier( "BEN Veggie Patch 2", &iAct2, &iTier2 );
	AQuestManager::GetSingleton()->FindQuestTier( "Otis Veggie Patch 3", &iAct3, &iTier3 );

	if ( iAct > iAct3 || ( iAct == iAct3 && iTier > iTier3 ) )
	{
		m_abVariants[ 2 ] = TTRUE;
		return TTRUE;
	}

	if ( ( iAct == iAct2 && iTier > iTier2 ) || ( iAct == iAct3 && iTier < iTier3 ) )
	{
		m_abVariants[ 1 ] = TTRUE;
		return TTRUE;
	}

	if ( ( iAct == iAct1 && iTier > iTier1 ) || ( iAct == iAct2 && iTier < iTier2 ) )
	{
		m_abVariants[ 0 ] = TTRUE;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005472b0
TBOOL AReplayChickDefender::IsAvailable()
{
	TINT iTier = GetStoryTier();
	TINT iAct  = GetStoryAct();

	ClearVariants( m_abVariants );

	if ( !IsItNight() )
		return TFALSE;

	TINT iQuestAct, iQuestTier;
	AQuestManager::GetSingleton()->FindQuestTier( "Miles Coyote Tipping", &iQuestAct, &iQuestTier );

	if ( iAct > iQuestAct || ( iAct == iQuestAct && iTier > iQuestTier ) )
	{
		m_abVariants[ 0 ] = TTRUE;
		return TTRUE;
	}

	AQuestManager::GetSingleton()->FindQuestTier( "Otis Chicken Coop 2: Coyotes", &iQuestAct, &iQuestTier );

	if ( iAct > iQuestAct || ( iAct == iQuestAct && iTier > iQuestTier ) )
	{
		m_abVariants[ 1 ] = TTRUE;
		return TTRUE;
	}

	AQuestManager::GetSingleton()->FindQuestTier( "Ben (Night) Chicken Coop 1", &iQuestAct, &iQuestTier );

	if ( iAct > iQuestAct || ( iAct == iQuestAct && iTier > iQuestTier ) )
	{
		m_abVariants[ 0 ] = TTRUE;
		return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 005473a0
TBOOL AReplayDarts::IsAvailable()
{
	ClearVariants( m_abVariants );

	// Item 971 of the gopher shop
	if ( !IsItNight() || AGopherShopInterface::GetNumBought( 971 ) <= 0 )
		return TFALSE;

	m_abVariants[ 0 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 005473e0
TBOOL AReplayBarnPool::IsAvailable()
{
	ClearVariants( m_abVariants );

	if ( IsItNight() )
	{
		TINT iAct  = GetStoryAct();
		TINT iTier = GetStoryTier();

		TINT iQuestAct, iQuestTier;
		AQuestManager::GetSingleton()->FindQuestTier( "Duke (Night) Barn Pool", &iQuestAct, &iQuestTier );

		if ( iAct > iQuestAct || ( iAct == iQuestAct && iTier > iQuestTier ) )
		{
			m_abVariants[ 2 ] = TTRUE;
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00547520
TBOOL AReplayTeaseMailMan::IsAvailable()
{
	ClearVariants( m_abVariants );

	if ( IsItNight() || !IsTierReached() )
		return TFALSE;

	m_abVariants[ 1 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00547570
TBOOL AReplayTeaseBeady::IsAvailable()
{
	ClearVariants( m_abVariants );

	if ( !IsItNight() || !IsTierReached() )
		return TFALSE;

	m_abVariants[ 1 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 005475c0
TBOOL AReplayGolfBackNine::IsAvailable()
{
	if ( IsItNight() || !IsTierReached() || !IsNotInGopherTrapTier() )
		return TFALSE;

	m_abVariants[ 1 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00547610
TBOOL AReplayGolfFrontNine::IsAvailable()
{
	if ( IsItNight() || !IsTierReached() || !IsNotInGopherTrapTier() )
		return TFALSE;

	m_abVariants[ 0 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00547660
TBOOL AReplayBarntender::IsAvailable()
{
	// Only while the barn is explored
	if ( !AReplayManager::ms_pBarnExploreState || !IsTierReached() || !IsItNight() )
		return TFALSE;

	m_abVariants[ 0 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00547450
TBOOL AReplayCowTipping::IsAvailable()
{
	ClearVariants( m_abVariants );

	if ( !IsItNight() )
		return TFALSE;

	TINT iAct  = GetStoryAct();
	TINT iTier = GetStoryTier();

	TINT iQuestAct, iQuestTier;
	AQuestManager::GetSingleton()->FindQuestTier( "Bessy Raccoon Tipping", &iQuestAct, &iQuestTier );

	if ( iAct >= iQuestAct && iTier >= iQuestTier )
	{
		m_abVariants[ 1 ] = TTRUE;
		return TTRUE;
	}

	if ( !IsTierReached() )
		return TFALSE;

	iQuestAct  = -1;
	iQuestTier = -1;
	AQuestManager::GetSingleton()->FindQuestTier( "Jerseys Joy Ride 5", &iQuestAct, &iQuestTier );

	if ( iAct >= iQuestAct && iTier > iQuestTier )
		return TFALSE;

	m_abVariants[ 0 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 005476b0
TBOOL AReplayWhack::IsAvailable()
{
	// Item 972 of the gopher shop
	if ( !IsItNight() || AGopherShopInterface::GetNumBought( 972 ) <= 0 )
		return TFALSE;

	m_abVariants[ 0 ] = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 00548420
AReplayHoneyCollecting::AReplayHoneyCollecting()
{
	m_aiHoney[ 0 ] = 5;
	m_aiHoney[ 1 ] = 5;
	m_aiHoney[ 2 ] = 6;
}

// $Barnyard: FUNCTION 00548490
TBOOL AReplayHoneyCollecting::IsAvailable()
{
	ClearVariants( m_abVariants );

	if ( AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ACoinPickupMicroGame ) ) || IsItNight() )
		return TFALSE;

	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	if ( pQuestManager->GetNumAssignedQuests() > 0 )
	{
		// Only these quests can be assigned while the honey is collected
		AQuestInstance* pHoneyQuest    = pQuestManager->FindAssignedQuest( TPS8D( "HANNA Honey Collecting" ), 1 );
		AQuestInstance* pBirthdayQuest = pQuestManager->FindAssignedQuest( TPS8D( "Daisy Happy Birthday" ), 1 );

		if ( !pHoneyQuest && !pBirthdayQuest )
			return TFALSE;
	}

	m_abVariants[ 0 ] = TTRUE;

	TINT iAct = GetStoryAct();
	return iAct > m_iAct || ( iAct == m_iAct && GetStoryTier() >= m_iTier );
}

// $Barnyard: FUNCTION 00546e10
void AReplayHoneyCollecting::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < NUM_HIVES; i++ )
		a_pBuffer->WriteInt( m_aiHoney[ i ] );
}

// $Barnyard: FUNCTION 00546e40
void AReplayHoneyCollecting::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	for ( TINT i = 0; i < NUM_HIVES; i++ )
		a_pBuffer->ReadInt( m_aiHoney[ i ] );
}

// $Barnyard: FUNCTION 00546e70
void AReplayHoneyCollecting::Reset()
{
	m_aiHoney[ 0 ] = 5;
	m_aiHoney[ 1 ] = 5;
	m_aiHoney[ 2 ] = 6;
}

// $Barnyard: FUNCTION 00546dd0
void AReplayHoneyCollecting::RefillHives()
{
	// NOTE: retail also clamps the new values to [0, max] (TMath::Clip)
	for ( TINT i = 0; i < NUM_HIVES; i++ )
		m_aiHoney[ i ] = ( i == 2 ) ? 6 : 5;
}

//-----------------------------------------------------------------------------
// AReplayManager
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0054a060
AReplayManager::AReplayManager()
    : m_pReplays( TNULL )
    , m_bAllUnlocked( TFALSE )
    , m_bReplaysDisabled( TFALSE )
    , m_iBlockStartTier( -1 )
    , m_iBlockStartAct( -1 )
    , m_iBlockEndTier( -1 )
    , m_iBlockEndAct( -1 )
    , m_iCoyoteAct( -1 )
    , m_iCoyoteTier( -1 )
    , m_bStarsCreated( TFALSE )
{
	CreateReplays();
}

// $Barnyard: FUNCTION 0054ae00
// $Barnyard: FUNCTION 0054ade0
AReplayManager::~AReplayManager()
{
}

// $Barnyard: FUNCTION 0054add0
AFunctionTable* AReplayManager::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 00548610
void AReplayManager::CreateReplays()
{
	AQuestManager* pQuestManager = AQuestManager::GetSingleton();

	m_pReplays = &m_oChickenLaunch1;

	m_oChickenLaunch1.Create( TPS8( chickenlaunch_one_replay ), TPS8D( "AChickenFireMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( chickenlaunch_one_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Peck Chicken Launch 1", &m_oChickenLaunch1.m_iAct, &m_oChickenLaunch1.m_iTier );
	ClearVariants( m_oChickenLaunch1.m_abVariants );
	m_oChickenLaunch1.m_abVariants[ 0 ]  = TTRUE;
	m_oChickenLaunch1.m_iHighScoreRecord = 55;
	m_oChickenLaunch1.m_pNext            = &m_oChickenLaunch2;

	m_oChickenLaunch2.Create( TPS8( chickenlaunch_two_replay ), TPS8D( "AChickenFireMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( chickenlaunch_two_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Peck Chicken Launch 2", &m_oChickenLaunch2.m_iAct, &m_oChickenLaunch2.m_iTier );
	ClearVariants( m_oChickenLaunch2.m_abVariants );
	m_oChickenLaunch2.m_abVariants[ 1 ]  = TTRUE;
	m_oChickenLaunch2.m_pNext            = &m_oChickenLaunch3;
	m_oChickenLaunch2.m_iHighScoreRecord = 56;

	m_oChickenLaunch3.Create( TPS8( chickenlaunch_four_replay ), TPS8D( "AChickenFireMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( chickenlaunch_four_replay ), TPString8() );
	pQuestManager->FindQuestTier( "PECK Chicken Launch 3", &m_oChickenLaunch3.m_iAct, &m_oChickenLaunch3.m_iTier );
	ClearVariants( m_oChickenLaunch3.m_abVariants );
	m_oChickenLaunch3.m_abVariants[ 2 ]  = TTRUE;
	m_oChickenLaunch3.m_iHighScoreRecord = 57;
	m_oChickenLaunch3.m_pNext            = &m_oVeggiePatch;

	m_oVeggiePatch.Create( TPS8( veggie_patch_replay ), TPS8D( "AVeggiePatchDefenderGame" ), AReplayableGame::LOCATION_FARM, TPS8( veggie_patch_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Ben Veggie Patch 1", &m_oVeggiePatch.m_iAct, &m_oVeggiePatch.m_iTier );
	m_oVeggiePatch.m_iHighScoreRecord = 30;
	m_oVeggiePatch.m_pNext            = &m_oChickCoop;

	m_oChickCoop.Create( TPS8( chick_coop_replay ), TPS8D( "AChickenCoopDefender" ), AReplayableGame::LOCATION_FARM, TPS8( chick_coop_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Ben (Night) Chicken Coop 1", &m_oChickCoop.m_iAct, &m_oChickCoop.m_iTier );
	m_oChickCoop.m_iHighScoreRecord = 23;
	m_oChickCoop.m_pNext            = &m_oChaseChick;

	m_oChaseChick.Create( TPS8( chase_chick_replay ), TPS8D( "AChasingChicksMiniGame" ), AReplayableGame::LOCATION_FARM, TPS8( chase_chick_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Hanna Chasing Chicks 1", &m_oChaseChick.m_iAct, &m_oChaseChick.m_iTier );
	ClearVariants( m_oChaseChick.m_abVariants );
	m_oChaseChick.m_abVariants[ 0 ]  = TTRUE;
	m_oChaseChick.m_iHighScoreRecord = 25;
	m_oChaseChick.m_pNext            = &m_oChaseChickTwo;

	m_oChaseChickTwo.Create( TPS8( chase_chick_two_replay ), TPS8D( "AChasingChicksMiniGame" ), AReplayableGame::LOCATION_FARM, TPS8( chase_chick_two_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Hanna Chasing Chicks 2", &m_oChaseChickTwo.m_iAct, &m_oChaseChickTwo.m_iTier );
	ClearVariants( m_oChaseChickTwo.m_abVariants );
	m_oChaseChickTwo.m_abVariants[ 1 ]  = TTRUE;
	m_oChaseChickTwo.m_iHighScoreRecord = 26;
	m_oChaseChickTwo.m_pNext            = &m_oBeadyCarJoyride;

	m_oBeadyCarJoyride.Create( TPS8( beady_car_joyride ), TPS8D( "ACarChaseMiniGameState" ), AReplayableGame::LOCATION_FARM, TPS8( beady_car_joyride ), TPString8() );
	pQuestManager->FindQuestTier( "Jerseys Joy Ride 5", &m_oBeadyCarJoyride.m_iAct, &m_oBeadyCarJoyride.m_iTier );
	ClearVariants( m_oBeadyCarJoyride.m_abVariants );
	m_oBeadyCarJoyride.m_abVariants[ 6 ]  = TTRUE;
	m_oBeadyCarJoyride.m_iHighScoreRecord = 52;
	m_oBeadyCarJoyride.m_iNight           = 1;
	m_oBeadyCarJoyride.m_pNext            = &m_oDarts;

	m_oDarts.Create( TPS8( darts_replay ), TPS8D( "ADartMiniGame" ), AReplayableGame::LOCATION_BARN, TPS8( darts_replay ), TPString8() );
	m_oDarts.m_iHighScoreRecord = 41;
	m_oDarts.m_iAct             = 4;
	m_oDarts.m_iTier            = 1;
	m_oDarts.m_pNext            = &m_oBarnPool;

	m_oBarnPool.Create( TPS8( barnpool_replay ), TPS8D( "ABarnYardPoolMiniGameState" ), AReplayableGame::LOCATION_BARN, TPS8( barnpool_replay ), TPString8() );
	m_oBarnPool.m_iHighScoreRecord = 66;
	pQuestManager->FindQuestTier( "Duke (Night) Barn Pool", &m_oBarnPool.m_iAct, &m_oBarnPool.m_iTier );
	m_oBarnPool.m_pNext = &m_oTeaseMailMan;

	m_oTeaseMailMan.Create( TPS8( tease_mailman_replay ), TPS8D( "AStatuesMiniGameState" ), AReplayableGame::LOCATION_FARM, TPS8( tease_mailman_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Otis Statues", &m_oTeaseMailMan.m_iAct, &m_oTeaseMailMan.m_iTier );
	m_oTeaseMailMan.m_iHighScoreRecord = 40;
	m_oTeaseMailMan.m_pNext            = &m_oTeaseBeady;

	m_oTeaseBeady.Create( TPS8( tease_beady_replay ), TPS8D( "ATeaseMrsBeadyGame" ), AReplayableGame::LOCATION_FARM, TPS8( tease_beady_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Otis Tease Mrs Beady", &m_oTeaseBeady.m_iAct, &m_oTeaseBeady.m_iTier );
	m_oTeaseBeady.m_iHighScoreRecord = 38;
	m_oTeaseBeady.m_pNext            = &m_oGolfFrontNine;

	m_oGolfFrontNine.Create( TPS8( golf_nineholes_replay ), TPS8D( "AGolfMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( golf_nineholes_replay ), TPString8() );
	pQuestManager->FindQuestTier( "OTIS Golf 1: Three holes", &m_oGolfFrontNine.m_iAct, &m_oGolfFrontNine.m_iTier );
	m_oGolfFrontNine.m_iHighScoreRecord = 31;
	m_oGolfFrontNine.m_pNext            = &m_oGolfBackNine;

	m_oGolfBackNine.Create( TPS8( golf_backnine_replay ), TPS8D( "AGolfMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( golf_backnine_replay ), TPString8() );
	pQuestManager->FindQuestTier( "OTIS Golf 2: Six holes", &m_oGolfBackNine.m_iAct, &m_oGolfBackNine.m_iTier );
	m_oGolfBackNine.m_iHighScoreRecord = 32;
	m_oGolfBackNine.m_pNext            = &m_oIzzyPrecisionGolf;

	m_oIzzyPrecisionGolf.Create( TPS8( izzy_precision_golf ), TPS8D( "AGolfMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( izzy_precision_golf ), TPString8() );
	pQuestManager->FindQuestTier( "Otis Golf 5", &m_oIzzyPrecisionGolf.m_iAct, &m_oIzzyPrecisionGolf.m_iTier );
	ClearVariants( m_oIzzyPrecisionGolf.m_abVariants );
	m_oIzzyPrecisionGolf.m_abVariants[ 4 ]  = TTRUE;
	m_oIzzyPrecisionGolf.m_iHighScoreRecord = 35;
	m_oIzzyPrecisionGolf.m_pNext            = &m_oPrecisionBackNine;

	m_oPrecisionBackNine.Create( TPS8( precision_back_nine ), TPS8D( "AGolfMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( precision_back_nine ), TPString8() );
	pQuestManager->FindQuestTier( "Otis Golf 6", &m_oPrecisionBackNine.m_iAct, &m_oPrecisionBackNine.m_iTier );
	ClearVariants( m_oPrecisionBackNine.m_abVariants );
	m_oPrecisionBackNine.m_abVariants[ 5 ]  = TTRUE;
	m_oPrecisionBackNine.m_iHighScoreRecord = 36;
	m_oPrecisionBackNine.m_pNext            = &m_oMudJumpers;

	m_oMudJumpers.Create( TPS8( mud_jumpers_replay ), TPS8D( "AGateCrashMinigameState" ), AReplayableGame::LOCATION_FARM, TPS8( mud_jumpers_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Pig Mud Jumpers", &m_oMudJumpers.m_iAct, &m_oMudJumpers.m_iTier );
	ClearVariants( m_oMudJumpers.m_abVariants );
	m_oMudJumpers.m_abVariants[ 0 ]  = TTRUE;
	m_oMudJumpers.m_iHighScoreRecord = 53;
	m_oMudJumpers.m_pNext            = &m_oBarntender;

	m_oBarntender.Create( TPS8( barntender_replay ), TPS8D( "ATapperMiniGameState" ), AReplayableGame::LOCATION_BARN, TPS8( barntender_replay ), TPString8() );
	m_oBarntender.m_abVariants[ 0 ]  = TTRUE;
	m_oBarntender.m_iHighScoreRecord = 59;
	pQuestManager->FindQuestTier( "Root (Night) Barntender", &m_oBarntender.m_iAct, &m_oBarntender.m_iTier );
	m_oBarntender.m_pNext = &m_oCowTipping;

	m_oCowTipping.Create( TPS8( cowtipping_replay ), TPS8D( "ACowTippingMiniGameState" ), AReplayableGame::LOCATION_FARM, TPS8( cowtipping_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Ben (Night) Cow Tipping", &m_oCowTipping.m_iAct, &m_oCowTipping.m_iTier );
	m_oCowTipping.m_iHighScoreRecord = 44;
	m_oCowTipping.m_pNext            = &m_oHoneyCollecting;

	m_oHoneyCollecting.Create( TPS8( honey_collecting_replay ), TPS8D( "AHoneyCollectingGame" ), AReplayableGame::LOCATION_FARM, TPString8(), TPS8( honey_collecting_replay2 ) );
	pQuestManager->FindQuestTier( "HANNA Honey Collecting", &m_oHoneyCollecting.m_iAct, &m_oHoneyCollecting.m_iTier );
	m_oHoneyCollecting.m_pNext = &m_oWhack;

	m_oWhack.Create( TPS8( whack_replay ), TPS8D( "AWhackARaccMiniGameState" ), AReplayableGame::LOCATION_BARN, TPS8( whack_replay ), TPString8() );
	m_oWhack.m_iHighScoreRecord = 42;
	m_oWhack.m_iAct             = 4;
	m_oWhack.m_iTier            = 1;
	m_oWhack.m_pNext            = &m_oBarnyardSquirt;

	m_oBarnyardSquirt.Create( TPS8( barnyard_squirt_replay ), TPS8D( "APrecisionSquirt" ), AReplayableGame::LOCATION_FARM, TPS8( barnyard_squirt_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Miles Target Squirting 1", &m_oBarnyardSquirt.m_iAct, &m_oBarnyardSquirt.m_iTier );
	ClearVariants( m_oBarnyardSquirt.m_abVariants );
	m_oBarnyardSquirt.m_abVariants[ 0 ]  = TTRUE;
	m_oBarnyardSquirt.m_iHighScoreRecord = 12;
	m_oBarnyardSquirt.m_pNext            = &m_oBenshillSquirt;

	m_oBenshillSquirt.Create( TPS8( benshill_squirt_replay ), TPS8D( "APrecisionSquirt" ), AReplayableGame::LOCATION_FARM, TPString8(), TPString8() );
	pQuestManager->FindQuestTier( "Miles Target Squirting 2", &m_oBenshillSquirt.m_iAct, &m_oBenshillSquirt.m_iTier );
	ClearVariants( m_oBenshillSquirt.m_abVariants );
	m_oBenshillSquirt.m_abVariants[ 1 ]  = TTRUE;
	m_oBenshillSquirt.m_iHighScoreRecord = 13;
	m_oBenshillSquirt.m_pNext            = &m_oBeadySquirt;

	m_oBeadySquirt.Create( TPS8( beady_squirt_replay ), TPS8D( "APrecisionSquirt" ), AReplayableGame::LOCATION_FARM, TPString8(), TPString8() );
	pQuestManager->FindQuestTier( "MILES Sharp Squirter 3: Beady", &m_oBeadySquirt.m_iAct, &m_oBeadySquirt.m_iTier );
	ClearVariants( m_oBeadySquirt.m_abVariants );
	m_oBeadySquirt.m_abVariants[ 2 ]  = TTRUE;
	m_oBeadySquirt.m_iHighScoreRecord = 14;
	m_oBeadySquirt.m_pNext            = &m_oGolfSquirt;

	m_oGolfSquirt.Create( TPS8( golf_squirt_replay ), TPS8D( "APrecisionSquirt" ), AReplayableGame::LOCATION_FARM, TPString8(), TPString8() );
	pQuestManager->FindQuestTier( "Miles Sharp Squirter 4", &m_oGolfSquirt.m_iAct, &m_oGolfSquirt.m_iTier );
	ClearVariants( m_oGolfSquirt.m_abVariants );
	m_oGolfSquirt.m_abVariants[ 4 ]  = TTRUE;
	m_oGolfSquirt.m_iHighScoreRecord = 16;
	m_oGolfSquirt.m_pNext            = &m_oFreddyDream;

	m_oFreddyDream.Create( TPS8( freddy_dream_replay ), TPS8D( "AChickenCoopDefender" ), AReplayableGame::LOCATION_FARM, TPS8( freddy_dream_replay ), TPString8() );
	pQuestManager->FindQuestTier( "Peck Chicken Coop 3", &m_oFreddyDream.m_iAct, &m_oFreddyDream.m_iTier );
	ClearVariants( m_oFreddyDream.m_abVariants );
	m_oFreddyDream.m_abVariants[ 2 ]  = TTRUE;
	m_oFreddyDream.m_iHighScoreRecord = 24;
	m_oFreddyDream.m_iNight           = 1;
	m_oFreddyDream.m_pNext            = TNULL;

	pQuestManager->FindQuestTier( "Miles Coyote Tipping", &m_iCoyoteAct, &m_iCoyoteTier );
	FindBlockedTiers();
}

// $Barnyard: FUNCTION 00547c30
void AReplayManager::FindBlockedTiers()
{
	AQuestManager::GetSingleton()->FindQuestTier( "Daisy Flowers For Ben", &m_iBlockStartAct, &m_iBlockStartTier );
	AQuestManager::GetSingleton()->FindQuestTier( "Twink Round Up Chicks", &m_iBlockEndAct, &m_iBlockEndTier );
}

// $Barnyard: FUNCTION 00546ef0
TBOOL AReplayManager::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteString( "AReplayManager" );

	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
		pReplay->Save( a_pBuffer );

	a_pBuffer->WriteBool( m_bAllUnlocked );
	return TTRUE;
}

// $Barnyard: FUNCTION 00546f40
TBOOL AReplayManager::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->CheckString( "AReplayManager" );

	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
		pReplay->Load( a_pBuffer );

	a_pBuffer->ReadBool( m_bAllUnlocked );
	return TTRUE;
}

// $Barnyard: FUNCTION 00547040
void AReplayManager::Reset()
{
	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
		pReplay->Reset();

	m_bReplaysDisabled = TFALSE;
}

// $Barnyard: FUNCTION 00547bf0
TBOOL AReplayManager::IsInBlockedTier() const
{
	TINT iAct  = GetStoryAct();
	TINT iTier = GetStoryTier();

	// NOTE: retail checks the acts with ||, any act passes
	return ( iAct >= m_iBlockStartAct || iAct <= m_iBlockEndAct ) && iTier >= m_iBlockStartTier && iTier <= m_iBlockEndTier;
}

// NOTE: inlined in retail (IsReplayUnlocked, UpdateStar)
TBOOL AReplayManager::IsInCoyoteTier() const
{
	return GetStoryAct() == m_iCoyoteAct && GetStoryTier() == m_iCoyoteTier;
}

// $Barnyard: FUNCTION 00549ed0
AReplayableGame* AReplayManager::FindReplay( TPString8 a_strName )
{
	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( TStringManager::String8CompareNoCase( pReplay->m_strName.GetString(), a_strName.GetString() ) == 0 )
			return pReplay;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00549c70
TBOOL AReplayManager::IsReplayUnlocked( TPString8 a_strName, TBOOL a_bCheckStar )
{
	if ( IsInBlockedTier() || IsInCoyoteTier() )
		return TFALSE;

	AReplayableGame* pReplay = FindReplay( a_strName );

	if ( !pReplay )
		return TFALSE;

	if ( m_bAllUnlocked == TTRUE )
		return TTRUE;

	if ( a_bCheckStar == TTRUE )
	{
		AReplayableGame::STARSTATE eState = pReplay->m_eStarState;

		if ( eState != AReplayableGame::STARSTATE_AVAILABLE && eState != AReplayableGame::STARSTATE_COMPLETED && eState != AReplayableGame::STARSTATE_HIDDEN )
			return TFALSE;
	}

	return pReplay->IsAvailable();
}

// $Barnyard: FUNCTION 0054aff0
void AReplayManager::ReplayMiniGame( TPString8 a_strName )
{
	AReplayableGame* pReplay = FindReplay( a_strName );

	if ( !pReplay )
		return;

	if ( pReplay->HasMultipleVariants() )
	{
		// The player picks the variant
		AGameStateController::GetSingleton()->PushState( new AReplayVariantSelectState( pReplay ) );
	}
	else
	{
		TINT iVariant = -1;

		for ( TINT i = 0; i < AReplayableGame::MAX_NUM_VARIANTS; i++ )
		{
			if ( pReplay->m_abVariants[ i ] == TTRUE )
			{
				iVariant = i;
				break;
			}
		}

		if ( m_bAllUnlocked == TTRUE && iVariant == -1 )
			iVariant = 0;

		AMiniGameManager::GetSingleton()->SetNextMinigameVariant( pReplay->m_strMiniGame, iVariant + 1 );
		AMiniGameManager::ms_bHasFade = TFALSE;
	}
}

// $Barnyard: FUNCTION 005477f0
void AReplayManager::CreateStars( AReplayableGame::LOCATION a_eLocation )
{
	AAnimatableObjectManager* pManager             = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pStarType            = pManager->FindType( TPS8( star ) );
	AAnimatableObjectType*    pStarUnavailableType = pManager->FindType( TPS8( star_unavailable ) );
	AAnimatableObjectType*    pStarCompletedType   = pManager->FindType( TPS8( star_completed ) );

	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( a_eLocation == AReplayableGame::LOCATION_ALL || pReplay->m_eLocation == a_eLocation )
		{
			pReplay->m_eStarState = AReplayableGame::STARSTATE_NONE;

			pReplay->m_pStar = g_pModelRepos->InstantiateModel( pStarType->GetModel() );
			pReplay->m_pStar->SetSkeletonUpdating( TFALSE );

			pReplay->m_pStarCompleted = g_pModelRepos->InstantiateModel( pStarCompletedType->GetModel() );
			pReplay->m_pStarCompleted->SetSkeletonUpdating( TFALSE );

			pReplay->m_pStarUnavailable = g_pModelRepos->InstantiateModel( pStarUnavailableType->GetModel() );
			pReplay->m_pStarUnavailable->SetSkeletonUpdating( TFALSE );
		}
	}

	m_bStarsCreated = TTRUE;
}

// $Barnyard: FUNCTION 00547740
void AReplayManager::DestroyStars()
{
	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( pReplay->m_pStar )
		{
			g_pModelRepos->DestroyModelInstance( pReplay->m_pStar );
			pReplay->m_pStar = TNULL;
		}

		if ( pReplay->m_pStarCompleted )
		{
			g_pModelRepos->DestroyModelInstance( pReplay->m_pStarCompleted );
			pReplay->m_pStarCompleted = TNULL;
		}

		if ( pReplay->m_pStarUnavailable )
		{
			g_pModelRepos->DestroyModelInstance( pReplay->m_pStarUnavailable );
			pReplay->m_pStarUnavailable = TNULL;
		}

		ASimMiniMap* pMiniMap = AHudManager::GetSingleton()->FindElement<ASimMiniMap>();

		if ( pReplay->m_iMapIcon != -1 )
		{
			pMiniMap->RemoveIcon( pReplay->m_iMapIcon );
			pReplay->m_iMapIcon = -1;
		}

		if ( pReplay->m_pEffect )
		{
			pReplay->m_pEffect->GetInstance()->Kill();
			pReplay->m_pEffect = TNULL;
		}
	}

	m_bStarsCreated = TFALSE;
}

// $Barnyard: FUNCTION 005480f0
void AReplayManager::OnEnterFarm()
{
	CreateStars( AReplayableGame::LOCATION_FARM );
	ms_pBarnExploreState = TNULL;
}

// $Barnyard: FUNCTION 00549bf0
void AReplayManager::OnEnterBarn()
{
	CreateStars( AReplayableGame::LOCATION_BARN );
	ms_pBarnExploreState = AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8D( "ABarnExploreMinigameState" ) );
}

// $Barnyard: FUNCTION 00547b40
void AReplayManager::OnDateChange( AGameTimeManager::DAYPHASE a_eDayPhase )
{
	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		// The hives are full again every morning
		if ( a_eDayPhase == AGameTimeManager::DAYPHASE_DAWN && AMiniGameManager::GetSingleton()->GetMiniGame( pReplay->m_iMiniGame )->GetName() == TPS8( AHoneyCollectingGame ) )
			TSTATICCAST( AReplayHoneyCollecting, pReplay )->RefillHives();
	}
}

// $Barnyard: FUNCTION 0054b100
void AReplayManager::UpdateReplays()
{
	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( pReplay->m_pStar )
		{
			PlaceStar( pReplay );
			UpdateStar( pReplay );
		}
	}

	DisableStars();

	// The honey collecting game is started from the walnut entrance (AHintManager)
	m_oHoneyCollecting.m_eStarState = AReplayableGame::STARSTATE_HIDDEN;

	if ( m_oHoneyCollecting.m_pEffect )
	{
		m_oHoneyCollecting.m_pEffect->GetInstance()->Kill();
		m_oHoneyCollecting.m_pEffect = TNULL;
	}
}

// $Barnyard: FUNCTION 00547b90
TBOOL AReplayManager::DisableStars()
{
	if ( AQuestManager::GetSingleton()->GetNumAssignedQuests() <= 0 && !m_bReplaysDisabled )
		return TFALSE;

	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( pReplay->m_eStarState == AReplayableGame::STARSTATE_AVAILABLE || pReplay->m_eStarState == AReplayableGame::STARSTATE_COMPLETED )
		{
			pReplay->m_eStarState = AReplayableGame::STARSTATE_UNAVAILABLE;

			if ( pReplay->m_pEffect )
			{
				pReplay->m_pEffect->GetInstance()->Kill();
				pReplay->m_pEffect = TNULL;
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00549dd0
void AReplayManager::UpdateStar( AReplayableGame* a_pReplay )
{
	TBOOL bBlocked = IsInBlockedTier() || IsInCoyoteTier();

	if ( !a_pReplay->m_pStar )
		return;

	if ( m_bAllUnlocked || ( !a_pReplay->m_bDisabled && a_pReplay->IsAvailable() ) )
	{
		if ( bBlocked )
		{
			a_pReplay->m_eStarState = AReplayableGame::STARSTATE_UNAVAILABLE;
			return;
		}

		if ( a_pReplay->m_iHighScoreRecord == -1 || !AHighScoreManager::HasHumanStar( a_pReplay->m_iHighScoreRecord ) )
			a_pReplay->m_eStarState = AReplayableGame::STARSTATE_AVAILABLE;
		else
			a_pReplay->m_eStarState = AReplayableGame::STARSTATE_COMPLETED;

		PlayStarEffect( a_pReplay );
	}
	else
	{
		a_pReplay->m_eStarState = AReplayableGame::STARSTATE_NONE;

		if ( a_pReplay->m_pEffect )
		{
			a_pReplay->m_pEffect->GetInstance()->Kill();
			a_pReplay->m_pEffect = TNULL;
		}

		// Played in the story but not available now
		if ( a_pReplay->m_pStarUnavailable && a_pReplay->IsTierReached() )
			a_pReplay->m_eStarState = AReplayableGame::STARSTATE_UNAVAILABLE;
	}
}

// Finds the position of a locator, or of the interior area if there's no such locator
// NOTE: inlined in retail (PlaceStar)
static void GetLocatorPosition( const TPString8& a_rcLocator, TVector4& a_rOutPosition )
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
	const TCHAR*       szName   = a_rcLocator.GetString();

	if ( pTerrain->FindLocator( szName ) )
	{
		TQuaternion quatRotation;
		pTerrain->GetLocatorPosAndRot( szName, a_rOutPosition, quatRotation );
		return;
	}

	AInteriorNetwork* pNetwork = AInteriorNetworkManager::GetSingleton()->GetDefaultNetwork();

	if ( pNetwork )
	{
		TINT iArea = pNetwork->FindAreaIndex( a_rcLocator );

		if ( iArea >= 0 )
		{
			pNetwork->m_pINetwork->m_pAreas[ iArea ].GetPosition( a_rOutPosition );
			a_rOutPosition.y -= 20.0f;
			a_rOutPosition.y = pTerrain->GetGroundHeight( a_rOutPosition, TCollisionCommon::TOSHICGROUPFLAG_NONE ) - 2.0f;
		}
	}
}

// $Barnyard: FUNCTION 005478c0
void AReplayManager::PlaceStar( AReplayableGame* a_pReplay )
{
	TVector4 vecPosition = TVector4::VEC_ZERO;
	GetLocatorPosition( a_pReplay->m_strLocator, vecPosition );

	a_pReplay->m_pStar->SetTransform( vecPosition, TVector4::VEC_NEGX );

	if ( a_pReplay->m_pStarUnavailable )
		a_pReplay->m_pStarUnavailable->SetTransform( vecPosition, TVector4::VEC_NEGX );

	if ( a_pReplay->m_pStarCompleted )
		a_pReplay->m_pStarCompleted->SetTransform( vecPosition, TVector4::VEC_NEGX );

	// NOTE: retail also looks for the position of m_strLocator2 but doesn't use it
	if ( !a_pReplay->m_strLocator2.IsEmpty() )
	{
		TVector4 vecPosition2 = TVector4::VEC_ZERO;
		GetLocatorPosition( a_pReplay->m_strLocator2, vecPosition2 );
	}
}

// $Barnyard: FUNCTION 00548110
void AReplayManager::PlayStarEffect( AReplayableGame* a_pReplay )
{
	if ( !a_pReplay->m_pEffect )
	{
		TINT iEffect = AParticleEffectManager::FindEffect( "Gamezone" );

		if ( iEffect != -1 )
			a_pReplay->m_pEffect = AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, TVector4::VEC_ZERO, TVector4::VEC_POSZ );
	}

	if ( a_pReplay->m_pEffect )
		UpdateStarEffect( a_pReplay );
}

// Puts the effect on the ground below the star
// $Barnyard: FUNCTION 00547f10
void AReplayManager::UpdateStarEffect( AReplayableGame* a_pReplay )
{
	AParticleEffectHandle* pEffect = a_pReplay->m_pEffect;

	TVector4 vecPosition;
	a_pReplay->GetStarPosition( vecPosition );

	if ( !pEffect )
		return;

	TINT     iAlpha = 0;
	TVector3 vecRotation;
	TVector4 vecNormal;

	if ( ATerrainInterface::GetSingleton()->GetGroundShadowInfo( vecPosition, -0.2f, vecPosition, iAlpha, vecRotation, vecNormal, 0.5f, 10.0f, TCollisionCommon::TOSHICGROUPFLAG_NONE ) )
	{
		TMatrix44& rTransform = pEffect->GetInstance()->GetTransform();
		rTransform.Identity();
		rTransform.GetTranslation() = vecPosition;
		rTransform.LookAtDirection( vecNormal, TVector4::VEC_POSZ );
		rTransform.RotateX( -TMath::HALF_PI );
		rTransform.Scale( 1.0f, 1.0f, 1.0f );
	}
}

// $Barnyard: FUNCTION 00546f90
void AReplayManager::RotateStars( TFLOAT a_fDeltaTime )
{
	TFLOAT fAngle = TMath::PI * a_fDeltaTime;

	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( pReplay->m_pStar && pReplay->m_eStarState == AReplayableGame::STARSTATE_AVAILABLE )
			RotateStarModel( fAngle, pReplay->m_pStar );

		if ( pReplay->m_pStarCompleted && pReplay->m_eStarState == AReplayableGame::STARSTATE_COMPLETED )
			RotateStarModel( fAngle, pReplay->m_pStarCompleted );

		if ( pReplay->m_pStarUnavailable && pReplay->m_eStarState == AReplayableGame::STARSTATE_UNAVAILABLE )
			RotateStarModel( fAngle, pReplay->m_pStarUnavailable );
	}
}

// $Barnyard: FUNCTION 00547c80
void AReplayManager::Render( TBOOL a_bOnTop )
{
	if ( ASSeqState::IsPlaying() || AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASleepCycleState ) ) )
		return;

	if ( a_bOnTop && !ms_bRenderOnTop )
		return;

	if ( !m_bStarsCreated || !m_pReplays )
		return;

	TRenderInterface* pRender  = TRenderInterface::GetSingleton();
	TRenderContext*   pContext = pRender->GetCurrentContext();

	// Light the stars evenly
	TMatrix44 mOldLightColour = pRender->GetLightColour();
	TVector4  vecOldAmbient   = pContext->GetAmbientColour();
	TMatrix44 mLightColour    = mOldLightColour;
	TMatrix44 mOldLightDirection;

	if ( a_bOnTop )
	{
		pContext->SetAmbientColour( 1.0f, 1.0f, 1.0f );

		mLightColour.m_f11 = 0.0f;
		mLightColour.m_f12 = 0.0f;
		mLightColour.m_f13 = 0.0f;
		mLightColour.m_f14 = 1.0f;
	}
	else
	{
		pContext->SetAmbientColour( 0.5f, 0.5f, 0.5f );

		mLightColour.m_f11 = 0.5f;
		mLightColour.m_f12 = 0.5f;
		mLightColour.m_f13 = 0.5f;
		mLightColour.m_f14 = 1.0f;

		// The light comes from the camera
		mOldLightDirection        = pRender->GetLightDirection();
		TMatrix44 mLightDirection = mOldLightDirection;

		const TMatrix44& rcCamera = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
		TVector4&        rLight   = mLightDirection.AsBasisVector4( BASISVECTOR_RIGHT );
		rLight.x                  = -rcCamera.m_f31 + rcCamera.m_f11 + rcCamera.m_f21;
		rLight.y                  = -rcCamera.m_f32 + rcCamera.m_f12 + rcCamera.m_f22;
		rLight.z                  = -rcCamera.m_f33 + rcCamera.m_f13 + rcCamera.m_f23;
		rLight.Normalise();
		pRender->SetLightDirectionMatrix( mLightDirection );
	}

	pRender->SetLightColourMatrix( mLightColour );

	for ( AReplayableGame* pReplay = m_pReplays; pReplay != TNULL; pReplay = pReplay->m_pNext )
	{
		if ( pReplay->m_eStarState == AReplayableGame::STARSTATE_AVAILABLE )
		{
			if ( pReplay->m_pStar )
				RenderStarModel( pReplay->m_pStar );
		}
		else if ( pReplay->m_eStarState == AReplayableGame::STARSTATE_COMPLETED )
		{
			if ( pReplay->m_pStarCompleted )
				RenderStarModel( pReplay->m_pStarCompleted );
		}
		else if ( !a_bOnTop && pReplay->m_eStarState == AReplayableGame::STARSTATE_UNAVAILABLE )
		{
			if ( pReplay->m_pStarUnavailable )
				RenderStarModel( pReplay->m_pStarUnavailable );
		}
	}

	// NOTE: retail restores the w of the ambient colour from a wrong variable
	pContext->SetAmbientColour( vecOldAmbient );
	pRender->SetLightColourMatrix( mOldLightColour );

	if ( !a_bOnTop )
		pRender->SetLightDirectionMatrix( mOldLightDirection );
}
