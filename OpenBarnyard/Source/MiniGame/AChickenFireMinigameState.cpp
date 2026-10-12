#include "pch.h"
#include "AChickenFireMinigameState.h"
#include "AMiniGameManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AAnimationBreakpointCallback.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "FX/AAniEffectManager.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleSystem.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GUI/AFadeManager.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AHighScoreManager.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/ATreeManager.h"
#include "HUD/AHudManager.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Render/AShadowManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/ASoundManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitTransform.h"
#include "Units/ASSeqController.h"
#include "Units/AAnimatedUnit.h"
#include "UnitMovement/Steering/AStationarySteer.h"

#include <File/TTRB.h>
#include <GUI/T2GUIFontManager.h>
#include <Input/TInputDeviceMouse.h>
#include <Math/TMathInline.h>
#include <Plugins/PPropertyParser/PBProperties.h>
#include <Render/TRenderInterface.h>
#include <Render/TSkeleton.h>
#include <Toshi/T2String8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( peckFireProgram );
TPSTRING8_DECLARE( cfp_americanf );
TPSTRING8_DECLARE( cfp_americanm );
TPSTRING8_DECLARE( cfp_freddy );
TPSTRING8_DECLARE( cfp_goggles );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( ReleaseBP );
TPSTRING8_DECLARE( flightloop );
TPSTRING8_DECLARE( americanm_pullback );
TPSTRING8_DECLARE( launchPullBack );
TPSTRING8_DECLARE( peckattachpoint );
TPSTRING8_DECLARE( BashInc );
TPSTRING8_DECLARE( BashRed );
TPSTRING8_DECLARE( MinBash );
TPSTRING8_DECLARE( UnderPowerBash );
TPSTRING8_DECLARE( OverPowerBash );
TPSTRING8_DECLARE( IdealPowerBash );
TPSTRING8_DECLARE( UnderPowerTimeout );
TPSTRING8_DECLARE( OverPowerTimeout );
TPSTRING8_DECLARE( LaunchTimeout );
TPSTRING8_DECLARE( BoostTimeout );
TPSTRING8_DECLARE( PeckBoostAccel );

// NOTE: GetClass 004797d0, CreateObject 0047d6e0, CreateObjectInPlace 0047d740 (the constructor is inlined in both)
TDEFINE_CLASS( AChickenFireMinigameState );

// Sounds
static constexpr ASoundId SOUND_AMBIENCE = 135; // played at two positions while the minigame runs
static constexpr ASoundId SOUND_END_TURN = 160;

// High score tables (AHighScoreManager::AddScore) by variant
static constexpr TINT HIGHSCORE_TABLE_1 = 55; // variants 0 and 3
static constexpr TINT HIGHSCORE_TABLE_2 = 56; // variant 1
static constexpr TINT HIGHSCORE_TABLE_3 = 57; // variant 2

// Locale strings
static constexpr TINT LOCALE_RESULTS_1038 = 1038;
static constexpr TINT LOCALE_RESULTS_1039 = 1039;
static constexpr TINT LOCALE_RESULTS_606  = 606;
static constexpr TINT LOCALE_INSTRUCTIONS = 1045;

static constexpr TFLOAT DEFAULT_FOV = 1.0471976f; // 60 degrees

// $Barnyard: FUNCTION 0047b290
AChickenFireMinigameState::AChickenFireMinigameState()
    : m_vTargetPoint( TVector4::VEC_ZERO )
    , m_vPeckPos( TVector4::VEC_ZERO )
    , m_vPeckPrevPos( TVector4::VEC_ZERO )
    , m_vPeckDir( TVector4::VEC_NEGZ )
    , m_vUnk_BEC( TVector4::VEC_ZERO )
    , m_fPitchRate( 0.0f )
    , m_fYawRate( 0.0f )
    , m_fBankAngle( 0.0f )
    , m_fPower( 0.0f )
{
	m_iCue_C3C = -1;
	m_iCue_C40 = -1;
	m_iCue_C44 = -1;
	m_iCue_C48 = -1;
	m_iCue_C4C = -1;
	m_iCue_C50 = -1;
	m_iCue_C54 = -1;
	m_iCue_C58 = -1;
	m_iCue_C5C = -1;
	m_iCue_C60 = -1;

	m_fPlayerPullbackDuration = -1.0f;
	m_fPeckPullbackDuration   = -1.0f;
	m_fPlayerPullbackTime     = 0.0f;
	m_fPeckPullbackTime       = 0.0f;
	m_fStallTime              = 0.0f;
	m_fUnk_1FD4               = 0.0f;
	m_fUnk_1FD8               = 0.0f;
	m_pReleaseEmitter         = TNULL;
	m_bReleased               = TFALSE;
	m_bBoosted                = TFALSE;
	m_fTimer                  = 0.0f;
	m_fEndTimer               = 0.0f;
	m_fShakeRadius            = 0.0f;
	m_fFlightStep             = 0.0f;
	m_fTextTimer              = 0.0f;
	m_iNumBoosts              = 0;
	m_fUnk_2044               = 0.0f;
	m_pFlightFeathersL        = TNULL;
	m_pFlightFeathersR        = TNULL;
	m_bOrbitCamera            = TFALSE;
	m_fWrongWayTime           = 0;
	m_iVariant                = 2;
	m_iNumPlayers2            = 0;
	m_fFlightTime             = 0.0f;
	m_iTimeBonus              = 0;

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
		m_apUnk_287C[ i ] = TMalloc( 32 );

	m_iUnk_30F4 = 0;
	m_iUnk_30F8 = 0;
}

