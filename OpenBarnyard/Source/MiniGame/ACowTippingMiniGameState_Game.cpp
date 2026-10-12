#include "pch.h"
#include "ACowTippingMiniGameState.h"
#include "AMiniGameManager.h"
#include "ARandom.h"
#include "Helpers/APlayerProgress.h"
#include "Tasks/ARootTask.h"
#include "GameInterface/AAnticGameOverState.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ACamera.h"
#include "Terrain/ATerrainInterface.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitPlayer.h"
#include "Units/Projectiles/AProjectile.h"
#include "Units/Projectiles/AProjectileLauncher.h"
#include "UnitMovement/Steering/AInteriorSteer.h"
#include "UnitMovement/Steering/AStationarySteer.h"
#include "UnitMovement/PathfindingNetworks/AInteriorNetwork.h"
#include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"
#include "GUI/AFadeManager.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AGameTimeFX.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AMilkManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AMusicManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Input/AInputHandler.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <File/TTRB.h>
#include <Math/TMathInline.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>
#include <Toshi/TStringManager.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>
#include "SoundBank/cowtip.h"
#include "SoundBank/dagboss.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( end_taunt );
TPSTRING8_DECLARE( throw );
TPSTRING8_DECLARE( throw2 );
TPSTRING8_DECLARE( reload );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_lp2 );
TPSTRING8_DECLARE( up );
TPSTRING8_DECLARE( lower );
TPSTRING8_DECLARE( out_start );
TPSTRING8_DECLARE( in );

// NOTE: ACowTippingMiniGameState is split over several .cpp files, this one has the [Game] part.

//-----------------------------------------------------------------------------
// NOTE: typed views of provisional fields, until the header types them.
//-----------------------------------------------------------------------------

// Reinterprets a provisional field as another type (e.g. 4 consecutive fields as a vector)
#define CT_AS( TYPE, FIELD ) ( *TREINTERPRETCAST( TYPE*, &( FIELD ) ) )

// A field of an embedded HUD object that has no accessor yet
#define CT_RAW( TYPE, OFFSET ) ( *TREINTERPRETCAST( TYPE*, TREINTERPRETCAST( TUINTPTR, this ) + ( OFFSET ) ) )

// The camera settings of the current stage
#define CT_CAMERA ( m_aCameraSettings[ m_aStages[ m_iStage ].iCameraSettings ] )

namespace
{

static constexpr TINT NUM_GRID_ROWS    = 6;
static constexpr TINT NUM_GRID_COLUMNS = 5;

} // namespace

// NOTE: the first node of the list, without the null check of the iterator
static ACowTippingMiniGameState::PopUpUnit* GetFirstFreeRaccoon( ACowTippingMiniGameState* a_pState )
{
	return TSTATICCAST( ACowTippingMiniGameState::PopUpUnit, a_pState->m_llFreeRaccoons.T2GenericSList::Begin() );
}

static AAnimatableObject* GetPlayerObject( ACowTippingMiniGameState* a_pState )
{
	return a_pState->m_pLauncher;
}

static TBOOL IsModelUpdatingSkeleton( AAnimatableObject* a_pObject )
{
	AModelInstanceRef pModelInstance = a_pObject->GetModelInstance();
	return pModelInstance && pModelInstance->IsUpdatingSkeleton();
}

static T2GenericSList& GetHeldProjectiles( ACowTippingMiniGameState::ProjectileList& a_rList )
{
	return CT_AS( T2GenericSList, a_rList.llHeld );
}

static T2GenericSList& GetFreeProjectiles( ACowTippingMiniGameState::ProjectileList& a_rList )
{
	return CT_AS( T2GenericSList, a_rList.llFree );
}

static AAnimatableObject* GetProjectileObject( ACowTippingMiniGameState::Projectile* a_pProjectile )
{
	return a_pProjectile->pObject;
}

// Time and count of the a_iPowerUp'th power up of the player (6 pairs at PlayerData +0x00..+0x30)
static TFLOAT& GetPowerUpTime( ACowTippingMiniGameState::PlayerData& a_rPlayer, TINT a_iPowerUp )
{
	return a_rPlayer.aPowerUps[ a_iPowerUp ].fTime;
}

static TINT& GetPowerUpCount( ACowTippingMiniGameState::PlayerData& a_rPlayer, TINT a_iPowerUp )
{
	return a_rPlayer.aPowerUps[ a_iPowerUp ].iCharges;
}

// $Barnyard: FUNCTION 004a6b90
static TBOOL IsPopupHiding( AGUI2ElementPopup* a_pPopup )
{
	return a_pPopup->m_fTargetScale == 0.0f;
}

// Same as T2GUIElement::IsVisible
// $Barnyard: FUNCTION 004a6bb0
static TBOOL IsElementVisible( T2GUIElement* a_pElement )
{
	return a_pElement->IsVisible();
}

// Turns the matrix of a locator into a camera matrix
// $Barnyard: FUNCTION 004a7320
static void LocatorToCameraMatrix( TMatrix44& a_rMatrix )
{
	TVector4 vForward = a_rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	TVector4 vUp      = a_rMatrix.AsBasisVector4( BASISVECTOR_UP );

	a_rMatrix.AsBasisVector4( BASISVECTOR_UP )      = TVector4( -vForward.x, -vForward.y, -vForward.z, vForward.w );
	a_rMatrix.AsBasisVector4( BASISVECTOR_FORWARD ) = vUp;
}

// Clears the levels read from CowTipping.trb (a_rCount is m_iNumLevels, followed by m_aLevels)
// $Barnyard: FUNCTION 004a7c30
static void ClearLevels( TINT& a_rCount, ACowTippingMiniGameState::Level* a_pLevels )
{
	for ( TINT i = 0; i < a_rCount; i++ )
		a_pLevels[ i ] = ACowTippingMiniGameState::Level{};

	a_rCount = 0;
}

// Int of the "Ints" block (from 0xbfc) for the type of the cow, 1 for the special types
// $Barnyard: FUNCTION 004a7cb0
static TINT GetCowTypeInt( ACowTippingMiniGameState::Enemy* a_pCow )
{
	if ( a_pCow->eType >= ACowTippingMiniGameState::Enemy::ENEMYTYPE_COYOTE )
		return 1;

	return ACowTippingMiniGameState::GetSingleton()->m_aiUnitMaxHits[ a_pCow->eType ];
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs aim at the first unit that tips a cow and throw tomatoes at it
static void Dev_AutoPlay( ACowTippingMiniGameState* a_pState, TFLOAT a_fDeltaTime )
{
	static TBOOL  s_bChecked  = TFALSE;
	static TBOOL  s_bEnabled  = TFALSE;
	static TFLOAT s_fCooldown = 0.0f;

	if ( !s_bChecked )
	{
		s_bChecked = TTRUE;
		s_bEnabled = T2String8::FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;
	}

	if ( !s_bEnabled || a_pState->GetElapsedTime() < 1.0f )
		return;

	s_fCooldown -= a_fDeltaTime;

	T2GenericSList& rEnemies = *TREINTERPRETCAST( T2GenericSList*, &a_pState->m_llActiveEnemies );

	for ( T2GenericSList::Node* pNode = rEnemies.Begin(); pNode != rEnemies.End(); pNode = pNode->Next() )
	{
		ACowTippingMiniGameState::Enemy* pEnemy = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, pNode );
		AAnimatedUnit*                   pUnit  = pEnemy->pUnit;

		if ( !pUnit || !pUnit->IsVisible() )
			continue;

		// Aim at the enemy (and lock on to it) every frame, throw when the cooldown is over
		const TVector4* pPosition = pUnit->GetSteer()->GetPosition();
		TVector4&       rTarget   = a_pState->m_vAimTarget;
		rTarget.x                 = pPosition->x;
		rTarget.y                 = pPosition->y - 0.5f;
		rTarget.z                 = pPosition->z;
		a_pState->m_pLockEnemy    = pEnemy;

		if ( s_fCooldown <= 0.0f && a_pState->StartThrow() )
		{
			TINFO( "[devautoplay] threw a tomato at (%.2f, %.2f, %.2f), score %d\n", pPosition->x, pPosition->y, pPosition->z, a_pState->m_aPlayers[ a_pState->m_iCurrentPlayer ].iScore );
			s_fCooldown = 0.8f;
		}

		break;
	}
}

#endif // BARNYARD_COMMUNITY_PATCH

