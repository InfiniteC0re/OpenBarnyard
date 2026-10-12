#include "pch.h"
#include "ABarnYardPoolMiniGameState.h"
#include "AOptions.h"
#include "Cameras/APoolCamera.h"
#include "FX/AParticleSystem.h"
#include "FX/AParticleEffectManager.h"
#include "GameInterface/AAnticGameOverState.h"
#include "GUI/AGUI2.h"
#include "Player/APlayerManager.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitPoolBall.h"
#include "Units/AUnitPoolCue.h"
#include "UnitMovement/Steering/APoolBallSteer.h"

#include <Input/TInputDeviceMouse.h>
#include <Math/TMath.h>
#include <Math/TMathInline.h>
#include <Toshi/TSystem.h>
#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Speed of the aim while the input is held: starts at MIN, goes up to MAX
static constexpr TFLOAT AIM_SPEED_MIN = 0.007f;
static constexpr TFLOAT AIM_SPEED_MAX = 0.05f;

// $Barnyard: FUNCTION 004d3c40
TBOOL ABarnYardPoolMiniGameState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// Shoot went down (and the power started to change)
	static TBOOL s_bShootPressed = TFALSE; // 0x007831d4

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) && a_eInputCommand == AInputCommand_Quit )
		return TTRUE;

	if ( FindChildState( &TGetClass( AAnticGameOverState ) ) )
		return TFALSE; // RETURNS_FALSE (006528c0)

	if ( !m_oStartHudElement.IsFinished() )
		return AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

	TBOOL bResult = TFALSE;
	TINT  iSlot   = APlayerManager::GetSingleton()->GetCurrentPlayerSet()->GetPlayerSlotIndexByDevice( a_pInputEvent->GetSource() );
	Team& rTeam   = GetCurrentTeam();

	if ( ( iSlot == rTeam.GetCurrentPlayer() ||
	       ( rTeam.GetCurrentPlayer() == PLAYER_AI && iSlot == rTeam.GetNextPlayer() ) ||
	       ( rTeam.GetCurrentPlayer() == PLAYER_AI && rTeam.GetNextPlayer() == PLAYER_AI ) ) &&
	     a_eInputCommand == AInputCommand_Select )
	{
		bResult = TTRUE;
	}

	if ( rTeam.GetCurrentPlayer() == PLAYER_AI )
	{
		AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
		return TTRUE;
	}

	if ( iSlot != rTeam.GetCurrentPlayer() )
	{
		// Only the first player can pause while another one plays
		if ( iSlot == 0 )
			AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

		return TTRUE;
	}

	if ( m_fInputDelay > 0.0f )
		return TTRUE;

	switch ( a_eInputCommand )
	{
		case AInputCommand_Move:
			if ( a_pInputEvent->GetSource()->IsA( &TGetClass( TInputDeviceMouse ) ) )
			{
				m_fMouseX = a_pInputEvent->GetMagnitudeFloat( 0 ) * AOptions::GetSingleton()->GetSensitivity();
				m_fMouseY = a_pInputEvent->GetMagnitudeFloat( 1 ) * AOptions::GetSingleton()->GetSensitivity();

				TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
				TFLOAT fScaleX    = 800.0f / AGUI2::GetRootElement()->GetWidth();
				TFLOAT fScaleY    = 600.0f / AGUI2::GetRootElement()->GetHeight();
				TFLOAT fInvDelta  = 1.0f / fDeltaTime;
				TFLOAT fX         = TMath::Pow( TMath::Abs( m_fMouseX ), 1.3f ) * fInvDelta * fScaleX;
				TFLOAT fY         = TMath::Pow( TMath::Abs( m_fMouseY ), 1.3f ) * fInvDelta * fScaleY;

				if ( m_fMouseX < 0.0f )
					fX = -fX;

				if ( m_fMouseY < 0.0f )
					fY = -fY;

				TFLOAT fFrameScale = fDeltaTime * 0.15f;
				fX *= fFrameScale;
				fY *= fFrameScale;

				// Changing the direction resets the speed
				if ( ( fX > 0.0f && m_fAimX < 0.0f ) || ( fX < 0.0f && m_fAimX > 0.0f ) )
					m_afAimSpeed[ 1 ] = AIM_SPEED_MIN;

				m_fAimX = fX * m_afAimSpeed[ 1 ];

				if ( ( fY > 0.0f && m_fAimY < 0.0f ) || ( fY < 0.0f && m_fAimY > 0.0f ) )
					m_afAimSpeed[ 0 ] = AIM_SPEED_MIN;

				m_fAimY = fY * m_afAimSpeed[ 0 ];

				m_abAimInput[ 1 ] = m_fAimX < -0.00001f || m_fAimX > 0.00001f;
				m_abAimInput[ 0 ] = m_fAimY < -0.00001f || m_fAimY > 0.00001f;
				m_bMouseAim       = TTRUE;
				m_pCamera->SetSpeed( 15.0f );
				return TTRUE;
			}
			else
			{
				TFLOAT fX = a_pInputEvent->GetMagnitudeFloat( 0 );
				TFLOAT fY = a_pInputEvent->GetMagnitudeFloat( 1 );

				if ( TMath::Abs( fX ) <= 0.0f && TMath::Abs( fY ) <= 0.0f )
				{
					m_bMouseAim = TFALSE;
					return TTRUE;
				}

				if ( ( fX > 0.0f && m_fAimX < 0.0f ) || ( fX < 0.0f && m_fAimX > 0.0f ) )
					m_afAimSpeed[ 1 ] = AIM_SPEED_MIN;

				m_fAimX = fX * m_afAimSpeed[ 1 ];

				if ( ( fY > 0.0f && m_fAimY < 0.0f ) || ( fY < 0.0f && m_fAimY > 0.0f ) )
					m_afAimSpeed[ 0 ] = AIM_SPEED_MIN;

				m_fAimY = fY * m_afAimSpeed[ 0 ];

				m_abAimInput[ 1 ] = m_fAimX < -0.0001f || m_fAimX > 0.0001f;
				m_abAimInput[ 0 ] = m_fAimY < -0.0001f || m_fAimY > 0.0001f;
				m_bMouseAim       = TFALSE;
				return TTRUE;
			}
		case AInputCommand_Left:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			bResult = TTRUE;

			if ( m_fAimX > 0.0f )
				m_afAimSpeed[ 1 ] = AIM_SPEED_MIN;

			m_bMouseAim       = TFALSE;
			m_fAimX           = -m_afAimSpeed[ 1 ];
			m_abAimInput[ 1 ] = m_fAimX < -0.00001f || m_fAimX > 0.00001f;
			m_pCamera->SetSpeed( 2.5f );
			return bResult;
		case AInputCommand_Right:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			bResult = TTRUE;

			if ( m_fAimX < 0.0f )
				m_afAimSpeed[ 1 ] = AIM_SPEED_MIN;

			m_fAimX           = m_afAimSpeed[ 1 ];
			m_abAimInput[ 1 ] = m_fAimX < -0.00001f || m_fAimX > 0.00001f;
			m_pCamera->SetSpeed( 2.5f );
			return bResult;
		case AInputCommand_Forward:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			bResult = TTRUE;

			if ( m_fAimY > 0.0f )
				m_afAimSpeed[ 0 ] = AIM_SPEED_MIN;

			m_fAimY           = -m_afAimSpeed[ 0 ];
			m_abAimInput[ 0 ] = m_fAimY < -0.0001f || m_fAimY > 0.0001f;
			m_pCamera->SetSpeed( 2.5f );
			return bResult;
		case AInputCommand_Backward:
			a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			bResult = TTRUE;

			if ( m_fAimY < 0.0f )
				m_afAimSpeed[ 0 ] = AIM_SPEED_MIN;

			// NOTE: retail checks the old value here, so the first press after stopping doesn't move the aim
			m_abAimInput[ 0 ] = m_fAimY < -0.0001f || m_fAimY > 0.0001f;
			m_fAimY           = m_afAimSpeed[ 0 ];
			m_pCamera->SetSpeed( 2.5f );
			return bResult;
		case AInputCommand_Shoot:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				a_pInputEvent->StartRepeat( 0.0f, 0.0f );
				m_pCue->ResetPower();
				s_bShootPressed = TTRUE;

				if ( m_eState == STATE_HUMAN_AIM )
					m_eState = STATE_HUMAN_POWER;
			}

			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP && s_bShootPressed == TTRUE )
			{
				if ( m_eState == STATE_HUMAN_POWER )
				{
					m_vShotVelocity = m_pCue->GetShotVelocity();
					m_eState        = STATE_HUMAN_SHOT;
				}
				else if ( m_eState == STATE_HUMAN_PLACE_BALL && m_pCueBall->IsPlacementValid() )
				{
					m_eState      = STATE_START_TURN;
					m_fInputDelay = 0.1f;
				}

				s_bShootPressed = TFALSE;
			}

			m_iUnkE40 = 0;
			return TTRUE;
		case AInputCommand_Jump:
			return TTRUE;
		default:
			if ( bResult == TFALSE )
				bResult = AMiniGameState::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );

			return bResult;
	}
}

