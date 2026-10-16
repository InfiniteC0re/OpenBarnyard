#include "pch.h"
#include <Render/TCameraObject.h>
#include "AHoneyCollectingGame.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimationBreakpointAttachment.h"
#include "Animation/AAlienAnimController.h"
#include "Animation/AModelRepos.h"
#include "Animation/ANamedAnimation.h"
#include "Animation/ANamedAnimationSet.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "FX/AMilkManager.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/ASSeqState.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/APlayerProgress.h"
#include "Helpers/AReplayManager.h"
#include "HUD/AHudManager.h"
#include "HUD/AInteractionHudElement.h"
#include "Input/AInputMapManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Sound/AMusicManager.h"
#include "Sound/AMultiplexStream.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/ABeeSwarm.h"
#include "Units/ASSeqController.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/AUnitTransform.h"
#include "Units/ASimplePlayerControlComponent.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "Units/Projectiles/ALaunchAmmo.h"
#include "Units/Projectiles/ASimpleProjectileManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "UnitMovement/Steering/ASteerKinematics.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Physics/ACollisionObjectSimple.h"
#include "Physics/ACollisionObjectMesh.h"
#include "Physics/ACollisionObjectUnit.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>
#include <Math/TMathInline.h>
#include <Math/TQuaternion.h>
#include <Render/TSkeleton.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

// Opcode defines PI
#undef PI

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0048bf30, CreateObject 00492710, CreateObjectInPlace 00492770 (the constructor is inlined in both)
TDEFINE_CLASS( AHoneyCollectingGame );

TPSTRING8_DECLARE( AHoneyCollectingGame );
TPSTRING8_DECLARE( player_hc );
TPSTRING8_DECLARE( simplayer );
TPSTRING8_DECLARE( sunglasses1 );
TPSTRING8_DECLARE( beeswarm );
TPSTRING8_DECLARE( woods );
TPSTRING8_DECLARE( beehive );
TPSTRING8_DECLARE( beehive_empty );
TPSTRING8_DECLARE( beehive_bee );
TPSTRING8_DECLARE( honey_collecting_replay );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( fly_away );
TPSTRING8_DECLARE( honey );
TPSTRING8_DECLARE( honeyglob );
TPSTRING8_DECLARE( honey_start );
TPSTRING8_DECLARE( honey_lp );
TPSTRING8_DECLARE( honey_end );
TPSTRING8_DECLARE( hide_start );
TPSTRING8_DECLARE( hide_loop );
TPSTRING8_DECLARE( hide_end );
TPSTRING8_DECLARE( player_loc );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( stinging );
TPSTRING8_DECLARE( stung );
TPSTRING8_DECLARE( swat_lp );
TPSTRING8_DECLARE( swat_run_lp );
TPSTRING8_DECLARE( default );
TPSTRING8_DECLARE( defeat );
TPSTRING8_DECLARE( entrance );
TPSTRING8_DECLARE( entrance2 );

// Locale strings
static constexpr TINT LOCALE_ROYAL_JELLY = 553;

// Size of the icons of the minimap
#ifdef BARNYARD_LATEST
static constexpr TFLOAT MAP_ICON_SIZE = 0.14f; // latest version (Wii 806af298)
#else
static constexpr TFLOAT MAP_ICON_SIZE = 0.1f;
#endif

#ifdef BARNYARD_LATEST
// Lives of the difficulty (latest version, 806af294), set by the constructor. Normal is the PC retail MAX_LIVES
static TINT s_iNumLives = AHoneyCollectingGame::MAX_LIVES;
#endif
static constexpr TINT LOCALE_LEAVE_WOODS = 554;
static constexpr TINT LOCALE_HINT_STOP   = 542;
static constexpr TINT LOCALE_HINT_UNHIDE = 543;
static constexpr TINT LOCALE_HINT_SWAT   = 544;
static constexpr TINT LOCALE_HINT_RUN    = 546;
static constexpr TINT LOCALE_HINT_HIDE   = 547;
static constexpr TINT LOCALE_HINT_HONEY  = 548;
static constexpr TINT LOCALE_LEAVE       = 549;
static constexpr TINT LOCALE_HINT_ANGRY  = 556;

// Sounds
static constexpr ASoundId SOUND_COMB_DONE  = 182;
static constexpr ASoundId SOUND_COLLECTING = 284;

// High score table of the game
static constexpr TINT HIGHSCORE_TABLE = 63;

// HUD flags of the game (and while collecting)
static constexpr TUINT HUDFLAGS_GAME       = AHudElement::MASK_UNK2 | AHudElement::MASK_INTERACTION | AHudElement::MASK_HONEY_COUNT;
static constexpr TUINT HUDFLAGS_COLLECTING = AHudElement::MASK_UNK2 | AHudElement::MASK_INTERACTION | AHudElement::MASK_UNK17 | AHudElement::MASK_HONEY_COUNT;

// Radius of the mouse cursor around the comb
static constexpr TFLOAT COMB_CURSOR_RADIUS = 70.0f;

// Text of the hint (00782960)
static TWCHAR s_wszHint[ 256 ];

// Smoothed stick (00782b94, 00782b90, initialised once, 00782b98)
static TBOOL  s_bStickXInitialised = TFALSE;
static TBOOL  s_bStickYInitialised = TFALSE;
static TFLOAT s_fStickX;
static TFLOAT s_fStickY;

// Where the camera goes while the player collects the honey of each hive (00782b9c, initialised once, 00782bcc)
static const TVector4 s_avHiveCameraPositions[ AHoneyCollectingGame::MAX_HIVES ] = {
	TVector4( -11.8f, -2.48f, 7.2f, 1.0f ),
	TVector4( 13.22f, -3.33f, 19.43f, 1.0f ),
	TVector4( 2.37f, -3.34f, -9.05f, 1.0f ),
};

#ifdef BARNYARD_COMMUNITY_PATCH
// Development: "-devautoplay" leaves the woods (see OnUpdate)
static TBOOL s_bDevLeave = TFALSE;
#endif

// Sets the text and the hint
static void ShowHint( AGUI2TextBox& a_rHint, AHoneyCollectingGame::HINT& a_reHint, TINT a_iLocale, AHoneyCollectingGame::HINT a_eHint )
{
	ALocale::FormatInputTags( s_wszHint, TARRAYSIZE( s_wszHint ), ALocaleManager::Interface()->GetString( a_iLocale ) );
	a_rHint.SetText( s_wszHint );
	a_reHint = a_eHint;
}

// The squared distance on the XZ plane (006c74d0)
static TFLOAT MagnitudeSqXZ( const TVector4& a_rcVec )
{
	return a_rcVec.x * a_rcVec.x + a_rcVec.z * a_rcVec.z;
}

//-----------------------------------------------------------------------------
// AHoneyCollectingGame::BeePatrol
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0048bf40
AHoneyCollectingGame::BeePatrol::BeePatrol( const TCHAR* a_szFileName, const TCHAR* a_szSymbol )
{
	oTRB.Load( a_szFileName, 0 );
	oSpline.Load( oTRB, a_szSymbol );
	oTRB.DeleteSymbolTable();
}

// $Barnyard: FUNCTION 0048bfc0
AHoneyCollectingGame::BeePatrol::~BeePatrol()
{
	oTRB.Close();
}

//-----------------------------------------------------------------------------
// AHoneyCollectingGame::MilkHitListener(s)
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0048cd60
AHoneyCollectingGame::MilkHitListener::MilkHitListener( TGenericEmitter& a_rEmitter, void* a_pCaller, TGenericListener::t_Callback a_fnCallback, TINT a_iPriority )
{
	pEmitter = &a_rEmitter;
	oListener.Connect( a_rEmitter, a_pCaller, a_fnCallback, a_iPriority );
}

// $Barnyard: FUNCTION 0048c220
AHoneyCollectingGame::MilkHitListener::~MilkHitListener()
{
	oListener.Disconnect();
}

AHoneyCollectingGame::MilkHitListeners::MilkHitListeners( AHoneyCollectingGame* a_pOwner, TGenericListener::t_Callback a_fnCallback )
    : pOwner( a_pOwner )
    , fnCallback( a_fnCallback )
{
}

// $Barnyard: FUNCTION 0048e660
AHoneyCollectingGame::MilkHitListeners::~MilkHitListeners()
{
	RemoveAll();
}

// $Barnyard: FUNCTION 0048ce80
void AHoneyCollectingGame::MilkHitListeners::Add( TGenericEmitter& a_rEmitter, TINT a_iPriority )
{
	oList.PushBack( new MilkHitListener( a_rEmitter, pOwner, fnCallback, a_iPriority ) );
}

// $Barnyard: FUNCTION 0048cf00
void AHoneyCollectingGame::MilkHitListeners::RemoveAll()
{
	while ( !oList.IsEmpty() )
	{
		MilkHitListener* pListener = oList.Head();
		pListener->Remove();
		delete pListener;
	}
}

//-----------------------------------------------------------------------------
// AHoneyCollectingGame
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0048e6b0
AHoneyCollectingGame::AHoneyCollectingGame()
    : m_fTime( 0.0f )
    , m_bUnkbb0( TFALSE )
    , m_iUnkbb4( 0 )
    , m_bCollecting( TFALSE )
    , m_iNumCombs( 0 )
    , m_iUnk16d8( 0 )
    , m_iUnk16dc( 0 )
    , m_pPlayer( TNULL )
    , m_pSunglasses( TNULL )
    , m_iHive( -1 )
    , m_iNumStinging( 0 )
    , m_iNumAngry( 0 )
    , m_pArrow( TNULL )
    , m_iUnk1848( 0 )
    , m_bWalkingToHive( TFALSE )
    , m_bLeavingHive( TFALSE )
    , m_bHiding( TFALSE )
    , m_bLeavingHide( TFALSE )
    , m_eAction( ACTION_NONE )
    , m_eHint( HINT_NONE )
    , m_iNumAttacking( 0 )
    , m_pHintFont( TNULL )
    , m_iUnk18fc( 0 )
    , m_iNumFlowerBeds( 0 )
    , m_iNumHidingSpots( 0 )
    , m_iUnk1ad0( 0 )
    , m_fStingTime( 0.0f )
    , m_fStingInterval( 0.0f )
    , m_iCollectCue( -1 )
    , m_bDefeated( TFALSE )
    , m_iSectionsAtStart( 0 )
    , m_iCombsAtStart( 0 )
    , m_pHoneyGlob( TNULL )
    , m_bWon( TFALSE )
    , m_fOldNear( 1.0f )
    , m_iHidingSpot( -1 )
    , m_bAtExit( TFALSE )
    , m_iUnk1ca0( 0 )
    , m_aPatrols{
	    BeePatrol( "Data\\MiniGameData\\HoneyCollecting\\beepatrol1.trb", "Line01" ),
	    BeePatrol( "Data\\MiniGameData\\HoneyCollecting\\beepatrol2.trb", "Line02" ),
	    BeePatrol( "Data\\MiniGameData\\HoneyCollecting\\beepatrol3.trb", "Line03" ),
	    BeePatrol( "Data\\MiniGameData\\HoneyCollecting\\beepatrol4.trb", "Line04" )
    }
    , m_fUnkTextTime( 0.0f )
    , m_oMilkHitListeners( this, TREINTERPRETCAST( TGenericListener::t_Callback, OnMilkHit ) )
    , m_iUnk8204( -1 )
    , m_fCombX( TVector2::VEC_ZERO.x )
    , m_fCombY( TVector2::VEC_ZERO.y )
    , m_iLives( MAX_LIVES )
    , m_iNumEmptiedHives( 0 )
    , m_bRoyalJelly( TFALSE )
    , m_vHideExitPosition( TVector4::VEC_ZERO )
    , m_vHideExitDirection( TVector4::VEC_NEGZ )
    , m_vStick( TVector2::VEC_ZERO )
    , m_vKeys( TVector2::VEC_ZERO )
    , m_vLastKeys( TVector2::VEC_ZERO )
    , m_vSmoothKeys( TVector2::VEC_ZERO )
    , m_bMoveCamera( TFALSE )
{
	m_aUnkbb8[ 0 ] = 0;

	m_oMapViewport.iX      = 0;
	m_oMapViewport.iY      = 0;
	m_oMapViewport.iWidth  = 0;
	m_oMapViewport.iHeight = 0;

#ifdef BARNYARD_LATEST
	// Latest version (Wii 801b1ee8): 7 lives on Easy
	s_iNumLives = APlayerProgress::GetSingleton()->IsEasyDifficulty() ? 7 : MAX_LIVES;
	m_iLives    = s_iNumLives;
#endif

	// The texts in the middle of the screen
	T2GUIFont*    pFont         = T2GUIFontManager::FindFont( "Rekord26" );
	const TWCHAR* wszRoyalJelly = ALocaleManager::Interface()->GetString( LOCALE_ROYAL_JELLY );
	m_oRoyalJellyText.Create( pFont, pFont->GetTextWidth( wszRoyalJelly, 1.0f ) );
	m_oRoyalJellyText.SetTranslation( 0.0f, 20.0f );
	m_oRoyalJellyText.SetText( wszRoyalJelly );

	const TWCHAR* wszLeave = ALocaleManager::Interface()->GetString( LOCALE_LEAVE_WOODS );
	m_oLeaveText.Create( pFont, pFont->GetTextWidth( wszLeave, 1.0f ) );
	m_oLeaveText.SetText( wszLeave );
	m_oLeaveText.SetAttachment( T2GUIATTACHMENT( 1 ), T2GUIATTACHMENT( 1 ) );
	m_oLeaveText.SetTransform( 0.0f, 80.0f, 0.0f );

	m_GUIElement.AddChildTail( m_oRoyalJellyText );
	m_oRoyalJellyText.Hide();
	m_GUIElement.AddChildTail( m_oLeaveText );
	m_oLeaveText.Hide();

	m_iDustPuffEffect = AParticleEffectManager::FindEffect( "dustpuff" );
	m_bRenderReticles = TFALSE;
}