// $Barnyard: FUNCTION 0047d780
// $Barnyard: FUNCTION 0047b5b0
AChickenFireMinigameState::~AChickenFireMinigameState()
{
	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
		TFree( m_apUnk_287C[ i ] );
}

// $Barnyard: FUNCTION 00479690
void AChickenFireMinigameState::Reflect( TVector4& a_rOut, const TVector4& a_rcVec, const TVector4& a_rcNormal )
{
	TVector4 vInverted( -a_rcVec.x, -a_rcVec.y, -a_rcVec.z, a_rcVec.w );

	TVector4 vProjected;
	vProjected.Multiply( a_rcNormal, TVector4::DotProduct3( vInverted, a_rcNormal ) );

	TVector4 vDoubled;
	vDoubled.Multiply( vProjected, 2.0f );

	a_rOut.x = vDoubled.x - vInverted.x;
	a_rOut.y = vDoubled.y - vInverted.y;
	a_rOut.z = vDoubled.z - vInverted.z;
	a_rOut.w = 1.0f;
}

// $Barnyard: FUNCTION 004797e0
void AChickenFireMinigameState::AddRandomOffset( TVector4& a_rVec, TFLOAT a_fRadius )
{
	// A random angle in [0, 2PI)
	TFLOAT fAngle = TFLOAT( m_oRandom.RandRaw() >> 1 ) * 2.9258362e-9f;

	TVector4 vOffset = TVector4::VEC_POSY;
	vOffset.AsVector3().RotateZ( fAngle );
	vOffset.Multiply( a_fRadius );

	a_rVec.x += vOffset.x;
	a_rVec.y += vOffset.y;
	a_rVec.z += vOffset.z;
}

// $Barnyard: FUNCTION 00479890
TBOOL AChickenFireMinigameState::LoadData( const TCHAR* a_szFileName )
{
	TTRB oTRB;

	if ( oTRB.Load( a_szFileName ) != TTRB::ERROR_OK )
		return TFALSE;

	const PBProperties* pMain = TSTATICCAST( const PBProperties, oTRB.GetSymbolAddress( "Main" ) );
	oTRB.DeleteSymbolTable();

	const PBProperties* pBashParams = pMain->GetProperty( "BashParams" )->GetProperties();
	pBashParams->GetOptionalFloatOrIntValue( m_fBashInc, TPS8( BashInc ) );
	pBashParams->GetOptionalFloatOrIntValue( m_fBashRed, TPS8( BashRed ) );
	pBashParams->GetOptionalFloatOrIntValue( m_fMinBash, TPS8( MinBash ) );
	pBashParams->GetOptionalFloatOrIntValue( m_fUnderPowerBash, TPS8( UnderPowerBash ) );
	TMath::Clip( m_fUnderPowerBash, 0.0f, 1.0f );
	pBashParams->GetOptionalFloatOrIntValue( m_fOverPowerBash, TPS8( OverPowerBash ) );
	TMath::Clip( m_fOverPowerBash, 0.0f, 1.0f );
	pBashParams->GetOptionalFloatOrIntValue( m_fIdealPowerBash, TPS8( IdealPowerBash ) );
	TMath::Clip( m_fIdealPowerBash, 0.0f, 1.0f );

	const PBProperties* pTimeOuts = pMain->GetProperty( "TimeOuts" )->GetProperties();
	pTimeOuts->GetOptionalFloatOrIntValue( m_fUnderPowerTimeout, TPS8( UnderPowerTimeout ) );
	pTimeOuts->GetOptionalFloatOrIntValue( m_fOverPowerTimeout, TPS8( OverPowerTimeout ) );
	pTimeOuts->GetOptionalFloatOrIntValue( m_fLaunchTimeout, TPS8( LaunchTimeout ) );
	pTimeOuts->GetOptionalFloatOrIntValue( m_fBoostTimeout, TPS8( BoostTimeout ) );

	const PBProperties* pMisc = pMain->GetProperty( "Misc" )->GetProperties();
	pMisc->GetOptionalFloatOrIntValue( m_fPeckBoostAccel, TPS8( PeckBoostAccel ) );

	return TTRUE;
}

