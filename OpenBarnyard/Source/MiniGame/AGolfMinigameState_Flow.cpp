#include "pch.h"
#include <Render/TCameraObject.h>
#include "AGolfMinigameState.h"
#include "Helpers/APlayerProgress.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/ABoneAttachCameraHelper.h"
#include "Cameras/AMultiCameraHelper.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GameInterface/AGameStateController.h"
#include "GUI/AFadeManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Render/ARenderer.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/golf.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AGolfGopherUnit.h"
#include "UnitMovement/Steering/ABallSteer.h"
#include "UnitMovement/Steering/AGopherMoundSteer.h"
#include "ARandom.h"

#include <Math/TMathInline.h>
#include <Toshi/T2String8.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

#ifdef BARNYARD_LATEST
// Latest version: the time limit of each hole number (1..18). The Wii golfdata.trb raises them
// (PC: 45/60/45/45/45/60/60/60/90, 60/120/60/60/60/90/145/160/145). We load the PC data, so they are set here.
// They make golf easier (made for the Wii controls), so they're only used on Easy, where the Easy x1.1 of
// SetupPlayerTurn applies on top of them (as on the Wii, whose Easy scales its raised limits).
static constexpr TFLOAT s_afHoleTimeLimits[ 18 ] = {
	90.0f, 90.0f, 60.0f, 50.0f, 50.0f, 70.0f, 70.0f, 70.0f, 100.0f,
	70.0f, 130.0f, 70.0f, 70.0f, 70.0f, 100.0f, 160.0f, 180.0f, 160.0f
};
#endif

TPSTRING8_DECLARE( idle );
TPSTRING8_DECLARE( idle_lp );
TPSTRING8_DECLARE( idle_lp2 );
TPSTRING8_DECLARE( idle_lp3 );
TPSTRING8_DECLARE( golf_start );
TPSTRING8_DECLARE( golf_start_attach );
TPSTRING8_DECLARE( golf_start_detach );
TPSTRING8_DECLARE( golf_swingbk );
TPSTRING8_DECLARE( golf_swingfg );
TPSTRING8_DECLARE( golf_swingth );
TPSTRING8_DECLARE( golf_cam_loc );
TPSTRING8_DECLARE( golf_tee_hit );
TPSTRING8_DECLARE( win );
TPSTRING8_DECLARE( lose );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( c_l_upperarm );
TPSTRING8_DECLARE( c_r_upperarm );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( cb_llocator );

// "Hole %d" (formatted with the hole number)
static constexpr TINT LOCALE_HOLE_NUMBER = 513;

// Sound of the ball hitting an obstacle, by hole number (ABallSteer::SetObstacleHitSound)
static const ASoundId s_aHoleObstacleHitSounds[ AGolfMinigameState::MAX_HOLES + 1 ] = {
	1,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_STONE,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
	soundbank::GOLF_HIT_WOOD01,
};

// Launch angle of the tee shot (flt_78281C, initialised at startup)
static TFLOAT s_fTeeShotAngle = TMath::PI * 0.25f;

// The unit of a proxy (TNULL if there's no proxy)
// NOTE: inlined everywhere in retail
static AAnimatedUnit* GetAnimatedUnit( const TRef<AUnitProxy>& a_rcProxy )
{
	return a_rcProxy ? TSTATICCAST( AAnimatedUnit, a_rcProxy->GetUnit() ) : TNULL;
}

// Normalises the vector on the XZ plane (y = 0), (0, 0, 1) if it's zero. Returns the original XZ length.
// NOTE: same as the static NormaliseXZ of AGTAStyleCameraHelper.cpp (TVector4 method 006c7540)
static TFLOAT NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return fMagSq;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;

	return fMag;
}

// $Barnyard: FUNCTION 00478800
TBOOL AGolfMinigameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_bHoleLoaded )
	{
		m_oStartHudElement.SetForceHide( TTRUE );
		TRenderInterface::GetSingleton()->FlushDyingResources();
		LoadHole();
		m_bHoleLoaded = TTRUE;
		AMiniGameState::OnStartHudFinished();
	}

	if ( m_bReloadHole == TTRUE )
	{
		m_bReloadHole = TFALSE;
		LoadHole();
		return TTRUE;
	}

	UpdateHud( a_fDeltaTime );

	// Spin the bucks of the charity variant
	if ( m_pCourse->bCharity )
	{
		for ( TINT i = 0; i < m_iNumBucks; i++ )
		{
			TMatrix44 matTransform;
			m_apBucks[ i ]->GetModelInstance()->GetLocalTransform( matTransform );

			TVector4 vTranslation = matTransform.GetTranslation();
			matTransform.Identity();
			matTransform.RotateY( m_fElapsedTime + m_fElapsedTime );
			matTransform.RotateX( TMath::HALF_PI );
			matTransform.GetTranslation() = vTranslation;

			m_apBucks[ i ]->GetModelInstance()->SetTransform( matTransform );
		}
	}

#ifdef BARNYARD_COMMUNITY_PATCH
	DevAutoPlay( a_fDeltaTime );