// $Barnyard: FUNCTION 0048eb90
// $Barnyard: FUNCTION 004927b0
AHoneyCollectingGame::~AHoneyCollectingGame()
{
	m_oRoyalJellyText.RemoveSelf();
	m_oLeaveText.RemoveSelf();
}

// $Barnyard: FUNCTION 0048c030
TBOOL AHoneyCollectingGame::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	return m_oStartHudElement.IsForceHidden() && AGameState::ProcessInput( a_pInputEvent );
}

// $Barnyard: FUNCTION 0048cf90
TBOOL AHoneyCollectingGame::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( !m_oStartHudElement.IsForceHidden() )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

	if ( a_eInputCommand == AInputCommand_Use || a_eInputCommand == AInputCommand_Inventory )
		return TTRUE;

	if ( m_bDefeated == TTRUE )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	if ( a_eInputCommand == AInputCommand_Jump )
	{
		// Swat the bees away
		if ( a_pInputEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN )
			return TFALSE;

		if ( m_iNumStinging <= 0 )
			return TFALSE;

		if ( m_eAction == ACTION_COLLECT )
			return TTRUE;

		TFLOAT fPower = m_oSwatBar.m_fTargetValue + 0.15f;
		m_oSwatBar.SetValue( fPower, TTRUE );

		TPString8 strSet = m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet()->GetName();

		if ( strSet == TPS8( stinging ) )
		{
			if ( m_pPlayer->GetSteer()->GetSpeed() <= 0.05f && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( swat_lp ), 0.0f ) )
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( swat_lp ) );
			else if ( !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( swat_run_lp ), 0.0f ) )
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( swat_run_lp ) );
		}

		return TTRUE;
	}

	if ( a_eInputCommand == AInputCommand_Shoot )
	{
		if ( m_bCollecting == TTRUE )
		{
			// The mouse button sweeps the comb
			if ( a_pInputEvent->GetSource()->GetClass()->IsA( &TGetClass( TInputDeviceMouse ) ) && a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 )
			{
				if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
				{
					AGUI2::GetMouseCursor().SetRadialField( m_fCombX, m_fCombY, COMB_CURSOR_RADIUS );
					m_oCollectHud.SetMouseInput( TTRUE );

					if ( ACameraManager::GetSingleton()->GetCurrentCameraHelper()->GetClass()->IsA( &TGetClass( AGTAStyleCameraHelper ) ) == TTRUE )
						TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() )->SetMouseInput( TFALSE );
				}
				else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
				{
					AGUI2::GetMouseCursor().ClearRadialField();
					m_oCollectHud.SetMouseInput( TFALSE );

					if ( ACameraManager::GetSingleton()->GetCurrentCameraHelper()->GetClass()->IsA( &TGetClass( AGTAStyleCameraHelper ) ) == TTRUE )
						TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() )->SetMouseInput( TTRUE );
				}
			}

			return TTRUE;
		}

		// No milk while the bees sting
		if ( m_iNumStinging > 0 )
			return TTRUE;
	}
	else
	{
		if ( m_bHiding == TTRUE && a_eInputCommand != AInputCommand_Interact && a_eInputCommand != AInputCommand_RotateCamera &&
		     a_eInputCommand != AInputCommand_Quit && a_eInputCommand != AInputCommand_Cancel )
			return TTRUE;

		if ( a_eInputCommand == AInputCommand_SuperSpeed )
		{
			if ( m_iNumStinging > 0 )
				return TTRUE;

			// No sprinting with the honey animations
			if ( m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet()->GetName() != TPS8( default ) )
				return TTRUE;
		}
	}

	if ( !m_bCollecting && a_eInputCommand != AInputCommand_RotateCamera &&
	     ( pPlayerObject->IsAnimPlaying( TPS8( honey_lp ), 0.0f ) == TTRUE || pPlayerObject->IsAnimPlaying( TPS8( honey_end ), 0.0f ) == TTRUE ||
	       pPlayerObject->IsAnimPlaying( TPS8( stung ), 0.0f ) == TTRUE ) )
		return TTRUE;

	if ( a_eInputCommand == AInputCommand_Interact )
	{
		if ( m_bAtExit == TTRUE )
		{
			// Leave the woods, the honey left in the hives is kept for the farm (see AGameLoader)
			AHudManager::GetSingleton()->FindElement<AInteractionHudElement>()->SetText( TNULL, 2, 0, -1, 0, -1, -1.0f );

			APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
			pProfile->SetHoneyReplay( 0, m_aHives[ 0 ].iHoney );
			pProfile->SetHoneyReplay( 1, m_aHives[ 1 ].iHoney );
			pProfile->SetHoneyReplay( 2, m_aHives[ 2 ].iHoney );
			AReplayManager::ms_bRestoreHoneyCollecting = TTRUE;

			AGameLoader::LeaveMiniGame( TTRUE );
			return TTRUE;
		}

		if ( m_bWalkingToHive == TTRUE || m_bLeavingHive == TTRUE )
			return TTRUE;

		if ( m_bCollecting == TTRUE )
		{
			StopCollecting( TFALSE );
			return TTRUE;
		}

		if ( m_bHiding == TTRUE )
		{
			// Get out of the stump
			if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( hide_loop ), 0.0f ) != TTRUE )
				return TFALSE;

			m_bHiding      = TFALSE;
			m_bLeavingHide = TTRUE;
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( hide_end ) );

			AModelInstance* pSpotModel = m_aHidingSpots[ m_iHidingSpot ].pModel;
			TINT            iBone      = pSpotModel->GetBoneIndex( TPS8( player_loc ) );

			TMatrix44 matBone;
			pSpotModel->GetBoneTransform( iBone, matBone );

			m_vHideExitPosition   = matBone.AsBasisVector4( BASISVECTOR_TRANSLATION );
			m_vHideExitPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( m_vHideExitPosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );

			matBone.GetBasisVector4( BASISVECTOR_UP, m_vHideExitDirection );
			m_vHideExitDirection.x = -m_vHideExitDirection.x;
			m_vHideExitDirection.y = -m_vHideExitDirection.y;
			m_vHideExitDirection.z = -m_vHideExitDirection.z;
			m_vHideExitDirection.w = 1.0f;

			// NOTE: retail doesn't check the class of the camera helper
			AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() );
			pHelper->SetTargetPositionOverride2( *m_pPlayer->GetSteer()->GetPosition() );

			return TTRUE;
		}

		m_pPlayer->GetSteer()->GetPosition();

		if ( m_eAction == ACTION_COLLECT )
		{
			// Walk to the hive
			APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

			if ( pSteer->GetState() != APlayerSteer::STATE_WALKING && pSteer->GetState() != APlayerSteer::STATE_STILL )
				return TTRUE;

			m_pPlayer->SelectLauncher( TFALSE );
			m_bWalkingToHive = TTRUE;
			pSteer->SetState( APlayerSteer::STATE_STILL );

			TVector4 vSide = m_avHiveDirections[ m_iHive ];
			vSide.x        = -vSide.x;
			vSide.y        = -vSide.y;
			vSide.z        = -vSide.z;
			vSide.AsVector3().RotateY( TMath::HALF_PI );

			m_vHiveDirection   = vSide;
			m_vHiveDirection.w = 1.0f;

			vSide.x = -vSide.x;
			vSide.y = -vSide.y;
			vSide.z = -vSide.z;

			TVector4 vOffset;
			vOffset.Multiply( vSide, 1.0f );
			vOffset.Multiply( vOffset, 1.3f );
			m_vHivePosition.Add( m_avHivePositions[ m_iHive ], vOffset );
			m_vHivePosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( m_vHivePosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );

			m_bMoveCamera = TTRUE;

			AAnimatableObject* pHeld = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( m_pPlayer->GetAnimatableObject(), TPS8( cb_rlocator ), 0 );

			if ( pHeld )
				pHeld->SetSkeletonUpdating( TFALSE, TTRUE );
		}
		else if ( m_eAction == ACTION_HIDE )
		{
			// Hide in the stump
			APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

			if ( pSteer->GetState() != APlayerSteer::STATE_WALKING && pSteer->GetState() != APlayerSteer::STATE_STILL )
				return TTRUE;

			m_bHiding = TTRUE;
			m_pPlayer->SelectLauncher( TFALSE );
			TSTATICCAST( AAlienAnimController, m_pPlayer->GetAnimatableObject()->FindAnimController( &TGetClass( AAlienAnimController ) ) )->SetAnimEnabled( TFALSE );

			HidingSpot& rSpot = m_aHidingSpots[ m_iHidingSpot ];
			rSpot.pCollision->SetType( rSpot.pCollision->GetType() | ACollisionObject::TYPE_IGNORED );

			TVector4 vTarget   = rSpot.vPosition;
			TVector4 vPosition = *m_pPlayer->GetSteer()->GetPosition();
			vPosition.y        = vTarget.y;

			TVector4 vDirection;
			vDirection.Substract( vTarget, vPosition );
			vDirection.Normalise();
			m_pPlayer->SetSteerDirection( vDirection );

			pPlayerObject = m_pPlayer->GetAnimatableObject();
			pPlayerObject->FinishAnimation( TPS8( idle_lp ) );
			m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( swat_lp ) );
			m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( swat_run_lp ) );
			pSteer->SetRestoreAnimationSet( TTRUE );

			ANamedAnimationSetRef pSet = m_pPlayer->GetAnimatableObject()->GetObjectType()->FindAnimationSet( TPS8( default ) );
			m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pSet );
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( hide_start ) );

			m_pPlayer->SetMovement( TFALSE );
			m_pPlayer->SetControlFlags( TFALSE );

			AAnimatableObject* pHeld = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( m_pPlayer->GetAnimatableObject(), TPS8( cb_rlocator ), 0 );

			if ( pHeld )
				pHeld->SetSkeletonUpdating( TFALSE, TTRUE );
		}

		return TTRUE;
	}

	// The keys turn around the comb
	if ( ( a_eInputCommand == AInputCommand_Left || a_eInputCommand == AInputCommand_Right ) && m_bCollecting == TTRUE )
	{
		m_vKeys.x = ( a_eInputCommand == AInputCommand_Left ) ? -1.0f : 1.0f;

		// NOTE: retail calls a copy of InputEvent::StartRepeat (006c3cf0)
		a_pInputEvent->StartRepeat( 0.0f, 0.0f );
		return TTRUE;
	}

	if ( ( a_eInputCommand == AInputCommand_Forward || a_eInputCommand == AInputCommand_Backward ) && m_bCollecting == TTRUE )
	{
		m_vKeys.y = ( a_eInputCommand == AInputCommand_Backward ) ? 1.0f : -1.0f;
		a_pInputEvent->StartRepeat( 0.0f, 0.0f );
		return TTRUE;
	}

	if ( a_eInputCommand == AInputCommand_Move )
	{
		if ( m_bCollecting != TTRUE )
			return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

		// The stick turns around the comb
		TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
		TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

		if ( !s_bStickXInitialised )
		{
			s_bStickXInitialised = TTRUE;
			s_fStickX            = fX;
		}

		if ( !s_bStickYInitialised )
		{
			s_bStickYInitialised = TTRUE;
			s_fStickY            = fY;
		}

		fX        = TMath::LERP( s_fStickX, fX, 0.2f );
		s_fStickY = TMath::LERP( s_fStickY, fY, 0.2f );

		m_vStick.y = s_fStickY;
		s_fStickX  = fX;
		m_vStick.x = fX;
		m_vStick.ClampMagnitude( 1.0f );

		if ( m_vStick.x * m_vStick.x + m_vStick.y * m_vStick.y < 0.64000005f )
			m_vStick = TVector2::VEC_ZERO;

		return TTRUE;
	}

	if ( m_bCollecting != TTRUE )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	switch ( a_eInputCommand )
	{
		case AInputCommand_Left:
		case AInputCommand_Right:
		case AInputCommand_Forward:
		case AInputCommand_Backward:
		case AInputCommand_Shoot:
		case AInputCommand_Switch:
			return TTRUE;
		case AInputCommand_RotateCamera:
		{
			// The camera can't be turned to the back of the hive
			ASteer*  pSteer  = m_pPlayer->GetSteer();
			ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
			TVector4 vToCamera;
			vToCamera.Substract( pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ), *pSteer->GetPosition() );
			vToCamera.Normalise();

			TVector4 vSide = m_avHiveDirections[ m_iHive ];
			vSide.AsVector3().RotateY( TMath::HALF_PI );

			TFLOAT fDot   = vSide.z * vToCamera.z + vSide.x * vToCamera.x;
			TFLOAT fCross = vSide.z * vToCamera.x - vToCamera.z * vSide.x;
			TFLOAT fInput = a_pInputEvent->GetMagnitudeFloat( 0 );

			if ( fDot >= 0.5f || ( fCross >= 0.0f || fInput >= 0.0f ) && ( fCross <= 0.0f || fInput <= 0.0f ) )
				return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

			return TTRUE;
		}
		default:
			return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 0048c140
void AHoneyCollectingGame::OnRender( TFLOAT a_fDeltaTime )
{
	if ( m_bRenderReticles )
		m_oReticles.UpdateSim( a_fDeltaTime );

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Render();

	m_oMilkBottles.Render();
}

// $Barnyard: FUNCTION 0048c1c0
void AHoneyCollectingGame::OnPause()
{
	m_pPlayer->SetVisible( TTRUE );

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		if ( m_apSwarms[ i ] )
			m_apSwarms[ i ]->SetVisible( TTRUE );
	}
}