// $Barnyard: FUNCTION 00479a80
TINT AChickenFireMinigameState::GetSuccessIndex( const TString8& a_rcName )
{
	if ( a_rcName.Compare( "success" ) == 0 ) return 5;
	if ( a_rcName.Compare( "success_2" ) == 0 ) return 6;
	if ( a_rcName.Compare( "success_3" ) == 0 ) return 7;
	if ( a_rcName.Compare( "success_4" ) == 0 ) return 8;
	if ( a_rcName.Compare( "success_5" ) == 0 ) return 9;
	if ( a_rcName.Compare( "success_6" ) == 0 ) return 10;
	if ( a_rcName.Compare( "success_7" ) == 0 ) return 11;
	if ( a_rcName.Compare( "success_8" ) == 0 ) return 12;
	if ( a_rcName.Compare( "success_9" ) == 0 ) return 13;

	return ( a_rcName.Compare( "success_10" ) == 0 ) ? 14 : 0;
}

// $Barnyard: FUNCTION 00479ba0
void AChickenFireMinigameState::SetText( const TWCHAR* a_wszText )
{
	if ( a_wszText )
	{
		m_oTextBox.Show();
		m_oTextBox.SetText( a_wszText );
	}
	else
	{
		m_oTextBox.Hide();
	}
}

// $Barnyard: FUNCTION 00479be0
void AChickenFireMinigameState::OnStartHudFinished()
{
	// NOTE: AMiniGameState::OnStartHudFinished isn't called
	m_oBashHud.SetForceHide( TFALSE );
	m_oFeathers.SetVisible( TTRUE );

	if ( m_iVariant == 4 )
		m_oPickUpHudElement.SetForceHide( TFALSE );

	m_oPortrait.Show();
}

// $Barnyard: FUNCTION 00479c40
TBOOL AChickenFireMinigameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	if ( ( FindChildState( &TGetClass( AAnticGameOverState ) ) && a_eInputCommand == AInputCommand_Quit ) || a_eInputCommand == AInputCommand_Start )
		return TTRUE;

	if ( m_eState == STATE_FALLING || m_eState == STATE_SPLASHED || m_eState == STATE_RESULTS )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	PlayerInput& rInput = m_aInputs[ APlayerManager::GetSingleton()->GetCurrentPlayerIterator() ];

	switch ( a_eInputCommand )
	{
		case AInputCommand_Select:
			rInput.bBash = TTRUE;
			return TTRUE;
		case AInputCommand_Move:
			if ( a_pInputEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				rInput.fAimX += a_pInputEvent->GetMagnitudeFloat( 0 ) * 0.2f;
				rInput.fAimY -= a_pInputEvent->GetMagnitudeFloat( 1 ) * 0.2f;
			}
			else
			{
				rInput.fAimX += a_pInputEvent->GetMagnitudeFloat( 0 );
				rInput.fAimY -= a_pInputEvent->GetMagnitudeFloat( 1 );
			}

			return TTRUE;
		case AInputCommand_Up:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			rInput.fAimY += 1.0f;
			return TTRUE;
		case AInputCommand_Down:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			rInput.fAimY -= 1.0f;
			return TTRUE;
		case AInputCommand_Left:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			rInput.fAimX -= 1.0f;
			return TTRUE;
		case AInputCommand_Right:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			rInput.fAimX += 1.0f;
			return TTRUE;
		case AInputCommand_Jump:
			rInput.bCmd57 = TTRUE;
			return TTRUE;
		default:
			return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
	}
}

// $Barnyard: FUNCTION 00479e90
void AChickenFireMinigameState::OnDeactivate()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TTRUE );
	m_oPortrait.Hide();
	m_oFeathers.SetVisible( TFALSE );

	if ( ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) == TTRUE )
		ASoundManager::GetSingleton()->PauseCue( m_iCue_C58, TTRUE );

	AMiniGameState::OnDeactivate();
}

// $Barnyard: FUNCTION 00479ef0
void AChickenFireMinigameState::GetLocatorPos( TVector4& a_rOutPos, const TCHAR* a_szLocator )
{
	a_rOutPos = ATerrainInterface::GetSingleton()->GetLocatorPos( a_szLocator );
}

// $Barnyard: FUNCTION 00479f30
void AChickenFireMinigameState::SetPeckTransform( TVector4 a_vPosition, TVector4 a_vDirection )
{
	GetPeckUnit()->GetModelInstance()->SetTransformUnusedUp( a_vPosition, a_vDirection, TVector4::VEC_NEGY );
}

// $Barnyard: FUNCTION 0047a140
void AChickenFireMinigameState::DestroyObjects()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	while ( m_vecObjects1.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects1[ m_vecObjects1.Size() - 1 ];
		m_vecObjects1.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}

	while ( m_vecObjects2.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects2[ m_vecObjects2.Size() - 1 ];
		m_vecObjects2.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}

	while ( m_vecObjects4.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects4[ m_vecObjects4.Size() - 1 ];
		m_vecObjects4.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}
}