#endif

	AAnimatableObject* pPlayerObject = GetAnimatedUnit( m_pPlayerUnit )->GetAnimatableObject();

	switch ( m_eGameState )
	{
		case GAMESTATE_INTRO:
		{
			// The camera flies over the hole, the swing button speeds it up
			ABoneAttachCameraHelper* pCameraHelper = TSTATICCAST( ABoneAttachCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH ) );

			if ( pCameraHelper->IsAnimationPlaying() )
			{
				Player& rPlayer = GetCurrentPlayer();

				if ( rPlayer.bSwingPressed )
					pCameraHelper->SetAnimationSpeed( 4.0f );
				else if ( rPlayer.bSwingReleased )
					pCameraHelper->SetAnimationSpeed( 1.5f );
			}
			else
			{
				m_oTextBox.Show();
				m_eGameState = GAMESTATE_TEE;
			}

			break;
		}
		case GAMESTATE_TEE:
		{
			static TFLOAT s_fNumIdleAnims   = 4.0f;
			static TFLOAT s_fIdle3Threshold = ( s_fNumIdleAnims - 1.0f ) / s_fNumIdleAnims;
			static TFLOAT s_fIdle2Threshold = 2.0f / s_fNumIdleAnims;

			// Random idle animations while waiting
			if ( !pPlayerObject->GetToshiAnimInterface().GetAnim() )
			{
				// NOTE: retail picks the delay between 0 and 0, so a new idle starts right after the last one
				if ( 0.0f == m_fTimer )
					m_fTimer = ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 0.0f, 0.0f );

				m_fIdleTimer += a_fDeltaTime;

				if ( m_fIdleTimer > m_fTimer )
				{
					TFLOAT fRandom = ARandom::GetSingleton()->m_oRandom.GetFloat();

					if ( fRandom > s_fIdle3Threshold )
						pPlayerObject->PlayAnimation( TPS8( idle_lp3 ) );
					else if ( fRandom > s_fIdle2Threshold )
						pPlayerObject->PlayAnimation( TPS8( idle_lp2 ) );
					else
						pPlayerObject->PlayAnimation( TPS8( idle_lp ) );

					m_fIdleTimer = 0.0f;
					m_fTimer     = 0.0f;
				}
			}

			if ( GetCurrentPlayer().bSwingPressed == TTRUE )
			{
				pPlayerObject->PlayAnimation( TPS8( golf_swingbk ) );
				m_oSwingListener.Disconnect();

				// Put the ball on the tee
				AAnimatedUnit* pBall = GetAnimatedUnit( GetCurrentPlayer().pBall );

				if ( pBall->GetAnimatableObject()->GetParentObject() )
					AAnimatableObjectManager::GetSingleton()->DetachObject( pBall->GetAnimatableObject(), pPlayerObject );

				pBall->SetVisible( TTRUE );
				pBall->SetSteerPosition( m_vPlayerPosition );

				m_eGameState                     = GAMESTATE_POWER_UP;
				GetCurrentPlayer().bSwingPressed = TFALSE;
				GetCurrentPlayer().fPower        = 0.0f;

				m_oPowerBar.m_fTargetValue = 0.0f;
				m_oPowerBar.SetTargetBandStart( -1.0f );
				m_oPowerBar.SetBracketPos( 0.2f );
				m_oPowerBar.SetBracket2Pos( 0.8f );
				m_oPowerBar.SetForceHide( TFALSE );
				m_oPowerBar.SetBracketsVisible( TFALSE );
				m_oPowerBar.SetArrowTarget( 1 );
				m_oPowerBar.SetResult( TUINT( -1 ), TFALSE );
				m_oPowerBar.SetText( TNULL, -1.0f, 0.0f );
				m_oPowerBar.StartFade( -1.0f, 0.0f, -1.0f );

				m_oTextBox.Hide();
			}

			break;
		}
		case GAMESTATE_POWER_UP:
		{
			Player& rPlayer = GetCurrentPlayer();

			if ( rPlayer.fPower >= 1.0f || ( rPlayer.fPower >= 0.5f && rPlayer.bSwingPressed == TTRUE ) )
			{
				// Power is set, the error is the distance from the upper bracket
				m_oPowerBar.SetArrowTarget( 0 );
				m_fPowerError = TMath::Abs( GetCurrentPlayer().fPower - m_oPowerBar.m_fBracket2Pos );
				m_oPowerBar.SetResult( 0, m_fPowerError < 0.05f );
				m_eGameState = GAMESTATE_POWER_DOWN;
			}
			else
			{
				rPlayer.fPower += a_fDeltaTime;
				GetCurrentPlayer().fPower  = TMath::Min( GetCurrentPlayer().fPower, 1.0f );
				m_oPowerBar.m_fTargetValue = GetCurrentPlayer().fPower;
			}

			break;
		}
		case GAMESTATE_POWER_DOWN:
		{
			Player& rPlayer = GetCurrentPlayer();

			if ( rPlayer.fPower <= 0.0f || ( rPlayer.fPower <= 0.5f && rPlayer.bSwingPressed == TTRUE ) )
			{
				// Accuracy is set, the error is the distance from the lower bracket
				m_oPowerBar.SetArrowTarget( -1 );
				TFLOAT fAccuracyError = TMath::Abs( GetCurrentPlayer().fPower - m_oPowerBar.m_fBracketPos );
				m_oPowerBar.SetResult( 1, fAccuracyError < 0.05f );

				if ( m_fPowerError >= 0.05f || fAccuracyError >= 0.05f )
					m_fPowerError = fAccuracyError + m_fPowerError;
				else
					m_fPowerError = 0.0f;

				m_oPowerBar.StartFade( 1.0f, 3.0f, -1.0f );
				pPlayerObject->PlayAnimation( TPS8( golf_swingfg ) );
				m_eGameState = GAMESTATE_SWING;
			}
			else
			{
				rPlayer.fPower -= a_fDeltaTime;
				GetCurrentPlayer().fPower = TMath::Max( GetCurrentPlayer().fPower, 0.0f );
				m_oPowerBar.SetTargetBandStart( GetCurrentPlayer().fPower );
			}

			break;
		}
		case GAMESTATE_SWING:
		{
			if ( pPlayerObject->IsAnimationFinished( TPS8( golf_swingfg ) ) )
			{
				pPlayerObject->PlayAnimation( TPS8( golf_swingth ) );
				m_eGameState = GAMESTATE_TEE_FLIGHT;
				m_fTimer     = 0.0f;

				// The smaller the error, the better the zone the ball flies to
				TINT iAccuracy;

				if ( 0.0f == m_fPowerError )
					iAccuracy = 3;
				else if ( m_fPowerError < 0.15f )
					iAccuracy = 2;
				else if ( m_fPowerError < 0.35f )
					iAccuracy = 1;
				else
					iAccuracy = 0;

				// Each zone of the accuracy is used once per hole
				TINT iZone;
				do
				{
					iZone = ARandom::GetSingleton()->m_oRandom.GetInt( 4 );
				} while ( m_aUsedZones[ iAccuracy ] & TUINT8( 1 << iZone ) );

				m_aUsedZones[ iAccuracy ] |= TUINT8( 1 << iZone );

				TString8 strZone;
				strZone.Format( "zone%1d_%02d", iAccuracy + 1, iZone + 1 );
				TVector4 vZonePos = ATerrainInterface::GetSingleton()->GetLocatorPos( strZone.GetString() );

				ABallSteer* pSteer = TSTATICCAST( ABallSteer, GetAnimatedUnit( GetCurrentPlayer().pBall )->GetSteer() );

				TVector4 vBallPos;
				pSteer->CopyPosition( vBallPos );

				// Speed of the shot at s_fTeeShotAngle that lands at the zone
				static TFLOAT s_fCos = TMath::Cos( s_fTeeShotAngle );
				static TFLOAT s_fSin = TMath::Sin( s_fTeeShotAngle );

				TVector4 vDelta;
				vDelta.x = vZonePos.x - vBallPos.x;
				vDelta.y = vZonePos.y - vBallPos.y;
				vDelta.z = vZonePos.z - vBallPos.z;
				vDelta.w = vZonePos.w;

				TFLOAT fDistanceSq = vDelta.x * vDelta.x + vDelta.z * vDelta.z;
				TFLOAT fSpeedSq    = 9.8f * fDistanceSq * -0.5f / ( ( -vDelta.y - sqrtf( fDistanceSq ) / s_fCos * s_fSin ) * s_fCos * s_fCos );

				TVector4 vVelocity = vDelta;
				NormaliseXZ( vVelocity );
				pSteer->SetDirection( vVelocity );

				vVelocity.Multiply( s_fCos );
				vVelocity.y = -s_fSin;
				vVelocity.Multiply( sqrtf( fSpeedSq ) );
				pSteer->SetVelocity( vVelocity );

				// Ball physics of the hole
				const HoleData* pHoleData = GetHoleData();
				TFLOAT          fValue;

				fValue = pHoleData->fBounce;
				if ( 0.0f == fValue ) fValue = 0.25f;
				pSteer->SetBounce( fValue );

				fValue = GetHoleData()->fMinBounceRatio;
				if ( 0.0f == fValue ) fValue = 0.2f;
				pSteer->SetMinBounceRatio( fValue );

				fValue = GetHoleData()->fRollingFriction;
				if ( 0.0f == fValue ) fValue = 0.3f;
				pSteer->SetRollingFriction( fValue );

				fValue = GetHoleData()->fBounceFriction;
				if ( 0.0f == fValue ) fValue = 0.4f;
				pSteer->SetBounceFriction( fValue );

				pSteer->SetPlayHitSounds( TTRUE );
				pSteer->SetObstacleHitSound( s_aHoleObstacleHitSounds[ m_pCourse->aHoles[ m_iCurrentHole ] ] );

				// Follow the ball
				ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_GOLF );
				ACameraManager::GetSingleton()->GetCurrentCameraHelper()->SetTarget( GetCurrentPlayer().pBall );

				ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::GOLF_GOLFHIT, vBallPos );
			}

			break;
		}
		case GAMESTATE_TEE_FLIGHT:
		{
			ASteer* pSteer = GetAnimatedUnit( GetCurrentPlayer().pBall )->GetSteer();

			// The ball fell off the world, the player tees off again
			if ( pSteer->GetPosition()->y > 10.0f )
			{
				m_iCurrentPlayer--;
				NextPlayer();
			}

			m_fTimer += a_fDeltaTime;

			if ( pSteer->GetSpeed() < 0.2f || m_fTimer > 12.0f )
			{
				// Stop the ball
				TVector4 vPosition = *pSteer->GetPosition();
				pSteer->SetPosition( vPosition );

				m_fTimer = 0.0f;
				m_fIdleTimer += a_fDeltaTime;

				if ( m_fIdleTimer > 1.0f )
				{
					m_oPowerBar.SetForceHide( TTRUE );
					m_fIdleTimer = 0.0f;

					if ( NextPlayer() == TTRUE )
					{
						m_eGameState = GAMESTATE_PLAY;
						StartPlay();
					}
				}
			}

			break;
		}
		case GAMESTATE_PLAY:
			UpdatePlay( a_fDeltaTime );
			break;
		case GAMESTATE_END:
		{
			// The camera looks through the "golf_cam_loc" bone of the player
			AAnimatableObject* pObject        = GetAnimatedUnit( m_pPlayerUnit )->GetAnimatableObject();
			AModelInstance*    pModelInstance = pObject->GetModelInstance();

			TMatrix44 matBone;
			pModelInstance->GetBoneTransform( pModelInstance->GetBoneID( TPS8( golf_cam_loc ) ), matBone );

			TVector4 vPosition = matBone.GetTranslation();
			TVector4 vDirection( matBone.m_f31, matBone.m_f32, matBone.m_f33, matBone.m_f34 );
			vDirection.Normalise();
			vDirection.x = -vDirection.x;
			vDirection.w = 1.0f;
			vDirection.y = -vDirection.y;
			vDirection.z = -vDirection.z;

			ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
			pCamera->GetMatrix().GetTranslation() = vPosition;
			pCamera->LookAtDirection( vDirection );
			pCamera->SetFOV( 1.2217305f );

			if ( pObject->IsAnimationFinished( TPS8( win ) ) && pObject->IsAnimationFinished( TPS8( lose ) ) )
			{
				GetAnimatedUnit( m_aPlayers[ 0 ].pBall )->GetSteer()->SetPosition( TVector4::VEC_ZERO );
				AGameStateController::GetSingleton()->PushState( new AAnticGameOverState( GetResults(), TFALSE ) );
			}

			break;
		}
		default:
			break;
	}

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		m_aPlayers[ i ].bSwingPressed  = TFALSE;
		m_aPlayers[ i ].bSwingReleased = TFALSE;
	}

	m_fElapsedTime += a_fDeltaTime;
	m_bCommand31 = TFALSE;
	m_bCommand30 = TFALSE;

	// NOTE: skips AMiniGameState::OnUpdate
	return AGameState::OnUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00471bf0