// $Barnyard: FUNCTION 004d2180
TBOOL ABarnYardPoolMiniGameState::UpdateAim( TFLOAT a_fDeltaTime )
{
	// Time since the input stopped, the speed of the aim resets after a moment
	static TFLOAT s_fStopTimer; // 0x007831d0

	TVector4& rvAimTarget  = m_vAimTarget;
	TVector4  vOldPosition = rvAimTarget;

	if ( m_eState == STATE_HUMAN_PLACE_BALL )
		vOldPosition = m_pCueBall->GetPosition();

	// The speed goes up while the input is held
	TFLOAT fDeltaTime2 = a_fDeltaTime * a_fDeltaTime;

	for ( TINT i = 0; i < 2; i++ )
	{
		if ( m_abAimInput[ i ] )
		{
			m_afAimSpeed[ i ] += fDeltaTime2 * 0.9f;
			TMath::Clip( m_afAimSpeed[ i ], AIM_SPEED_MIN, AIM_SPEED_MAX );
		}
		else
		{
			m_afAimSpeed[ i ] -= fDeltaTime2 * 0.03f;
			TMath::Clip( m_afAimSpeed[ i ], AIM_SPEED_MIN, AIM_SPEED_MAX );

			s_fStopTimer += a_fDeltaTime;

			if ( s_fStopTimer <= 0.5f )
				continue;

			m_afAimSpeed[ i ] = AIM_SPEED_MIN;
		}

		s_fStopTimer = 0.0f;
	}

	// Forward and right of the camera
	TVector4 vForward = m_pCamera->GetDirection();
	vForward.y        = 0.0f;
	vForward.Normalise();

	TVector4 vRight = vForward;
	vRight.AsVector3().RotateY( TMath::PI * 0.5f );

	m_iUnkE40 = 0;

	if ( m_eState == STATE_HUMAN_PLACE_BALL )
	{
		// Moves the cue ball
		if ( m_abAimInput[ 1 ] )
		{
			TVector4 vPosition = m_pCueBall->GetPosition();
			TVector4 vMove;
			vMove.Multiply( vRight, m_fAimX );
			m_pCueBall->SetPosition( TVector4( vMove.x + vPosition.x, vMove.y + vPosition.y, vMove.z + vPosition.z, vPosition.w ) );
		}

		if ( m_abAimInput[ 0 ] )
		{
			TVector4 vPosition = m_pCueBall->GetPosition();
			TVector4 vMove;
			vMove.Multiply( vForward, m_fAimY );
			m_pCueBall->SetPosition( TVector4( vPosition.x - vMove.x, vPosition.y - vMove.y, vPosition.z - vMove.z, vPosition.w ) );
		}

		TVector4 vClamped;
		if ( !ABarnYardPoolAI::ClampToBallInHandArea( m_pCueBall->GetPosition(), m_vCueBallStart, m_fBallInHandRadius, m_vForward, &vClamped ) )
			m_pCueBall->SetPosition( vClamped );
	}
	else
	{
		// Moves the aim target around the cue ball
		TVector4 vOldTarget = rvAimTarget;

		if ( m_abAimInput[ 1 ] )
		{
			TVector4 vMove;
			vMove.Multiply( vRight, m_fAimX );
			rvAimTarget.x += vMove.x;
			rvAimTarget.y += vMove.y;
			rvAimTarget.z += vMove.z;

			if ( m_bAimForwardOnly == TTRUE )
			{
				TVector4 vPosition = m_pCueBall->GetPosition();
				TVector4 vToTarget( rvAimTarget.x - vPosition.x, rvAimTarget.y - vPosition.y, rvAimTarget.z - vPosition.z, rvAimTarget.w );

				if ( TVector4::DotProduct3( vToTarget, m_vForward ) < 0.0f )
					rvAimTarget = vOldTarget;
			}

			TVector4 vPosition = m_pCueBall->GetPosition();
			TVector4 vDirection( rvAimTarget.x - vPosition.x, 0.0f, rvAimTarget.z - vPosition.z, rvAimTarget.w );
			vDirection.Normalise();
			vDirection.Multiply( 1.0f );

			vPosition = m_pCueBall->GetPosition();
			rvAimTarget.Set( vDirection.x + vPosition.x, vDirection.y + vPosition.y, vDirection.z + vPosition.z, vDirection.w );
			m_fGuideScale = -0.1f;
		}
	}

	// Keeps the target off the cue ball
	TFLOAT fRadius = m_pCueBall->GetRadius();

	if ( TMath::SphereVsSphere( m_pCueBall->GetPosition(), fRadius, rvAimTarget, m_pCueBall->GetRadius() ) )
	{
		TVector4 vPosition = m_pCueBall->GetPosition();
		TVector4 vDirection( rvAimTarget.x - vPosition.x, rvAimTarget.y - vPosition.y, rvAimTarget.z - vPosition.z, rvAimTarget.w );
		vDirection.Normalise();

		TFLOAT fDistance = m_pCueBall->GetRadius();
		fDistance += m_pCueBall->GetRadius();

		TVector4 vOffset;
		vOffset.Multiply( vDirection, fDistance );

		vPosition = m_pCueBall->GetPosition();
		rvAimTarget.Set( vOffset.x + vPosition.x, vOffset.y + vPosition.y, vOffset.z + vPosition.z, vPosition.w );
	}

	rvAimTarget.y     = m_pCueBall->GetStartPosition().y;
	m_abAimInput[ 0 ] = TFALSE;
	m_abAimInput[ 1 ] = TFALSE;

	TVector4 vNewPosition = rvAimTarget;

	if ( m_eState == STATE_HUMAN_PLACE_BALL )
		vNewPosition = m_pCueBall->GetPosition();

	return vNewPosition.IsEqual( vOldPosition );
}