// $Barnyard: FUNCTION 0047a220
void AChickenFireMinigameState::DestroyObjects3()
{
	AAnimatableObjectManager* pManager = AAnimatableObjectManager::GetSingleton();

	while ( m_vecObjects3.Size() != 0 )
	{
		AAnimatableObject* pObject = m_vecObjects3[ m_vecObjects3.Size() - 1 ];
		m_vecObjects3.PopBack();
		pManager->DestroyAnimatableObject( pObject );
	}
}

// $Barnyard: FUNCTION 0047a580
TBOOL AChickenFireMinigameState::OnReleaseBreakpoint( AChickenFireMinigameState* a_pThis, void* a_pOwner, void* a_pData )
{
	a_pThis->m_bReleased = TTRUE;
	return TTRUE;
}

// $Barnyard: FUNCTION 0047b1d0
void AChickenFireMinigameState::OnStart()
{
	// NOTE: the original passes a fourth argument (TTRUE) that AFadeManager::StartFade doesn't have
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
	AMiniGameState::OnStart();
}

// $Barnyard: FUNCTION 0047b6d0
void AChickenFireMinigameState::OnRemoval()
{
	ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( &m_oWindmillHelper );
	m_oWindmillHelper.OnDestroy();
	AWindmillHelper::ms_fWindDirection = m_fSavedWindDirection;

	ATreeManager::GetSingleton()->SetCollisionEnabled( m_bSavedTreeCollision );

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( DEFAULT_FOV );
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();
	pAnimObjManager->DestroyAnimatableObject( m_pPlayerModel );
	pAnimObjManager->DestroyAnimatableObject( m_pFreddy );

	if ( m_iVariant == 4 )
		DestroyCharityActors();

	m_oBashHud.Detach();
	m_oFeathers.RemoveSelf();
	AAniEffectManager::GetSingleton()->DeleteAllEffects();

	AUnitManager::GetSingleton()->DestroyUnit( GetPeckUnit() );
	DestroyObjects();
	DestroyObjects3();

	AGameLoader::DestroyLevel();
	AUnitManager::GetSingleton()->RemoveAllUnits();
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->FlushDyingResources();

	AMiniGameState::OnRemoval();

	ARenderer::GetSingleton()->SetFar( m_fSavedFarDistance );
	ATreeManager::GetSingleton()->SetDistances( m_fSavedTreeLODDistance, m_fSavedTreeRenderDistance, m_fSavedTreeBlendingDistance );
	AGameTimeFXManager::GetSingleton()->ClearForcedFX();

	StopSounds( TFALSE );

	if ( m_pFlightFeathersL )
	{
		m_pFlightFeathersL->GetInstance()->Kill();
		m_pFlightFeathersL = TNULL;
	}

	if ( m_pFlightFeathersR )
	{
		m_pFlightFeathersR->GetInstance()->Kill();
		m_pFlightFeathersR = TNULL;
	}

	AInputHandler* pInputHandler = AInputHandler::GetSingleton();
	pInputHandler->StopAllRumbles();
	// DOES_NOTHING (00628f40) is called on AInputHandler with TFALSE
	pInputHandler->StopRumble( pInputHandler->GetMainController() );

	APositionalSoundSet::GetSingleton()->Unload();

	for ( TINT i = 0; i < NUM_BUTTONS; i++ )
		m_aButtons[ i ].RemoveSelf();

	m_oTextBox.RemoveSelf();
	m_oFeathers.Destroy();

	AMusicManager::GetSingleton()->ClearForcedSound( TFALSE );
	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( ACamera::sm_fCurrentFOV );
	AParticleEffectManager::GetSingleton()->KillAll();
	// DOES_NOTHING (00628f40) is called on AInputHandler with TFALSE
}

// $Barnyard: FUNCTION 0047d390
void AChickenFireMinigameState::EndTurn()
{
	StopSounds( TTRUE );
	m_eState = STATE_FALLING;

	if ( m_pFlightFeathersL )
	{
		m_pFlightFeathersL->GetInstance()->Stop();
		m_pFlightFeathersL = TNULL;
	}

	if ( m_pFlightFeathersR )
	{
		m_pFlightFeathersR->GetInstance()->Stop();
		m_pFlightFeathersR = TNULL;
	}

	m_iCue_C60 = ASoundManager::GetSingleton()->PlayCue( SOUND_END_TURN );
}