void AGolfMinigameState::UpdateHud( TFLOAT a_fDeltaTime )
{
	m_oInstructionsText.Hide();

	if ( m_eGameState == GAMESTATE_PLAY )
	{
		for ( TINT i = 0; i < m_iNumPlayers; i++ )
		{
			Player& rPlayer = m_aPlayers[ i ];

			if ( m_bPlayFinished || rPlayer.iBallState == BALLSTATE_HELD || rPlayer.iBallState == BALLSTATE_HOLED )
			{
				rPlayer.oReticle.SetVisible( TFALSE );
			}
			else
			{
				rPlayer.oReticle.SetVisible( TTRUE );

				// Show the instructions while a ball is off the screen
				if ( rPlayer.oReticle.Update( *GetAnimatedUnit( rPlayer.pBall )->GetSteer()->GetPosition(), a_fDeltaTime ) && !m_bCommand31 )
					m_oInstructionsText.Show();

				if ( rPlayer.iBallState != BALLSTATE_ON_COURSE )
				{
					rPlayer.oReticle.SetLocked( TFALSE );
				}
				else
				{
					rPlayer.fWaterTime += a_fDeltaTime;
					rPlayer.oReticle.SetLocked( rPlayer.fWaterTime > 1.0f );
				}
			}
		}

		if ( !m_bPlayFinished )
		{
			if ( m_oFlagReticle.Update( m_vHolePosition, a_fDeltaTime ) && !m_bCommand31 )
				m_oInstructionsText.Show();

			m_oFlagReticle.SetLocked( m_iNumPlayers == 1 && m_aPlayers[ 0 ].iBallState == BALLSTATE_HELD );
		}

		if ( !m_pCourse->bStrokeLimit && !m_oTimer.IsPaused() )
			m_oTimer.SetTime( m_fTimeLimit - m_fElapsedTime );
	}

	m_oPopup.Update( a_fDeltaTime );

	if ( m_iNumPlayers == 1 )
	{
		// Fade the timer/strokes box while the ball (or the gopher) is behind it
		m_pTimerOrStrokes->SetAlpha( 1.0f );

		AAnimatedUnit* pUnit     = GetAnimatedUnit( m_aPlayers[ 0 ].iGopherState != GOPHERSTATE_FREE ? m_aPlayers[ 0 ].pGopher : m_aPlayers[ 0 ].pBall );
		TVector4       vPosition = *pUnit->GetSteer()->GetPosition();

		TVector2 vScreenPos;
		if ( ARenderer::GetSingleton()->ProjectToGUI( ACameraManager::GetSingleton()->GetCurrentCamera(), vPosition, vScreenPos ) )
		{
			ARenderer::GetSingleton()->ScreenToGUIViewport( vScreenPos, vScreenPos );

			// NOTE: retail gets the size of the GUI viewport and doesn't use it
			TVector2 vGUISize;
			ARenderer::GetSingleton()->GetGUIViewportSize( vGUISize );

			TFLOAT fMaxX;
			TFLOAT fMaxY;
			m_pTimerOrStrokes->GetDimensions( fMaxX, fMaxY );

			const TVector2& rcTranslation = m_pTimerOrStrokes->GetTransform().GetTranslation();
			fMaxX += rcTranslation.x + 16.0f;
			fMaxY += rcTranslation.y + 16.0f;

			if ( vScreenPos.x > 0.0f && vScreenPos.x < fMaxX && vScreenPos.y > 0.0f && vScreenPos.y < fMaxY )
				m_pTimerOrStrokes->SetAlpha( 0.3f );
		}
	}

	// Fade the hole text in, keep it for m_fHoleTextTime and fade it out
	if ( m_oHoleText.IsVisible() )
	{
		TFLOAT fAlpha = m_oHoleText.GetAlpha();

		if ( fAlpha < 1.0f && m_bHoleTextFadeIn == TTRUE )
		{
			fAlpha = a_fDeltaTime + a_fDeltaTime + fAlpha;
			TMath::Clip( fAlpha, 0.0f, 1.0f );

			if ( fAlpha >= 1.0f )
			{
				fAlpha            = 1.0f;
				m_bHoleTextFadeIn = TFALSE;
			}
		}
		else
		{
			m_fHoleTextTime -= a_fDeltaTime;

			if ( m_fHoleTextTime > 0.0f )
				return;

			m_fHoleTextTime = 0.0f;
			fAlpha          = fAlpha - ( a_fDeltaTime + a_fDeltaTime );

			if ( fAlpha < 0.0f )
			{
				m_oHoleText.Hide();
				m_wszHoleText[ 0 ] = L'\0';
				return;
			}
		}

		m_oHoleText.SetAlpha( fAlpha );
	}
}