// $Barnyard: FUNCTION 004c4380
void ACowTippingMiniGameState::UpdateGame( TFLOAT a_fDeltaTime )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	Dev_AutoPlay( this, a_fDeltaTime );
#endif

	if ( m_eEndState != ENDSTATE_NONE )
	{
		if ( m_eEndState == ENDSTATE_FADING_OUT )
		{
			// Waiting for the fade out to end
			if ( !AFadeManager::GetSingleton()->HasAnyFadesInProgress( TTRUE ) )
			{
				m_eEndState = ENDSTATE_CUTSCENE;

				// NOTE: retail StartFade (0045cb20) has a 4th argument, TTRUE here, that our signature doesn't have yet
				AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 1.0f );

				ACamera*  pCamera = ACameraManager::GetSingleton()->GetCurrentCamera();
				TMatrix44 matCamera;
				ATerrainInterface::GetSingleton()->GetLocatorTransform( "camera_FM_end", matCamera );
				LocatorToCameraMatrix( matCamera );
				pCamera->SetMatrix( matCamera );

				AAnimatedUnit* pUnit = GetFirstFreeRaccoon( this )->pUnit;
				pUnit->GetModelInstance()->GetLocalTransform( m_matDagTransform );
				pUnit->SetVisible( TTRUE );
				pUnit->GetAnimatableObject()->PlayAnimation( TPS8( end_taunt ) );

				TMatrix44 matUnit;
				ATerrainInterface::GetSingleton()->GetLocatorTransform( "FM_dag_loc_end", matUnit );
				matUnit.m_f21 = -matUnit.m_f21;
				matUnit.m_f22 = -matUnit.m_f22;
				matUnit.m_f23 = -matUnit.m_f23;
				pUnit->GetModelInstance()->SetTransform( matUnit );

				m_bEndCutscene = TTRUE;
				FreezeForEndCutscene();
			}
		}
		else if ( m_eEndState == ENDSTATE_CUTSCENE )
		{
			AAnimatedUnit* pUnit = GetFirstFreeRaccoon( this )->pUnit;

			if ( pUnit->GetAnimatableObject()->IsAnimationFinished( TPS8( end_taunt ) ) )
			{
				EndTurn( TFALSE );
				m_eEndState = ENDSTATE_NONE;
			}
		}

		return;
	}

	if ( m_bStoryEnding )
		return;

	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode == Stage::STAGETYPE_JUNKYARD_BOSS || iMode == Stage::STAGETYPE_JUNKYARD )
	{
		m_pOtis->GetLauncher( 1 )->Update( a_fDeltaTime );

		if ( AMilkManager::GetSingletonSafe() )
			AMilkManager::GetSingleton()->Update( a_fDeltaTime );
	}

	UpdateTimeOfDay( a_fDeltaTime );

	iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
		UpdateBounceObjects( a_fDeltaTime );

	if ( ARootTask::GetSingleton()->IsPaused() )
		return;

	m_fUnk_13340 -= a_fDeltaTime;
	UpdateUnkHud_0EC58( a_fDeltaTime );
	m_fGameTime += a_fDeltaTime;

	if ( UpdateRestart() )
		return;

	if ( m_fReticleTimer >= 0.0f )
	{
		m_fReticleTimer -= a_fDeltaTime;

		if ( m_fReticleTimer < 0.0f )
			m_oTargetReticle.SetActive( TTRUE, TFALSE, 0.0f );
	}

	if ( m_aLivesTexts[ 0 ].IsVisible() && IsPowerUpActive( 3 ) && !IsElementVisible( &m_oLives1Hud.m_aGlows[ 0 ] ) )
		m_oLives1Hud.StartGlow( "Flare", m_afPowerUpTimes[ 3 ] - 1.0f );

	if ( m_oElementPopup.IsVisible() && !IsPopupHiding( &m_oElementPopup ) && m_fPopupHideTimer < 0.0f )
		m_fPopupHideTimer = 4.0f;

	m_fPopupHideTimer -= a_fDeltaTime;

	if ( m_fPopupHideTimer < 0.0f )
		m_oElementPopup.SetShown( TFALSE );

	if ( m_eTurnEnd != TURNEND_NONE )
	{
		m_fTurnEndTimer -= a_fDeltaTime;

		if ( m_fTurnEndTimer < 0.0f )
		{
			if ( m_eTurnEnd == TURNEND_GAME_END )
			{
				m_eTurnEnd = TURNEND_NONE;
				EndTurn( TTRUE );
				return;
			}

			if ( m_eTurnEnd == TURNEND_OUT_OF_LIVES )
			{
				m_eTurnEnd = TURNEND_NONE;

				TINT iStageMode = m_aStages[ m_iStage ].iStageType;

				if ( iStageMode == Stage::STAGETYPE_JUNKYARD_BOSS || iStageMode == Stage::STAGETYPE_JUNKYARD )
				{
					if ( m_eEndState == ENDSTATE_NONE )
						StartEndFade();
				}
				else
				{
					EndTurn( TFALSE );
				}

				return;
			}
		}
	}

	m_fStreakTimer -= a_fDeltaTime;
	m_fFixedStepTime += a_fDeltaTime;

	// Fixed time step
	if ( m_fFixedStepTime > 0.02f )
	{
		do
		{
			UpdateProjectiles( 0.02f );
			m_fFixedStepTime -= 0.02f;
		} while ( m_fFixedStepTime > 0.02f );
	}

	UpdateMusicVolume( a_fDeltaTime );

	if ( m_eEndState == ENDSTATE_NONE )
		UpdatePlayer( a_fDeltaTime );

	UpdateEnemies( a_fDeltaTime );
	UpdateKids( a_fDeltaTime );

	if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
		UpdateFadingUnits( a_fDeltaTime );

	UpdateCoyoteTimer( a_fDeltaTime );
	UpdateCowTipping( a_fDeltaTime );
	UpdateRunner( a_fDeltaTime );
	UpdateShooter( a_fDeltaTime );
	TakeNextProjectile();
	UpdateAmmoHud( a_fDeltaTime );
	UpdateTomatoesAnim( a_fDeltaTime );
	UpdateCowDust( a_fDeltaTime );
	UpdateCamera( a_fDeltaTime );

	if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
	{
		UpdatePickups( a_fDeltaTime );
		UpdatePowerUps( a_fDeltaTime );
	}

	UpdatePopups( a_fDeltaTime );

	// NOTE: retail passes the delta time (see RunLevelScript in [Flow])
	RunLevelScript( CT_AS( TINT, a_fDeltaTime ) );

	if ( IsPowerUpActive( 2 ) )
	{
		GetPlayerObject( this )->SetAnimSpeed( m_fThrowAnimSpeedPowerUp2 );

		if ( m_bUnk_13408 )
		{
			if ( m_pLockEnemy != 0 || m_pLockRaccoon != 0 )
				StartThrow();
			else
				StartReload();
		}
	}
	else if ( IsPowerUpActive( 0 ) )
	{
		GetPlayerObject( this )->SetAnimSpeed( m_fThrowAnimSpeedPowerUp0 * 0.5f );
	}
	else if ( IsPowerUpActive( 1 ) )
	{
		GetPlayerObject( this )->SetAnimSpeed( m_fThrowAnimSpeedPowerUp1 );
	}

	if ( !CheckTurnEnd( a_fDeltaTime ) )
		m_fPlayTime += a_fDeltaTime;
}

// Called when a power up runs out
// $Barnyard: FUNCTION 004a75e0
void ACowTippingMiniGameState::OnPowerUpEnded( TINT a_iPowerUp )
{
	if ( a_iPowerUp == 3 )
	{
		m_oLives1Hud.StartGlow( TNULL, 1.0f );
		m_aLivesTexts[ 0 ].Hide();
		return;
	}

	if ( a_iPowerUp == 4 )
		GetProjectileObject( &m_aProjectiles[ 0 ] )->SetGlow( TFALSE );

	m_iMaxAmmo = m_iStartAmmo;
	m_oAmmoCounter.SetMode( 0, m_iStartAmmo );
	m_oAmmoCounter.SetText( L"" );
}

// $Barnyard: FUNCTION 004a7650
void ACowTippingMiniGameState::StopRunnerPipe()
{
	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode == Stage::STAGETYPE_JUNKYARD_BOSS || iMode == Stage::STAGETYPE_JUNKYARD )
		m_pWildMikeBox->SetSkeletonUpdating( TFALSE, TTRUE );
}

// $Barnyard: FUNCTION 004a7680
TINT ACowTippingMiniGameState::HideRunner()
{
	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode == Stage::STAGETYPE_JUNKYARD_BOSS || iMode == Stage::STAGETYPE_JUNKYARD )
	{
		AAnimatedUnit* pUnit = m_pWildMike;
		pUnit->SetVisible( TFALSE );

		m_eRunnerState  = RUNNERSTATE_NONE;
		m_pRunnerTarget = 0;
		pUnit->ChangeSteer( &TGetClass( AStationarySteer ) );
	}

	return iMode;
}

// $Barnyard: FUNCTION 004a76e0
void ACowTippingMiniGameState::UpdatePopups( TFLOAT a_fUnk1 )
{
	m_oLevelPopup.Update( a_fUnk1 );
	m_aBonusPopups[ 0 ].Update( a_fUnk1 );
	m_aBonusPopups[ 1 ].Update( a_fUnk1 );
	m_aBonusPopups[ 2 ].Update( a_fUnk1 );
	m_oEndPopup.Update( a_fUnk1 );
	m_oTurnEndPopup.Update( a_fUnk1 );
}

// Shows the life HUDs needed by the stage mode
// $Barnyard: FUNCTION 004a7b30
TINT ACowTippingMiniGameState::SetupLifeHuds()
{
	TINT iMode = m_aStages[ m_iStage ].iStageType;

	switch ( iMode )
	{
		case Stage::STAGETYPE_JUNKYARD_BOSS:
			m_oLives1Hud.SetVisible( TTRUE );
			m_oLives2Hud.SetVisible( TTRUE );
			m_oPortrait.Hide();
			m_oButtonSlamHud.SetForceHide( TFALSE );
			m_oButtonSlamHud.SetAlpha( 0 );
			break;
		case Stage::STAGETYPE_JUNKYARD:
			m_oLives1Hud.SetVisible( TFALSE );
			m_oLives2Hud.SetVisible( TTRUE );
			m_oPortrait.Hide();
			m_oButtonSlamHud.SetForceHide( TTRUE );
			break;
		case Stage::STAGETYPE_KIDS:
			m_oLives1Hud.SetVisible( TFALSE );
			break;
	}

	return iMode;
}

// Reads the camera locator of the stage
// $Barnyard: FUNCTION 004a7e40
void ACowTippingMiniGameState::ReadCameraLocator()
{
	static TCHAR s_szLocatorName[ 64 ];
	const TCHAR* pszLocatorName;

	if ( m_aStages[ m_iStage ].iLocatorIndex != 0 )
	{
		TStringManager::String8Format( s_szLocatorName, sizeof( s_szLocatorName ), "camera%d", m_aStages[ m_iStage ].iLocatorIndex );
		pszLocatorName = s_szLocatorName;
	}
	else
	{
		pszLocatorName = "camera";
	}

	TVector4& rvLocatorPos = m_vCameraLocatorPos;
	TVector4& rvLocatorDir = m_vCameraLocatorDir;
	TVector4& rvCameraDir  = m_vCameraDir;
	TVector4& rvCameraPos  = m_vCameraPos;

	ATerrainInterface::GetSingleton()->GetLocatorPosAndAxis( pszLocatorName, rvLocatorPos, rvLocatorDir, BASISVECTOR_FORWARD );

	rvCameraDir = rvLocatorDir;
	rvLocatorPos.y -= CT_CAMERA.fLocatorHeightOffset;

	TVector4 vOffset;
	vOffset.Multiply( rvCameraDir, CT_CAMERA.fLocatorForwardOffset );
	rvLocatorPos.x += vOffset.x;
	rvLocatorPos.y += vOffset.y;
	rvLocatorPos.z += vOffset.z;

	rvCameraPos = rvLocatorPos;

	TVector4 vBelow( TVector4::VEC_NEGY.x + rvLocatorPos.x, TVector4::VEC_NEGY.y + rvLocatorPos.y, TVector4::VEC_NEGY.z + rvLocatorPos.z, rvLocatorPos.w );
	TMath::TriangleNormalCounterClockwise( CT_AS( TVector3, m_vCameraPitchAxis ), rvLocatorPos.AsVector3(), vBelow.AsVector3(), CT_AS( TVector3, m_vPlayerCentre ) );
}

// Places the camera at the camera locator
// $Barnyard: FUNCTION 004a80e0
void ACowTippingMiniGameState::ResetCamera()
{
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	TVector4& rvCameraPos = m_vCameraPos;
	TVector4& rvCameraDir = m_vCameraDir;
	rvCameraPos           = m_vCameraLocatorPos;
	rvCameraDir           = m_vCameraLocatorDir;

	ACamera* pCamera                                               = ACameraManager::GetSingleton()->GetCurrentCamera();
	pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION ) = rvCameraPos;
	m_vCameraBasePos                                               = rvCameraPos;
	pCamera->LookAtDirection( rvCameraDir );
}

// Detaches the objects of the 6x5 grid from their parents
// $Barnyard: FUNCTION 004a8670
TINT ACowTippingMiniGameState::DetachGridObjects()
{
	ACowTippingMiniGameState::Crate* pGrid = &m_aCrates[ 0 ][ 0 ];

	for ( TINT i = 0; i < NUM_GRID_ROWS; i++ )
	{
		for ( TINT k = NUM_GRID_COLUMNS - 1; k >= 0; k-- )
		{
			AAnimatableObject* pObject = pGrid[ i * NUM_GRID_COLUMNS + k ].pObject;

			if ( pObject->GetParentObject() )
				AAnimatableObjectManager::GetSingleton()->DetachObject( pObject, pObject->GetParentObject() );
		}
	}

	return 0;
}