// $Barnyard: FUNCTION 0048e490
TBOOL AHoneyCollectingGame::OnUpdate( TFLOAT a_fDeltaTime )
{
	// The entrance of the woods, where the player leaves with the honey
	AInteriorArea*          pArea           = TSTATICCAST( ACommonSteer, m_pPlayer->GetSteer() )->GetCurrentArea();
	AInteractionHudElement* pInteractionHud = AHudManager::GetSingleton()->FindElement<AInteractionHudElement>();

	if ( pArea && !pArea->m_strName->IsEmpty() )
	{
		if ( pArea != m_pInteractionArea )
		{
			TPString8 strName = *pArea->m_strName;

			if ( strName == TPS8( entrance ) || strName == TPS8( entrance2 ) )
			{
				pInteractionHud->SetText( m_wszLeaveText, 2, 0, -1, 0, -1, AGUI2::GetRootElement()->GetWidth() * 0.5f );
				m_bAtExit = TTRUE;
			}
			else
			{
				pInteractionHud->SetText( TNULL, 2, 0, -1, 0, -1, -1.0f );
				m_bAtExit = TFALSE;
			}
		}
	}
	else
	{
		pInteractionHud->SetText( TNULL, 2, 0, -1, 0, -1, -1.0f );
		m_bAtExit = TFALSE;
	}

	m_pInteractionArea = pArea;

#ifdef BARNYARD_COMMUNITY_PATCH
	TBOOL bResult = AMiniGameState::OnUpdate( a_fDeltaTime );

	if ( s_bDevLeave )
	{
		// Development: "-devautoplay" leaves the woods here, the state is destroyed right away
		s_bDevLeave = TFALSE;
		TINFO( "[honeyauto] leave the woods (combs %d, royal jelly %d)\n", m_iNumCombs, m_bRoyalJelly );

		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		TBOOL                       bStopEvents = TFALSE;
		ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
	}

	return bResult;
#else
	return AMiniGameState::OnUpdate( a_fDeltaTime );
#endif
}

// $Barnyard: FUNCTION 0048eda0
// $BarnyardWiiJP: FUNCTION 801b378c
void AHoneyCollectingGame::OnInsertion()
{
	m_strWorldHoney     = TPS8D( "worldhoney" );
	m_bShowPlayerFloats = TFALSE;
	m_HUDParams.SetFlags( HUDFLAGS_GAME );
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( TPS8( AHoneyCollectingGame ) );

	m_fOldNear = ARenderer::GetSingleton()->GetNear();
	ARenderer::GetSingleton()->SetNear( 0.1f );

	AGameLoader::Terrain_Set( AGameLoader::Terrain_EnvWoods, TFALSE, TTRUE, 0, 0, 0, 0 );

	// The player
	APlayerManager::GetSingleton()->GetCurrentPlayerSet()->ResetPlayerUnits();
	APlayerManager::GetSingleton()->GetCurrentPlayerSet()->Reset();
	APlayerManager::GetSingleton()->GetCurrentPlayerSet()->AddHumanPlayer( AInputHandler::GetSingleton()->GetMainController(), -1 );

	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
	TINT                   iGender         = pProfileManager->GetMainGender();

	if ( iGender == APlayerProfile::GENDER_NONE )
		iGender = APlayerProfile::GENDER_MALE;

	AUnitTransform oPlayerTransform( TVector4::VEC_ZERO, TVector4::VEC_NEGZ, 0, iGender, -1, TTRUE );
	m_pPlayer = TSTATICCAST( AUnitPlayer, AUnitManager::GetSingleton()->SpawnUnit( TPS8( player_hc ), &oPlayerTransform ) );
	m_pPlayer->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );
	AUnitNameManager::GetSingleton()->SetUnitName( TPS8( simplayer ), m_pPlayer->GetId() );
	m_pPlayer->SetMovement( TFALSE );
	m_pPlayer->SetControlFlags( TFALSE );

	// The bees disperse when hit by the milk
	AProjectileLauncher* pPrimaryLauncher = m_pPlayer->GetPrimaryLauncher();

	// NOTE: retail would connect to the address 0x38 when there's no launcher
	if ( pPrimaryLauncher )
		m_oMilkHitListeners.Add( pPrimaryLauncher->GetHitEmitter(), 0 );

	AProjectileLauncher* pSecondaryLauncher = m_pPlayer->GetSecondaryLauncher();

	if ( pSecondaryLauncher )
		m_oMilkHitListeners.Add( pSecondaryLauncher->GetHitEmitter(), 0 );

	// The player wears sunglasses
	AAnimatableObjectManager* pObjectManager  = AAnimatableObjectManager::GetSingleton();
	AAnimatableObjectType*    pSunglassesType = pObjectManager->FindType( TPS8( sunglasses1 ) );

	if ( pSunglassesType )
	{
		m_pSunglasses = pObjectManager->CreateAnimatableObject( pSunglassesType, m_pPlayer, 0 );
		TINT iBone    = -1;

		if ( pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE )
			iBone = m_pSunglasses->GetModelInstance()->GetSkeleton()->GetBoneID( "americanf_loc", 0 );

		TINT iHeadBone = m_pPlayer->GetModelInstance()->GetSkeleton()->GetBoneID( "c_head", 0 );
		pObjectManager->AttachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject(), TINT8( iBone ), TINT8( iHeadBone ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
		m_pSunglasses->SetSkeletonUpdating( TTRUE, TTRUE );
	}

	// The camera
	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->SetSplitscreenCameraHelper( 1 );
	ACameraHelper* pCameraHelper = pCameraManager->GetSplitscreenCameraHelper( 1 );
	pCameraHelper->SetTarget( m_pPlayer->GetSelfProxy() );
	pCameraHelper->ResetPosition( TTRUE );

	m_pNetwork = AInteriorNetworkManager::GetSingleton()->SetDefaultNetwork( TPS8( woods ) );

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	if ( m_oMilkBottles.LoadFromLocators( pTerrain->GetLocatorList() ) == 0 )
		m_oMilkBottles.LoadFromNetwork( m_pNetwork );

	TPString8 strFlowerBed = TPS8D( "data\\models\\hc_beeflowerbed" );
	TPString8 strHiding    = TPS8D( "data\\models\\hc_p_hiding" );

	// The minimap
	m_iNumHives = 0;

	TFLOAT fScreenWidth;
	TFLOAT fScreenHeight;
	T2GUI::GetContext()->GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );
	m_oMapViewport.iX      = 0;
	m_oMapViewport.iY      = 0;
	m_oMapViewport.iWidth  = TINT( fScreenWidth );
	m_oMapViewport.iHeight = TINT( fScreenHeight );

	TVector4    vMapBottomRight;
	TQuaternion quatMapBottomRight;
	TVector4    vMapTopLeft;
	TQuaternion quatMapTopLeft;
	pTerrain->GetLocatorPosAndRot( "map_bottom_right", vMapBottomRight, quatMapBottomRight );
	pTerrain->GetLocatorPosAndRot( "map_top_left", vMapTopLeft, quatMapTopLeft );

	m_oMap.Create( "Map_Woods", 0.3f, 0.3f, m_oMapViewport, vMapBottomRight, TVector2( 122.0f, 380.0f ), vMapTopLeft, TVector2( 376.0f, 70.0f ), TTRUE, TNULL, TTRUE, 50 );
	m_oMap.SetAttachment( T2GUIATTACHMENT( 6 ), T2GUIATTACHMENT( 6 ) );
	m_oMap.Show();
	m_GUIElement.AddChildTail( m_oMap );
	m_oMap.SetZoom( 0.51f );
	m_oMap.SetAttachment( T2GUIATTACHMENT( 6 ), T2GUIATTACHMENT( 6 ) );

	// The hives, the flower beds and the hiding spots
	TINT                 iNumFlowerBeds = 0;
	ATerrainLocatorList* pLocators      = pTerrain->GetLocatorList();
	TINT                 iNumLocators   = pLocators->GetNumLocators();

	TString8  strName;
	TMatrix44 matWorld;
	pLocators->GetWorldTransform().GetLocalMatrixImp( matWorld );

	for ( TINT i = 0; i < iNumLocators; i++ )
	{
		strName = pLocators->GetLocatorName( i );

		if ( strName.Find( "stump", 0 ) == -1 && strName.Find( "beehive_", 0 ) != -1 )
		{
			if ( m_iNumHives < MAX_HIVES )
			{
				TMatrix44 matLocator;
				pLocators->FindLocator( strName )->GetMatrix( matLocator );

				TMatrix44 matTransform;
				matTransform.Multiply( matWorld, matLocator );

				m_avHivePositions[ m_iNumHives ] = matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
				TVector4& rDirection             = m_avHiveDirections[ m_iNumHives ];
				rDirection                       = matTransform.AsBasisVector4( BASISVECTOR_UP );
				rDirection.x                     = -rDirection.x;
				rDirection.y                     = -rDirection.y;
				rDirection.z                     = -rDirection.z;
				m_iNumHives++;
			}
		}
		else if ( strName.Find( "BeeFlowerBed", 0 ) != -1 )
		{
			// NOTE: retail counts the bed before checking it, so only nine of the ten slots are used
			iNumFlowerBeds++;

			if ( iNumFlowerBeds < MAX_FLOWER_BEDS )
			{
				TMatrix44 matLocator;
				pLocators->FindLocator( strName )->GetMatrix( matLocator );

				TMatrix44 matTransform;
				matTransform.Multiply( matWorld, matLocator );

				AModelInstance* pModel = AModelRepos::GetSingleton()->InstantiateNewModel( strFlowerBed, TNULL );
				pModel->GetTransform().SetMatrix( matTransform );
				pModel->SetTransform( matTransform );
				pModel->SetSkeletonUpdating( TTRUE );

				FlowerBed& rBed = m_aFlowerBeds[ m_iNumFlowerBeds ];
				rBed.pModel     = pModel;
				rBed.vPosition  = matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
				m_iNumFlowerBeds++;
			}
		}
		else if ( strName.Find( "P_hiding_", 0 ) != -1 )
		{
			TMatrix44 matLocator;
			pLocators->FindLocator( strName )->GetMatrix( matLocator );

			TMatrix44 matTransform;
			matTransform.Multiply( matWorld, matLocator );

			AModelInstance* pModel = AModelRepos::GetSingleton()->InstantiateNewModel( strHiding, TNULL );
			pModel->GetTransform().SetMatrix( matTransform );
			pModel->SetTransform( matTransform );
			pModel->SetSkeletonUpdating( TTRUE );

			ACollisionObjectMesh* pCollision = new ACollisionObjectMesh();
			pCollision->Create( pModel->GetSceneObject() );

			HidingSpot& rSpot = m_aHidingSpots[ m_iNumHidingSpots ];
			rSpot.pModel      = pModel;
			rSpot.pCollision  = pCollision;
			rSpot.vPosition   = matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
			rSpot.iMapIcon    = m_oMap.AddIcon( rSpot.vPosition, TVector4::VEC_POSZ, AGUI2MiniMap::ICONTYPE_HIDE, AGUI2MiniMap::ICON_WHITE_STAR, MAP_ICON_SIZE );
			m_oMap.SetIconColour( rSpot.iMapIcon, 0xFF3C96E0 );
			m_iNumHidingSpots++;
		}
	}

	AAnimatableObjectType* pHiveType      = pObjectManager->FindType( TPS8( beehive ) );
	AAnimatableObjectType* pEmptyHiveType = pObjectManager->FindType( TPS8( beehive_empty ) );
	AAnimatableObjectType* pBeeType       = pObjectManager->FindType( TPS8( beehive_bee ) );

	ASteer* pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.AddIcon( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection(), AGUI2MiniMap::ICONTYPE_PLAYER, AGUI2MiniMap::ICON_PLAYER, MAP_ICON_SIZE );
	m_oMap.SetCentre( *m_pPlayer->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetDirection() );

	// The honey left in the hives since the last time
	AReplayHoneyCollecting* pReplay = TNULL;

	if ( AGameLoader::IsStoryMode() == TTRUE )
		pReplay = TSTATICCAST( AReplayHoneyCollecting, AReplayManager::GetSingleton()->FindReplay( TPS8( honey_collecting_replay ) ) );

	for ( TINT i = 0; i < m_iNumHives; i++ )
	{
		Hive&           rHive      = m_aHives[ i ];
		const TVector4& rcPosition = m_avHivePositions[ i ];
		const TVector4& rcDir      = m_avHiveDirections[ i ];

		rHive.pHive = pObjectManager->CreateAnimatableObject( pHiveType, TNULL, 0 );
		rHive.pHive->GetModelInstance()->SetTransform( rcPosition, rcDir );

		rHive.pEmptyHive = pObjectManager->CreateAnimatableObject( pEmptyHiveType, TNULL, 0 );
		rHive.pEmptyHive->GetModelInstance()->SetTransform( rcPosition, rcDir );

		rHive.iMapIcon = m_oMap.AddIcon( rcPosition, rcDir, AGUI2MiniMap::ICONTYPE_EDGE, AGUI2MiniMap::ICON_HONEY, MAP_ICON_SIZE );
		m_oMap.SetIconRotates( rHive.iMapIcon, TFALSE );

		TINT  iHoney    = HIVE_HONEY;
		TBOOL bHasHoney = TTRUE;

		if ( pReplay )
		{
			iHoney = pReplay->m_aiHoney[ i ];

			if ( iHoney == ROYAL_HIVE_HONEY )
				rHive.bRoyalJelly = TTRUE;
			else if ( iHoney <= 0 )
				bHasHoney = TFALSE;
		}

		if ( bHasHoney )
		{
			rHive.pHive->SetSkeletonUpdating( TTRUE, TTRUE );
			rHive.pEmptyHive->SetSkeletonUpdating( TFALSE, TTRUE );
			m_oMap.SetIconVisible( rHive.iMapIcon, TTRUE );
		}
		else
		{
			rHive.pHive->SetSkeletonUpdating( TFALSE, TTRUE );
			rHive.pEmptyHive->SetSkeletonUpdating( TTRUE, TTRUE );
			m_oMap.SetIconVisible( rHive.iMapIcon, TFALSE );
		}

		rHive.iHoney = iHoney;
		rHive.oSections.Create( AHoneyCollectHudElement::NUM_SECTIONS, 1 );
		rHive.iUnk1c = 0;

		ACollisionObjectSimple* pCollision = new ACollisionObjectSimple();
		rHive.pCollision                   = pCollision;
		pCollision->CreateActive( rcPosition, 1.0f, 1.0f, 0.0f, 0, TNULL );

		rHive.pBee = pObjectManager->CreateAnimatableObject( pBeeType, TNULL, 0 );
		rHive.pBee->GetModelInstance()->SetTransform( rcPosition, rcDir );
		rHive.pBee->SetSkeletonUpdating( TTRUE, TTRUE );
		rHive.pBee->PlayAnimation( TPS8( idle_lp ) );
	}

	if ( !pReplay )
	{
		// The last hive has the royal jelly
		m_aHives[ m_iNumHives - 1 ].iHoney      = ROYAL_HIVE_HONEY;
		m_aHives[ m_iNumHives - 1 ].bRoyalJelly = TTRUE;
	}

	// The player starts hidden in the first stump
	AUnit*  pPlayerUnit = m_pPlayer->GetSelfProxy()->GetUnit();
	ASteer* pSteer      = pPlayerUnit->GetSteer();

	m_iHidingSpot      = 0;
	TVector4 vPosition = m_aHidingSpots[ m_iHidingSpot ].vPosition;
	vPosition.y -= 0.05f;
	pSteer->SetPosition( vPosition );

	TVector4 vDirection;
	m_aHidingSpots[ m_iHidingSpot ].pModel->GetUpVector( vDirection );
	vDirection.x = -vDirection.x;
	vDirection.y = -vDirection.y;
	vDirection.z = -vDirection.z;
	vDirection.Normalise();
	vDirection.w = 1.0f;
	m_pPlayer->SetSteerDirection( vDirection );

	m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( hide_loop ) );
	m_bHiding = TTRUE;

	// The bees
	m_apSwarms[ 0 ] = TNULL;
	m_apSwarms[ 1 ] = TNULL;
	m_apSwarms[ 2 ] = TNULL;
	m_apSwarms[ 3 ] = TNULL;
	CreateSwarms();

	m_aHives[ 0 ].pSwarm = m_apSwarms[ 0 ] ? m_apSwarms[ 0 ] : TNULL;
	m_aHives[ 1 ].pSwarm = m_apSwarms[ 1 ] ? m_apSwarms[ 1 ] : TNULL;
	m_aHives[ 2 ].pSwarm = m_apSwarms[ 3 ] ? m_apSwarms[ 3 ] : TNULL;

	// NOTE: retail also stores m_apSwarms[ 2 ] as the swarm of a fourth hive, past the end of m_aHives (in
	// m_oCollectHud, which Create below overwrites)
	ABeeSwarm* pFourthSwarm = m_apSwarms[ 2 ] ? m_apSwarms[ 2 ] : TNULL;

	if ( m_aHives[ 0 ].pSwarm )
		m_aHives[ 0 ].pSwarm->SetFlowerBed( m_aFlowerBeds[ 0 ].vPosition );

	if ( m_aHives[ 1 ].pSwarm )
		m_aHives[ 1 ].pSwarm->SetFlowerBed( m_aFlowerBeds[ 3 ].vPosition );

	if ( m_aHives[ 2 ].pSwarm )
		m_aHives[ 2 ].pSwarm->SetFlowerBed( m_aFlowerBeds[ 2 ].vPosition );

	if ( pFourthSwarm )
		pFourthSwarm->SetFlowerBed( m_aFlowerBeds[ 1 ].vPosition );

	// The HUD
	m_oCollectHud.Create( this );
	m_oCollectHud.Hide();
	AHudManager::GetSingleton()->AddElement( &m_oCountHud, TFALSE, TTRUE, TFALSE );
	m_oCountHud.SetForceHide( TTRUE );

	ASimpleProjectileManager::GetSingleton()->CreateDefaultPools();
	AMilkManager::Load( "Data/MilkParticles/WalnutWoodsMilk.trb" );

	// The arrow above the hive or the exit
	m_pArrow = AModelRepos::GetSingleton()->InstantiateNewModel( TPS8D( "data\\models\\honey_arrow" ), TNULL );
	m_pArrow->GetTransform().SetScale( 0.5f, 0.5f, 0.5f );
	m_pArrow->SetOpacity( 0.75f );

	// The hint
	m_pHintFont = T2GUIFontManager::FindFont( "Rekord18" );
	m_oHint.Create( m_pHintFont, AGUI2::GetRootElement()->GetWidth() * 0.5f );
	m_oHint.SetAttachment( T2GUIATTACHMENT( 7 ), T2GUIATTACHMENT( 7 ) );
	m_oHint.SetTransform( 0.0f, -10.0f, 0.0f );
	m_oHint.Hide();
	m_GUIElement.AddChildTail( m_oHint );
	m_GUIElement.AddChildTail( m_oMap );

	// The honey glob of the "honey" animations
	ANamedAnimationSetRef pHoneySet = m_pPlayer->GetAnimatableObject()->GetObjectType()->FindAnimationSetOrDefault( TPS8( honey ) );
	TINT                  iNumAnims = pHoneySet->GetNumNonOverlayAnimations();

	for ( TINT i = 0; i < iNumAnims; i++ )
	{
		ANamedAnimation* pAnimation      = pHoneySet->GetNonOverlayAnimation( i );
		TINT             iNumBreakpoints = pAnimation->GetBreakpoints().Size();

		for ( TINT k = 0; k < iNumBreakpoints; k++ )
		{
			AAnimationBreakpoint* pBreakpoint = pAnimation->GetBreakpoints()[ k ];

			if ( pBreakpoint->GetClass()->IsA( &TGetClass( AAnimationBreakpointAttachment ) ) )
			{
				AAnimationBreakpointAttachment* pAttachment = TSTATICCAST( AAnimationBreakpointAttachment, pBreakpoint );

				if ( pAttachment->GetMode() == AAnimationBreakpointAttachment::MODE_CREATE_CHILD && pAttachment->GetModelName() == TPS8( honeyglob ) )
					m_pHoneyGlob = pAttachment;
			}
		}
	}

	// NOTE: retail reads this string and doesn't use it
	ALocaleManager::Interface()->GetString( 338 );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	ALocale::FormatInputTags( m_wszLeaveText, TARRAYSIZE( m_wszLeaveText ), ALocaleManager::Interface()->GetString( LOCALE_LEAVE ) );

	// NOTE: retail looks the texture section up and doesn't use it
	T2GUITextureSectionManager::GetTextureSection( "Jar_Full" );

	m_oUnkText.Create( pFont, 0.0f );
	m_oUnkText.Hide();
	m_GUIElement.AddChildTail( m_oUnkText );

	m_oReticles.Create( m_pPlayer, 50.0f, &m_GUIElement );
	m_oCollectHud.GetCentre( m_fCombX, m_fCombY );

	AMultiplexStream::GetSingleton()->Open( "Honey" );

	m_oLifeHud.Create( AGUI2::GetRootElement(), TNULL, 8, "Score_Wood_Thin", T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ), 1.0f, TFALSE );