// $Barnyard: FUNCTION 004d1f20
void ABarnYardPoolMiniGameState::UpdateCueViews()
{
	if ( m_eState != STATE_BALL_IN_HAND && m_eState != STATE_HUMAN_PLACE_BALL && m_eState != STATE_AI_PLACE_BALL && !m_pCue->IsVisible() )
		return;

	if ( m_eState == STATE_SHOOT || m_eState == STATE_CUE_STRIKE || m_eState == STATE_CUE_FOLLOW )
		return;

	// Behind the cue, at three heights
	m_aViews[ 4 ].vPosition = m_pCue->GetCuePosition();
	m_aViews[ 4 ].vPosition.y -= 1.0f;
	m_aViews[ 4 ].vLookAt = m_vAimTarget;

	m_aViews[ 5 ].vPosition = m_pCue->GetCuePosition();
	m_aViews[ 5 ].vPosition.y -= 2.0f;
	m_aViews[ 5 ].vLookAt = m_vAimTarget;

	m_aViews[ 6 ].vPosition = m_pCue->GetCuePosition();
	m_aViews[ 6 ].vPosition.y -= 2.5f;
	m_aViews[ 6 ].vLookAt = m_vAimTarget;
}

// $Barnyard: FUNCTION 004d2090
void ABarnYardPoolMiniGameState::ChooseShotView( TVector4 a_vShot )
{
	if ( m_pCamera->GetCurrentViewUserData() != 1 )
		return;

	TVector4 vSide = m_vForward;
	vSide.AsVector3().RotateY( TMath::PI * 0.5f );

	TFLOAT fForward = TVector4::DotProduct3( m_vForward, a_vShot );
	TFLOAT fSide    = TVector4::DotProduct3( vSide, a_vShot );

	if ( TMath::Abs( fSide ) >= TMath::Abs( fForward ) )
		m_pCamera->SetCurrentView( ( fSide <= 0.0f ) ? 1 : 3 );
	else
		m_pCamera->SetCurrentView( ( fForward <= 0.0f ) ? 2 : 0 );
}