// Updates the camera that orbits the player
// $Barnyard: FUNCTION 004a8720
void ACowTippingMiniGameState::UpdateCamera( TFLOAT a_fUnk1 )
{
	if ( m_eEndState != ENDSTATE_NONE )
		return;

	TVector4& rvCameraDir  = m_vCameraDir;
	TVector4& rvCameraPos  = m_vCameraPos;
	TVector4& rvCameraBase = m_vCameraBasePos;
	TVector4& rvCentre     = m_vPlayerCentre;

	if ( m_pCameraTarget != 0 )
	{
		// Look at the target unit
		TVector4 vTargetPos;
		m_pCameraTarget->GetSteer()->CopyPosition( vTargetPos );

		TVector4 vToTarget;
		vToTarget.Substract( rvCentre, vTargetPos );
		vToTarget.Normalise();

		// NOTE: the results of these are not used
		TQuaternion quatTarget;
		quatTarget.SetVectorDirection( vToTarget.AsVector3(), TVector3::VEC_POSZ );
		TQuaternion quatCamera;
		quatCamera.SetVectorDirection( rvCameraDir.AsVector3(), TVector3::VEC_POSZ );

		TVector4 vTarget( vToTarget.x, vToTarget.y, 0.0f, vToTarget.w );
		TVector4 vCamera( rvCameraDir.x, rvCameraDir.y, 0.0f, rvCameraDir.w );

		TVector4 vCross;
		vCross.CrossProduct( vCamera, vTarget );

		if ( vCross.z > 0.0f )
			m_fCameraTargetYaw = vTarget.GetAngleBetweenVector( vCamera );
		else
			m_fCameraTargetYaw = -vTarget.GetAngleBetweenVector( vCamera );

		m_fCameraTargetTime += a_fUnk1;
	}

	if ( m_fCameraYawDelta != 0.0f || m_fCameraPitchDelta != 0.0f || a_fUnk1 == 1000.0f )
	{
		TVector4 vOffset( rvCameraBase.x - rvCentre.x, rvCameraBase.y - rvCentre.y, rvCameraBase.z - rvCentre.z, rvCameraBase.w );

		TFLOAT fYawProgress;

		if ( m_pCameraTarget == 0 )
		{
			fYawProgress = a_fUnk1 * CT_CAMERA.fYawSpeed;

			if ( m_bUnk_1340A )
				fYawProgress = 1.0f;
		}
		else
		{
			fYawProgress = a_fUnk1 * CT_CAMERA.fTargetYawSpeed;
		}

		TMath::Clip( fYawProgress, 0.0f, 1.0f );
		m_fCameraYaw = TMath::LerpAngle( m_fCameraYaw, m_fCameraTargetYaw, fYawProgress );

		TFLOAT fMaxYaw = CT_CAMERA.fMaxYaw;

		if ( m_fCameraYaw > fMaxYaw )
			m_fCameraYaw = fMaxYaw;
		else if ( -fMaxYaw > m_fCameraYaw )
			m_fCameraYaw = -fMaxYaw;

		TFLOAT fPitchProgress = a_fUnk1 * CT_CAMERA.fPitchSpeed;

		if ( m_bUnk_1340A )
			fPitchProgress = 1.0f;

		TMath::Clip( fPitchProgress, 0.0f, 1.0f );
		m_fCameraPitch = TMath::LerpAngle( m_fCameraPitch, m_fCameraTargetPitch, fPitchProgress );

		TFLOAT fMaxPitch = -CT_CAMERA.fMinPitch;
		TFLOAT fMinPitch = -CT_CAMERA.fMaxPitch;

		if ( m_fCameraPitch > fMaxPitch )
			m_fCameraPitch = fMaxPitch;
		else if ( m_fCameraPitch < fMinPitch )
			m_fCameraPitch = fMinPitch;

		vOffset.AsVector3().RotateY( CT_CAMERA.fYawOffset + m_fCameraYaw );
		vOffset.RotateAroundAxis( m_vCameraPitchAxis, -m_fCameraPitch - CT_CAMERA.fPitchOffset );

		rvCameraPos.x = vOffset.x + rvCentre.x;
		rvCameraPos.y = vOffset.y + rvCentre.y;
		rvCameraPos.z = vOffset.z + rvCentre.z;
		rvCameraPos.w = vOffset.w;

		ACamera*  pCamera    = ACameraManager::GetSingleton()->GetCurrentCamera();
		TVector4& rvCamTrans = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_TRANSLATION );
		rvCamTrans           = rvCameraPos;

		rvCameraDir.x = vOffset.x;
		rvCameraDir.z = vOffset.z;
		rvCameraDir.Normalise();
		rvCameraDir.y = -CT_CAMERA.fLookDownY;
		rvCameraDir.AsVector3().RotateX( m_fCameraPitch );
		rvCameraDir.Normalise();
		rvCameraDir.x = -rvCameraDir.x;
		rvCameraDir.y = -rvCameraDir.y;
		rvCameraDir.z = -rvCameraDir.z;
		pCamera->LookAtDirection( rvCameraDir );

		// Shake
		const TVector4& rcRight = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_RIGHT );
		const TVector4& rcUp    = pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_UP );
		TVector4        vDown( -rcUp.x, -rcUp.y, -rcUp.z, 1.0f );
		TVector4        vRight( rcRight.x, rcRight.y, rcRight.z, 1.0f );

		TVector4 vNewPos = rvCamTrans;

		TVector4 vShake;
		vShake.Multiply( vRight, m_fCameraShakeX );
		vNewPos.x += vShake.x;
		vNewPos.y += vShake.y;
		vNewPos.z += vShake.z;

		vShake.Multiply( vDown, m_fCameraShakeY );
		vNewPos.x += vShake.x;
		vNewPos.y += vShake.y;
		vNewPos.z += vShake.z;

		rvCamTrans.x = vNewPos.x;
		rvCamTrans.y = vNewPos.y;
		rvCamTrans.z = vNewPos.z;
	}

	if ( CT_CAMERA.fTargetTime < m_fCameraTargetTime )
		m_pCameraTarget = 0;
}

// Puts the projectile the player holds back to the free list
// $Barnyard: FUNCTION 004a8db0
void ACowTippingMiniGameState::DropHeldProjectile()
{
	if ( AAnimatableObjectManager::GetSingleton()->GetAttachedObject( GetPlayerObject( this ), m_iLauncherGunEndBone, 0 ) )
	{
		ProjectileList&       rList   = m_aProjectileLists[ m_iProjectileType ];
		T2GenericSList::Node* pNode   = GetHeldProjectiles( rList ).PopBack();
		AAnimatableObject*    pObject = GetProjectileObject( TREINTERPRETCAST( Projectile*, pNode ) );

		AAnimatableObjectManager::GetSingleton()->DetachObject( pObject, GetPlayerObject( this ) );
		pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		pObject->SetVisible( TFALSE );

		GetFreeProjectiles( m_aProjectileLists[ m_iProjectileType ] ).PushBack( pNode );
		m_aProjectileLists[ m_iProjectileType ].iAmmo++;
	}
}

// Starts the throw animation
// $Barnyard: FUNCTION 004a8e50
TBOOL ACowTippingMiniGameState::StartThrow()
{
	static const TPString8* s_apThrowAnims[] = { &TPS8( throw ), &TPS8( throw2 ) };

	if ( m_iReloadState != RELOADSTATE_NONE )
		return TFALSE;

	AAnimatableObject* pPlayer = GetPlayerObject( this );

	if ( pPlayer->IsAnimPlaying( TPS8( throw ), 0.0f ) || pPlayer->IsAnimationPlanned( TPS8( throw ) ) ||
	     pPlayer->IsAnimPlaying( TPS8( throw2 ), 0.0f ) || pPlayer->IsAnimationPlanned( TPS8( throw2 ) ) )
		return TFALSE;

	if ( !AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pPlayer, m_iLauncherGunEndBone, 0 ) )
		return TFALSE;

	pPlayer->PlayAnimation( *s_apThrowAnims[ m_iThrowAnim ] );

	if ( ++m_iThrowAnim > 1 )
		m_iThrowAnim = 0;

	m_pThrowUnit       = m_pLockUnit;
	m_pThrowEnemy      = m_pLockEnemy;
	m_iThrowTargetBone = m_iLockTargetBone;
	m_pThrowRaccoon    = m_pLockRaccoon;

	if ( IsPowerUpActive( 2 ) )
		m_fShakeTimeLeft2 = m_fShakeTime2;

	return TTRUE;
}

// Gives every projectile back to the free lists
// $Barnyard: FUNCTION 004a94e0
void ACowTippingMiniGameState::ResetProjectiles()
{
	for ( TINT i = 0; i < 2; i++ )
	{
		ProjectileList& rList = m_aProjectileLists[ i ];
		rList.iAmmo           = 0;
		GetFreeProjectiles( rList ).Clear();
		GetHeldProjectiles( rList ).Clear();

		for ( TINT k = 0; k < MAX_PROJECTILES / 2; k++ )
		{
			Projectile&        rProjectile = m_aProjectiles[ i * ( MAX_PROJECTILES / 2 ) + k ];
			AAnimatableObject* pObject     = GetProjectileObject( &rProjectile );

			pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			pObject->SetVisible( TFALSE );
			rProjectile.vVelocity.Set( 0.0f, 0.0f, 0.0f, 1.0f );

			GetFreeProjectiles( rList ).PushFront( &rProjectile );
		}
	}
}

// Resets the state of the player's turn and the camera
// $Barnyard: FUNCTION 004a9580
void ACowTippingMiniGameState::ResetGame()
{
	m_iObjectsLeft = m_iNumObjects;

	TFLOAT fFill = TFLOAT( m_iNumObjects + 45 ) / TFLOAT( m_iNumObjects + 45 );
	TMath::Clip( fFill, 0.0f, 1.0f );
	CT_RAW( TFLOAT, 0xf404 ) = fFill; // m_oButtonSlamHud +0x48c (m_fTargetValue)

	m_iMaxAmmo          = m_iStartAmmo;
	m_fCoyoteTimer      = m_fCoyoteInterval;
	m_fThrowCooldown    = -m_fHitShakeTime - 0.01f;
	m_iNextFlarePopup   = 0;
	m_fUnk_132D0        = 0.0f;
	m_iThrowAnim        = 0;
	m_iCurrentPlayer    = 0;
	m_iNextCowSkin      = 0;
	m_fShakeTime        = 0;
	m_fUnk_13340        = 0.0f;
	m_fReticleTimer     = 0.0f;
	m_aiSkinIndex[ 0 ]  = 0;
	m_aiSkinIndex[ 1 ]  = 0;
	m_aiSkinIndex[ 2 ]  = 0;
	m_aiSkinIndex[ 3 ]  = 0;
	m_aiSkinIndex[ 4 ]  = 0;
	m_pCameraTarget     = 0;
	m_iReloadState      = RELOADSTATE_NONE;
	m_fLastInputX       = 0;
	m_fLastInputY       = 0;
	m_fSpeedY           = 0;
	m_bSideStepping     = TFALSE;
	m_bNoInputX         = TFALSE;
	m_bNoInputY         = TFALSE;
	m_fCameraTargetTime = 0.0f;

	TVector4& rvCameraDir = m_vCameraDir;
	TVector4& rvCameraPos = m_vCameraPos;
	TVector4& rvCentre    = m_vPlayerCentre;

	rvCameraDir      = TVector4( -0.34f, 0.2f, 0.92f, 1.0f );
	rvCameraPos      = TVector4( 27.0f, -5.0f, -86.0f, 1.0f );
	m_vCameraBasePos = rvCameraPos;
	rvCentre         = TVector4( 26.0f, -1.6f, -83.0f, 1.0f );

	static TCHAR s_szLocatorName[ 64 ];
	const TCHAR* pszLocatorName;

	if ( m_aStages[ m_iStage ].iLocatorIndex != 0 )
	{
		TStringManager::String8Format( s_szLocatorName, sizeof( s_szLocatorName ), "player_center%d", m_aStages[ m_iStage ].iLocatorIndex );
		pszLocatorName = s_szLocatorName;
	}
	else
	{
		pszLocatorName = "player_center";
	}

	TVector4    vCentre;
	TQuaternion quatCentre;
	ATerrainInterface::GetSingleton()->GetLocatorPosAndRot( pszLocatorName, vCentre, quatCentre );
	rvCentre = vCentre;

	m_fPlayTime        = 0.0f;
	m_fCameraYawDelta  = 0.0f;
	m_iFloatTextIndex  = 0;
	m_fCameraYaw       = 0.0f;
	m_fCameraTargetYaw = 0.0f;
	m_pLockUnit        = 0;
	m_fInputX          = 0;
	m_fMoveY           = 0.0f;
	m_fGameTime        = 0.0f;

	m_fCameraStartPitch = CT_CAMERA.fStartPitch;
	TFLOAT fPitch       = m_fCameraStartPitch * CT_CAMERA.fTargetPitchScale;

	m_iEnemyScore        = 0;
	m_iRaccoonScore      = 0;
	m_iCoyoteScore       = 0;
	m_fCameraTargetPitch = fPitch;
	m_iScoreTotal        = 0;
	m_fCameraPitch       = fPitch;
	m_iTurnScore         = 0;
	m_iCrateScore        = 0;
	m_iBonusScore        = 0;
	m_fPlayTime          = 0.0f;
	m_fTurnEndTimer      = m_fTurnEndDelay;

	AGameTimeFX* pFX;

	if ( m_aStages[ m_iStage ].iCameraSettings != 0 )
		pFX = AGameTimeFXManager::GetSingleton()->FindFX( "FinalMission" );
	else
		pFX = AGameTimeFXManager::GetSingleton()->FindFX( "COWTIP" );

	AGameTimeFXManager::GetSingleton()->SetForcedFX( pFX );

	ReadCameraLocator();
	ResetCamera();

	m_fMusicTimer        = 0.0f;
	m_fPopupHideTimer    = 0.0f;
	m_fLevelProgress     = 0.0f;
	m_fTimeOfDayProgress = 0.0f;
}