// $Barnyard: FUNCTION 00474830
// $BarnyardWiiJP: FUNCTION 80194040
void AGolfMinigameState::SetupPlayerTurn()
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		rPlayer.oReticle.SetVisible( TFALSE );

		// Hide the gopher with the ball and its snorkel and floaties
		rPlayer.pGopherGolf->SetSkeletonUpdating( TFALSE, TTRUE );
		rPlayer.pGopherGolf->FindAttachedObject( TPS8( c_head ) )->SetSkeletonUpdating( TFALSE, TTRUE );
		rPlayer.pGopherGolf->FindAttachedObject( TPS8( c_l_upperarm ) )->SetSkeletonUpdating( TFALSE, TTRUE );
		rPlayer.pGopherGolf->FindAttachedObject( TPS8( c_r_upperarm ) )->SetSkeletonUpdating( TFALSE, TTRUE );
		rPlayer.oAnimListener.Disconnect();

		rPlayer.fWaterTime = 0.0f;
		rPlayer.pBallModel->SetSkeletonUpdating( TFALSE, TTRUE );

		if ( rPlayer.pBallModel->GetParentObject() )
			pAnimObjManager->DetachObject( rPlayer.pBallModel, rPlayer.pBallModel->GetParentObject() );

		// Hide the gopher in its hole near the flag
		AGolfGopherUnit* pGopher = TSTATICCAST( AGolfGopherUnit, GetAnimatedUnit( rPlayer.pGopher ) );
		pGopher->GetAnimatableObject()->PlayAnimation( TPS8( idle ) );
		pGopher->SetVisible( TFALSE );

		AGopherMoundSteer* pGopherSteer = TSTATICCAST( AGopherMoundSteer, pGopher->GetSteer() );
		pGopher->ResetTrail( *pGopherSteer->GetPosition() );
		pGopherSteer->SetStayHidden( TTRUE );
		pGopherSteer->SetObstacle( m_vHolePosition, 0.8f );

		// Hide the ball
		AAnimatedUnit* pBall = GetAnimatedUnit( rPlayer.pBall );
		pBall->GetSteer()->SetPosition( TVector4::VEC_ZERO );
		pBall->SetVisible( TFALSE );

		if ( pBall->GetAnimatableObject()->GetParentObject() )
			pAnimObjManager->DetachObject( pBall->GetAnimatableObject(), GetAnimatedUnit( m_pPlayerUnit )->GetAnimatableObject() );
	}

	StartFlyOver();

	m_oFlagReticle.SetVisible( TFALSE );
	m_pTimerOrStrokes->Hide();

	if ( m_iNumPlayers == 1 )
		m_oLifeHud.SetVisible( TFALSE );

	m_eGameState        = GAMESTATE_INTRO;
	m_fHoleEndTime      = 0.0f;
	m_fCourseOverTime   = 0.0f;
	m_bLoseLife         = TTRUE;
	m_bCourseOverCamera = TFALSE;
	m_iCurrentPlayer    = 0;
	m_bHoleEndNotified  = TFALSE;
	m_bPlayFinished     = TFALSE;
	m_oPlayListeners.Clear();

	m_fTimeLimit = m_pCourse->afTimeLimits[ m_iCurrentHole ];