#ifdef BARNYARD_LATEST
	m_oLifeHud.SetLives( s_iNumLives );
#else
	m_oLifeHud.SetLives( MAX_LIVES );
#endif
	m_oLifeHud.SetTranslation( 0.0f, 50.0f );
	m_oLifeHud.SetVisible( TFALSE );

	ASSeqController::GetSingleton()->LoadScript( "ssch_main.tso", TVector4::VEC_ZERO );
	m_oCountHud.SetRoyalJellyVisible( TFALSE );

	// The milk
	m_oAmmoBar.Create( 32.5f, 1.625f, 0.3f );
	m_oAmmoBar.SetAttachment( T2GUIATTACHMENT( 0 ), T2GUIATTACHMENT( 0 ) );
	m_oAmmoBar.Hide();
	AGUI2::GetRootElement()->AddChildTail( m_oAmmoBar );

	// The swat bar
	m_oSwatBar.Create( 0.33f, &m_GUIElement, AHudElement::Mask( HUDFLAGS_GAME ) );
	m_oSwatBar.GetBarElement().SetAttachment( T2GUIATTACHMENT( 1 ), T2GUIATTACHMENT( 1 ) );
	m_oSwatBar.GetBarElement().SetTranslation( 0.0f, m_GUIElement.GetHeight() * 0.25f );
	m_oSwatBar.SetValue( 0.0f, TTRUE );
	m_oSwatBar.SetBarVisible( TFALSE );

	m_oCountHud.SetHoney( 0.0f );
	pProfileManager->GetMainProfile()->SetUpdateStamina( TFALSE );

	AMiniGameState::OnInsertion();
}

// $Barnyard: FUNCTION 0048dc40
void AHoneyCollectingGame::OnRemoval()
{
	if ( m_pSunglasses )
	{
		if ( m_pPlayer )
			AAnimatableObjectManager::GetSingleton()->DetachObject( m_pSunglasses, m_pPlayer->GetAnimatableObject() );

		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_pSunglasses );
		m_pSunglasses = TNULL;
	}

	m_oMilkHitListeners.RemoveAll();

	ARenderer::GetSingleton()->SetNear( m_fOldNear );
	m_fOldNear = 1.0f;

	m_oCollectHud.Destroy();
	m_oCountHud.Detach();
	APlayerManager::GetSingleton()->SetCurrentPlayerSet( 0 );

	if ( AGameLoader::IsStoryMode() == TTRUE )
	{
		AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );

		// The honey and the royal jelly found go to the farm
		APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

		if ( m_bWon == TTRUE )
			pProfile->SetNumHoneycombs( 1 );

		pProfile->SetRoyalJelly( m_bRoyalJelly );
	}

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->ResetHelperTargets();
	pCameraManager->DetachSplitscreenCameraHelpers();
	pCameraManager->DetachCameraHelpers();

	AUnitManager::GetSingleton()->DestroyUnit( AUnitNameManager::GetSingleton()->RetrieveByName( TPS8( simplayer ) ) );

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
		AUnitManager::GetSingleton()->DestroyUnit( m_apSwarms[ i ] );

	for ( TINT i = 0; i < m_iNumHives; i++ )
	{
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_aHives[ i ].pHive );
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_aHives[ i ].pEmptyHive );
		AAnimatableObjectManager::GetSingleton()->DestroyAnimatableObject( m_aHives[ i ].pBee );

		if ( m_aHives[ i ].pCollision )
			delete m_aHives[ i ].pCollision;
	}

	m_oMilkBottles.Clear();
	ASimpleProjectileManager::GetSingleton()->DestroyAllPools();

	if ( AMilkManager::GetSingleton() )
		delete AMilkManager::GetSingleton();

	if ( m_pArrow )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_pArrow );
		m_pArrow = TNULL;
	}

	m_oRoyalJellyText.RemoveSelf();
	m_oAmmoBar.RemoveSelf();

	// NOTE: retail fills the freed slots with an uninitialised structure
	while ( m_iNumHidingSpots != 0 )
	{
		HidingSpot& rSpot = m_aHidingSpots[ m_iNumHidingSpots - 1 ];
		AModelRepos::GetSingleton()->DestroyModelInstance( rSpot.pModel );

		if ( rSpot.pCollision )
			delete rSpot.pCollision;

		m_iNumHidingSpots--;
	}

	while ( m_iNumFlowerBeds != 0 )
	{
		AModelRepos::GetSingleton()->DestroyModelInstance( m_aFlowerBeds[ m_iNumFlowerBeds - 1 ].pModel );
		m_iNumFlowerBeds--;
	}

	AInputHandler::GetSingleton()->StopAllRumbles();
	AInputHandler::GetSingleton()->StopRumble( AInputHandler::GetSingleton()->GetMainController() );

	m_oUnkText.RemoveSelf();
	AMultiplexStream::GetSingleton()->Close( TTRUE );
	APlayerProfileManager::GetSingleton()->GetMainProfile()->SetUpdateStamina( TTRUE );
	AHudManager::GetSingleton()->FindElement<AInteractionHudElement>()->UpdateWidth();

	AMiniGameState::OnRemoval();
}

// $Barnyard: FUNCTION 0048c270
void AHoneyCollectingGame::OnActivate()
{
	m_fTime = 0.0f;
	AMiniGameState::OnActivate();

	if ( m_iUnkbb4 == 1 )
		m_iUnkbb4 = 2;

	if ( m_oStartHudElement.IsForceHidden() == TTRUE )
	{
		if ( m_bCollecting == TTRUE && m_aHives[ m_iHive ].iHoney == 1 && m_aHives[ m_iHive ].bRoyalJelly == TTRUE )
			m_oRoyalJellyText.Show();

		m_oHint.Show();

		if ( m_iNumStinging > 0 )
			m_oSwatBar.SetBarVisible( TTRUE );

		m_oLifeHud.SetVisible( TTRUE );
		m_oAmmoBar.Show();
	}

	AInputHandler::GetSingleton()->SetRumblePaused( TFALSE );

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		if ( m_apSwarms[ i ] )
			m_apSwarms[ i ]->PauseSound( TFALSE );
	}

	if ( m_bCollecting == TTRUE )
	{
		AGUI2::GetMouseCursor().SetRadialField( m_fCombX, m_fCombY, COMB_CURSOR_RADIUS );

		if ( m_bCollecting == TTRUE )
			m_oCollectHud.Show();
	}

	AMultiplexStream::GetSingleton()->Pause( TFALSE );
}

// $Barnyard: FUNCTION 0048c380
void AHoneyCollectingGame::OnDeactivate()
{
	m_oRoyalJellyText.Hide();
	m_oHint.Hide();
	m_oSwatBar.SetBarVisible( TFALSE );
	m_oCollectHud.Hide();
	AInputHandler::GetSingleton()->SetRumblePaused( TTRUE );

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		if ( m_apSwarms[ i ] )
			m_apSwarms[ i ]->PauseSound( TTRUE );
	}

	if ( m_bCollecting == TTRUE )
		AGUI2::GetMouseCursor().ClearRadialField();

	m_oLifeHud.SetVisible( TFALSE );
	m_oAmmoBar.Hide();

	// NOTE: retail doesn't check the class of the camera helper
	if ( ACameraManager::GetSingleton()->GetCurrentCameraHelper() )
		TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() )->SetMouseInput( TTRUE );

	m_oCollectHud.SetMouseInput( TFALSE );
	AMultiplexStream::GetSingleton()->Pause( TTRUE );

	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 0048cb80