// Shows the "Level N" popups (and the level bonus with a_iUnk1)
// $Barnyard: FUNCTION 004a98a0
TBOOL ACowTippingMiniGameState::ShowLevelPopups( TINT a_iUnk1 )
{
	ALocale::FormatTags( m_wszLevelText, 63, 567, PPropertyValue( m_aPlayers[ m_iCurrentPlayer ].iLevel + 1 ) );

	m_oLevelPopup.m_eMode         = AGUI2AnythingPopup::MODE_FADE;
	m_oLevelPopup.m_fPulseSpeed   = 1.0f;
	m_oLevelPopup.m_fStartScale   = 1.45f;
	m_oLevelPopup.m_fScale        = 1.5f;
	m_oLevelPopup.m_fMoveDuration = m_fLevelPopupMoveDuration;
	m_oLevelPopup.m_fDuration     = m_fLevelPopupDuration;
	m_oLevelPopup.SetText( m_wszLevelText, 0xFFFFFFFF, &GetGUIElement() );

	for ( TINT i = 0; i < 3; i++ )
	{
		AGUI2AnythingPopup& rPopup = m_aBonusPopups[ i ];
		rPopup.m_eMode             = AGUI2AnythingPopup::MODE_FADE;
		rPopup.m_fPulseSpeed       = 1.0f;
		rPopup.m_fStartScale       = 1.0f;
		rPopup.m_fScale            = 1.05f;
		rPopup.m_fMoveDuration     = m_fBonusPopupMoveDuration;
		rPopup.m_fDuration         = m_fBonusPopupDuration;
	}

	if ( a_iUnk1 )
	{
		TINT iLevel = m_aPlayers[ m_iCurrentPlayer ].iLevel;

		// The bonus of the level that has just been completed
		if ( m_aLevels[ iLevel - 1 ].iBonus > 0 )
		{
			ALocale::FormatTags( m_wszBonusText, 63, 570, PPropertyValue( m_aLevels[ iLevel - 1 ].iBonus ) );

			AGUI2AnythingPopup& rPopup = m_aBonusPopups[ 1 ];
			rPopup.m_fDelay            = m_fBonusPopupDelay;
			rPopup.m_fStartScale       = 1.35f;
			rPopup.m_fScale            = 1.4f;
			rPopup.SetText( m_wszBonusText, 0xFFFFFFFF, &GetGUIElement() );
		}
	}

	return TTRUE;
}

// Loads CowTipping.trb
// $Barnyard: FUNCTION 004aa470
TBOOL ACowTippingMiniGameState::LoadData()
{
	TTRB oTRB;

	if ( oTRB.Load( "Data\\MinigameData\\CowTipping\\CowTipping.trb", 0 ) != TTRB::ERROR_OK )
		return TFALSE;

	TUtil::MemCopy( &m_fUnk_00C98, oTRB.GetSymbolAddress( "Floats" ), 0x420 );
	TUtil::MemCopy( &m_iStartScore, oTRB.GetSymbolAddress( "Ints" ), 0xEC );

	Level* pLevels = oTRB.CastSymbol<Level>( "Levels" );
	ClearLevels( m_iNumLevels, m_aLevels );

	TINT iNumLevels = *oTRB.CastSymbol<TINT>( "LCount" );

	for ( TINT i = 0; i < iNumLevels; i++ )
		m_aLevels[ m_iNumLevels++ ] = pLevels[ i ];

	Stage* pStages = oTRB.CastSymbol<Stage>( "Stage" );
	m_iNumStages   = *oTRB.CastSymbol<TINT>( "StageCount" );

	for ( TINT i = 0; i < m_iNumStages; i++ )
		m_aStages[ i ] = pStages[ i ];

	oTRB.Close();

#ifdef BARNYARD_LATEST
	// Latest version (Wii 801f6c70): 2 more lives on Easy, at most 9 (806af748)
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
	{
		for ( TINT i = 0; i < m_iNumStages; i++ )
		{
			m_aStages[ i ].iLives2 += 2;
			m_aStages[ i ].iMaxLives2 = TMath::Min( m_aStages[ i ].iMaxLives2 + 2, 9 );
			m_aStages[ i ].iLives2    = TMath::Min( m_aStages[ i ].iLives2, m_aStages[ i ].iMaxLives2 );
		}
	}
#endif

	AProjectile::ResetTrajectoryGravity();
	AProjectile::SetTrajectoryGravity( m_fTrajectoryGravity );

	TINT iVariant = AMiniGameManager::GetSingleton()->GetMiniGameVariant();
	m_iStage      = ( iVariant <= m_iNumStages ) ? iVariant : m_iNumStages;

	ACameraManager::GetSingleton()->GetCurrentCamera()->SetFOV( GetFOV() );

	return TTRUE;
}

// Resets the power ups of the player
// $Barnyard: FUNCTION 004aa760
void ACowTippingMiniGameState::ResetPowerUps( TINT a_iPlayer )
{
	PlayerData& rPlayer             = m_aPlayers[ a_iPlayer ];
	rPlayer.aPowerUps[ 0 ].fTime    = 0.0f;
	rPlayer.aPowerUps[ 0 ].iCharges = 0;
	rPlayer.aPowerUps[ 1 ].fTime    = 0.0f;
	rPlayer.aPowerUps[ 1 ].iCharges = 0;
	rPlayer.aPowerUps[ 2 ].fTime    = 0.0f;
	rPlayer.aPowerUps[ 2 ].iCharges = 0;
	rPlayer.aPowerUps[ 3 ].fTime    = 0.0f;
	rPlayer.aPowerUps[ 3 ].iCharges = 0;
	rPlayer.aPowerUps[ 4 ].fTime    = 0.0f;
	rPlayer.aPowerUps[ 4 ].iCharges = 0;
	rPlayer.aPowerUps[ 5 ].fTime    = 0.0f;
	rPlayer.aPowerUps[ 5 ].iCharges = 0;
}

// Uses up one charge of the active power ups (but the 4th)
// $Barnyard: FUNCTION 004aa7c0
void ACowTippingMiniGameState::UsePowerUpCharges( TINT a_iPlayer )
{
	for ( TINT i = 0; i < 6; i++ )
	{
		if ( i == 3 )
			continue;

		PlayerData& rPlayer = m_aPlayers[ a_iPlayer ];

		if ( GetPowerUpTime( rPlayer, i ) != 0.0f && --GetPowerUpCount( rPlayer, i ) <= 0 )
		{
			GetPowerUpTime( rPlayer, i ) = 0.0f;
			OnPowerUpEnded( i );
		}
	}
}

// Counts down the active power ups
// $Barnyard: FUNCTION 004aa980
void ACowTippingMiniGameState::UpdatePowerUps( TFLOAT a_fUnk1 )
{
	for ( TINT i = 0; i < 6; i++ )
	{
		TFLOAT& rfTime = GetPowerUpTime( m_aPlayers[ m_iCurrentPlayer ], i );

		if ( rfTime > 0.0f )
		{
			rfTime -= a_fUnk1;

			if ( rfTime < 0.0f )
			{
				rfTime = 0.0f;
				OnPowerUpEnded( i );
			}
		}
	}
}

// $Barnyard: FUNCTION 004ab060
void ACowTippingMiniGameState::ResetBounceObjects()
{
	ACowTippingMiniGameState::Coin* pObjects = m_aCoins;

	for ( TINT i = 0; i < NUM_COINS; i++ )
	{
		pObjects[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );
		pObjects[ i ].iState = Coin::COINSTATE_FREE;
	}
}

// Fades out (end of the final mission)
// $Barnyard: FUNCTION 004ab090
void ACowTippingMiniGameState::StartEndFade()
{
	m_eEndState = ENDSTATE_FADING_OUT;

	// NOTE: retail StartFade (0045cb20) has a 4th argument, TTRUE here, that our signature doesn't have yet
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 1.0f );
	AMusicManager::GetSingleton()->SetVolume( m_fSavedMusicVolume );
}

// Plays the dust effect of the running cows
// $Barnyard: FUNCTION 004ac080
void ACowTippingMiniGameState::UpdateCowDust( TFLOAT a_fDeltaTime )
{
	ACowTippingMiniGameState::Enemy* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, &m_llActiveEnemies );

	for ( ACowTippingMiniGameState::Enemy* pCow = pRoot->Next(); pCow != pRoot; )
	{
		ACowTippingMiniGameState::Enemy* pNext = pCow->Next();

		m_fDustTimer -= a_fDeltaTime;

		if ( m_fDustTimer <= 0.0f )
			m_fDustTimer = 0.0f;

		if ( m_fDustTimer <= 0.0f && pCow->iRunning == 1 && pCow->pUnit->GetSteer()->GetSpeed() > 2.5f )
		{
			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pCow->pUnit->GetSteer() );

			if ( pSteer->GetMode() != AInteriorSteer::MODE_STILL )
			{
				TVector4 vPosition = *pSteer->GetPosition();
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iDustPuffSmallEffect, vPosition, TVector4::VEC_NEGY );
				m_fDustTimer = 0.05f;
			}
		}

		pCow = pNext;
	}
}