// $Barnyard: FUNCTION 0047d400
AMiniGameState::S1* AChickenFireMinigameState::GetResults()
{
	// NOTE: the result isn't used
	AMiniGameManager::GetSingleton()->GetNextMiniGame();

	m_oResults.bWon = m_bWon;

	if ( m_oResults.bWon == TTRUE && APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetHumanPlayerCount() == 1 )
		m_oResults.iResult = 0;

	TINT        iPlayer = APlayerManager::GetSingleton()->GetCurrentPlayerIterator();
	PlayerInfo& rInfo   = m_oResults.oPlayerList[ iPlayer ];

	while ( rInfo.vecRows.Size() != 0 )
	{
		rInfo.vecRows[ rInfo.vecRows.Size() - 1 ] = ResultRow();
		rInfo.vecRows.PopBack();
	}

	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_RESULTS_1038 ), m_iRingScore } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_RESULTS_1039 ), 10 * ( m_iLandingScore + 1 ) } );
	rInfo.vecRows.PushBack( ResultRow{ ALocaleManager::Interface()->GetString( LOCALE_RESULTS_606 ), m_iTimeBonus } );
	m_oResults.ComputeTotal( iPlayer );

	m_oResults.iMiniGameIndex = AMiniGameManager::GetSingleton()->GetNextMiniGameIndex();

	for ( TINT i = 0; i < MAX_PLAYERS; i++ )
		m_oResults.oPlayerList[ i ].bUnk1 = TFALSE;

	if ( m_iNumPlayers2 <= 1 )
	{
		m_oResults.oPlayerList[ iPlayer ].bUnk1 = TTRUE;
	}
	else
	{
		for ( TINT i = 0; i < m_iNumPlayers2; i++ )
			m_oResults.oPlayerList[ i ].bUnk1 = TTRUE;
	}

	if ( m_bSubmitHighScore )
	{
		m_bSubmitHighScore = TFALSE;

		TINT iTable;

		switch ( AMiniGameManager::GetSingleton()->GetMiniGameVariant() )
		{
			case 0:
			case 3:
				iTable = HIGHSCORE_TABLE_1;
				break;
			case 1:
				iTable = HIGHSCORE_TABLE_2;
				break;
			case 2:
				iTable = HIGHSCORE_TABLE_3;
				break;
			default:
				return &m_oResults;
		}

		AHighScoreManager::AddScore( iTable, TFLOAT( m_oResults.oPlayerList[ 0 ].iTotal ), m_oResults.bWon, AHighScoreManager::PLAYER_1, TFALSE );
	}

	return &m_oResults;
}

// $Barnyard: FUNCTION 0047d5d0
TINT AChickenFireMinigameState::CompareObjectDepth( AAnimatableObject* a_pA, AAnimatableObject* a_pB )
{
	TVector4 vPosA;
	a_pA->GetModelInstance()->GetPosition( vPosA );

	TVector4 vPosB;
	a_pB->GetModelInstance()->GetPosition( vPosB );

	if ( vPosA.z > vPosB.z )
		return -1;

	return vPosA.z != vPosB.z;
}

// $Barnyard: FUNCTION 0047d7a0
void AChickenFireMinigameState::AttachPeckToPlayer()
{
	TINT iBone = m_pPlayerModel->GetModelInstance()->GetBoneID( TPS8( peckattachpoint ) );

	TMatrix44 matBone;
	m_pPlayerModel->GetModelInstance()->GetBoneTransform( iBone, matBone );

	m_vPeckPos         = matBone.GetTranslation();
	TVector4 vPosition = m_vPeckPos;

	GetPeckUnit()->GetModelInstance()->SetTransform( vPosition, TVector4::VEC_NEGZ );
}