#ifdef BARNYARD_LATEST
	// Courses with a stroke limit have no time limits (0)
	TINT iHoleNumber = m_pCourse->aHoles[ m_iCurrentHole ];

	if ( m_fTimeLimit > 0.0f && iHoleNumber >= 1 && iHoleNumber <= TARRAYSIZE( s_afHoleTimeLimits ) && APlayerProgress::GetSingleton()->IsEasyDifficulty() )
		m_fTimeLimit = s_afHoleTimeLimits[ iHoleNumber - 1 ];
#endif

#ifdef BARNYARD_LATEST
	// Latest version (Wii 80194040): more time on Easy
	if ( APlayerProgress::GetSingleton()->IsEasyDifficulty() )
		m_fTimeLimit *= 1.1f;
#endif

	if ( m_pCourse->bCharity )
	{
		// Place the bucks at the "coin_loc" locators
		m_iNumBucks = 0;

		ATerrainLocatorList* pLocatorList = ATerrainInterface::GetSingleton()->GetLocatorList();

		for ( TINT i = 0; i < pLocatorList->GetNumLocators(); i++ )
		{
			if ( T2String8::Compare( "coin_loc", pLocatorList->GetLocatorName( i ), 8 ) == 0 )
			{
				// NOTE: retail doesn't check if there are more than NUM_BUCKS locators
				m_apBucks[ m_iNumBucks ]->SetSkeletonUpdating( TTRUE, TTRUE );

				const ATerrainLocator* pLocator = pLocatorList->GetLocator( i );
				TVector4               vPosition( pLocator->vecPosition.x, pLocator->vecPosition.y, pLocator->vecPosition.z, 1.0f );

				// NOTE: retail uses the transform object as the matrix directly
				TMatrix44::TransformVector( vPosition, TREINTERPRETCAST( const TMatrix44&, pLocatorList->GetWorldTransform() ), vPosition );
				vPosition.y -= 0.5f;

				m_apBucks[ m_iNumBucks ]->GetModelInstance()->SetTransform( vPosition, TVector4::VEC_POSX );
				m_iNumBucks++;
			}
		}

		for ( TINT i = m_iNumBucks; i < NUM_BUCKS; i++ )
			m_apBucks[ i ]->SetSkeletonUpdating( TFALSE, TTRUE );

		m_iStartPickups = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetMiniGamePickups();
	}

	// "Hole N"
	TStringManager::String16Format( m_wszHoleText, TARRAYSIZE( m_wszHoleText ), ALocaleManager::Interface()->GetString( LOCALE_HOLE_NUMBER ), m_pCourse->aHoles[ m_iCurrentHole ] );
	m_oHoleText.SetText( m_wszHoleText );
	m_oHoleText.Show();
	m_fHoleTextTime   = 2.0f;
	m_bHoleTextFadeIn = TTRUE;

	*TREINTERPRETCAST( TUINT32*, m_aUsedZones ) = 0;

	SetupTee();
}