// $Barnyard: FUNCTION 004ac190
void ACowTippingMiniGameState::UpdateUnkHud_0EC58( TFLOAT a_fDeltaTime )
{
	if ( GetNumUnhit13268() == 0 )
		m_oRaccoonWarning.SetShown( TFALSE );

	if ( !m_bShootHintShown || GetHeldProjectiles( m_aProjectileLists[ 1 ] ).Size() > 0 )
		return;

	ACowTippingMiniGameState::PopUpUnit* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::PopUpUnit*, &m_llActiveRaccoons );

	for ( ACowTippingMiniGameState::PopUpUnit* pNode = pRoot->Next(); pNode != pRoot; pNode = pNode->Next() )
	{
		if ( pNode->iState == PopUpUnit::POPUPSTATE_DRAWING )
			return;
	}

	if ( m_bShootHintShown == TTRUE )
	{
		// NOTE: the string isn't used
		ALocaleManager::Interface()->GetString( 589 );
		CT_RAW( TFLOAT, 0xeda0 ) = 5.0f; // m_oRaccoonWarning +0x148 (m_fPulseSpeed)
		m_bShootHintShown        = TFALSE;
	}
}

// Sets the time of day FX of the level progress (0..1)
// $Barnyard: FUNCTION 004ac230
TINT ACowTippingMiniGameState::SetTimeOfDay( TFLOAT a_fUnk1 )
{
	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode == Stage::STAGETYPE_JUNKYARD_BOSS || iMode == Stage::STAGETYPE_JUNKYARD )
		return iMode;

	AGameTimeFX* pTo;
	TFLOAT       fProgress;

	if ( a_fUnk1 <= 0.8f )
	{
		pTo       = m_pMoonFX;
		fProgress = a_fUnk1 * 1.25f;
	}
	else
	{
		pTo       = m_pDawnFX;
		fProgress = ( a_fUnk1 - 0.8f ) * 5.0f;
	}

	// NOTE: both halves interpolate from m_pCowTipFX
	AGameTimeFX oFX = *m_pCowTipFX;
	oFX.Interpolate( oFX, *pTo, fProgress );
	AGameTimeFXManager::GetSingleton()->SetForcedFX( &oFX );

	return 0;
}

// $Barnyard: FUNCTION 004ac660
void ACowTippingMiniGameState::FUN_004ac660()
{
	ACowTippingMiniGameState::PopUpUnit* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::PopUpUnit*, &m_llActiveGophers );

	for ( ACowTippingMiniGameState::PopUpUnit* pNode = pRoot->Next(); pNode != pRoot; )
	{
		ACowTippingMiniGameState::PopUpUnit* pNext = pNode->Next();
		RemoveFadedUnit( TREINTERPRETCAST( TINT, pNode ) );
		pNode = pNext;
	}
}

// Resets the turn of the current player
// $Barnyard: FUNCTION 004ac6b0
TINT ACowTippingMiniGameState::ResetTurn()
{
	m_fKidShootTimer       = 0.0f;
	m_auiUsedCoinBits[ 0 ] = 0;
	m_auiUsedCoinBits[ 1 ] = 0;
	m_iLastRaccoonSpot     = -1;
	m_bRaccoonsShootNow    = TFALSE;
	m_fFixedStepTime       = 0.0f;
	m_fAccelTimerX         = TMath::MAXFLOAT;
	m_fAccelTimerY         = TMath::MAXFLOAT;
	m_bUnk_13408           = TFALSE;

	m_vUnk_011AC = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );

	m_fCoyoteTimer                        = m_fCoyoteInterval;
	m_fThrowCooldown                      = -m_fHitShakeTime - 0.01f;
	m_fGopherTimer                        = m_fGopherDelay;
	m_iUnk_132F4                          = 0;
	m_fStreakTimer                        = 0.0f;
	m_iStreak                             = 0;
	m_fRaccoonShootTimer                  = m_fRaccoonShootDelay;
	m_fUnk_13288                          = 0;
	m_fUnk_132D0                          = 0.0f;
	m_aPlayers[ m_iCurrentPlayer ].iLevel = 0;
	m_iScriptCommand                      = 0;
	m_iScriptLoops                        = 0;
	m_fCameraPitchDelta                   = 0.0f;
	m_fDustTimer                          = 0.0f;
	m_bUnk_1340A                          = TFALSE;
	m_bUnk_13410                          = TFALSE;
	m_pCameraTarget                       = 0;
	m_iReloadState                        = RELOADSTATE_NONE;
	m_oAmmoCounter.SetTargetCount( 0 );
	m_aPlayers[ m_iCurrentPlayer ].iTurnShots = 0;
	m_aPlayers[ m_iCurrentPlayer ].iTurnHits  = 0;
	m_fScriptWaitStart                        = g_oSystemManager.GetScheduler()->GetTotalTime();

	UsePowerUpCharges( m_iCurrentPlayer );
	UpdateTomatoesAnim( 1000.0f );

	m_fCameraShakeX          = 0.0f;
	m_fCameraShakeY          = 0.0f;
	m_fHitShakeIntervalTimer = 0.0f;
	m_fRecoilX               = 0;
	m_fRecoilY               = 0.0f;
	m_fShakeIntervalTimer2   = 0.0f;
	m_fShakeTimeLeft2        = 0.0f;
	m_bLevelDone             = TFALSE;
	m_fLevelTimer            = TMath::MAXFLOAT;
	m_fLevelTime             = TMath::MAXFLOAT;

	m_aLivesTexts[ 0 ].Hide();
	m_aLivesTexts[ 1 ].Hide();
	m_oElementPopup.SetShownInstantly( TFALSE );
	m_oLives2Hud.SetFrameAnimation( TNULL, 0, 0.0f );
	m_oAmmoCounter.SetText( L"" );
	ClearUnitsFlag9();

	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
		ResetBounceObjects();

	m_oRunnerIcon.SetShown( TFALSE );

	iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode == Stage::STAGETYPE_JUNKYARD_BOSS || iMode == Stage::STAGETYPE_JUNKYARD )
		m_pWildMikeBox->SetSkeletonUpdating( TFALSE, TTRUE );

	TINT iResult      = SetTimeOfDay( 0.0f );
	m_bShooterTurning = TFALSE;

	return iResult;
}

// Starts the reload animation if the player isn't throwing
// $Barnyard: FUNCTION 004acb30
TBOOL ACowTippingMiniGameState::StartReload()
{
	if ( m_iReloadState != RELOADSTATE_NONE )
		return TFALSE;

	AAnimatableObject* pPlayer = GetPlayerObject( this );

	if ( pPlayer->IsAnimPlaying( TPS8( throw ), 0.0f ) || pPlayer->IsAnimationPlanned( TPS8( throw ) ) ||
	     pPlayer->IsAnimPlaying( TPS8( throw2 ), 0.0f ) || pPlayer->IsAnimationPlanned( TPS8( throw2 ) ) )
		return TFALSE;

	if ( m_iProjectileType >= 0 && m_aProjectileLists[ m_iProjectileType ].iAmmo >= 9 )
		return TFALSE;

	UsePowerUpCharges( m_iCurrentPlayer );
	pPlayer->PlayAnimation( TPS8( reload ) );
	m_iReloadState = RELOADSTATE_RELOADING;
	m_iThrowAnim   = 0;
	m_bUnk_13408   = TFALSE;

	return TTRUE;
}

// Freezes the game for the end cutscene
// $Barnyard: FUNCTION 004ad150
void ACowTippingMiniGameState::FreezeForEndCutscene()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TTRUE );

	m_oLevelPopup.Hide();
	m_aBonusPopups[ 0 ].Hide();
	m_aBonusPopups[ 1 ].Hide();
	m_aBonusPopups[ 2 ].Hide();
	m_oEndPopup.Hide();
	m_oTurnEndPopup.Hide();
	m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );
	GetPlayerObject( this )->SetSkeletonUpdating( TFALSE, TTRUE );

	for ( TINT i = 0; i < m_iNumCows; i++ )
		m_aCows[ i ].pObject->SetSkeletonUpdating( TFALSE, TTRUE );

	ACowTippingMiniGameState::Enemy* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, &m_llActiveEnemies );

	for ( ACowTippingMiniGameState::Enemy* pCow = pRoot->Next(); pCow != pRoot; pCow = pCow->Next() )
		pCow->pUnit->SetVisible( TFALSE );

	GetGUIElement().Hide();
}

// Unfreezes the game after the end cutscene
// $Barnyard: FUNCTION 004ad250
void ACowTippingMiniGameState::UnfreezeAfterEndCutscene()
{
	AInputHandler::GetSingleton()->SetRumblePaused( TFALSE );

	m_oLevelPopup.ShowIfHasText();
	m_aBonusPopups[ 0 ].ShowIfHasText();
	m_aBonusPopups[ 1 ].ShowIfHasText();
	m_aBonusPopups[ 2 ].ShowIfHasText();
	m_oEndPopup.ShowIfHasText();
	m_oTurnEndPopup.ShowIfHasText();
	m_oTargetReticle.SetActive( TTRUE, TTRUE, 50.0f );
	GetPlayerObject( this )->SetSkeletonUpdating( TTRUE, TTRUE );

	for ( TINT i = 0; i < m_iNumCows; i++ )
		m_aCows[ i ].pObject->SetSkeletonUpdating( TTRUE, TTRUE );

	ACowTippingMiniGameState::Enemy* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, &m_llActiveEnemies );

	for ( ACowTippingMiniGameState::Enemy* pCow = pRoot->Next(); pCow != pRoot; pCow = pCow->Next() )
		pCow->pUnit->SetVisible( TTRUE );

	GetGUIElement().Show();
}

// Resets the score and the lives of the player
// $Barnyard: FUNCTION 004ad460
TINT ACowTippingMiniGameState::ResetPlayer( TINT a_iUnk1 )
{
	PlayerData& rPlayer         = m_aPlayers[ a_iUnk1 ];
	rPlayer.iScore              = 0;
	rPlayer.iLastExtraLifeScore = 0;
	rPlayer.iPowerUp0Shots      = 0;
	rPlayer.iScore              = m_iStartScore;

	if ( m_iStartScore < 0 )
		rPlayer.iScore = 0;

	GetSingleton()->SetScore( rPlayer.iScore );

	rPlayer.iLives     = 0;
	rPlayer.iLives2    = 0;
	rPlayer.iHits      = 0;
	rPlayer.iShots     = 0;
	rPlayer.iTurnShots = 0;

	rPlayer.iLives = m_aStages[ m_iStage ].iLives1;
	m_oLives1Hud.SetLives( m_aStages[ m_iStage ].iLives1 );
	rPlayer.iLives2 = m_aStages[ m_iStage ].iLives2;
	m_oLives2Hud.SetLives( m_aStages[ m_iStage ].iLives2 );

	ResetPowerUps( a_iUnk1 );
	return SetupLifeHuds();
}

// Puts Otis at his locator (final mission)
// $Barnyard: FUNCTION 004ae160
void ACowTippingMiniGameState::PlaceOtis()
{
	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
		return;

	AAnimatableObject* pObject = m_pHanna;
	pObject->KillAllAnimations();
	pObject->PlayAnimation( TPS8( idle_lp ) );

	TMatrix44 matOtis;
	ATerrainInterface::GetSingleton()->GetLocatorTransform( "otis_loc", matOtis );

	AAnimatedUnit* pOtis = TREINTERPRETCAST( AAnimatedUnit*, m_pOtis );
	pOtis->GetAnimatableObject()->GetModelInstance()->SetTransform( matOtis );
	pOtis->GetAnimatableObject()->KillAllAnimations();
	pOtis->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
}