// $Barnyard: FUNCTION 004839d0
void AChickenFireMinigameState::OnInsertion()
{
	m_bShowPlayerFloats = TFALSE;

	APlayerManager*           pPlayerManager  = APlayerManager::GetSingleton();
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	m_iNumPlayers = pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount();

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		// NOTE: the original copies the static floats at 007d3018 (zero)
		m_aInputs[ i ].fAimX  = 0.0f;
		m_aInputs[ i ].fAimY  = 0.0f;
		m_aInputs[ i ].iUnk08 = 0;
		m_aInputs[ i ].iUnk0C = 0;
		m_aInputs[ i ].bBash  = TFALSE;
		m_aInputs[ i ].bUnk12 = TFALSE;
		m_aInputs[ i ].bCmd57 = TFALSE;
		m_aInputs[ i ].bUnk13 = TFALSE;
	}

	// Load the Peck range with the tree collision
	ATreeManager* pTreeManager     = ATreeManager::GetSingleton();
	TBOOL         bCreateCollision = pTreeManager->GetCreateCollision();
	m_bSavedTreeCollision          = pTreeManager->IsCollisionEnabled();
	pTreeManager->SetCreateCollision( TTRUE );
	pTreeManager->SetCollisionEnabled( TTRUE );
	AGameLoader::Terrain_Set( AGameLoader::Terrain_PeckRange, TFALSE, TTRUE, 0, 0, 0, 0 );
	AGameLoader::Terrain_Load();
	pTreeManager->SetCreateCollision( bCreateCollision );

	ACameraManager::GetSingleton()->DetachCameraHelpers();

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	m_vPeckPos = TVector4::VEC_NEGZ;
	m_vPeckPos = pTerrain->GetLocatorPos( "player_start" );

	// Peck
	AUnitTransform oTransform( m_vPeckPos, TVector4::VEC_NEGZ, 0, 0, 0, TTRUE );
	AAnimatedUnit* pPeck = TSTATICCAST( AAnimatedUnit, AUnitManager::GetSingleton()->SpawnUnit( TPS8( peckFireProgram ), &oTransform ) );
	AShadowManager::GetSingleton()->RemoveShadow( pPeck );
	m_oPeckProxy = pPeck->GetSelfProxy();
	pPeck->ChangeSteer( &TGetClass( AStationarySteer ) );

	ASSeqController::GetSingleton()->LoadScript( "cfp_overpwr_failcase.tso", m_vPeckPos );
	ASSeqController::GetSingleton()->LoadScript( "cfp_underpwr_failcase.tso", m_vPeckPos );

	m_vPeckPos.z += 1.25f;

	// The player
	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
	const TPString8&       rcPlayerType    = ( pProfileManager->GetMainGender() != APlayerProfile::GENDER_MALE ) ? TPS8( cfp_americanf ) : TPS8( cfp_americanm );

	m_pPlayerModel = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( rcPlayerType ), TNULL, 0 );
	m_pPlayerModel->ApplyMaterial( pProfileManager->GetMainCostume(), TTRUE );
	m_pPlayerModel->GetModelInstance()->SetTransform( m_vPeckPos, TVector4::VEC_NEGZ );
	m_pPlayerModel->SetSkeletonUpdating( TTRUE, TTRUE );
	m_pPlayerModel->SetVisible( TTRUE );
	m_pPlayerModel->PlayAnimation( TPS8( idle_lp ) );
	m_pReleaseEmitter = m_pPlayerModel->GetAnimationEmitter( TPS8( ReleaseBP ) );

	// Freddy
	m_pFreddy = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( cfp_freddy ) ), TNULL, 0 );
	m_pFreddy->GetModelInstance()->SetTransform( m_vPeckPos, TVector4::VEC_NEGZ );
	m_pFreddy->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pFreddy->SetVisible( TTRUE );

	// Peck's goggles
	AAnimatableObject* pGoggles = pAnimObjManager->CreateAnimatableObject( pAnimObjManager->FindType( TPS8( cfp_goggles ) ), TNULL, 0 );

	AAnimatableObject* pPeckObject = pPeck->GetAnimatableObject();
	pPeckObject->DestroyAnimControllers();
	pPeckObject->PlayAnimation( TPS8( flightloop ) );
	TINT iHeadBone = pPeckObject->GetModelInstance()->GetSkeleton()->GetBoneID( "CG_Head" );

	pAnimObjManager->AttachObject( pGoggles, pPeckObject, -1, TINT8( iHeadBone ), TVector4( 0.0f, -0.03f, 0.0f, 1.0f ), TQuaternion::IDENTITY, 1.0f );

	// HUD
	AHudManager::GetSingleton()->AddElement( &m_oBashHud, TFALSE, TTRUE, TFALSE );
	m_oBashHud.SetForceHide( TTRUE );

	m_oPortrait.Create( pProfileManager->GetMainProfile()->GetHideTextureName( -1, APlayerProfile::GENDER_NONE ), AGUI2Portrait::FLAGS_LEFT, TNULL );
	m_oPortrait.SetScore( 0 );
	m_oPortrait.SetAttachment( T2GUIATTACHMENT_TOPLEFT, T2GUIATTACHMENT_TOPLEFT );
	m_oPortrait.Hide();
	m_GUIElement.AddChildTail( m_oPortrait );

	m_oFeathers.Create( 5 );
	m_oFeathers.SetAttachment( T2GUIATTACHMENT_BOTTOMRIGHT, T2GUIATTACHMENT_BOTTOMRIGHT );
	m_oFeathers.SetVisible( TFALSE );
	m_oFeathers.SetNumFeathers( 0 );
	m_GUIElement.AddChildTail( m_oFeathers );

	// Targets
	m_vTargetPoint.Set( 0.0f, -2.5f, -198.0f, 1.0f );
	m_vTargetPoint = pTerrain->GetLocatorPos( "target_point00" );
	m_vPeckPos.y -= 3.5f;

	TVector4 vTarget1 = pTerrain->GetLocatorPos( "target_point01" );
	m_fTargetDist1    = TVector4::Distance( vTarget1, m_vTargetPoint );

	TVector4 vTarget2 = pTerrain->GetLocatorPos( "target_point02" );
	m_fTargetDist2    = TVector4::Distance( vTarget1, vTarget2 );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord18" );

	m_iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant() + 1;

	if ( m_iVariant == 4 )
	{
		// Everet and Daisy watch from the side
		CreateCharityActors( TFALSE );

		TVector4 vEveretPos( 4.52f, -4.5f, 173.0f, 1.0f );
		vEveretPos.y = pTerrain->GetGroundHeight( vEveretPos, 0 );

		TVector4 vEveretDir( m_vPeckPos.x - vEveretPos.x, 0.0f, m_vPeckPos.z - vEveretPos.z, m_vPeckPos.w );
		vEveretDir.Normalise();

		TVector4 vDaisyPos( 4.52f, -4.5f, 171.0f, 1.0f );
		vDaisyPos.y = pTerrain->GetGroundHeight( vDaisyPos, 0 );

		TVector4 vDaisyDir( m_vPeckPos.x - vDaisyPos.x, 0.0f, m_vPeckPos.z - vDaisyPos.z, m_vPeckPos.w );
		vDaisyDir.Normalise();

		PlaceCharityActors( vEveretPos, vDaisyPos, vEveretDir, vDaisyDir );
	}

	CreateRings();
	CreateScorecards();

	pTreeManager->GetDistances( m_fSavedTreeLODDistance, m_fSavedTreeRenderDistance, m_fSavedTreeBlendingDistance );
	pTreeManager->SetDistances( 25.0f, 100.0f, 65.0f );

	m_fSavedFarDistance = ARenderer::GetSingleton()->SetFar( 400.0f );

	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( "PeckRange" ) );

	LoadData( "Data/MinigameData/ChickenLaunch/cfp_data.trb" );

	// Bash dial
	// NOTE: the original uses a shared setter of +0x24 (00622df0)
	m_oBashHud.m_fUnk24 = m_fMinBash;

	TFLOAT fUnderPower = m_fUnderPowerBash;
	TMath::Clip( fUnderPower, 0.0f, 1.0f );
	m_oBashHud.m_fUnk18 = fUnderPower;

	TFLOAT fOverPower = m_fOverPowerBash;
	TMath::Clip( fOverPower, 0.0f, 1.0f );
	m_oBashHud.m_fUnk1C = fOverPower;

	m_oBashHud.SetUnk20( m_fIdealPowerBash );

	m_oTextBox.Hide();

	m_iCue_C50 = ASoundManager::GetSingleton()->PlayCueAtPositionEx( SOUND_AMBIENCE, TVector4( 4.0f, -32.0f, -156.0f, 1.0f ), 0.3f, TFALSE, 0.0f, -1 );
	m_iCue_C54 = ASoundManager::GetSingleton()->PlayCueAtPositionEx( SOUND_AMBIENCE, TVector4( -6.0f, -17.0f, -356.0f, 1.0f ), 0.3f, TFALSE, 0.0f, -1 );

	// Peck's bones and the effects
	m_pPeckModelInstance    = pPeck->GetAnimatableObject()->GetModelInstance();
	m_iPeckLeftForearmBone  = m_pPeckModelInstance->GetSkeletonInstance()->GetSkeleton()->GetBoneID( "cg_l_forearm" );
	m_iPeckRightForearmBone = m_pPeckModelInstance->GetSkeletonInstance()->GetSkeleton()->GetBoneID( "cg_r_forearm" );

	m_iFeatherFlightEffect   = AParticleEffectManager::FindEffect( "PeckFeth_flight" );
	m_iWaterSplashAEffect    = AParticleEffectManager::FindEffect( "watersplash2a" );
	m_iWaterSplashBEffect    = AParticleEffectManager::FindEffect( "watersplash2b" );
	m_iWaterRippleEffect     = AParticleEffectManager::FindEffect( "waterripple" );
	m_iFeatherExplodeEffect  = AParticleEffectManager::FindEffect( "PeckFeth_explode" );
	m_iFeatherExplodeEffect2 = AParticleEffectManager::FindEffect( "PeckFeth_explode" );
	m_iDustPuffEffect        = AParticleEffectManager::FindEffect( "dustpuff" );
	m_iLeavesFallEffect      = AParticleEffectManager::FindEffect( "leavesfall" );

	m_iNumPlayers2 = pPlayerManager->GetCurrentPlayerSet()->GetHumanPlayerCount();

	// DOES_NOTHING (0054df00) is called with the main controller and &m_vPeckPos

	APositionalSoundSet::GetSingleton()->Load( "Data/Sound/launch.trb" );

	m_oTextBox.Create( pFont, m_GUIElement.GetWidth() * 0.8f );
	m_oTextBox.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	m_GUIElement.AddChildTail( m_oTextBox );

	ALocale::FormatInputTags( m_wszInstructions, TARRAYSIZE( m_wszInstructions ), ALocaleManager::Interface()->GetString( LOCALE_INSTRUCTIONS ) );

	// The windmills face a fixed direction while the minigame runs
	m_fSavedWindDirection              = AWindmillHelper::ms_fWindDirection;
	AWindmillHelper::ms_fWindDirection = 0.0f;

	m_oWindmillHelper.AddWindmill( "wmill_01", 1.5f, 180.0f, 200.0f );
	m_oWindmillHelper.AddWindmill( "wmill_02", 1.5f, 180.0f, 200.0f );
	m_oWindmillHelper.AddWindmill( "wmill_03", 1.5f, 180.0f, 200.0f );
	ASimAnimModelHelperManager::GetSingleton()->AddModelHelper( &m_oWindmillHelper );

	AMiniGameState::OnInsertion();
	ResetTurn( TFALSE );

	if ( m_iVariant == 4 )
		m_oPickUpHudElement.SetForceHide( TFALSE );
}