void AHoneyCollectingGame::Pause()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "ssch_main", "ssch_main", "SSCH_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TFALSE, TFALSE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 0048cc20
void AHoneyCollectingGame::PlayLoseSequence()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "ssch_defeat", "ssch_defeat", "SSCHdefeat_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TFALSE, TTRUE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 0048ccc0
void AHoneyCollectingGame::PlayWinSequence()
{
	AAnimatableObjectManager::GetSingleton()->Activate( TTRUE );
	AGameStateController::GetSingleton()->PushState(
	    new ASSeqState( "ssch_victory", "ssch_victory", "SSCHvictory_CameraB", TVector4::VEC_ZERO, 1, TTRUE, TTRUE, TFALSE, TTRUE, TFALSE, TFALSE, TTRUE, TFALSE )
	);
}

// $Barnyard: FUNCTION 0048c180
void AHoneyCollectingGame::OnPreStart()
{
	m_pPlayer->SetVisible( TFALSE );

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		if ( m_apSwarms[ i ] )
			m_apSwarms[ i ]->SetVisible( TFALSE );
	}
}

// $Barnyard: FUNCTION 0048c450
void AHoneyCollectingGame::UpdateAlways( TFLOAT a_fDeltaTime )
{
	// The swarms and their field of view on the minimap
	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		ABeeSwarm* pSwarm     = m_apSwarms[ i ];
		TVector4   vPosition  = *pSwarm->GetSteer()->GetPosition();
		TVector4   vDirection = *pSwarm->GetSteer()->GetDirection();
		m_oMap.SetIconTransform( pSwarm->GetMapIcon(), vPosition, vDirection );

		vDirection.x = -vDirection.x;
		vDirection.y = -vDirection.y;
		vDirection.z = -vDirection.z;

		TVector4 vOffset;
		vOffset.Multiply( vDirection, 3.0f );

		TVector4 vFOVPosition( vPosition.x - vOffset.x, vPosition.y - vOffset.y, vPosition.z - vOffset.z, vPosition.w );
		m_oMap.SetIconTransform( pSwarm->GetFOVMapIcon(), vFOVPosition, vDirection );
	}

	for ( TINT i = 0; i < m_iNumHidingSpots; i++ )
		m_oMap.SetIconTransform( m_aHidingSpots[ i ].iMapIcon, m_aHidingSpots[ i ].vPosition, TVector4::VEC_POSZ );

	ASteer* pPlayerSteer = m_pPlayer->GetSteer();
	m_oMap.SetCentre( *pPlayerSteer->GetPosition(), *pPlayerSteer->GetDirection() );

	if ( m_oUnkText.IsVisible() )
	{
		m_fUnkTextTime -= a_fDeltaTime;

		if ( m_fUnkTextTime <= 0.0f )
		{
			m_fUnkTextTime = 0.0f;

			TFLOAT fAlpha = m_oUnkText.GetAlpha() - ( a_fDeltaTime + a_fDeltaTime );

			if ( fAlpha < 0.0f )
				fAlpha = 0.0f;

			m_oUnkText.SetAlpha( fAlpha );
		}
	}

	// The music
	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( pStream->GetState() != AMultiplexStream::STATE_PLAYING && pStream->Play( 1.0f ) == TTRUE )
	{
		pStream->SetVolume( 1.0f, 0, TTRUE );
		pStream->SetVolume( 1.0f, 1, TTRUE );
		pStream->SetVolume( 0.0f, 2, TTRUE );
		pStream->SetVolume( 0.0f, 3, TTRUE );
	}
}

// $Barnyard: FUNCTION 00490130
void AHoneyCollectingGame::UpdateGame( TFLOAT a_fDeltaTime )
{
	// The milk
	AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();
	pLauncher->Update( a_fDeltaTime );
	TINT iMaxAmmo = pLauncher->GetAmmo()->GetMaxAmmo();
	m_oAmmoBar.SetAmmo( TINT( pLauncher->GetAmmo()->GetAmmo() ), iMaxAmmo );

	m_bRenderReticles = TTRUE;
	m_oMilkBottles.Update( a_fDeltaTime, TTRUE );

	// The bees in front of the hives
	for ( TINT i = 0; i < m_iNumHives; i++ )
	{
		Hive& rHive = m_aHives[ i ];

		if ( rHive.iHoney == 0 || ( m_bCollecting == TTRUE && i == m_iHive && !rHive.pBee->IsAnimPlaying( TPS8( fly_away ), 0.0f ) ) )
		{
			rHive.pBee->SetSkeletonUpdating( TFALSE, TTRUE );
		}
		else if ( m_iHive == -1 && !rHive.pBee->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) )
		{
			rHive.pBee->SetSkeletonUpdating( TTRUE, TTRUE );
			rHive.pBee->PlayAnimation( TPS8( idle_lp ) );
		}
	}

	m_fTime += a_fDeltaTime;
	m_pPlayer->GetActiveLauncher()->Update( a_fDeltaTime );

	if ( AMilkManager::GetSingleton() )
		AMilkManager::GetSingleton()->Update( a_fDeltaTime );

	if ( m_bDefeated == TTRUE )
	{
		// The game is over once the defeat animation has ended
		m_pPlayer->SetMovement( TFALSE );
		m_pPlayer->SetControlFlags( TFALSE );

		if ( m_pPlayer->GetAnimatableObject()->IsAnimationFinished( TPS8( defeat ) ) == TTRUE )
		{
			if ( AGameLoader::IsStoryMode() == TTRUE )
			{
				APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();

				for ( TINT i = 0; i < AReplayHoneyCollecting::NUM_HIVES; i++ )
					pProfile->SetHoneyReplay( i, m_aHives[ i ].iHoney );

				AReplayManager::ms_bRestoreHoneyCollecting = TTRUE;
			}

			m_oResults.bWon    = TFALSE;
			m_oResults.iResult = -1;
			m_oResults.bFlag5  = TTRUE;
			AMusicManager::GetSingleton()->SetEnabled( TTRUE, TTRUE );
			AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( &m_oResults, TFALSE ) );
		}

		return;
	}

	TVector4 vPlayerPosition;

	if ( !m_bWon && ( m_iNumCombs >= HONEY_TO_WIN || m_oCountHud.IsFull() ) )
	{
		// The jar is full: the player takes the honey home
		m_bWon = TTRUE;
		m_oHint.Hide();
		m_oRoyalJellyText.Hide();
		m_oLeaveText.Hide();

		AAnimatableObjectManager* pObjectManager = AAnimatableObjectManager::GetSingleton();
		AAnimatableObject*        pHoney         = pObjectManager->CreateAnimatableObject( pObjectManager->FindType( m_strWorldHoney ), TNULL, 0 );
		pHoney->SetSkeletonUpdating( TFALSE, TTRUE );
		pObjectManager->AttachObject( pHoney, m_pPlayer->GetAnimatableObject(), TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 0.5f );

		// The bees calm down
		for ( TINT i = 0; i < NUM_SWARMS; i++ )
		{
			ABeeSwarm* pSwarm = m_apSwarms[ i ];

			if ( pSwarm && pSwarm->GetState() != ABeeSwarm::STATE_STING && pSwarm->GetState() != ABeeSwarm::STATE_ATTACK )
			{
				pSwarm->SetAngry( TTRUE );
				pSwarm->SetState( ABeeSwarm::STATE_RETURN );
			}
		}
	}
	else if ( m_bCollecting == TTRUE )
	{
		// The stick wins over the keys
		if ( !( m_vStick == TVector2::VEC_ZERO ) )
			m_vKeys = TVector2::VEC_ZERO;

		if ( m_vLastKeys == TVector2::VEC_ZERO )
			m_vSmoothKeys = m_vKeys;
		else
			m_vSmoothKeys.Lerp( m_vSmoothKeys, m_vKeys, a_fDeltaTime * 10.0f );

		Hive& rHive = m_aHives[ m_iHive ];

		if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( honey_lp ), 0.0f ) == TTRUE )
		{
			if ( m_oCollectHud.IsFull() != TTRUE || rHive.iHoney <= 0 || ( m_vSmoothKeys == TVector2::VEC_ZERO && m_vStick == TVector2::VEC_ZERO ) )
			{
				m_oCollectHud.SetHighlightedSection( -1 );

				if ( m_iCollectCue != -1 )
				{
					ASoundManager::GetSingleton()->StopCue( m_iCollectCue );
					m_iCollectCue = -1;
				}
			}
			else
			{
				// The section the stick points at
				TVector2 vInput = ( m_vSmoothKeys == TVector2::VEC_ZERO ) ? m_vStick : m_vSmoothKeys;
				vInput.Normalize();

				TFLOAT fCos = TVector2::VEC_NEGY.x * vInput.x + TVector2::VEC_NEGY.y * vInput.y;
				TMath::Clip( fCos, -1.0f, 1.0f );

				TFLOAT fAngle = TMath::ACos( fCos );

				if ( vInput.x < 0.0f )
					fAngle = TMath::TWO_PI - fAngle;

				TMath::Clip( fAngle, 0.0f, TMath::TWO_PI );

				if ( fAngle == TMath::TWO_PI )
					fAngle = 0.0f;

				TINT iSection = TINT( fAngle / ( TMath::TWO_PI * 0.015625f ) );
				TMath::Clip( iSection, 0, AHoneyCollectHudElement::NUM_SECTIONS );
				m_oCollectHud.SetHighlightedSection( iSection );

				if ( m_oCollectHud.GetSections()->GetBit( iSection ) )
				{
					m_oCollectHud.CollectSection( iSection );

					if ( m_iCollectCue == -1 || !ASoundManager::GetSingleton()->IsCueUsed( m_iCollectCue ) )
						m_iCollectCue = ASoundManager::GetSingleton()->PlayCueAtPositionEx( SOUND_COLLECTING, m_avHivePositions[ m_iHive ], 1.0f, TFALSE, 0.0f, -1 );
				}
			}
		}

		m_vLastKeys = m_vKeys;
		m_vKeys     = TVector2::VEC_ZERO;

		m_pArrow->SetSkeletonUpdating( TFALSE );

		if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( honey_lp ), 0.0f ) == TTRUE )
		{
			m_HUDParams.SetFlags( HUDFLAGS_COLLECTING );
			m_oCollectHud.Show();
		}

		if ( m_oCollectHud.GetNumFullSections() == 0 )
		{
			// The comb is empty
			TINT iHoney = rHive.iHoney;

			if ( iHoney > 0 )
			{
				m_iNumCombs++;
				AHudManager::GetSingleton()->FindElement<AHoneyCountHudElement>();

				// The swarm of the hive gets angry
				if ( rHive.pSwarm )
					rHive.pSwarm->SetAngry( TTRUE );

				if ( iHoney == 1 && rHive.bRoyalJelly )
				{
					m_bRoyalJelly = TTRUE;
					m_oCountHud.SetRoyalJellyVisible( TTRUE );
				}

				ASoundManager::GetSingleton()->PlayCue( SOUND_COMB_DONE );
			}

			rHive.iHoney--;
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( honey_end ) );

			if ( rHive.iHoney <= 0 )
				m_oCollectHud.SetVisible( TFALSE );

			m_oCollectHud.SetNumCollected( 0 );
			m_oCollectHud.SetHive( ( rHive.bRoyalJelly == TTRUE ) ? ROYAL_HIVE_HONEY : HIVE_HONEY, rHive.iHoney, &rHive.oSections );
			m_oCollectHud.Reset( TTRUE );
			StopCollecting( TFALSE );
		}
	}
	else if ( m_bLeavingHide != TTRUE )
	{
		if ( m_bWalkingToHive == TTRUE )
		{
			// Walk to the front of the hive
			TVector4 vPosition  = *m_pPlayer->GetSteer()->GetPosition();
			TVector4 vDirection = *m_pPlayer->GetSteer()->GetDirection();
			TFLOAT   fT         = a_fDeltaTime * 5.0f;

			vPosition.Lerp4( vPosition, m_vHivePosition, fT );
			vPosition.w = 1.0f;
			vDirection.Lerp4( vDirection, m_vHiveDirection, fT );
			vDirection.Normalise();

			m_pPlayer->SetSteerPosition( vPosition );
			m_pPlayer->SetSteerDirection( vDirection );
			m_pPlayer->SetControlFlags( TFALSE );

			TVector4 vOffset;
			vOffset.Substract( vPosition, m_vHivePosition );
			TFLOAT fDot = TVector4::DotProduct3( vDirection, m_vHiveDirection );

			if ( MagnitudeSqXZ( vOffset ) <= 0.040000003f && fDot >= 0.99f )
			{
				m_pPlayer->SetSteerPosition( m_vHivePosition );
				m_pPlayer->SetSteerDirection( m_vHiveDirection );
				m_bWalkingToHive = TFALSE;
				StartCollecting();
			}
		}
		else if ( m_bLeavingHive == TTRUE )
		{
			// Step back from the hive
			TVector4 vPosition = *m_pPlayer->GetSteer()->GetPosition();
			vPosition.Lerp4( vPosition, m_vHivePosition, a_fDeltaTime * 5.0f );
			vPosition.w = 1.0f;
			m_pPlayer->GetSteer()->SetPosition( vPosition );

			AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

			TVector4 vOffset;
			vOffset.Substract( vPosition, m_vHivePosition );

			if ( MagnitudeSqXZ( vOffset ) <= 0.040000003f && !pPlayerObject->IsAnimationPlanned( TPS8( honey_end ) ) &&
			     !pPlayerObject->IsAnimPlaying( TPS8( honey_end ), 0.0f ) )
			{
				m_bLeavingHive = TFALSE;
				m_pPlayer->SetControlFlags( TTRUE );

				AAnimatableObject* pHeld = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( m_pPlayer->GetAnimatableObject(), TPS8( cb_rlocator ), 0 );

				if ( pHeld )
					pHeld->SetSkeletonUpdating( TTRUE, TTRUE );
			}
		}

		// The closest hive
		TVector4 vPosition = *m_pPlayer->GetSteer()->GetPosition();
		TFLOAT   fMinDist  = TMath::MAXFLOAT;
		TINT     iClosest  = -1;

		for ( TINT i = 0; i < m_iNumHives; i++ )
		{
			TVector4 vOffset( m_avHivePositions[ i ].x - vPosition.x, m_avHivePositions[ i ].y - vPosition.y, m_avHivePositions[ i ].z - vPosition.z, m_avHivePositions[ i ].w );

			if ( vOffset.MagnitudeSq() <= fMinDist )
			{
				fMinDist = vOffset.MagnitudeSq();
				iClosest = i;
			}
		}

		TVector4 vArrowPosition;

		if ( m_aHives[ iClosest ].iHoney <= 0 || m_bDefeated )
		{
			if ( !m_bWon )
			{
				m_pArrow->SetSkeletonUpdating( TFALSE );
				goto LABEL_UPDATE_ACTIONS;
			}
		}
		else if ( !m_bWon )
		{
			// Above the closest hive
			vArrowPosition = m_avHivePositions[ iClosest ];
			vArrowPosition.y += TFLOAT( sin( TMath::TWO_PI * m_fTime ) ) * 0.25f - 1.5f;
			m_pArrow->SetSkeletonUpdating( TTRUE );
			goto LABEL_SET_ARROW;
		}

		{
			// Above the closest exit
			AInteriorArea* pEntrance  = m_pNetwork->GetArea( m_pNetwork->FindAreaIndex( TPS8( entrance ) ) );
			AInteriorArea* pEntrance2 = m_pNetwork->GetArea( m_pNetwork->FindAreaIndex( TPS8( entrance2 ) ) );

			TVector4 vEntrance;
			TVector4 vEntrance2;
			pEntrance->GetPosition( vEntrance );
			pEntrance2->GetPosition( vEntrance2 );

			TFLOAT fDX1 = vPosition.x - vEntrance.x;
			TFLOAT fDZ1 = vPosition.z - vEntrance.z;
			TFLOAT fDX2 = vPosition.x - vEntrance2.x;
			TFLOAT fDZ2 = vPosition.z - vEntrance2.z;

			if ( fDX2 * fDX2 + fDZ2 * fDZ2 <= fDX1 * fDX1 + fDZ1 * fDZ1 )
				vArrowPosition = vEntrance2;
			else
				vArrowPosition = vEntrance;

			vArrowPosition.y = TFLOAT( sin( TMath::TWO_PI * m_fTime ) ) * 0.25f - 1.5f + ( vArrowPosition.y - 1.0f );
			m_pArrow->SetSkeletonUpdating( TTRUE );
		}

	LABEL_SET_ARROW:
	{
		m_pArrow->GetTransform().SetTranslate( vArrowPosition.AsVector3() );
		TVector3 vEuler( TMath::PI * m_fTime, 0.0f, TMath::HALF_PI );
		m_pArrow->GetTransform().SetEuler( vEuler );
	}
	}
	else
	{
		// Get out of the stump
		AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() );
		m_eAction                      = ACTION_NONE;

		if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( hide_end ), 0.0f ) != TTRUE ||
		     m_pPlayer->GetAnimatableObject()->IsAnimContinuing( TPS8( hide_end ), a_fDeltaTime ) )
		{
			// The camera follows the way out
			TVector4 vTarget = pHelper->GetTargetPositionOverride();
			vTarget.Lerp4( vTarget, m_vHideExitPosition, a_fDeltaTime * 5.0f );
			vTarget.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vTarget, TCollisionCommon::TOSHICGROUPFLAG_NONE );
			pHelper->SetTargetPositionOverride( vTarget );
		}
		else
		{
			m_bLeavingHide = TFALSE;
			m_pPlayer->SetMovement( TTRUE );
			m_pPlayer->SetControlFlags( TTRUE );
			TSTATICCAST( AAlienAnimController, m_pPlayer->GetAnimatableObject()->FindAnimController( &TGetClass( AAlienAnimController ) ) )->SetAnimEnabled( TTRUE );

			m_vHideExitPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( m_vHideExitPosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );
			m_pPlayer->SetSteerPosition( m_vHideExitPosition );
			m_pPlayer->SetSteerDirection( m_vHideExitDirection );
			pHelper->SetTargetPositionOverride( TVector4::VEC_ZERO );

			AAnimatableObject* pHeld = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( m_pPlayer->GetAnimatableObject(), TPS8( cb_rlocator ), 0 );

			if ( pHeld )
				pHeld->SetSkeletonUpdating( TTRUE, TTRUE );
		}
	}