// Updates the counter of the HUD element at 0x57d4
// $Barnyard: FUNCTION 004af780
void ACowTippingMiniGameState::UpdateAmmoHud( TFLOAT a_fDeltaTime )
{
	TINT iCount = GetAmmoCount( m_iProjectileType );
	TINT iValue = iCount % 20;

	if ( iValue == 0 )
	{
		if ( iCount <= 9 )
		{
			m_oAmmoCounter.SetTargetCount( iValue );
			m_oAmmoCounter.Update( a_fDeltaTime );
			return;
		}

		iValue = 20;
	}

	if ( m_aPlayers[ m_iCurrentPlayer ].aPowerUps[ 2 ].fTime <= 0.0f )
		m_oAmmoCounter.SetTargetCount( iValue );

	m_oAmmoCounter.Update( a_fDeltaTime );
}

// Moves the time of day towards the level progress
// $Barnyard: FUNCTION 004af8b0
void ACowTippingMiniGameState::UpdateTimeOfDay( TFLOAT a_fUnk1 )
{
	if ( m_aStages[ m_iStage ].iCameraSettings == 0 && m_fLevelProgress != m_fTimeOfDayProgress )
	{
		TFLOAT fProgress = a_fUnk1 / m_fTimeOfDayDuration + m_fTimeOfDayProgress;

		if ( fProgress > m_fLevelProgress )
			fProgress = m_fLevelProgress;

		m_fTimeOfDayProgress = fProgress;
		SetTimeOfDay( fProgress );
	}
}

// Puts the next projectile in the hand of the player
// $Barnyard: FUNCTION 004afa90
void ACowTippingMiniGameState::TakeNextProjectile()
{
	if ( m_fThrowCooldown > 0.0f || AAnimatableObjectManager::GetSingleton()->GetAttachedObject( GetPlayerObject( this ), m_iLauncherGunEndBone, 0 ) )
		return;

	if ( m_iProjectileType >= 0 && m_aProjectileLists[ m_iProjectileType ].iAmmo <= 0 )
		SelectAmmoList();

	if ( m_iProjectileType < 0 || m_aProjectileLists[ m_iProjectileType ].iAmmo <= 0 )
	{
		StartReload();
		return;
	}

	ProjectileList& rList = m_aProjectileLists[ m_iProjectileType ];

	if ( !GetFreeProjectiles( rList ).IsLinked() )
		return;

	if ( IsPowerUpActive( 0 ) )
	{
		PlayerData& rPlayer = m_aPlayers[ m_iCurrentPlayer ];

		if ( rPlayer.iPowerUp0Shots == 0 )
			rPlayer.iPowerUp0Shots = m_iPowerUp0Shots;

		CT_RAW( TFLOAT, 0x58b0 ) = TFLOAT( rPlayer.iPowerUp0Shots-- ) / TFLOAT( m_iPowerUp0Shots ); // m_oAmmoCounter +0xdc (m_fLastIconState)

		if ( m_aPlayers[ m_iCurrentPlayer ].iPowerUp0Shots <= 0 )
			m_aProjectileLists[ m_iProjectileType ].iAmmo--;
	}
	else
	{
		rList.iAmmo--;
	}

	T2GenericSList::Node* pNode = GetFreeProjectiles( m_aProjectileLists[ m_iProjectileType ] ).PopFront();
	GetHeldProjectiles( m_aProjectileLists[ m_iProjectileType ] ).PushBack( pNode );

	Projectile*        pProjectile = TREINTERPRETCAST( Projectile*, pNode );
	AAnimatableObject* pObject     = GetProjectileObject( pProjectile );

	TQuaternion quatRotation = TQuaternion::IDENTITY;
	pObject->GetModelInstance()->SetTranslation( CT_AS( TVector3, m_vPlayerPos ) );
	AAnimatableObjectManager::GetSingleton()->AttachObject( pObject, GetPlayerObject( this ), 0, TINT8( m_iLauncherGunEndBone ), TVector4::VEC_ZERO, quatRotation, 1.0f );
	pObject->SetSkeletonUpdating( TTRUE, TTRUE );
	pObject->SetVisible( TTRUE );

	ResetProjectileScale( pProjectile );
	pProjectile->fScale = 1.0f;

	if ( IsPowerUpActive( 4 ) )
		pObject->SetGlow( TTRUE );
	else
		pObject->SetGlow( TFALSE );
}

// $Barnyard: FUNCTION 004b0be0
void ACowTippingMiniGameState::FUN_004b0be0()
{
	ACowTippingMiniGameState::Enemy* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, &m_llActiveEnemies );

	for ( ACowTippingMiniGameState::Enemy* pCow = pRoot->Next(); pCow != pRoot; )
	{
		ACowTippingMiniGameState::Enemy* pNext = pCow->Next();
		RemoveEnemy( pCow );
		pCow = pNext;
	}
}

// $Barnyard: FUNCTION 004b0c30
void ACowTippingMiniGameState::FUN_004b0c30()
{
	ACowTippingMiniGameState::PopUpUnit* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::PopUpUnit*, &m_llActiveRaccoons );

	for ( ACowTippingMiniGameState::PopUpUnit* pNode = pRoot->Next(); pNode != pRoot; )
	{
		ACowTippingMiniGameState::PopUpUnit* pNext = pNode->Next();
		RemoveKid( TREINTERPRETCAST( TINT, pNode ) );
		pNode = pNext;
	}
}

// $Barnyard: FUNCTION 004b0c80
void ACowTippingMiniGameState::FUN_004b0c80()
{
	ACowTippingMiniGameState::Crate* pGrid = &m_aCrates[ 0 ][ 0 ];

	for ( TINT i = 0; i < NUM_GRID_ROWS; i++ )
	{
		for ( TINT k = NUM_GRID_COLUMNS - 1; k >= 0; k-- )
			HidePickup( &pGrid[ i * NUM_GRID_COLUMNS + k ] );
	}
}

// Resets the objects of m_aCows
// $Barnyard: FUNCTION 004b0cd0
void ACowTippingMiniGameState::ResetCows()
{
	for ( TINT i = 0; i < m_iNumCows; i++ )
	{
		Cow&               rUnk    = m_aCows[ i ];
		AAnimatableObject* pObject = rUnk.pObject;

		pObject->KillAllAnimations();
		pObject->PlayAnimation( TPS8( idle_lp ) );
		pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		pObject->GetModelInstance()->SetTransform( rUnk.matStart );
		CT_AS( TBOOL, rUnk.bActive ) = TTRUE;

		TINT iMode = m_aStages[ m_iStage ].iStageType;

		if ( iMode == Stage::STAGETYPE_JUNKYARD || iMode == Stage::STAGETYPE_JUNKYARD_BOSS )
		{
			AAnimatableObject* pAttached = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pObject, m_iCowTipSpotBone, 0 );

			if ( pAttached )
				AAnimatableObjectManager::GetSingleton()->DetachObject( pAttached, pObject );
		}
	}
}

// Adds to the score of the player
// $Barnyard: FUNCTION 004b0de0
void ACowTippingMiniGameState::AddScore( TINT a_iScore )
{
	PlayerData& rPlayer = m_aPlayers[ m_iCurrentPlayer ];
	rPlayer.iScore += a_iScore;

	if ( rPlayer.iScore < 0 )
		rPlayer.iScore = 0;

	GetSingleton()->SetScore( rPlayer.iScore );
	m_iTurnScore += a_iScore;
}

// Sends the runner back into its pipe (final mission)
// $Barnyard: FUNCTION 004b5260
TBOOL ACowTippingMiniGameState::SendRunnerBack()
{
	AInteriorSteer*    pSteer = TSTATICCAST( AInteriorSteer, TREINTERPRETCAST( AUnit*, m_pWildMike )->GetSteer() );
	AAnimatableObject* pPipe  = m_pWildMikeBox;
	m_eRunnerState            = RUNNERSTATE_RETURNING;

	TVector4 vPosition;
	pPipe->GetModelInstance()->GetPosition( vPosition );

	TVector4 vDown;
	pPipe->GetModelInstance()->GetUpVector( vDown );
	vDown.x = -vDown.x;
	vDown.y = -vDown.y;
	vDown.z = -vDown.z;

	TVector4 vOffset;
	vOffset.Multiply( vDown, 0.3f );

	m_vRunnerGoal.x = vOffset.x + vPosition.x;
	m_vRunnerGoal.y = vOffset.y + vPosition.y;
	m_vRunnerGoal.z = vOffset.z + vPosition.z;
	m_vRunnerGoal.w = vPosition.w;

	pSteer->Command_CancelCurrentCommandNoStop();
	return pSteer->SeekPosition( m_vRunnerGoal );
}

// Makes the runner come out of its pipe (final mission)
// $Barnyard: FUNCTION 004b55a0
void ACowTippingMiniGameState::ReleaseRunner()
{
	m_fRunnerTime = m_fRunnerOutTime;

	AAnimatableObject* pPipe = m_pWildMikeBox;

	TVector4 vPosition;
	pPipe->GetModelInstance()->GetPosition( vPosition );

	TVector4 vDown;
	pPipe->GetModelInstance()->GetUpVector( vDown );
	vDown.x = -vDown.x;
	vDown.y = -vDown.y;
	vDown.z = -vDown.z;

	TVector4 vOffset;
	vOffset.Multiply( vDown, 0.3f );
	vPosition.x += vOffset.x;
	vPosition.y += vOffset.y;
	vPosition.z += vOffset.z;

	AUnit* pRunner = TREINTERPRETCAST( AUnit*, m_pWildMike );
	pRunner->SetSteerPosition( vPosition );
	pRunner->ChangeSteer( &TGetClass( AInteriorSteer ) );

	AInteriorSteer*   pSteer   = TSTATICCAST( AInteriorSteer, pRunner->GetSteer() );
	AInteriorNetwork* pNetwork = m_pNetwork;
	pSteer->SetNetwork( pNetwork, 0 );
	pSteer->SnapToArea( pNetwork->FindAreaAtPosition( vPosition, -1 ) );

	pRunner->SetSteerPosition( vPosition );
	m_pWildMike->SetVisible( TTRUE );
	pRunner->SetSteerDirection( vDown );

	m_eRunnerState = RUNNERSTATE_CHASING;
}