// $Barnyard: FUNCTION 004d2aa0
void ABarnYardPoolMiniGameState::UpdateAimGuide( TFLOAT a_fDeltaTime )
{
	m_oAimLine.Reset();

	TBOOL bHidden = m_pCue->IsVisible() == TFALSE;

	switch ( m_eState )
	{
		case STATE_START:
		case STATE_GAME_DECIDED:
		case STATE_UNK_2:
		case STATE_GAME_OVER_WAIT:
		case STATE_SHOW_RESULTS:
		case STATE_BALLS_ROLLING:
		case STATE_UNK_6:
		case STATE_BALL_IN_HAND:
		case STATE_HUMAN_PLACE_BALL:
		case STATE_AI_PLACE_BALL:
		case STATE_UNK_25:
		case STATE_UNK_26:
		case STATE_RESULTS:
		case STATE_HUMAN_SHOT:
		case STATE_UNK_11:
		case STATE_AI_SHOT:
		case STATE_AI_PLAN:
		case STATE_AI_WAIT:
		case STATE_AI_SHOW_CUE:
		case STATE_AI_WAIT_CAMERA:
		case STATE_AI_POWER:
		case STATE_SHOOT:
		case STATE_CUE_STRIKE:
		case STATE_CUE_FOLLOW:
			bHidden = TTRUE;
			break;
		default:
			break;
	}

	if ( bHidden || GetCurrentTeam().GetCurrentPlayer() == PLAYER_AI )
	{
		AParticleEmitLine::SetEffectColour( m_pArrowEffect, 0 );
		return;
	}

	// Direction of the shot
	TVector4 vCuePosition = m_pCue->GetCuePosition();
	TVector4 vDirection;
	vDirection.Substract( m_pCueBall->GetPosition(), vCuePosition );
	vDirection.y = 0.0f;
	vDirection.Normalise();

	TVector4 vPosition = m_pCueBall->GetPosition();
	vPosition.y        = m_pCueBall->GetPosition().y;

	// Moves the dots along the line
	TFLOAT fOffsetStep = a_fDeltaTime * 3.0f;
	m_fLineOffset += fOffsetStep;

	if ( m_fLineOffset > 1.0f )
		m_fLineOffset -= 1.0f;

	TFLOAT   fSpeed       = 100.0f; // length of the direction after a ball hit
	TFLOAT   fRemaining   = 100.0f;
	TINT     iPocket      = -1;
	TINT     iNumSegments = 0;
	AUnit*   pHitUnit     = TNULL;
	TFLOAT   fDotOffset   = 0.0f;
	TFLOAT   fScale       = 1.0f;
	TVector4 vEnd         = vPosition;
	TVector4 vLastDirection;

	do
	{
		TFLOAT   fDistance = fRemaining;
		TVector4 vNormal;
		TFLOAT   fStepHeight;
		TFLOAT   fPenetration;

		TBOOL bHitCushion = ATerrainInterface::GetSingleton()->CollideSweptSphere( vPosition, m_pCueBall->GetRadius(), vDirection, fDistance, vNormal, 0.01f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );
		TBOOL bHitBall    = APoolBallSteer::CollideBalls( vPosition, vDirection, fDistance, pHitUnit, m_pCueBall, m_pCueBall->GetRadius() );
		TBOOL bHitPocket  = APoolBallSteer::CollidePockets( vPosition, vDirection, fDistance, iPocket, NUM_POCKETS, m_pCueBall->GetRadius(), m_avPockets, m_fPocketRadius );

		if ( bHitBall && !bHitPocket )
		{
			// Stop the line a bit before the other ball
			fDistance -= 0.1f;

			if ( fDistance < 0.0f )
				fDistance = 0.0f;
		}

		fDistance *= fScale;
		vLastDirection = vDirection;

		if ( fDistance > 0.2f )
		{
			TVector4 vMove;
			vMove.Multiply( vDirection, fDistance );
			vEnd.Set( vPosition.x + vMove.x, vPosition.y + vMove.y, vPosition.z + vMove.z, vPosition.w );

			TVector4 vStart;
			vStart.Multiply( vDirection, 0.2f );
			vStart.Set( vPosition.x + vStart.x, vPosition.y + vStart.y, vPosition.z + vStart.z, vPosition.w );

			m_oAimLine.AddLine( vStart, vEnd, fDotOffset + m_fLineOffset );
		}
		else
		{
			vEnd = vPosition;
		}

		fDotOffset -= 0.01f;
		fRemaining -= fDistance;
		vPosition = vEnd;

		if ( bHitPocket == TTRUE )
		{
			// Ends in a pocket
			vDirection = TVector4::VEC_ZERO;
			bHitBall   = TFALSE;
			fRemaining = 0.0f;
		}
		else if ( bHitBall == TTRUE )
		{
			// Follows the cue ball after the hit
			vDirection.Multiply( fSpeed );

			TVector4 vOtherVelocity = pHitUnit->GetSteer()->GetVelocity();

			TVector4 vNormal;
			vNormal.Substract( *pHitUnit->GetSteer()->GetPosition(), vPosition );
			vNormal.Normalise();

			TFLOAT fDot      = TVector4::DotProduct3( vDirection, vNormal );
			TFLOAT fOtherDot = TVector4::DotProduct3( vOtherVelocity, vNormal );

			if ( fDot > 0.0f || fOtherDot < 0.0f )
			{
				TVector4 vImpulse;
				vImpulse.Multiply( vNormal, fDot - fOtherDot );
				// NOTE: retail computes the impulse but never takes it off the direction, so the line goes straight on
			}
			else
			{
				bHitBall = TFALSE;
			}

			fSpeed = vDirection.NormaliseMag();

			if ( fSpeed < 0.05f )
			{
				vDirection = TVector4::VEC_ZERO;
				fSpeed     = 0.0f;
				fRemaining = 0.0f;
			}
		}
		else if ( bHitCushion == TTRUE )
		{
			// Bounces off the cushion
			vNormal.y = 0.0f;
			vNormal.Normalise();

			TVector4 vReflect;
			vReflect.Multiply( vNormal, TVector4::DotProduct3( vNormal, vDirection ) * 2.0f );
			vDirection.Substract( vDirection, vReflect );
			vDirection.Normalise();

			if ( m_fGuideScale > 0.0f )
				fScale = m_fGuideScale;
			else
				fScale = 0.0f;
		}

		if ( ++iNumSegments > 1 || ( bHitBall && !bHitPocket ) )
			break;

	} while ( fRemaining > 0.0f );

	// The part of the line after a cushion grows back after the aim moved
	m_fGuideScale += fOffsetStep;

	if ( m_fGuideScale > 1.0f )
		m_fGuideScale = 1.0f;

	// The arrow at the end of the line
	AParticleEmitLine::SetEffectColour( m_pArrowEffect, 0x33ffffff );

	TVector4 vArrowForward = vLastDirection;
	TVector4 vArrowRight   = vArrowForward;
	vArrowRight.AsVector3().RotateY( TMath::HALF_PI );
	TVector4 vArrowUp = TVector4::VEC_NEGY;

	vEnd.y += 0.03f;

	TMatrix44& rTransform = m_pArrowHandle->GetInstance()->GetTransform();
	rTransform.Identity();
	rTransform.m_f11 = vArrowRight.x;
	rTransform.m_f12 = vArrowRight.y;
	rTransform.m_f13 = vArrowRight.z;
	rTransform.m_f21 = vArrowForward.x;
	rTransform.m_f22 = vArrowForward.y;
	rTransform.m_f23 = vArrowForward.z;
	rTransform.m_f31 = vArrowUp.x;
	rTransform.m_f32 = vArrowUp.y;
	rTransform.m_f33 = vArrowUp.z;
	rTransform.SetTranslation( vEnd );
	rTransform.Scale( 0.2f, 0.2f, 0.2f );

	m_oAimLine.SetColours( 0, m_oAimLine.GetNumPoints(), 0x88ffffff, 0x33ffffff );
}