// $Barnyard: FUNCTION 00484530
void AChickenFireMinigameState::OnActivate()
{
	GetPeckUnit()->SetVisible( TTRUE );
	m_pPlayerModel->SetSkeletonUpdating( TTRUE, TTRUE );

	ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();

	if ( m_eState == STATE_SEQUENCE )
	{
		// NOTE: the original passes a fourth argument (TTRUE) that AFadeManager::StartFade doesn't have
		AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );
		ResetTurn( TFALSE );
	}
	else if ( m_eState <= 1 )
	{
		ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();
		TVector4           vCamPos  = pTerrain->GetLocatorPos( "start_cam_pos" );
		TVector4           vLookAt  = pTerrain->GetLocatorPos( "start_cam_lookat" );

		pCamera->GetMatrix().GetTranslation() = vCamPos;
		pCamera->LookAtPoint( vLookAt );
		pCamera->SetFOV( DEFAULT_FOV );
	}

	if ( TUINT( m_eState ) <= 1 )
		m_pPlayerModel->SetSkeletonUpdating( TTRUE, TTRUE );

	m_fTextTimer = 0.2f;
	m_iCountdown = 4;
	AInputHandler::GetSingleton()->SetRumblePaused( TFALSE );

	if ( m_eState == STATE_BASHING )
	{
		// Resume the pull back animations where they were
		if ( m_pPlayerModel->IsAnimPlaying( TPS8( americanm_pullback ), 0.0f ) == TTRUE )
			m_pPlayerModel->SetAnimationTime( TPS8( americanm_pullback ), m_fPlayerPullbackTime );

		AAnimatableObject* pPeckObject = TSTATICCAST( AAnimatedUnit, GetPeckUnit() )->GetAnimatableObject();

		if ( pPeckObject->IsAnimPlaying( TPS8( launchPullBack ), 0.0f ) == TTRUE )
			pPeckObject->SetAnimationTime( TPS8( launchPullBack ), m_fPeckPullbackTime );

		m_fStateTime = 0.0f;
	}
	else
	{
		m_fTimer = 0.0f;
	}

	m_oPortrait.Show();
	m_oFeathers.SetVisible( TTRUE );

	if ( ASoundManager::GetSingleton()->IsCueUsed( m_iCue_C58 ) == TTRUE )
		ASoundManager::GetSingleton()->PauseCue( m_iCue_C58, TFALSE );

	AMiniGameState::OnActivate();
}