// Updates the 8 bouncing objects
// $Barnyard: FUNCTION 004b5980
void ACowTippingMiniGameState::UpdateBounceObjects( TFLOAT a_fDeltaTime )
{
	ACowTippingMiniGameState::Coin* pObjects = m_aCoins;

	for ( TINT i = 0; i < NUM_COINS; i++ )
	{
		ACowTippingMiniGameState::Coin& rObject = pObjects[ i ];

		if ( !IsModelUpdatingSkeleton( rObject.pObject ) || rObject.iState == Coin::COINSTATE_COLLECTED )
			continue;

		TMatrix44 matTransform;
		rObject.pObject->GetModelInstance()->GetLocalTransform( matTransform );

		TVector4 vTranslation = matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION );
		TFLOAT   fY           = a_fDeltaTime * rObject.fVelocity + vTranslation.y;

		rObject.fFallTime += a_fDeltaTime;
		rObject.fVelocity = rObject.fFallTime * a_fDeltaTime * 20.0f + rObject.fVelocity;

		if ( fY > rObject.vPosition.y && rObject.fVelocity > 0.0f && rObject.iState != Coin::COINSTATE_VANISHING )
		{
			// Bounce
			fY = rObject.vPosition.y;

			if ( rObject.iState == Coin::COINSTATE_ENDING )
				rObject.fVelocity = rObject.fVelocity * -0.5f;
			else
				rObject.fVelocity = rObject.fVelocity * -0.25f;

			rObject.fFallTime = 0.0f;
		}

		if ( rObject.iState == Coin::COINSTATE_VANISHING && rObject.vPosition.y + 2.0f < fY )
		{
			rObject.pObject->SetSkeletonUpdating( TFALSE, TTRUE );
			rObject.iState = Coin::COINSTATE_FREE;
		}

		matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = TVector4( vTranslation.x, fY, vTranslation.z, vTranslation.w );
		matTransform.RotateZ( 5.5f * a_fDeltaTime );
		rObject.pObject->GetModelInstance()->SetTransform( matTransform );

		rObject.fTimeLeft -= a_fDeltaTime;

		if ( m_fBounceTime - m_fBounceFallTime > rObject.fTimeLeft )
		{
			if ( rObject.fTimeLeft >= 0.0f )
			{
				if ( rObject.iState != Coin::COINSTATE_ENDING )
				{
					rObject.iState    = Coin::COINSTATE_ENDING;
					rObject.fVelocity = -1.0f;
				}
			}
			else
			{
				rObject.iState = Coin::COINSTATE_VANISHING;
			}
		}
	}
}

// Updates the runner of the final mission that comes out of its pipe and tips the cows
// $Barnyard: FUNCTION 004bc360
void ACowTippingMiniGameState::UpdateRunner( TFLOAT a_fUnk1 )
{
	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode != Stage::STAGETYPE_JUNKYARD && iMode != Stage::STAGETYPE_JUNKYARD_BOSS )
		return;

	AAnimatableObject* pPipe = m_pWildMikeBox;

	if ( m_eRunnerState == RUNNERSTATE_PIPE_UP && ( pPipe->IsAnimPlaying( TPS8( up ), 0.0f ) || !pPipe->IsAnimationPlanned( TPS8( up ) ) ) )
		pPipe->SetSkeletonUpdating( TTRUE, TTRUE );

	if ( !IsModelUpdatingSkeleton( pPipe ) )
		return;

	m_fRunnerPipeTimer -= a_fUnk1;

	if ( m_eRunnerState == RUNNERSTATE_PIPE_UP && IsModelUpdatingSkeleton( pPipe ) && m_fRunnerPipeTimer < 0.0f )
	{
		pPipe->PlayAnimation( TPS8( lower ) );
		m_eRunnerState = RUNNERSTATE_PIPE_CLOSING;
		m_oRunnerIcon.SetShown( TFALSE );
	}

	AAnimatedUnit* pRunner = m_pWildMike;

	TVector4 vRunnerPos;
	pRunner->GetModelInstance()->GetPosition( vRunnerPos );

	// Tip the cows the runner touches
	if ( pRunner->IsVisible() && TUINT( m_eRunnerState ) > RUNNERSTATE_COMING_OUT )
	{
		ACowTippingMiniGameState::Enemy* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, &m_llActiveEnemies );

		for ( ACowTippingMiniGameState::Enemy* pCow = pRoot->Next(); pCow != pRoot; )
		{
			ACowTippingMiniGameState::Enemy* pNext = pCow->Next();

			if ( pCow->eState != Enemy::ENEMYSTATE_INACTIVE && pCow->eState != Enemy::ENEMYSTATE_FADING && IsUnitNotYukking( TREINTERPRETCAST( TINT, pCow ) ) )
			{
				TFLOAT    fRadius = m_fRunnerTipRadius;
				TVector4  vCowPos;
				TVector4* pCowPos = TSTATICCAST( TVector4, GetUnitTargetPos( &vCowPos, TREINTERPRETCAST( TINT, pCow ) ) );

				if ( fRadius * fRadius > TVector4::DistanceSqXZ( *pCowPos, vRunnerPos ) )
				{
					OnUnitHit( pCow, 100, 0, 1 );
					ASoundManager::GetSingleton()->PlayCue( soundbank::DAGBOSS_WILD_MIKE );
				}
			}

			pCow = pNext;
		}
	}

	switch ( m_eRunnerState )
	{
		case RUNNERSTATE_COMING_OUT:
			// Coming out of the pipe
			if ( ( pPipe->IsAnimPlaying( TPS8( out_start ), 0.0f ) && pPipe->GetAnimationTime( TPS8( out_start ) ) < 0.55f ) ||
			     pPipe->IsAnimationFinished( TPS8( out_start ) ) )
				ReleaseRunner();

			break;
		case RUNNERSTATE_CHASING:
		{
			// Chasing the cows
			m_fRunnerTime -= a_fUnk1;

			AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pRunner->GetSteer() );

			TVector4 vSteerPos;
			pSteer->CopyPosition( vSteerPos );

			ACowTippingMiniGameState::Enemy* pClosest = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, FindClosestEnemyXZ( TREINTERPRETCAST( TINT, &vSteerPos ), 1000.0f ) );

			if ( pClosest )
			{
				ACowTippingMiniGameState::Enemy* pTarget = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, m_pRunnerTarget );

				if ( pClosest == pTarget )
				{
					pSteer->Command_CancelCurrentCommandNoStop();
					pSteer->SeekPosition( *pTarget->pUnit->GetSteer()->GetPosition() );
				}
				else
				{
					TBOOL bChangeTarget;

					if ( !pTarget )
					{
						bChangeTarget = TTRUE;
					}
					else
					{
						TVector4 vTargetPos   = *pTarget->pUnit->GetSteer()->GetPosition();
						TFLOAT   fTargetDist  = TVector4::DistanceXZ( vSteerPos, vTargetPos );
						TBOOL    bTargetLimit = GetCowTypeInt( pTarget ) <= pTarget->iHits;

						if ( pTarget->iNumTargeting > m_iAmmoPerTarget && !IsUnitNotYukking( TREINTERPRETCAST( TINT, pTarget ) ) )
						{
							bChangeTarget = TTRUE;
						}
						else if ( bTargetLimit )
						{
							bChangeTarget = TTRUE;
						}
						else
						{
							TVector4 vClosestPos = *pClosest->pUnit->GetSteer()->GetPosition();
							bChangeTarget        = fTargetDist * 0.9f > TVector4::DistanceXZ( vSteerPos, vClosestPos );
						}
					}

					if ( bChangeTarget )
					{
						pSteer->Command_CancelCurrentCommandNoStop();
						pSteer->Command_SeekUnit( pClosest->pUnit, 0.01f );
						m_pRunnerTarget = pClosest;
					}
				}
			}
			else if ( m_fRunnerOutTime - m_fRunnerChaseTime <= m_fRunnerTime )
			{
				if ( m_pRunnerTarget != 0 || !pSteer->IsFollowingPath() )
				{
					// Go to a random start goal
					TCHAR szGoal[ 32 ];
					TStringManager::String8Format( szGoal, sizeof( szGoal ), "%s%da", "start", ARandom::GetSingleton()->m_oRandom.GetInt( m_iNumCows - 1 ) );

					AInteriorGoal* pGoal = m_pNetwork->FindGoal( TPString8( szGoal ) );
					pGoal->GetPosition( m_vRunnerGoal );

					AInteriorSteer* pRunnerSteer = TSTATICCAST( AInteriorSteer, pRunner->GetSteer() );
					pRunnerSteer->Command_CancelCurrentCommandNoStop();
					pRunnerSteer->SeekPosition( m_vRunnerGoal );
					m_pRunnerTarget = 0;
				}
			}
			else
			{
				SendRunnerBack();
			}

			if ( m_fRunnerTime < 0.0f )
				SendRunnerBack();

			break;
		}
		case RUNNERSTATE_RETURNING:
			// Going back to the pipe
			if ( m_fExitRadius * m_fExitRadius > TVector4::DistanceSqXZ( m_vRunnerGoal, vRunnerPos ) )
			{
				AInteriorSteer* pSteer = TSTATICCAST( AInteriorSteer, pRunner->GetSteer() );
				pPipe->GetModelInstance()->GetPosition( m_vRunnerGoal );
				pSteer->Command_CancelCurrentCommandNoStop();
				pSteer->SeekPosition( m_vRunnerGoal );
				m_eRunnerState = RUNNERSTATE_ENTERING_PIPE;
			}

			break;
		case RUNNERSTATE_ENTERING_PIPE:
			if ( m_fExitRadius * m_fExitRadius > TVector4::DistanceSqXZ( m_vRunnerGoal, vRunnerPos ) )
			{
				HideRunner();
				pPipe->PlayAnimation( TPS8( in ) );
				m_eRunnerState = RUNNERSTATE_PIPE_CLOSING;
			}

			break;
		case RUNNERSTATE_PIPE_CLOSING:
			if ( pPipe->IsAnimationFinished( TPS8( in ) ) && pPipe->IsAnimationFinished( TPS8( lower ) ) )
			{
				StopRunnerPipe();
				m_eRunnerState = RUNNERSTATE_NONE;
			}

			break;
	}
}

// Ends the turn of the player (out of lives)
// $Barnyard: FUNCTION 004bcc20
void ACowTippingMiniGameState::EndTurnOutOfLives()
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return;

	ShowEndPopup( const_cast<TWCHAR*>( ALocaleManager::Interface()->GetString( 587 ) ) );
	APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );
	SendAllUnitsBackToStart();

	m_eTurnEnd      = TURNEND_OUT_OF_LIVES;
	m_fTurnEndTimer = m_fTurnEndDelay;
	AddLives1( TREINTERPRETCAST( TINT, &m_aPlayers[ m_iCurrentPlayer ] ), -1000 );

	m_oHudRoot.Hide();
	m_oRunnerIcon.SetShown( TFALSE );
	m_oElementPopup.SetShown( TFALSE );
	m_oRaccoonWarning.SetShown( TFALSE );
}

// Places the player and the camera at the start of the level
// $Barnyard: FUNCTION 004bed50
void ACowTippingMiniGameState::PlacePlayer()
{
	ReadCameraLocator();
	m_iThrowAnim = 0;

	TVector4& rvPosition   = m_vPlayerPos;
	TVector4& rvLocatorDir = m_vCameraLocatorDir;
	TVector4& rvCameraBase = m_vCameraBasePos;

	m_vPlayerVelocity = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
	rvPosition        = m_vPlayerCentre;

	TVector4 vOffset;
	vOffset.Multiply( rvLocatorDir, 100.0f );
	m_vAimTarget = TVector4( vOffset.x + rvCameraBase.x, vOffset.y + rvCameraBase.y, vOffset.z + rvCameraBase.z, rvCameraBase.w );

	vOffset.Multiply( rvLocatorDir, 12.0f );
	m_vReticleTarget = TVector4( vOffset.x + rvCameraBase.x, vOffset.y + rvCameraBase.y, vOffset.z + rvCameraBase.z, rvCameraBase.w );

	UpdateCamera( 1000.0f );

	const TVector4& rcCameraPos = m_vCameraPos;
	TVector4        vDir( rvPosition.x - rcCameraPos.x, 0.0f, rvPosition.z - rcCameraPos.z, rvPosition.w );

	vOffset.Multiply( m_vCameraDir, 0.5f );
	vDir.x += vOffset.x;
	vDir.y += vOffset.y;
	vDir.z += vOffset.z;
	vDir.Normalise();

	m_vAimDirection = TVector4( -vDir.x, -vDir.y, -vDir.z, vDir.w );

	static const TPString8* s_apIdleAnims[] = { &TPS8( idle_lp ), &TPS8( idle_lp2 ) };
	GetPlayerObject( this )->PlayAnimation( *s_apIdleAnims[ m_iThrowAnim ] );

	UpdatePlayerTransform( 1000.0f, 0 );
}