void AGolfMinigameState::StartFlyOver()
{
	// Fly over the hole, the camera ends behind the tee
	ABoneAttachCameraHelper* pCameraHelper = TSTATICCAST( ABoneAttachCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH ) );
	ACameraManager::GetSingleton()->SetCameraHelper( pCameraHelper );

	TString8 strCameraModel;
	strCameraModel.Format( "data\\models\\golf_hole_%d_camera", m_pCourse->aHoles[ m_iCurrentHole ] );
	TPString8 strPooledCameraModel( strCameraModel.GetString() );

	pCameraHelper->OnDetach();
	pCameraHelper->SetModel( strPooledCameraModel, "camera_base" );
	pCameraHelper->SetAnimation( "fly" );
	pCameraHelper->SetAnimationSpeed( 1.5f );
}

// NOTE: inlined into SetupPlayerTurn in retail. Split out so that, with 2+ players, the next player can tee off
// without setting the whole hole up again (see NextPlayer)
void AGolfMinigameState::SetupTee()
{
	// Place the player at the tee, facing "player_face"
	AAnimatedUnit* pPlayerUnit = GetAnimatedUnit( m_pPlayerUnit );
	pPlayerUnit->GetSteer()->SetPosition( m_vPlayerPosition );

	m_vPlayerDirection.x = m_vHolePosition.x - m_vPlayerPosition.x;
	m_vPlayerDirection.y = m_vHolePosition.y - m_vPlayerPosition.y;
	m_vPlayerDirection.z = m_vHolePosition.z - m_vPlayerPosition.z;
	m_vPlayerDirection.w = m_vHolePosition.w;
	NormaliseXZ( m_vPlayerDirection );

	TVector4 vFacePos = ATerrainInterface::GetSingleton()->GetLocatorPos( "player_face" );
	TVector4 vFaceDirection;
	vFaceDirection.x = vFacePos.x - m_vPlayerPosition.x;
	vFaceDirection.y = vFacePos.y - m_vPlayerPosition.y;
	vFaceDirection.z = vFacePos.z - m_vPlayerPosition.z;
	vFaceDirection.w = vFacePos.w;
	NormaliseXZ( vFaceDirection );

	GetAnimatedUnit( m_pPlayerUnit )->GetSteer()->SetDirection( vFaceDirection );

	Player& rPlayer      = GetCurrentPlayer();
	rPlayer.iBallState   = BALLSTATE_ON_COURSE;
	rPlayer.iHeldBall    = -1;
	rPlayer.iStrokes     = 0;
	rPlayer.iGopherState = GOPHERSTATE_FREE;
	rPlayer.fPower       = 0.0f;

	// Reset the tee
	if ( m_pTee->IsAnimPlaying( TPS8( golf_tee_hit ), 0.0f ) == TTRUE )
		m_pTee->SetAnimationTime( TPS8( golf_tee_hit ), 0.0f );

	m_pTee->KillAllAnimations();
	m_pTee->GetModelInstance()->SetTransform( m_vPlayerPosition, vFaceDirection );

	AAnimatableObject* pPlayerObject = GetAnimatedUnit( m_pPlayerUnit )->GetAnimatableObject();
	pPlayerObject->KillAllAnimations();
	pPlayerObject->PlayAnimation( TPS8( golf_start ) );

	m_oSwingListener.Disconnect();
	m_oSwingListener.Connect( pPlayerObject->GetAnimationEmitter( TPS8( golf_start_attach ) )->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnSwingAnimEventCallback ), 0 );

	if ( m_eGameState != GAMESTATE_INTRO )
	{
		m_eGameState = GAMESTATE_TEE;
		m_oTextBox.Show();
	}

	m_oPowerBar.SetForceHide( TTRUE );
	m_fElapsedTime = 0.0f;
	m_oTimer.Reset();

	// NOTE: the original passes a fourth argument (TTRUE) that AFadeManager::StartFade doesn't have
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 0.5f );
}