LABEL_UPDATE_ACTIONS:
	vPlayerPosition = *m_pPlayer->GetSteer()->GetPosition();
	m_eAction       = ACTION_NONE;

	// In front of a hive?
	if ( !m_bCollecting && !m_bWalkingToHive )
	{
		for ( TINT i = 0; i < m_iNumHives; i++ )
		{
			TVector4 vSide = m_avHiveDirections[ i ];
			vSide.AsVector3().RotateY( TMath::HALF_PI );

			TVector4 vOffset;
			vOffset.Multiply( vSide, 1.0f );

			// Where the player stands to collect
			TVector4 vStand( vOffset.x + m_avHivePositions[ i ].x, vOffset.y + m_avHivePositions[ i ].y, vOffset.z + m_avHivePositions[ i ].z, m_avHivePositions[ i ].w );
			vStand.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vStand, TCollisionCommon::TOSHICGROUPFLAG_NONE );

			TVector4 vToStand( vStand.x - vPlayerPosition.x, vStand.y - vPlayerPosition.y, vStand.z - vPlayerPosition.z, vStand.w );

			if ( MagnitudeSqXZ( vToStand ) <= 2.25f )
			{
				TVector4 vPlayerDirection = *m_pPlayer->GetSteer()->GetDirection();

				if ( TVector4::DotProduct3( vPlayerDirection, vSide ) <= -0.6f && m_oCollectHud.IsFull() == TTRUE && m_iNumStinging == 0 && m_aHives[ i ].iHoney > 0 )
				{
					m_eAction = ACTION_COLLECT;
					m_iHive   = i;
					break;
				}
			}

			m_iHive   = -1;
			m_eAction = ACTION_NONE;
		}
	}

	// Next to a hiding spot?
	if ( !m_bHiding && !m_bLeavingHide )
	{
		TFLOAT fMinDist = TMath::MAXFLOAT;
		m_iHidingSpot   = -1;

		for ( TINT i = 0; i < m_iNumHidingSpots; i++ )
		{
			TVector4 vOffset( m_aHidingSpots[ i ].vPosition.x - vPlayerPosition.x, m_aHidingSpots[ i ].vPosition.y - vPlayerPosition.y, m_aHidingSpots[ i ].vPosition.z - vPlayerPosition.z, m_aHidingSpots[ i ].vPosition.w );
			TFLOAT   fDist = MagnitudeSqXZ( vOffset );

			if ( fDist < fMinDist && fDist < 2.25f )
			{
				fMinDist      = fDist;
				m_iHidingSpot = i;
			}
		}

		if ( m_iHidingSpot != -1 )
			m_eAction = ACTION_HIDE;
	}

	// The swarms
	m_iNumStinging  = 0;
	m_iNumAttacking = 0;
	m_iNumAngry     = 0;

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		ABeeSwarm* pSwarm = m_apSwarms[ i ];

		if ( !pSwarm )
			continue;

		ABeeSwarm::STATE eState = pSwarm->GetState();

		if ( eState == ABeeSwarm::STATE_STING )
		{
			m_oMap.SetIconAlpha( pSwarm->GetFOVMapIcon(), 0.0f );

			if ( m_bHiding == TTRUE )
			{
				// The bees lose the player in the stump
				TVector4 vOffset;
				vOffset.Substract( vPlayerPosition, *pSwarm->GetSteer()->GetPosition() );

				if ( MagnitudeSqXZ( vOffset ) <= 0.25f )
				{
					pSwarm->SetState( ABeeSwarm::STATE_IDLE );
					pSwarm->SetAngry( TFALSE );
					pSwarm->SetHideTime( 0.0f );
				}
			}
			else if ( m_bDefeated == TTRUE )
			{
				pSwarm->SetState( ABeeSwarm::STATE_RETURN );
			}
			else
			{
				if ( m_fStingInterval == 0.0f )
				{
					m_fStingInterval = 10.0f;

					if ( m_iNumStinging != 0 )
						m_fStingTime = 0.0f;
					else
						m_fStingTime = 10.0f + 2.0f;
				}

				if ( !m_bHiding && !m_bLeavingHide )
					m_iNumStinging++;
			}
		}
		else if ( m_bHiding != TTRUE || eState != ABeeSwarm::STATE_IDLE )
		{
			if ( eState == ABeeSwarm::STATE_ATTACK )
			{
				m_oMap.SetIconAlpha( pSwarm->GetFOVMapIcon(), 0.0f );
				m_iNumAttacking++;
			}
			else
			{
				m_oMap.SetIconAlpha( pSwarm->GetFOVMapIcon(), 1.0f );
			}
		}
		else
		{
			// Waiting next to the stump
			pSwarm->SetHideTime( pSwarm->GetHideTime() + a_fDeltaTime );

			if ( pSwarm->GetHideTime() >= 3.0f )
			{
				pSwarm->SetState( ABeeSwarm::STATE_RETURN );
				pSwarm->SetHideTime( 0.0f );
			}
		}

		if ( m_bHiding == TTRUE && pSwarm->IsAngry() == TTRUE )
		{
			pSwarm->SetAngry( TFALSE );
			m_oMap.SetIconScale( pSwarm->GetMapIcon(), 1.0f );
		}

		if ( pSwarm->IsAngry() == TTRUE )
		{
			TFLOAT fScale = TFLOAT( sin( TMath::TWO_PI * m_fTime ) ) * 0.25f + 1.0f;
			m_oMap.SetIconScale( pSwarm->GetMapIcon(), fScale );
			m_iNumAngry++;
		}
	}

	// The danger music
	AMultiplexStream* pStream = AMultiplexStream::GetSingleton();

	if ( pStream->GetState() == AMultiplexStream::STATE_PLAYING )
	{
		if ( ( m_iNumAngry <= 0 && m_iNumAttacking <= 0 && m_iNumStinging <= 0 ) || m_bDefeated )
		{
			TFLOAT fVolume = pStream->GetVolume( 2 );

			if ( fVolume > 0.0f )
			{
				fVolume -= a_fDeltaTime + a_fDeltaTime;

				if ( fVolume < 0.0f )
					fVolume = 0.0f;

				pStream->SetVolume( fVolume, 2, TTRUE );
				pStream->SetVolume( fVolume, 3, TTRUE );
			}
		}
		else
		{
			pStream->SetVolume( 1.0f, 2, TTRUE );
			pStream->SetVolume( 1.0f, 3, TTRUE );
		}
	}

	// The hives
	// NOTE: retail reads m_aHives[ m_iHive ] even when m_iHive is -1 (AMiniGameState::m_pCharityEveret)
	for ( TINT i = 0; i < m_iNumHives; i++ )
	{
		Hive&      rHive  = m_aHives[ i ];
		ABeeSwarm* pSwarm = rHive.pSwarm;

		if ( !pSwarm )
			continue;

		ABeeSwarm::STATE eState = pSwarm->GetState();

		if ( eState != ABeeSwarm::STATE_POLLEN && eState != ABeeSwarm::STATE_SEEK_POLLEN && eState != ABeeSwarm::STATE_SEEK_HIVE )
		{
			if ( eState == ABeeSwarm::STATE_AT_HIVE )
			{
				// The swarm brought honey to its hive
				TINT iMaxHoney = HIVE_HONEY + ( rHive.bRoyalJelly != TFALSE );
				rHive.iHoney += pSwarm->GetHoney();
				TMath::Clip( rHive.iHoney, 0, iMaxHoney );

				if ( rHive.iHoney < iMaxHoney )
					pSwarm->SetState( ABeeSwarm::STATE_SEEK_POLLEN );
				else
					pSwarm->SetState( ABeeSwarm::STATE_RETURN );

				pSwarm->SetHoney( 0 );
			}
			else if ( rHive.iHoney >= 0 )
			{
				pSwarm->SetIgnorePlayer( TFALSE );
			}
			else
			{
				m_aHives[ m_iHive ].iHoney = 0;
			}
		}

		// The royal jelly is in the last comb of its hive
		if ( m_aHives[ m_iHive ].iHoney == 1 && m_aHives[ m_iHive ].bRoyalJelly == TTRUE && m_bCollecting == TTRUE )
		{
			if ( !m_oCollectHud.IsPulsing() )
			{
				m_oCollectHud.SetPulse( TTRUE );
				m_oRoyalJellyText.Show();
			}
			else
			{
				T2GUITransform& rTransform   = m_oRoyalJellyText.GetTransform();
				TVector2        vTranslation = rTransform.GetTranslation();
				rTransform.SetIdentity();
				rTransform.SetTranslation( vTranslation );

				TFLOAT fScale = TFLOAT( sin( TMath::TWO_PI * m_fTime ) ) * 0.25f + 1.0f;

				T2GUITransform oScale;
				oScale.m_aMatrixRows[ 0 ] = TVector2( fScale, 0.0f );
				oScale.m_aMatrixRows[ 1 ] = TVector2( 0.0f, fScale );
				oScale.m_vecTranslation   = TVector2::VEC_ZERO;
				rTransform.PreMultiply( oScale );
			}
		}
		else if ( m_oCollectHud.IsPulsing() == TTRUE )
		{
			m_oCollectHud.SetPulse( TFALSE );
			m_oRoyalJellyText.Hide();
		}

		if ( m_aHives[ m_iHive ].iHoney < 0 )
			m_aHives[ m_iHive ].iHoney = 0;

		AModelInstance* pEmptyModel = rHive.pEmptyHive->GetModelInstance();

		if ( rHive.iHoney != 0 || ( pEmptyModel && pEmptyModel->IsUpdatingSkeleton() ) )
		{
			if ( rHive.iHoney == HIVE_HONEY )
			{
				// Full again
				AModelInstance* pHiveModel = rHive.pHive->GetModelInstance();

				if ( !pHiveModel || !pHiveModel->IsUpdatingSkeleton() )
				{
					rHive.pHive->SetSkeletonUpdating( TTRUE, TTRUE );
					rHive.pEmptyHive->SetSkeletonUpdating( TFALSE, TTRUE );
				}
			}
		}
		else
		{
			// The hive has just been emptied
			rHive.pHive->SetSkeletonUpdating( TFALSE, TTRUE );
			rHive.pEmptyHive->SetSkeletonUpdating( TTRUE, TTRUE );
			m_oMap.SetIconVisible( rHive.iMapIcon, TFALSE );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffEffect, m_avHivePositions[ i ], TVector4::VEC_NEGY );
			m_iNumEmptiedHives++;
		}
	}

	AAnimatableObject* pPlayerObject = m_pPlayer->GetAnimatableObject();

	if ( m_bHiding == TTRUE )
	{
		// Get into the stump
		m_oSwatBar.SetValue( 0.0f, TTRUE );
		m_oSwatBar.SetBarVisible( TFALSE );

		HidingSpot& rSpot   = m_aHidingSpots[ m_iHidingSpot ];
		TVector4    vTarget = rSpot.vPosition;

		TVector4 vPosition;
		m_pPlayer->GetSteer()->CopyPosition( vPosition );

		TVector4 vOffset( vTarget.x - vPosition.x, vTarget.y - vPosition.y, vTarget.z - vPosition.z, vTarget.w );

		if ( !m_pPlayer->GetAnimatableObject()->IsAnimationPlanned( TPS8( hide_start ) ) && !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( hide_start ), 0.0f ) )
		{
			TVector4 vUp;
			rSpot.pModel->GetUpVector( vUp );
			vUp.x = -vUp.x;
			vUp.y = -vUp.y;
			vUp.z = -vUp.z;
			vUp.Normalise();
			vUp.w = 1.0f;
			m_pPlayer->SetSteerDirection( vUp );
		}

		if ( MagnitudeSqXZ( vOffset ) <= 0.1f )
		{
			vTarget.y -= 0.05f;
			m_pPlayer->GetSteer()->SetPosition( vTarget );
		}
		else
		{
			TVector4 vDirection = vOffset;
			vDirection.Normalise();
			m_pPlayer->SetSteerDirection( vDirection );

			TVector4 vStep;
			vStep.Multiply( vDirection, a_fDeltaTime );
			vStep.Multiply( vStep, 3.0f );

			vPosition.x += vStep.x;
			vPosition.z += vStep.z;
			vPosition.y = vTarget.y - 0.05f;
			m_pPlayer->GetSteer()->SetPosition( vPosition );
		}
	}
	else if ( m_iNumStinging > 0 && !pPlayerObject->IsAnimPlaying( TPS8( honey_start ), 0.0f ) && !pPlayerObject->IsAnimPlaying( TPS8( honey_end ), 0.0f ) )
	{
		// Stung: no sprinting, no jumping, slower
		APlayerSteer*                  pSteer    = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );
		TFLOAT                         fMaxSpeed = pSteer->GetTraits()->m_fLogicalSprintSpeed;
		ASimplePlayerControlComponent* pControl  = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );
		pControl->SetCanSprint( TFALSE );
		pControl->SetCanJump( TFALSE );

		if ( pSteer->GetMeasuredSpeed() > fMaxSpeed )
			pSteer->GetKinematics()->SetMeasuredSpeed( fMaxSpeed );

		TPString8 strSet = m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet()->GetName();

		if ( strSet != TPS8( stinging ) )
		{
			if ( pSteer->GetState() == APlayerSteer::STATE_STILL || pSteer->GetState() == APlayerSteer::STATE_WALKING )
			{
				pSteer->SetRestoreAnimationSet( TFALSE );
				m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().KillAllAnimations();

				ANamedAnimationSetRef pSet = m_pPlayer->GetAnimatableObject()->GetObjectType()->FindAnimationSet( TPS8( stinging ) );
				m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pSet );
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
			}
		}

		// The swat animations speed up with the swat bar
		if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( swat_lp ), 0.0f ) == TTRUE )
		{
			TFLOAT fPower = m_oSwatBar.m_fTargetValue;

			if ( fPower <= 0.05f || m_pPlayer->GetSteer()->GetSpeed() > 0.05f )
			{
				m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( swat_lp ) );
			}
			else
			{
				TFLOAT fSpeed = pPlayerObject->GetAnimationSpeed( TPS8( swat_lp ) );
				pPlayerObject->SetAnimationSpeed( TPS8( swat_lp ), TMath::LERP( fSpeed, fPower + 1.0f, a_fDeltaTime * 5.0f ) );
			}
		}
		else if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( swat_run_lp ), 0.0f ) == TTRUE )
		{
			TFLOAT fPower = m_oSwatBar.m_fTargetValue;

			if ( fPower <= 0.05f )
			{
				m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( swat_run_lp ) );
			}
			else
			{
				TFLOAT fSpeed = pPlayerObject->GetAnimationSpeed( TPS8( swat_run_lp ) );
				pPlayerObject->SetAnimationSpeed( TPS8( swat_run_lp ), TMath::LERP( fSpeed, fPower + 1.0f, a_fDeltaTime * 5.0f ) );
			}
		}

		if ( m_bCollecting == TTRUE )
			StopCollecting( TTRUE );

		if ( !m_oSwatBar.IsBarVisible() && !m_bDefeated )
			m_oSwatBar.SetBarVisible( TTRUE );
	}
	else if ( m_bHiding == TTRUE || m_bLeavingHide == TTRUE )
	{
		m_oSwatBar.SetBarVisible( TFALSE );
	}
	else
	{
		m_oSwatBar.SetBarVisible( TFALSE );

		TPString8 strSet = m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet()->GetName();

		if ( strSet == TPS8( stinging ) )
		{
			// The bees are gone
			m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( idle_lp ) );
			m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( swat_lp ) );
			m_pPlayer->GetAnimatableObject()->FinishAnimation( TPS8( swat_run_lp ) );
			TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->SetRestoreAnimationSet( TTRUE );

			ANamedAnimationSetRef pSet = m_pPlayer->GetAnimatableObject()->GetObjectType()->FindAnimationSet( TPS8( default ) );
			m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pSet );
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
		}

		ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, m_pPlayer->FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );
		pControl->SetCanSprint( TTRUE );
		pControl->SetCanJump( TTRUE );
	}

	if ( m_iNumStinging <= 0 )
	{
		// NOTE: retail stops a rumble with a stub that does nothing on PC (0054df00)
		m_fStingTime = 0.0f;
		m_iUnk8204   = -1;
	}
	else
	{
		// Enough swats send the bees away
		TFLOAT fPower = m_oSwatBar.m_fTargetValue;

		if ( fPower < 1.0f )
		{
			fPower -= a_fDeltaTime * 0.2f;
			TMath::Clip( fPower, 0.0f, 1.0f );
			m_oSwatBar.SetValue( fPower, TTRUE );
		}
		else
		{
			for ( TINT i = 0; i < NUM_SWARMS; i++ )
			{
				if ( m_apSwarms[ i ] && m_apSwarms[ i ]->GetState() == ABeeSwarm::STATE_STING )
				{
					m_apSwarms[ i ]->SetState( ABeeSwarm::STATE_DISPERSE );
					m_apSwarms[ i ]->SetAngry( TTRUE );
				}
			}

			m_iNumStinging = 0;
			m_oSwatBar.SetBarVisible( TFALSE );
			m_oSwatBar.SetValue( 0.0f, TTRUE );
		}

		APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );
		TPString8     strSet = m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().GetAnimationSet()->GetName();

		if ( strSet == TPS8( stinging ) && !m_bHiding && m_fStingTime > m_fStingInterval &&
		     !m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( stung ), 0.0f ) && pSteer->GetState() == APlayerSteer::STATE_WALKING && strSet == TPS8( stinging ) )
		{
			// Stung: a life is lost
			m_pPlayer->GetAnimatableObject()->KillAllAnimations();
			m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( stung ) );

			// NOTE: retail starts a rumble with a stub that does nothing on PC (005cc080)

			if ( m_iLives > 0 )
			{
				m_iLives--;
				m_oLifeHud.AddLives( -1 );
			}

			if ( m_iLives <= 0 )
			{
				// Defeat
				m_pPlayer->GetAnimatableObject()->KillAllAnimations();
				m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( defeat ) );
				m_bDefeated = TTRUE;
				m_oLeaveText.Hide();
				m_pArrow->SetSkeletonUpdating( TFALSE );

				for ( TINT i = 0; i < NUM_SWARMS; i++ )
				{
					ABeeSwarm* pSwarm = m_apSwarms[ i ];

					if ( pSwarm && ( pSwarm->GetState() == ABeeSwarm::STATE_STING || pSwarm->GetState() == ABeeSwarm::STATE_ATTACK ) )
					{
						pSwarm->SetState( ABeeSwarm::STATE_RETURN );
						pSwarm->SetAngry( TFALSE );
					}
				}

				if ( m_iCollectCue != -1 )
					ASoundManager::GetSingleton()->StopCue( m_iCollectCue );

				m_oSwatBar.SetBarVisible( TFALSE );
				m_HUDParams.SetFlags( 0 );
				m_oCollectHud.Hide();
				m_oHint.Hide();
				m_oLifeHud.SetVisible( TFALSE );
				m_pPlayer->SelectLauncher( TFALSE );
				return;
			}

			m_fStingTime = 0.0f;
		}

		m_fStingTime += a_fDeltaTime;
	}

	UpdateHint();
	AInputHandler::GetSingleton()->SetRumblePosition( AInputHandler::GetSingleton()->GetMainController(), vPlayerPosition );

	// The camera moves to the hive
	if ( ( m_bWalkingToHive == TTRUE || m_bCollecting == TTRUE ) && m_bMoveCamera == TTRUE )
	{
		AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() );

		TVector4        vCamera  = pHelper->GetCamera()->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		const TVector4& rcTarget = s_avHiveCameraPositions[ m_iHive ];

		TFLOAT fDX = rcTarget.x - vCamera.x;
		TFLOAT fDZ = rcTarget.z - vCamera.z;

		if ( fDX * fDX + fDZ * fDZ <= 0.5f )
			m_bMoveCamera = TFALSE;

		vCamera.Lerp4( vCamera, rcTarget, a_fDeltaTime * 5.0f );
		pHelper->SetCameraPosition( vCamera );
		pHelper->SetForceUpdate( TFALSE );
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif
}