// Resets everything for the turn of the current player
// $Barnyard: FUNCTION 004c1ae0
void ACowTippingMiniGameState::StartTurn()
{
	if ( m_bEndCutscene )
	{
		AAnimatedUnit* pUnit = GetFirstFreeRaccoon( this )->pUnit;
		pUnit->GetModelInstance()->SetTransform( m_matDagTransform );
		pUnit->SetVisible( TFALSE );
		UnfreezeAfterEndCutscene();
		m_bEndCutscene = TFALSE;
	}

	LoadData();
	ResetGame();
	ResetCows();
	ResetTurn();
	DropHeldProjectile();

	TINT iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
	{
		DetachGridObjects();
		FUN_004b0c80();
	}

	ResetProjectiles();
	FUN_004b0be0();
	FUN_004b0c30();
	FUN_004ac660();

	for ( TINT i = TARRAYSIZE( m_aRaccoons ) - 1; i >= 0; i-- )
	{
		if ( m_aRaccoons[ i ].pUnit != 0 )
			DropKidTomato( TREINTERPRETCAST( TINT, &m_aRaccoons[ i ] ) );
	}

	PlaceOtis();
	m_eEndState = ENDSTATE_NONE;
	m_oTurnEndPopup.SetText( TNULL, 0x5500FF00, TNULL );

	for ( TINT i = 0; i < m_iNumCows; i++ )
	{
		AAnimatableObject* pObject = m_aCows[ i ].pObject;
		pObject->SetSkeletonUpdating( TTRUE, TTRUE );
		pObject->SetVisible( TTRUE );
		m_aCows[ i ].fTipProgress = 0;
		m_aCows[ i ].fUnk1C       = 0;
	}

	PlacePlayer();
	StartReload();
	ResetPlayer( m_iCurrentPlayer );
	m_oMilkReticleManager.SetActive( TFALSE );
	m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );
	m_oLives1Hud.SetLives( 0 );
	m_oLives2Hud.SetLives( 0 );
	m_oRaccoonWarning.SetShownInstantly( TFALSE );
	m_oRunnerIcon.SetShownInstantly( TFALSE );
	APositionalSoundSet::GetSingleton()->SetPaused( TFALSE );
	GetGUIElement().Show();
	m_oHudRoot.Show();
	GetProjectileObject( &m_aProjectiles[ 0 ] )->SetGlow( TFALSE );

	iMode = m_aStages[ m_iStage ].iStageType;

	if ( iMode == Stage::STAGETYPE_JUNKYARD_BOSS || iMode == Stage::STAGETYPE_JUNKYARD )
	{
		AAnimatedUnit* pRunner = m_pWildMike;
		pRunner->SetVisible( TFALSE );
		m_eRunnerState  = RUNNERSTATE_NONE;
		m_pRunnerTarget = 0;
		pRunner->ChangeSteer( &TGetClass( AStationarySteer ) );
	}

	m_bSubmitHighScore = TTRUE;
}

// The level is done: gives the bonus and goes to the next level. Returns TTRUE if it was the last level.
// $Barnyard: FUNCTION 004c1d10
TBOOL ACowTippingMiniGameState::CompleteLevel()
{
	m_bLevelDone = TFALSE;
	AddScore( m_aLevels[ m_aPlayers[ m_iCurrentPlayer ].iLevel ].iBonus );
	m_iScriptCommand = 0;
	m_iScriptLoops   = 0;

	TINT  iLevel     = ++m_aPlayers[ m_iCurrentPlayer ].iLevel;
	TBOOL bLast      = TFALSE;
	m_fLevelProgress = TFLOAT( iLevel ) / TFLOAT( m_iNumLevels + 1 );

	if ( iLevel >= m_iNumLevels || iLevel >= m_aStages[ m_iStage ].iNumLevels )
	{
		EndGame();
		bLast         = TTRUE;
		m_fLevelTimer = 1.0e9f;
	}
	else
	{
		TINT iMode = m_aStages[ m_iStage ].iStageType;

		if ( iMode != Stage::STAGETYPE_JUNKYARD_BOSS && iMode != Stage::STAGETYPE_JUNKYARD )
		{
			ShowLevelPopups( TTRUE );
			ASoundManager::GetSingleton()->PlayCue( soundbank::COWTIP_LEVEL );
		}

		m_fLevelTimer = m_fLevelTime;
	}

	m_aPlayers[ m_iCurrentPlayer ].iTurnShots = 0;
	m_aPlayers[ m_iCurrentPlayer ].iTurnHits  = 0;
	GetSingleton()->SetScore( m_aPlayers[ m_iCurrentPlayer ].iScore );

	return bLast;
}

// Ends the turn of the player (a_bNextPlayer: the next player plays) or the game
// $Barnyard: FUNCTION 004c1e40
void ACowTippingMiniGameState::EndTurn( TBOOL a_bNextPlayer )
{
	ClearUnitsFlag9();
	m_oMilkReticleManager.SetActive( TFALSE );

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return;

	SendAllUnitsBackToStart();
	m_fTimeLimit   = m_fTimeLimit;
	m_fElapsedTime = 0.0f;

	if ( m_iCurrentPlayer >= m_iNumPlayers - 1 || !a_bNextPlayer )
	{
		// Game over
		m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );
		m_oFloatHudElement.Hide();
		m_oLevelPopup.Hide();
		m_aBonusPopups[ 0 ].Hide();
		m_aBonusPopups[ 1 ].Hide();
		m_aBonusPopups[ 2 ].Hide();
		m_oEndPopup.Hide();
		m_oTurnEndPopup.Hide();
		m_oRaccoonWarning.SetShown( TFALSE );
		m_oRunnerIcon.SetShown( TFALSE );
		APositionalSoundSet::GetSingleton()->SetPaused( TTRUE );
		GetGUIElement().Hide();
		m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );

		ACowTippingMiniGameState::Enemy* pRoot = TREINTERPRETCAST( ACowTippingMiniGameState::Enemy*, &m_llActiveEnemies );

		for ( ACowTippingMiniGameState::Enemy* pCow = pRoot->Next(); pCow != pRoot; )
		{
			ACowTippingMiniGameState::Enemy* pNext = pCow->Next();

			if ( pCow->eType != Enemy::ENEMYTYPE_COYOTE )
			{
				pCow->pUnit->GetAnimatableObject()->PlayAnimation( TPS8( idle_lp ) );
				SendUnitBackToStart( TREINTERPRETCAST( TINT, pCow ) );
			}

			pCow = pNext;
		}

		for ( TINT i = 0; i < m_iNumCows; i++ )
			m_aCows[ i ].pObject->PlayAnimation( TPS8( idle_lp ) );

		DropHeldProjectile();
		ResetProjectiles();
		m_bGameOverPending = TTRUE;

		TINT iMode = m_aStages[ m_iStage ].iStageType;

		if ( ( iMode == Stage::STAGETYPE_JUNKYARD || iMode == Stage::STAGETYPE_JUNKYARD_BOSS ) && a_bNextPlayer )
		{
			m_iGameOverStep = 0;
			m_bGameOver     = TTRUE;
			return;
		}

		AAnticGameOverState* pGameOverState = new AAnticGameOverState( GetResults(), TFALSE );
		AMusicManager::GetSingleton()->SetVolume( m_fSavedMusicVolume );
		AddChildState( pGameOverState );
	}
	else
	{
		// Next player
		m_iCurrentPlayer++;
		StartTurn();
		m_oTargetReticle.SetActive( TFALSE, TTRUE, 50.0f );
		m_oFloatHudElement.Hide();
		m_oLevelPopup.Hide();
		m_aBonusPopups[ 0 ].Hide();
		m_aBonusPopups[ 1 ].Hide();
		m_aBonusPopups[ 2 ].Hide();
		m_oEndPopup.Hide();
		m_oTurnEndPopup.Hide();
		m_oRaccoonWarning.SetShown( TFALSE );
		m_oRunnerIcon.SetShown( TFALSE );
		OnSetupPlayers();
	}
}

// Checks if the level is done or the player is out of lives. Returns TTRUE if the turn of the player ended.
// $Barnyard: FUNCTION 004c2260
TBOOL ACowTippingMiniGameState::CheckTurnEnd( TFLOAT a_fUnk1 )
{
	if ( m_eTurnEnd != TURNEND_NONE )
		return TFALSE;

	m_fLevelTimer -= a_fUnk1;
	TBOOL bLastLevel = TFALSE;

	if ( m_bLevelDone || ( m_fLevelTimer < 0.0f && m_fLevelTime < 10000.0f ) )
		bLastLevel = CompleteLevel();

	if ( !bLastLevel )
	{
		PlayerData& rPlayer = m_aPlayers[ m_iCurrentPlayer ];

		if ( rPlayer.iLives <= 0 || rPlayer.iLives2 <= 0 )
		{
			EndTurnOutOfLives();
			return TTRUE;
		}
	}

	return bLastLevel;
}

// Once the game over state is gone, restarts the game for the next player (or from the first one)
// $Barnyard: FUNCTION 004c3240
TBOOL ACowTippingMiniGameState::UpdateRestart()
{
	if ( m_bGameOverPending != TTRUE )
		return TFALSE;

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return TTRUE;

	StartTurn();

	if ( ++m_iCurrentPlayer < m_iNumPlayers )
	{
		StartTurn();
		m_oTargetReticle.SetActive( TTRUE, TTRUE, 50.0f );
		m_oFloatHudElement.Hide();
		m_oLevelPopup.Hide();
		m_aBonusPopups[ 0 ].Hide();
		m_aBonusPopups[ 1 ].Hide();
		m_aBonusPopups[ 2 ].Hide();
		m_oEndPopup.Hide();
		m_oTurnEndPopup.Hide();
	}
	else
	{
		m_iCurrentPlayer = 0;
		m_oTargetReticle.SetActive( TTRUE, TTRUE, 50.0f );
		m_oFloatHudElement.Hide();
		m_oLevelPopup.Hide();
		m_aBonusPopups[ 0 ].Hide();
		m_aBonusPopups[ 1 ].Hide();
		m_aBonusPopups[ 2 ].Hide();
		m_oEndPopup.Hide();
		m_oTurnEndPopup.Hide();
		m_oLives2Hud.SetLives( 0 );

		for ( TINT i = 0; i < m_iNumPlayers; i++ )
		{
			PlayerData& rPlayer    = m_aPlayers[ i ];
			rPlayer.iLives         = 0;
			rPlayer.iScore         = 0;
			rPlayer.iPowerUp0Shots = 0;
			rPlayer.iLives2        = 0;
			rPlayer.iHits          = 0;
			rPlayer.iShots         = 0;
			rPlayer.iTurnShots     = 0;

			rPlayer.iLives = m_aStages[ m_iStage ].iLives1;
			m_oLives1Hud.SetLives( m_aStages[ m_iStage ].iLives1 );
			rPlayer.iLives2 = m_aStages[ m_iStage ].iLives2;
			m_oLives2Hud.SetLives( m_aStages[ m_iStage ].iLives2 );

			ResetPowerUps( i );
		}
	}

	m_bGameOverPending = TFALSE;
	OnSetupPlayers();

	return TTRUE;
}