// $Barnyard: FUNCTION 00474360
void AGolfMinigameState::StartPlay()
{
	SetupHoleCamera();
	m_fElapsedTime = 0.0f;

	if ( m_pCourse->bStrokeLimit )
	{
		// NOTE: GetMaxStrokes is inlined here in retail
		TStringManager::String16Format( m_wszStrokes, TARRAYSIZE( m_wszStrokes ), L"%d", GetMaxStrokes() );
		m_oStrokesText.SetText( m_wszStrokes );
	}
	else
	{
		m_oTimer.SetDuration( m_fTimeLimit );
		m_oTimer.SetTime( m_fTimeLimit );
	}

	m_oFlagReticle.SetVisible( TTRUE );
	m_pTimerOrStrokes->Show();

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];
		rPlayer.oReticle.SetVisible( TTRUE );

		// The balls are played from where the tee shots landed
		AAnimatedUnit* pBall = GetAnimatedUnit( rPlayer.pBall );
		pBall->SetVisible( TTRUE );

		ABallSteer* pBallSteer = TSTATICCAST( ABallSteer, pBall->GetSteer() );
		pBallSteer->SetBounce( 0.7f );
		pBallSteer->SetMinBounceRatio( 0.2f );
		pBallSteer->SetRollingFriction( 0.3f );
		pBallSteer->SetBounceFriction( 0.0f );
		rPlayer.vBallPosition = *pBallSteer->GetPosition();

		// The gopher comes out at the ball
		AGolfGopherUnit* pGopher = TSTATICCAST( AGolfGopherUnit, GetAnimatedUnit( rPlayer.pGopher ) );
		TSTATICCAST( AGopherMoundSteer, pGopher->GetSteer() )->SetStayHidden( TFALSE );
		pGopher->SetVisible( TTRUE );

		TVector4 vBallPos = *pBallSteer->GetPosition();
		pGopher->SetSteerPosition( vBallPos );
		pGopher->ResetTrail( vBallPos );
		pGopher->SetSteerDirection( m_vPlayerDirection );
	}
}

// $Barnyard: FUNCTION 00473d60
void AGolfMinigameState::SetupHoleCamera()
{
	// Look from above, 30 degrees towards the player
	TVector4 vDirection = TVector4::VEC_POSY;
	TVector4 vAxis;
	vAxis.CrossProduct( m_vPlayerDirection, TVector4::VEC_NEGY );
	vDirection.RotateAroundAxis( vAxis, TMath::PI * 0.16666667f );

	ACameraManager::GetSingleton()->SetCameraHelper( ACameraManager::CAMERAHELPER_MULTICAMERA );
	AMultiCameraHelper* pCameraHelper = TSTATICCAST( AMultiCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_MULTICAMERA ) );
	pCameraHelper->RemoveAllTargets();

	if ( m_iNumPlayers == 1 )
	{
		// The gopher and the ball
		pCameraHelper->AddTarget( m_aPlayers[ 0 ].pGopher, TFALSE );
		pCameraHelper->AddTarget( m_aPlayers[ 0 ].pBall, TTRUE );
		m_bCameraOnBall = TTRUE;
	}
	else
	{
		// Every gopher and the flag
		for ( TINT i = 0; i < m_iNumPlayers; i++ )
			pCameraHelper->AddTarget( m_aPlayers[ i ].pGopher, TFALSE );

		pCameraHelper->AddTarget( m_vHolePosition, TTRUE );
	}

	pCameraHelper->SetViewDirection( vDirection );
	pCameraHelper->SetMinDistance( 12.0f );
	pCameraHelper->SetMaxDistance( 30.0f );
}

// $Barnyard: FUNCTION 00473d00
void AGolfMinigameState::OnPlayFinished()
{
	m_bPlayFinished = TTRUE;
	m_oPlayListeners.Clear();
	m_oFlagReticle.SetVisible( TFALSE );

	if ( m_bLoseLife )
	{
		m_oLifeHud.AddLives( -1 );
		m_oLifeHud.SetVisible( TTRUE );
	}
	else if ( m_aPlayers[ 0 ].iStrokes == 1 )
	{
		// Hole in one
		m_oLifeHud.AddLives( 1 );
		m_oLifeHud.SetVisible( TTRUE );
	}

	ACameraManager::GetSingleton()->DetachCameraHelpers();
}