// $Barnyard: FUNCTION 0048cde0
AMiniGameState::S1* AHoneyCollectingGame::GetResults()
{
	if ( m_bWon == TTRUE )
		m_oResults.iResult = 0;
	else if ( m_bDefeated == TTRUE )
		m_oResults.iResult = -1;

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	PlayerInfo& rInfo = m_oResults.oPlayerList[ 0 ];
	rInfo.bUnk1       = TTRUE;
	rInfo.iUnk2       = m_iNumCombs;
	m_oResults.ComputeTotal( 0 );

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		if ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() == 0 )
			AHighScoreManager::AddScore( HIGHSCORE_TABLE, TFLOAT( rInfo.iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 0048c060
void AHoneyCollectingGame::OnStartHudFinished()
{
	m_HUDParams.SetFlags( HUDFLAGS_GAME );
	m_oCollectHud.Hide();

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		if ( m_apSwarms[ i ] )
			m_apSwarms[ i ]->SetState( ABeeSwarm::STATE_PATROL );
	}

	if ( m_bCollecting == TTRUE && m_aHives[ m_iHive ].iHoney == 1 && m_aHives[ m_iHive ].bRoyalJelly == TTRUE )
		m_oRoyalJellyText.Show();

	m_oHint.Show();

	if ( m_iNumStinging > 0 )
		m_oSwatBar.SetBarVisible( TTRUE );

	m_oCountHud.SetForceHide( TFALSE );

	if ( m_bHiding == TFALSE )
	{
		m_pPlayer->SetMovement( TTRUE );
		m_pPlayer->SetControlFlags( TTRUE );
	}

	m_oLifeHud.SetVisible( TTRUE );
	m_oAmmoBar.Show();
}

// $Barnyard: FUNCTION 0048c200
void AHoneyCollectingGame::AddJarHoney( TFLOAT a_fHeight )
{
	m_oCountHud.AddHoney( a_fHeight );
}

// $Barnyard: FUNCTION 0048dfc0
// $BarnyardWiiJP: FUNCTION 801b8354
void AHoneyCollectingGame::CreateSwarms()
{
	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		AUnitTransform oTransform( TVector4::VEC_ZERO, TVector4::VEC_POSX, 0, 0, -1, TFALSE );
		ABeeSwarm*     pSwarm = TSTATICCAST( ABeeSwarm, AUnitManager::GetSingleton()->SpawnUnit( TPS8( beeswarm ), &oTransform ) );
		m_apSwarms[ i ]       = pSwarm;
		pSwarm->AddFlags( AUnit::FLAGS_MILK_TARGET );

		// The swarm and its field of view on the minimap
		TVector4 vPosition  = *pSwarm->GetSteer()->GetPosition();
		TVector4 vDirection = *pSwarm->GetSteer()->GetDirection();
		TINT     iMapIcon   = m_oMap.AddIcon( vPosition, vDirection, AGUI2MiniMap::ICONTYPE_HIDE, AGUI2MiniMap::ICON_ENEMY, MAP_ICON_SIZE );

		vDirection.x = -vDirection.x;
		vDirection.y = -vDirection.y;
		vDirection.z = -vDirection.z;

		TVector4 vOffset;
		vOffset.Multiply( vDirection, 3.0f );

		TVector4 vFOVPosition( vPosition.x - vOffset.x, vPosition.y - vOffset.y, vPosition.z - vOffset.z, vPosition.w );
		TINT     iFOVMapIcon = m_oMap.AddIcon( vFOVPosition, vDirection, AGUI2MiniMap::ICONTYPE_HIDE, "unit", "unit", MAP_ICON_SIZE );

		pSwarm->Setup( m_pNetwork, TTRUE, &m_aPatrols[ i ].oSpline, iMapIcon, iFOVMapIcon );

		// The player walks through the bees
		TSTATICCAST( ACollisionObjectUnit, m_pPlayer->GetCollisionObject() )->AddIgnoredUnit( pSwarm );
		pSwarm->GetAnimatableObject()->GetModelInstance()->SetUnlit();
	}
}