// Development: "-devautoplay" runs
void AChickenFireMinigameState::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fBashTimer = 0.0f;
	static TINT   s_iLastState = -1;

	s_fBashTimer += a_fDeltaTime;

	if ( m_eState != s_iLastState )
	{
		TINFO( "[devautoplay] state %d -> %d (power %.2f, score %d)\n", s_iLastState, m_eState, m_fPower, m_iScore );
		s_iLastState = m_eState;
	}

	PlayerInput& rInput = GetCurrentInput();

	switch ( m_eState )
	{
		case STATE_WAIT_FOR_BASH:
			// Start pulling back once the start hud has gone
			if ( m_bStarted && s_fBashTimer > 1.0f )
			{
				rInput.bBash = TTRUE;
				s_fBashTimer = 0.0f;
			}
			break;
		case STATE_BASHING:
			// Keep the power just under the ideal power
			if ( m_fPower < m_fIdealPowerBash && s_fBashTimer > 0.1f )
			{
				rInput.bBash = TTRUE;
				s_fBashTimer = 0.0f;
			}
			break;
		case STATE_FLYING:
		{
			// Boost now and then
			if ( s_fBashTimer > 1.5f )
			{
				rInput.bBash = TTRUE;
				s_fBashTimer = 0.0f;
			}

			// Steer towards the next ring (or the target once they're passed)
			TVector4 vAim = m_vTargetPoint;

			if ( m_iNextRing < m_vecObjects1.Size() )
				m_vecObjects1[ m_iNextRing ]->GetModelInstance()->GetPosition( vAim );

			TVector4 vTo   = vAim - m_vPeckPos;
			TFLOAT   fSide = m_vPeckDir.x * vTo.z - m_vPeckDir.z * vTo.x;

			rInput.fAimX = TMath::Max( TMath::Min( -fSide * 0.2f, 1.0f ), -1.0f );
			rInput.fAimY = TMath::Max( TMath::Min( vTo.y * 0.2f, 1.0f ), -1.0f );

			break;
		}
		default:
			break;
	}
}

AChickenFireMinigameState::PlayerInput& AChickenFireMinigameState::GetCurrentInput()
{
	return m_aInputs[ APlayerManager::GetSingleton()->GetCurrentPlayerIterator() ];
}