// $Barnyard: FUNCTION 00473b30
TBOOL AGolfMinigameState::OnSwingAnimEvent( AAnimationBreakpointCallback* a_pBreakpoint, AAnimationBreakpointCallback::Event* a_pEvent )
{
	AAnimatableObject* pBallObject   = GetAnimatedUnit( GetCurrentPlayer().pBall )->GetAnimatableObject();
	AAnimatableObject* pPlayerObject = GetAnimatedUnit( m_pPlayerUnit )->GetAnimatableObject();

	if ( TPS8( golf_start_attach ) == a_pBreakpoint->GetName() )
	{
		// Put the ball in the hand of the player
		pBallObject->SetSkeletonUpdating( TTRUE, TTRUE );
		AAnimatableObjectManager::GetSingleton()->AttachObject( pBallObject, pPlayerObject, TPString8(), TPS8( cb_rlocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );

		m_oSwingListener.Disconnect();
		m_oSwingListener.Connect( pPlayerObject->GetAnimationEmitter( TPS8( golf_start_detach ) )->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnSwingAnimEventCallback ), 0 );
	}
	else if ( TPS8( golf_start_detach ) == a_pBreakpoint->GetName() )
	{
		// Put the ball on the tee
		AAnimatableObjectManager::GetSingleton()->DetachObject( pBallObject, pPlayerObject );
		m_oSwingListener.Disconnect();
		GetAnimatedUnit( GetCurrentPlayer().pBall )->GetSteer()->SetPosition( m_vPlayerPosition );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00474330
TBOOL AGolfMinigameState::OnSwingAnimEventCallback( AGolfMinigameState* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData )
{
	return a_pThis->OnSwingAnimEvent( a_pOwner, a_pData );
}

// $Barnyard: FUNCTION 00474350
TBOOL AGolfMinigameState::OnPlayFinishedCallback( AGolfMinigameState* a_pThis, void* a_pOwner, void* a_pData )
{
	a_pThis->OnPlayFinished();
	return TTRUE;
}

// $Barnyard: FUNCTION 00473f10
TBOOL AGolfMinigameState::OnGopherAnimEvent( AAnimationBreakpointCallback* a_pBreakpoint, AAnimationBreakpointCallback::Event* a_pEvent )
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		if ( a_pEvent->pObject != rPlayer.pGopherGolf )
			continue;

		// The gopher throws the ball out, unless the course is over and the gopher is hidden
		AModelInstanceRef pHeadInstance = rPlayer.pGopherGolf->FindAttachedObject( TPS8( c_head ) )->GetModelInstance();

		if ( ( pHeadInstance && pHeadInstance->IsUpdatingSkeleton() ) || !IsCourseOver() )
		{
			AAnimatedUnit* pBall = GetAnimatedUnit( rPlayer.pBall );
			pBall->SetVisible( TTRUE );
			pBall->GetSteer()->SetVelocity( rPlayer.vBallVelocity );
		}

		rPlayer.iBallState = BALLSTATE_ON_COURSE;

		TINT iBone = rPlayer.pGopherGolf->GetModelInstance()->GetBoneIndex( TPS8( cb_llocator ) );
		rPlayer.pGopherGolf->FindAttachedObject( iBone )->SetSkeletonUpdating( TFALSE, TTRUE );
		rPlayer.oAnimListener.Disconnect();

		break;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00474530
TBOOL AGolfMinigameState::OnGopherAnimEventCallback( AGolfMinigameState* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData )
{
	return a_pThis->OnGopherAnimEvent( a_pOwner, a_pData );
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Development: "-devautoplay" runs
void AGolfMinigameState::DevAutoPlay( TFLOAT a_fDeltaTime )
{
	if ( !T2String8::FindString( GetCommandLineA(), "-devautoplay" ) )
		return;

	static TFLOAT s_fWaitTimer  = 0.0f;
	static TINT   s_iLastState  = -1;
	static TINT   s_iLastPlayer = -2;

	s_fWaitTimer += a_fDeltaTime;

	if ( m_eGameState != s_iLastState || m_iCurrentPlayer != s_iLastPlayer )
	{
		TINFO( "[devautoplay] state %d -> %d (player %d, hole %d, power error %.3f)\n", s_iLastState, m_eGameState, m_iCurrentPlayer, m_iCurrentHole, m_fPowerError );
		s_iLastState  = m_eGameState;
		s_iLastPlayer = m_iCurrentPlayer;
		s_fWaitTimer  = 0.0f;
	}

	if ( m_iCurrentPlayer < 0 || m_iCurrentPlayer >= m_iNumPlayers )
		return;

	Player& rPlayer = GetCurrentPlayer();

	switch ( m_eGameState )
	{
		case GAMESTATE_TEE:
			// Swing a second after the tee is ready
			if ( s_fWaitTimer > 1.0f )
				rPlayer.bSwingPressed = TTRUE;
			break;
		case GAMESTATE_POWER_UP:
			// Stop just below the upper bracket
			if ( rPlayer.fPower + a_fDeltaTime >= m_oPowerBar.m_fBracket2Pos )
				rPlayer.bSwingPressed = TTRUE;
			break;
		case GAMESTATE_POWER_DOWN:
			// Stop just above the lower bracket
			if ( rPlayer.fPower - a_fDeltaTime <= m_oPowerBar.m_fBracketPos )
				rPlayer.bSwingPressed = TTRUE;
			break;
		default:
			break;
	}
}

#endif // BARNYARD_COMMUNITY_PATCH