// $Barnyard: FUNCTION 0048e260
void AHoneyCollectingGame::StartCollecting()
{
	m_bCollecting = TTRUE;

	Hive& rHive = m_aHives[ m_iHive ];

	// The bee of the hive flies away
	AModelInstance* pBeeModel = rHive.pBee->GetModelInstance();

	if ( pBeeModel && pBeeModel->IsUpdatingSkeleton() && rHive.pBee->IsAnimPlaying( TPS8( idle_lp ), 0.0f ) == TTRUE &&
	     m_aHives[ m_iHive ].pBee->GetAnimationTime( TPS8( idle_lp ) ) > 0.5f )
		m_aHives[ m_iHive ].pBee->PlayAnimation( TPS8( fly_away ) );

	m_pPlayer->SelectLauncher( TFALSE );
	m_oCollectHud.SetMouseInput( TFALSE );

	Hive& rCurrent = m_aHives[ m_iHive ];
	m_oCollectHud.SetHive( ( rCurrent.bRoyalJelly == TTRUE ) ? ROYAL_HIVE_HONEY : HIVE_HONEY, rCurrent.iHoney, &rCurrent.oSections );

	if ( m_aHives[ m_iHive ].pSwarm )
		m_aHives[ m_iHive ].iUnk1c = 1;

	if ( m_aHives[ m_iHive ].iHoney != 0 )
	{
		m_oCollectHud.SetVisible( TTRUE );
		TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() )->SetRestoreAnimationSet( TFALSE );

		ANamedAnimationSetRef pSet = m_pPlayer->GetAnimatableObject()->GetObjectType()->FindAnimationSet( TPS8( honey ) );
		m_pPlayer->GetAnimatableObject()->GetToshiAnimInterface().SetAnimationSet( pSet );
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( honey_start ) );
	}
	else
	{
		m_oCollectHud.SetVisible( TFALSE );
	}

	m_iSectionsAtStart = m_oCollectHud.GetNumFullSections();
	m_iCombsAtStart    = m_iNumCombs;
	m_pHoneyGlob->SetEnabled( TTRUE );

	if ( m_bWon == TTRUE )
		m_oLeaveText.Hide();
}

// $Barnyard: FUNCTION 0048c6f0
void AHoneyCollectingGame::StopCollecting( TBOOL a_bInterrupted )
{
	if ( !m_bCollecting )
		return;

	AGUI2::GetSingleton()->SetCursorVisible( TFALSE );
	AGUI2::GetMouseCursor().ClearRadialField();

	if ( m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( honey_start ), 0.0f ) == TTRUE ||
	     m_pPlayer->GetAnimatableObject()->IsAnimPlaying( TPS8( honey_end ), 0.0f ) == TTRUE )
		return;

	if ( a_bInterrupted == TTRUE )
	{
		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
	}
	else
	{
		// Nothing was collected: no honey glob on the hand
		if ( m_iNumCombs == m_iCombsAtStart && m_iSectionsAtStart == m_oCollectHud.GetNumFullSections() )
			m_pHoneyGlob->SetEnabled( TFALSE );

		m_pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( honey_end ) );
	}

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );
	pSteer->SetRestoreAnimationSet( TTRUE );
	pSteer->SetState( APlayerSteer::STATE_WALKING );

	// Step back from the hive
	TVector4 vSide = m_avHiveDirections[ m_iHive ];
	vSide.AsVector3().RotateY( TMath::HALF_PI );

	TVector4 vOffset;
	vOffset.Multiply( vSide, 1.0f );
	vOffset.Multiply( vOffset, 1.3f );
	m_vHivePosition.Add( m_avHivePositions[ m_iHive ], vOffset );
	m_vHivePosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( m_vHivePosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );

	m_bCollecting = TFALSE;
	m_HUDParams.SetFlags( HUDFLAGS_GAME );
	m_oCollectHud.Hide();

	// NOTE: retail doesn't check the class of the camera helper
	TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetCurrentCameraHelper() )->SetMouseInput( TTRUE );

	m_bLeavingHive = TTRUE;

	if ( m_iCollectCue != -1 )
	{
		ASoundManager::GetSingleton()->StopCue( m_iCollectCue );
		m_iCollectCue = -1;
	}

	if ( m_bWon == TTRUE )
		m_oLeaveText.Show();
}

// $Barnyard: FUNCTION 0048c910
void AHoneyCollectingGame::UpdateHint()
{
	// The interaction HUD has priority
	if ( AHudManager::GetSingleton()->FindElement<AInteractionHudElement>()->m_aLines[ 0 ].IsVisible() )
	{
		m_oHint.Hide();
		return;
	}

	if ( m_bCollecting == TTRUE )
	{
		if ( m_eHint != HINT_STOP )
		{
			m_oHint.Show();
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_STOP, HINT_STOP );
		}

		return;
	}

	if ( m_bHiding == TTRUE )
	{
		if ( m_eHint != HINT_UNHIDE )
		{
			m_oHint.Show();
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_UNHIDE, HINT_UNHIDE );
		}

		return;
	}

	if ( m_iNumStinging > 0 && m_eAction != ACTION_HIDE )
	{
		if ( m_eHint != HINT_SWAT )
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_SWAT, HINT_SWAT );

		m_oHint.Show();
		return;
	}

	if ( m_iNumAttacking > 0 && m_eAction != ACTION_HIDE )
	{
		if ( m_eHint != HINT_RUN )
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_RUN, HINT_RUN );

		m_oHint.Show();
		return;
	}

	if ( m_eAction == ACTION_COLLECT )
	{
		m_oHint.Show();

		// NOTE: retail compares with HINT_HIDE and sets HINT_COLLECT, so the text is formatted every frame
		if ( m_eHint != HINT_HIDE )
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_HONEY, HINT_COLLECT );

		return;
	}

	if ( m_eAction == ACTION_HIDE )
	{
		m_oHint.Show();

		if ( m_eHint != HINT_HIDE )
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_HIDE, HINT_HIDE );

		return;
	}

	if ( m_iNumAngry > 0 )
	{
		// NOTE: retail compares with 9 and sets HINT_ANGRY, so the text is formatted every frame
		if ( m_eHint != 9 )
		{
			m_oHint.Show();
			ShowHint( m_oHint, m_eHint, LOCALE_HINT_ANGRY, HINT_ANGRY );
		}

		return;
	}

	m_oHint.Hide();
	m_eHint = HINT_NONE;
}

// $Barnyard: FUNCTION 0048e630
TBOOL __stdcall AHoneyCollectingGame::OnMilkHit( AHoneyCollectingGame* a_pCaller, void* a_pOwner, TRef<AUnitProxy>* a_pUnit )
{
	ABeeSwarm* pSwarm = TSTATICCAST( ABeeSwarm, ( *a_pUnit )->GetUnit() );
	pSwarm->SetState( ABeeSwarm::STATE_DISPERSE_MILKED );
	pSwarm->SetAngry( TTRUE );

	return TTRUE;
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs. The player gets out of the stump, is moved in front
// of the first hive with honey and turns the stick around the comb until the jar is full, squirting
// milk at the attacking swarms and swatting the bees away when they sting. Then he is moved to the entrance of the woods and leaves (see OnUpdate).
void AHoneyCollectingGame::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fTimer = 0.0f;
	static TFLOAT s_fAngle = 0.0f;
	s_fTimer += a_fDeltaTime;

	TBOOL bStopEvents = TFALSE;

	// Swat the bees
	if ( m_iNumStinging > 0 && !m_bHiding )
	{
		if ( s_fTimer >= 0.15f )
		{
			s_fTimer = 0.0f;
			TINFO( "[honeyauto] swat (power %.2f, lives %d)\n", m_oSwatBar.m_fTargetValue, m_iLives );
			TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
			ProcessCommand( AInputCommand_Jump, &oEvent, bStopEvents );
		}

		return;
	}

	// Squirt milk at an attacking swarm (it disperses): face it every frame, squirt once facing it
	static TFLOAT s_fSquirtTimer = 0.0f;
	static TBOOL  s_abMilked[ NUM_SWARMS ];
	s_fSquirtTimer += a_fDeltaTime;

	for ( TINT i = 0; i < NUM_SWARMS; i++ )
	{
		TBOOL bMilked = m_apSwarms[ i ] && m_apSwarms[ i ]->GetState() == ABeeSwarm::STATE_DISPERSE_MILKED;

		if ( bMilked && !s_abMilked[ i ] )
			TINFO( "[honeyauto] swarm %d dispersed by the milk\n", i );

		s_abMilked[ i ] = bMilked;
	}

	if ( !m_bCollecting && !m_bHiding )
	{
		for ( TINT i = 0; i < NUM_SWARMS; i++ )
		{
			ABeeSwarm* pSwarm = m_apSwarms[ i ];

			if ( !pSwarm || ( pSwarm->GetState() != ABeeSwarm::STATE_ATTACK && pSwarm->GetState() != ABeeSwarm::STATE_ALERT ) )
				continue;

			TVector4 vDirection;
			vDirection.Substract( *pSwarm->GetSteer()->GetPosition(), *m_pPlayer->GetSteer()->GetPosition() );
			vDirection.y     = 0.0f;
			TFLOAT fDistance = vDirection.Magnitude();
			vDirection.Normalise();

			TFLOAT fFacing = TVector4::DotProduct3( vDirection, *m_pPlayer->GetSteer()->GetDirection() );
			m_pPlayer->GetSteer()->SetDirection( vDirection );

			AProjectileLauncher* pLauncher = m_pPlayer->GetActiveLauncher();

			if ( pLauncher && fFacing > 0.95f && fDistance < 8.0f && s_fSquirtTimer >= 0.3f )
			{
				s_fSquirtTimer = 0.0f;
				pLauncher->Fire();
				TINFO( "[honeyauto] squirt at swarm %d (state %d, distance %.1f)\n", i, pSwarm->GetState(), fDistance );
			}

			break;
		}
	}

	if ( m_bCollecting )
	{
		// Turn the stick around the comb
		s_fAngle += a_fDeltaTime * TMath::TWO_PI;

		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_MOVED, TFLOAT( sin( s_fAngle ) ), TFLOAT( -cos( s_fAngle ) ) );
		ProcessCommand( AInputCommand_Move, &oEvent, bStopEvents );
		return;
	}

	if ( s_fTimer < 1.0f || m_bWalkingToHive || m_bLeavingHive || m_bLeavingHide )
		return;

	s_fTimer = 0.0f;

	if ( m_bHiding )
	{
		TINFO( "[honeyauto] leave the stump\n" );
		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
		return;
	}

	APlayerSteer* pSteer = TSTATICCAST( APlayerSteer, m_pPlayer->GetSteer() );

	if ( m_bWon )
	{
		if ( m_bAtExit )
		{
			s_bDevLeave = TTRUE;
			return;
		}

		TINT iArea = m_pNetwork->FindAreaIndex( TPS8( entrance ) );

		if ( iArea != -1 )
		{
			TVector4 vPosition;
			m_pNetwork->GetArea( iArea )->GetPosition( vPosition );
			pSteer->SetPosition( vPosition );
			TINFO( "[honeyauto] go to the entrance (combs %d, royal jelly %d)\n", m_iNumCombs, m_bRoyalJelly );
		}

		return;
	}

	if ( m_eAction == ACTION_COLLECT )
	{
		TINFO( "[honeyauto] collect hive %d (honey %d)\n", m_iHive, m_aHives[ m_iHive ].iHoney );
		TInputInterface::InputEvent oEvent( TNULL, 0, TInputInterface::EVENT_TYPE_GONE_DOWN );
		ProcessCommand( AInputCommand_Interact, &oEvent, bStopEvents );
		return;
	}

	if ( !m_oCollectHud.IsFull() )
		return;

	// Go in front of the first hive with honey
	for ( TINT i = 0; i < m_iNumHives; i++ )
	{
		if ( m_aHives[ i ].iHoney <= 0 )
			continue;

		TVector4 vSide = m_avHiveDirections[ i ];
		vSide.AsVector3().RotateY( TMath::HALF_PI );

		TVector4 vPosition( m_avHivePositions[ i ].x + vSide.x, m_avHivePositions[ i ].y, m_avHivePositions[ i ].z + vSide.z, 1.0f );
		vPosition.y = ATerrainInterface::GetSingleton()->GetGroundHeight( vPosition, TCollisionCommon::TOSHICGROUPFLAG_NONE );

		TVector4 vDirection( -vSide.x, 0.0f, -vSide.z, 1.0f );
		vDirection.Normalise();

		pSteer->SetPosition( vPosition );
		pSteer->SetDirection( vDirection );
		TINFO( "[honeyauto] go to hive %d\n", i );
		return;
	}
}

#endif
