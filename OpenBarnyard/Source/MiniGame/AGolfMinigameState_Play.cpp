#include "pch.h"
#include <Render/TCameraObject.h>
#include "AGolfMinigameState.h"
#include "ALoadScreen.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/AModel.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AMultiCameraHelper.h"
#include "FX/AGameTimeFXManager.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleEffectLocatorManager.h"
#include "FX/ASplashManager.h"
#include "GUI/AFadeManager.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/ASimAnimModelHelperManager.h"
#include "Helpers/ATreeManager.h"
#include "Locale/ALocaleManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/APositionalSoundSet.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/golf.h"
#include "Tasks/ARootTask.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitManager.h"
#include "UnitMovement/Steering/ABallSteer.h"
#include "UnitMovement/Steering/AGopherMoundSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"

#include <Math/TMathInline.h>
#include <Math/TQuaternion.h>
#include <Render/TRenderInterface.h>
#include <Toshi/T2String8.h>
#include <Toshi/TString8.h>
#include <Toshi/TStringManager.h>

#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionModelInstance.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( aim );
TPSTRING8_DECLARE( ballsave );
TPSTRING8_DECLARE( c_head );
TPSTRING8_DECLARE( c_l_upperarm );
TPSTRING8_DECLARE( c_r_upperarm );
TPSTRING8_DECLARE( cb_llocator );
TPSTRING8_DECLARE( golf_cam_loc );
TPSTRING8_DECLARE( golf_particles );
TPSTRING8_DECLARE( hold_idle );
TPSTRING8_DECLARE( inground_dive );
TPSTRING8_DECLARE( lose );
TPSTRING8_DECLARE( pickup );
TPSTRING8_DECLARE( strike );
TPSTRING8_DECLARE( victory );
TPSTRING8_DECLARE( victory1 );
TPSTRING8_DECLARE( victory2 );
TPSTRING8_DECLARE( victory3 );
TPSTRING8_DECLARE( victory4 );
TPSTRING8_DECLARE( win );
TPSTRING8_DECLARE( windup );

static constexpr TINT LOCALE_TIMES_UP       = 509; // "Time's up!"
static constexpr TINT LOCALE_OUT_OF_STROKES = 510; // "Out of strokes!"
static constexpr TINT LOCALE_HOLE_IN_ONE    = 512; // "Hole-in-One!"
static constexpr TINT LOCALE_WATERLOGGED    = 516; // "Waterlogged!"

// Ignored by the ray between a gopher and the ball it wants to pick up
static constexpr TCollisionCommon::TOSHICGROUPFLAG IGNORE_PICKUP =
    TCollisionCommon::TOSHICGROUPFLAG_SUPPORT | TCollisionCommon::TOSHICGROUPFLAG_GRASS | TCollisionCommon::TOSHICGROUPFLAG_NOFRICTION |
    TCollisionCommon::TOSHICGROUPFLAG_LIQUID | TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

// Ignored when looking for the obstacles (out of bounds) the ball has rolled into
static constexpr TCollisionCommon::TOSHICGROUPFLAG IGNORE_OBSTACLE =
    TCollisionCommon::TOSHICGROUPFLAG_PLAYER | TCollisionCommon::TOSHICGROUPFLAG_GRASS | TCollisionCommon::TOSHICGROUPFLAG_TIMBER |
    TCollisionCommon::TOSHICGROUPFLAG_NOFRICTION | TCollisionCommon::TOSHICGROUPFLAG_LIQUID | TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

// Ignored when looking for the water the ball has fallen into
static constexpr TCollisionCommon::TOSHICGROUPFLAG IGNORE_WATER =
    TCollisionCommon::TOSHICGROUPFLAG_SUPPORT | TCollisionCommon::TOSHICGROUPFLAG_PLAYER | TCollisionCommon::TOSHICGROUPFLAG_GRASS |
    TCollisionCommon::TOSHICGROUPFLAG_TIMBER | TCollisionCommon::TOSHICGROUPFLAG_NOFRICTION | TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

// Ignored by the ray along the drift of the ball in the water (when it falls in)
static constexpr TCollisionCommon::TOSHICGROUPFLAG IGNORE_DRIFT_START =
    TCollisionCommon::TOSHICGROUPFLAG_SUPPORT | TCollisionCommon::TOSHICGROUPFLAG_GRASS | TCollisionCommon::TOSHICGROUPFLAG_TIMBER |
    TCollisionCommon::TOSHICGROUPFLAG_NOFRICTION | TCollisionCommon::TOSHICGROUPFLAG_LIQUID | TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

// Ignored by the ray along the drift of the ball in the water (UpdateBall)
static constexpr TCollisionCommon::TOSHICGROUPFLAG IGNORE_DRIFT =
    TCollisionCommon::TOSHICGROUPFLAG_GRASS | TCollisionCommon::TOSHICGROUPFLAG_TIMBER | TCollisionCommon::TOSHICGROUPFLAG_NOFRICTION |
    TCollisionCommon::TOSHICGROUPFLAG_LIQUID | TCollisionCommon::TOSHICGROUPFLAG_CAMERA;

// Points of a holed ball on the stroke limit courses, by the strokes left: [ strokes - GetMaxStrokes() + 17 ] (0071dc38)
static constexpr TINT s_aStrokePoints[] = { 60, 50, 45, 40, 35, 30, 25, 18, 15, 12, 10, 8, 6, 5, 4, 3, 2, 1 };

static AAnimatedUnit* GetAnimatedUnit( TRef<AUnitProxy>& a_rProxy )
{
	return TSTATICCAST( AAnimatedUnit, a_rProxy ? a_rProxy->GetUnit() : TNULL );
}

static AMultiCameraHelper* GetMultiCameraHelper()
{
	return TSTATICCAST( AMultiCameraHelper, ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_MULTICAMERA ) );
}

// $Barnyard: FUNCTION 00476350
// NOTE: one function in retail, split into UpdatePlayCamera, UpdateGophers, CollectBucks, UpdateBalls and UpdateBallSaves
void AGolfMinigameState::UpdatePlay( TFLOAT a_fDeltaTime )
{
	UpdatePlayCamera();
	UpdateGophers( a_fDeltaTime );

	if ( m_pCourse->bCharity )
		CollectBucks();

	UpdateBalls();
	UpdateBallSaves();

	UpdateBall( a_fDeltaTime );
	OnHoleFinished( a_fDeltaTime );
}

// Part of UpdatePlay (00476350)
void AGolfMinigameState::UpdatePlayCamera()
{
	if ( m_bPlayFinished )
	{
		// The camera looks out of the gopher of the single player
		AAnimatableObject* pGopherObject  = GetAnimatedUnit( m_aPlayers[ 0 ].pGopher )->GetAnimatableObject();
		AModelInstance*    pModelInstance = pGopherObject->GetModelInstance();

		TMatrix44 matBone;
		pModelInstance->GetBoneTransform( pModelInstance->GetBoneID( TPS8( golf_cam_loc ) ), matBone );

		TVector4 vDirection = matBone.AsBasisVector4( BASISVECTOR_FORWARD );
		vDirection.Normalise();
		vDirection.Negate3();
		vDirection.w = 1.0f;

		ACamera* pCamera                      = ACameraManager::GetSingleton()->GetCurrentCamera();
		pCamera->GetMatrix().GetTranslation() = matBone.GetTranslation();
		pCamera->LookAtDirection( vDirection );
		pCamera->SetFOV( TMath::DegToRad( 70.0f ) );
	}
	else if ( m_iNumPlayers == 1 )
	{
		// Zoom of the camera
		AMultiCameraHelper* pCameraHelper = GetMultiCameraHelper();

		if ( m_bCommand31 )
		{
			pCameraHelper->SetMinDistance( 40.0f );
			pCameraHelper->SetMaxDistance( 40.0f );
		}
		else if ( m_bCommand30 )
		{
			pCameraHelper->SetMinDistance( 10.0f );
			pCameraHelper->SetMaxDistance( 10.0f );
		}
		else
		{
			pCameraHelper->SetMinDistance( 12.0f );
			pCameraHelper->SetMaxDistance( 30.0f );
		}
	}
}

// Part of UpdatePlay (00476350)
void AGolfMinigameState::UpdateGophers( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

#ifdef BARNYARD_COMMUNITY_PATCH
		DevAutoPlayGopher( i );
#endif

		// Diagonal stick input as long as the straight one
		if ( rPlayer.fStickX != 0.0f )
		{
			TFLOAT fAngle   = TMath::ATan( rPlayer.fStickY / rPlayer.fStickX );
			rPlayer.fStickY = TMath::Abs( TMath::Sin( fAngle ) ) * rPlayer.fStickY;
			rPlayer.fStickX = TMath::Abs( TMath::Cos( fAngle ) ) * rPlayer.fStickX;
		}
		else if ( rPlayer.fStickY == 0.0f )
		{
			// Use the d-pad if the stick isn't used
			if ( rPlayer.fDPadX == 0.0f )
			{
				rPlayer.fStickY = rPlayer.fDPadY;
			}
			else
			{
				TFLOAT fAngle   = TMath::ATan( rPlayer.fDPadY / rPlayer.fDPadX );
				rPlayer.fStickY = TMath::Abs( TMath::Sin( fAngle ) ) * rPlayer.fDPadY;
				rPlayer.fStickX = TMath::Abs( TMath::Cos( fAngle ) ) * rPlayer.fDPadX;
			}

			rPlayer.fDPadX = TVector2::VEC_ZERO.x;
			rPlayer.fDPadY = TVector2::VEC_ZERO.y;
		}

		if ( IsCourseOver() )
		{
			rPlayer.fStickX = 0.0f;
			rPlayer.fStickY = 0.0f;
		}

		if ( rPlayer.iBallState == BALLSTATE_HOLED )
			continue;

		AAnimatedUnit* pGopher = GetAnimatedUnit( rPlayer.pGopher );

		if ( rPlayer.iGopherState == GOPHERSTATE_FREE )
		{
			if ( m_iNumPlayers == 1 && !m_bCameraOnBall )
			{
				// Once the struck ball is away from the gopher, the camera follows it instead of looking at the hole
				m_fCameraTime += a_fDeltaTime;

				const TVector4* pBallPos   = GetAnimatedUnit( m_aPlayers[ 0 ].pBall )->GetSteer()->GetPosition();
				const TVector4* pGopherPos = GetAnimatedUnit( m_aPlayers[ 0 ].pGopher )->GetSteer()->GetPosition();

				if ( TVector4::Distance( *pGopherPos, *pBallPos ) > 3.0f || m_fCameraTime > 1.0f )
				{
					AMultiCameraHelper* pCameraHelper = GetMultiCameraHelper();
					pCameraHelper->RemoveTarget( m_vHolePosition );
					pCameraHelper->AddTarget( m_aPlayers[ 0 ].pBall, TTRUE );
					m_bCameraOnBall = TTRUE;
				}
			}

			// Move the gopher relative to the camera
			TVector4 vImpulse = TVector4::VEC_ZERO;

			const TMatrix44& rCameraMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
			TVector4         vForward( rCameraMatrix.m_f31, 0.0f, rCameraMatrix.m_f33, 1.0f );
			vForward.Normalise();

			TVector4 vRight;
			vRight.CrossProduct( vForward, TVector4::VEC_NEGY );

			TVector4 vMove;
			vMove.Multiply( vForward, rPlayer.fStickY * 5.0f );
			vImpulse.Add( vMove );

			vMove.Multiply( vRight, rPlayer.fStickX * 5.0f );
			vImpulse.Add( vMove );

			TSTATICCAST( AGopherMoundSteer, pGopher->GetSteer() )->AddImpulse( vImpulse );
		}
		else if ( rPlayer.iGopherState == GOPHERSTATE_HOLDING )
		{
			AAnimatableObject* pGopherObject = pGopher->GetAnimatableObject();

			if ( rPlayer.bSwingPressed )
			{
				rPlayer.iGopherState = GOPHERSTATE_WINDUP;
				rPlayer.fWindupTime  = 0.0f;
				pGopherObject->PlayAnimation( TPS8( windup ) );
			}
			else
			{
				TFLOAT fAngle = 0.0f;

				if ( rPlayer.fStickX != 0.0f || rPlayer.fStickY != 0.0f )
				{
					// Turn the gopher towards the direction of the stick (relative to the camera)
					ASteer*  pGopherSteer = pGopher->GetSteer();
					TVector4 vDirection   = *pGopherSteer->GetDirection();

					TVector4 vForward;
					ACameraManager::GetSingleton()->GetCurrentCamera()->GetFlatDirection( vForward );

					TVector4 vRight = vForward;
					vRight.AsVector3().RotateY( TMath::PI / 2 );

					TVector4 vStickRight;
					vStickRight.Multiply( vRight, rPlayer.fStickX );

					TVector4 vStickForward;
					vStickForward.Multiply( vForward, rPlayer.fStickY );

					TVector4 vAim;
					vAim.Add( vStickForward, vStickRight );

					fAngle           = vAim.GetAngleBetweenVector( vDirection );
					TFLOAT fRotation = TMath::Sqrt( rPlayer.fStickX * rPlayer.fStickX + rPlayer.fStickY * rPlayer.fStickY ) * TMath::PI * fAngle * a_fDeltaTime;
					TFLOAT fSign     = ( vDirection.z * vAim.x - vAim.z * vDirection.x > 0.0f ) ? 1.0f : -1.0f;

					vDirection.AsVector3().RotateY( fSign * fRotation );
					pGopherSteer->SetDirection( vDirection );
				}

				if ( pGopherObject->IsAnimationFinished( TPS8( pickup ) ) )
				{
					if ( fAngle <= 0.0f )
						pGopherObject->PlayAnimation( TPS8( hold_idle ) );
					else
						pGopherObject->PlayAnimation( TPS8( aim ) );
				}
			}
		}
		else if ( rPlayer.iGopherState == GOPHERSTATE_WINDUP )
		{
			rPlayer.fWindupTime += a_fDeltaTime;

			if ( rPlayer.bSwingReleased )
			{
				// Strike the ball
				rPlayer.iGopherState = GOPHERSTATE_FREE;
				rPlayer.iStrokes++;

				if ( m_pCourse->bStrokeLimit )
				{
					TStringManager::String16Format( m_wszStrokes, TARRAYSIZE( m_wszStrokes ), L"%d", GetMaxStrokes() - TUINT8( rPlayer.iStrokes ) );
					m_oStrokesText.SetText( m_wszStrokes );
				}

				Player&        rBallOwner   = m_aPlayers[ rPlayer.iHeldBall ];
				ASteer*        pGopherSteer = pGopher->GetSteer();
				AAnimatedUnit* pBall        = GetAnimatedUnit( rBallOwner.pBall );
				ASteer*        pBallSteer   = pBall->GetSteer();

				// The strength oscillates between 2 and 15 while winding up
				TVector4 vVelocity = *pGopherSteer->GetDirection();
				vVelocity.y        = -0.3f;
				vVelocity.Normalise();
				vVelocity.Multiply( 8.5f - TMath::Cos( rPlayer.fWindupTime * TMath::PI ) * 6.5f );

				TVector4 vOffset;
				vOffset.Multiply( TVector4::VEC_NEGY, pBallSteer->GetTraits()->m_fRadius );

				TVector4 vPosition = *pGopherSteer->GetPosition();
				vPosition.Add( vOffset );

				pBallSteer->SetPosition( vPosition );
				pBallSteer->SetVelocity( vVelocity );

				AAnimatableObject* pGopherObject = pGopher->GetAnimatableObject();
				pGopherObject->PlayAnimation( TPS8( strike ) );

				pBall->SetVisible( TTRUE );
				rBallOwner.pBallModel->SetSkeletonUpdating( TFALSE, TTRUE );
				AAnimatableObjectManager::GetSingleton()->DetachObject( rBallOwner.pBallModel, pGopherObject );

				// NOTE: iHeldBall is kept, so the gopher doesn't pick the ball up again until it's away
				rBallOwner.iBallState = BALLSTATE_ON_COURSE;
				rBallOwner.fWaterTime = 0.0f;
				m_fCameraTime         = 0.0f;
			}
		}

		rPlayer.fStickY = 0.0f;
		rPlayer.fStickX = 0.0f;
	}
}

// Part of UpdatePlay (00476350)
// Charity variant: the gopher or the ball of the single player collects the bucks
void AGolfMinigameState::CollectBucks()
{
	TVector4 vGopherPos;
	GetAnimatedUnit( m_aPlayers[ 0 ].pGopher )->GetSteer()->CopyPosition( vGopherPos );

	TVector4 vBallPos;
	GetAnimatedUnit( m_aPlayers[ 0 ].pBall )->GetSteer()->CopyPosition( vBallPos );

	for ( TINT i = 0; i < m_iNumBucks; i++ )
	{
		AAnimatableObject* pBuck          = m_apBucks[ i ];
		AModelInstance*    pModelInstance = pBuck->GetModelInstance();

		if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() )
			continue;

		TVector4 vBuckPos;
		pModelInstance->GetPosition( vBuckPos );

		TFLOAT fGopherDistSq = ( vBuckPos.z - vGopherPos.z ) * ( vBuckPos.z - vGopherPos.z ) + ( vBuckPos.x - vGopherPos.x ) * ( vBuckPos.x - vGopherPos.x );
		TFLOAT fBallDistSq   = ( vBuckPos.z - vBallPos.z ) * ( vBuckPos.z - vBallPos.z ) + ( vBuckPos.x - vBallPos.x ) * ( vBuckPos.x - vBallPos.x );

		if ( fGopherDistSq < 0.64f || ( m_aPlayers[ 0 ].iBallState == BALLSTATE_ON_COURSE && fBallDistSq < 0.5f ) )
			StartCollectEffect( pBuck, 1, 30.0f );
	}
}

// Part of UpdatePlay (00476350)
// The gophers pick the balls up, the balls hit the obstacles and the water, get holed
void AGolfMinigameState::UpdateBalls()
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		if ( rPlayer.iBallState != BALLSTATE_ON_COURSE )
			continue;

		AAnimatedUnit* pBall      = GetAnimatedUnit( rPlayer.pBall );
		ABallSteer*    pBallSteer = TSTATICCAST( ABallSteer, pBall->GetSteer() );
		TVector4       vBallPos   = *pBallSteer->GetPosition();

		//-----------------------------------------------------------------------------
		// Can a gopher pick the ball up?
		//-----------------------------------------------------------------------------
		for ( TINT k = 0; k < m_iNumPlayers; k++ )
		{
			Player& rGopherPlayer = m_aPlayers[ k ];

			if ( rGopherPlayer.iBallState == BALLSTATE_HOLED )
				continue;

			ASteer*  pGopherSteer = GetAnimatedUnit( rGopherPlayer.pGopher )->GetSteer();
			TVector4 vGopherPos   = *pGopherSteer->GetPosition();

			TFLOAT fDistance = TVector4::DistanceXZ( vBallPos, vGopherPos );
			TFLOAT fRadius   = pBallSteer->GetTraits()->m_fRadius;
			TFLOAT fReach    = TMath::Max( TMath::Sqrt( 2.0f ) * ( 0.8f - fRadius ), fRadius + 0.8f );

			if ( fDistance >= fReach + 0.05f )
			{
				// The ball the gopher has struck is away, it can be picked up again
				if ( i == rGopherPlayer.iHeldBall )
					rGopherPlayer.iHeldBall = -1;

				continue;
			}

			if ( rGopherPlayer.iGopherState != GOPHERSTATE_FREE || i == rGopherPlayer.iHeldBall || IsCourseOver() )
				continue;

			// Is there anything between the gopher and the ball?
			vGopherPos.y -= pBallSteer->GetTraits()->m_fRadius;

			TVector4 vToBall( vBallPos.x - vGopherPos.x, vBallPos.y - vGopherPos.y, vBallPos.z - vGopherPos.z, vBallPos.w );
			TFLOAT   fRayLength = TMath::Max( vToBall.NormaliseMag() - pBallSteer->GetTraits()->m_fRadius, 0.0f );
			TBOOL    bBlocked   = pTerrain->CollideRay( vGopherPos, vToBall, fRayLength, IGNORE_PICKUP, TFALSE );

			if ( bBlocked && TMath::Abs( vGopherPos.y - vBallPos.y ) < 0.5f )
			{
				vGopherPos.y = vBallPos.y;
				vToBall.Set( vBallPos.x - vGopherPos.x, 0.0f, vBallPos.z - vGopherPos.z, vBallPos.w );
				fRayLength = TMath::Max( vToBall.NormaliseMag() - pBallSteer->GetTraits()->m_fRadius, 0.0f );
				bBlocked   = pTerrain->CollideRay( vGopherPos, vToBall, fRayLength, IGNORE_PICKUP, TFALSE );
			}

			if ( bBlocked && TMath::Abs( vGopherPos.y - vBallPos.y ) < 0.5f )
			{
				vGopherPos = *pGopherSteer->GetPosition();
				vGopherPos.y -= 0.01f;
				vToBall.Set( vBallPos.x - vGopherPos.x, 0.0f, vBallPos.z - vGopherPos.z, vBallPos.w );
				fRayLength = TMath::Max( vToBall.NormaliseMag() - pBallSteer->GetTraits()->m_fRadius, 0.0f );
				bBlocked   = pTerrain->CollideRay( vGopherPos, vToBall, fRayLength, IGNORE_PICKUP, TFALSE );
			}

			if ( bBlocked )
				continue;

			// Pick the ball up
			rGopherPlayer.iGopherState = GOPHERSTATE_HOLDING;
			rGopherPlayer.iHeldBall    = i;
			pBallSteer->SetPosition( vBallPos );

			AAnimatedUnit* pGopher = GetAnimatedUnit( rGopherPlayer.pGopher );
			pGopher->SetVisible( TTRUE );
			pGopher->GetAnimatableObject()->PlayAnimation( TPS8( pickup ) );
			pBall->SetVisible( TFALSE );

			AAnimatableObjectManager::GetSingleton()->AttachObject( rPlayer.pBallModel, pGopher->GetAnimatableObject(), TPString8(), TPS8( cb_llocator ), TVector4::VEC_ZERO, TQuaternion::IDENTITY, 1.0f );
			rPlayer.pBallModel->SetSkeletonUpdating( TTRUE, TTRUE );
			rPlayer.iBallState = BALLSTATE_HELD;

			if ( m_iNumPlayers == 1 && m_bCameraOnBall )
			{
				AMultiCameraHelper* pCameraHelper = GetMultiCameraHelper();
				pCameraHelper->RemoveTarget( m_aPlayers[ 0 ].pBall );
				pCameraHelper->AddTarget( m_vHolePosition, TTRUE );
				m_bCameraOnBall = TFALSE;
			}

			break;
		}

		if ( rPlayer.iBallState != BALLSTATE_ON_COURSE )
			continue;

		TFLOAT fHoleX = m_vHolePosition.x - vBallPos.x;
		TFLOAT fHoleZ = m_vHolePosition.z - vBallPos.z;

		if ( fHoleZ * fHoleZ + fHoleX * fHoleX >= 0.1f || vBallPos.y <= m_vHolePosition.y )
		{
			//-----------------------------------------------------------------------------
			// The ball is on the course
			//-----------------------------------------------------------------------------
			TVector4 vMove( vBallPos.x - rPlayer.vBallPosition.x, vBallPos.y - rPlayer.vBallPosition.y, vBallPos.z - rPlayer.vBallPosition.z, vBallPos.w );
			TFLOAT   fMoveLength = vMove.NormaliseMag();

			TVector4 vNormal;
			TFLOAT   fStepHeight;
			TFLOAT   fPenetration;

			if ( pTerrain->CollideSweptSphere( rPlayer.vBallPosition, pBallSteer->GetTraits()->m_fRadius, vMove, fMoveLength, vNormal, 0.0f, fStepHeight, fPenetration, IGNORE_OBSTACLE ) )
			{
				// The ball has hit an obstacle: the swimming gopher brings it back
				TVector4 vHitPos( rPlayer.vBallPosition.x + vMove.x * fMoveLength, rPlayer.vBallPosition.y + vMove.y * fMoveLength, rPlayer.vBallPosition.z + vMove.z * fMoveLength, rPlayer.vBallPosition.w );

				// Reflect the rest of the move off the obstacle
				// NOTE: retail also computes the reflected position of the ball (vHitPos + vReflected), it isn't used
				TVector4 vReflected( vBallPos.x - vHitPos.x, vBallPos.y - vHitPos.y, vBallPos.z - vHitPos.z, vBallPos.w );
				TVector4 vIntoNormal = vReflected;
				vIntoNormal.ProjectOnto( vNormal );
				vIntoNormal.Multiply( 2.0f );
				vReflected.Substract( vIntoNormal );

				pBallSteer->SetPosition( vHitPos );

				TVector4 vDirection( vReflected.x, 0.0f, vReflected.z, vBallPos.w );
				vDirection.Normalise();

				TVector4 vVelocity = vDirection;
				TFLOAT   fSpeed    = pBallSteer->GetSpeed();
				TMath::Clip( fSpeed, 5.0f, 5.0f );
				vVelocity.Multiply( fSpeed );

				vHitPos.y = pTerrain->GetGroundHeightBottom( vHitPos.x, vHitPos.z, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );

				AAnimatableObject* pGopherGolf = rPlayer.pGopherGolf;
				pGopherGolf->GetModelInstance()->SetTransform( vHitPos, vDirection );
				pGopherGolf->GetToshiAnimInterface().KillAnim( TPS8( ballsave ) );
				pGopherGolf->PlayAnimation( TPS8( ballsave ) );
				rPlayer.oAnimListener.Connect( pGopherGolf->GetAnimationEmitter( TPS8( ballsave ) )->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnGopherAnimEventCallback ), 0 );
				pGopherGolf->SetSkeletonUpdating( TTRUE, TTRUE );

				// Without the snorkel and the floaties
				pGopherGolf->FindAttachedObject( TPS8( c_head ) )->SetSkeletonUpdating( TFALSE, TTRUE );
				pGopherGolf->FindAttachedObject( TPS8( c_l_upperarm ) )->SetSkeletonUpdating( TFALSE, TTRUE );
				pGopherGolf->FindAttachedObject( TPS8( c_r_upperarm ) )->SetSkeletonUpdating( TFALSE, TTRUE );

				pBall->SetVisible( TFALSE );
				rPlayer.vBallVelocity = vVelocity;
				rPlayer.iBallState    = BALLSTATE_SAVED;
			}
			else if ( rPlayer.vBallPosition.y < vBallPos.y && pTerrain->CollideSweptSphere( rPlayer.vBallPosition, pBallSteer->GetTraits()->m_fRadius, vMove, fMoveLength, vNormal, 0.0f, fStepHeight, fPenetration, IGNORE_WATER ) )
			{
				// The ball has fallen into the water
				TVector4 vWaterPos( rPlayer.vBallPosition.x + vMove.x * fMoveLength, rPlayer.vBallPosition.y + vMove.y * fMoveLength, rPlayer.vBallPosition.z + vMove.z * fMoveLength, rPlayer.vBallPosition.w );

				AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterSplashAEffect, vWaterPos, TVector4::VEC_NEGY );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterSplashBEffect, vWaterPos, TVector4::VEC_NEGY );
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterRippleEffect, vWaterPos, TVector4::VEC_NEGY );
				ASplashManager::GetSingleton()->Spawn( vWaterPos, TVector4::VEC_NEGY, pBallSteer->GetTraits()->m_fRadius, ASplashManager::SPLASHTYPE_WATER );
				ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::GOLF_SPLASH_BALL, vWaterPos );

				// NOTE: ShowPopup is inlined in retail
				ShowPopup( ALocaleManager::Interface()->GetString( LOCALE_WATERLOGGED ) );

				if ( !IsCourseOver() )
				{
					rPlayer.vBallPosition = vWaterPos;
					rPlayer.iBallState    = BALLSTATE_IN_WATER;
					rPlayer.fWaterTime    = 0.0f;

					// It drifts back where it came from or, if there's no bank that way, to the bridge
					rPlayer.vBallVelocity = pBallSteer->GetVelocity();
					rPlayer.vBallVelocity.Multiply( -1.0f );
					rPlayer.vBallVelocity.NormaliseXZ();

					if ( pTerrain->GetRayHitDistance( vWaterPos, rPlayer.vBallVelocity, 100.0f, IGNORE_DRIFT_START, TFALSE ) >= 100.0f &&
					     pTerrain->FindLocator( "bridge_loc" ) )
					{
						TVector4 vBridgePos = pTerrain->GetLocatorPos( "bridge_loc" );
						rPlayer.vBallVelocity.Set( vBridgePos.x - vWaterPos.x, vBridgePos.y - vWaterPos.y, vBridgePos.z - vWaterPos.z, vBridgePos.w );
						rPlayer.vBallVelocity.NormaliseXZ();
					}
				}

				pBallSteer->SetPosition( vWaterPos );
				pBall->SetVisible( TFALSE );
			}
			else
			{
				// Adjust the physics of the ball to the ground below it
				ACollisionModelRayHitList oHitList;
				TFLOAT                    fMaxDistance = 50.0f;
				pTerrain->CollideRay( vBallPos, TVector4::VEC_POSY, fMaxDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_CAMERA, TFALSE );

				TFLOAT fGroundDistance = 50.0f;
				TBOOL  bNearGround     = TFALSE;

				if ( oHitList.iNumHits > 0 )
				{
					TINT iClosest = -1;

					for ( TINT k = 0; k < oHitList.iNumHits; k++ )
					{
						if ( oHitList.aHits[ k ].fDistance < fGroundDistance )
						{
							iClosest        = k;
							fGroundDistance = oHitList.aHits[ k ].fDistance;
						}
					}

					if ( iClosest != -1 )
					{
						ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosest ];
						TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );
						TModelCollisionGroup&           rGroup    = pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) );

						pBallSteer->SetRollingFriction( ( rGroup.eCollGroup == TCollisionCommon::TOSHICGROUP_NOFRICTION ) ? 0.0f : 0.3f );
					}

					bNearGround = fGroundDistance <= 5.0f;
				}

				// The higher it goes up, the stronger the gravity
				if ( bNearGround || pBallSteer->GetVelocity().y >= 0.0f )
					pBallSteer->SetGravity( 9.8f );
				else
					pBallSteer->SetGravity( fGroundDistance - 5.0f + 9.8f );

				rPlayer.vBallPosition = vBallPos;
			}
		}
		else
		{
			//-----------------------------------------------------------------------------
			// Holed out
			//-----------------------------------------------------------------------------
			rPlayer.iBallState = BALLSTATE_HOLED;

			TINT iPoints;

			if ( m_pCourse->bStrokeLimit )
				iPoints = s_aStrokePoints[ rPlayer.iStrokes - GetMaxStrokes() + 17 ];
			else
				iPoints = TMath::FloorToInt( m_fTimeLimit - m_fElapsedTime );

			rPlayer.iScore += iPoints;
			m_bLoseLife = TFALSE;

			AAnimatedUnit* pGopher = GetAnimatedUnit( rPlayer.pGopher );

			if ( rPlayer.iGopherState != GOPHERSTATE_FREE )
			{
				// The gopher drops the ball it holds
				Player&        rBallOwner = m_aPlayers[ rPlayer.iHeldBall ];
				AAnimatedUnit* pHeldBall  = GetAnimatedUnit( rBallOwner.pBall );
				ASteer*        pHeldSteer = pHeldBall->GetSteer();

				rPlayer.iGopherState = GOPHERSTATE_FREE;

				TVector4 vOffset;
				vOffset.Multiply( TVector4::VEC_NEGY, pHeldSteer->GetTraits()->m_fRadius );

				TVector4 vPosition = *pGopher->GetSteer()->GetPosition();
				vPosition.Add( vOffset );
				pHeldSteer->SetPosition( vPosition );

				pHeldBall->SetVisible( TTRUE );
				rBallOwner.iBallState = BALLSTATE_ON_COURSE;

				AAnimatableObject* pBallModel = rBallOwner.pBallModel;
				pBallModel->SetSkeletonUpdating( TFALSE, TTRUE );

				AAnimatableObject* pGopherObject = pGopher->GetAnimatableObject();
				AAnimatableObjectManager::GetSingleton()->DetachObject( pBallModel, pGopherObject );
				pGopherObject->PlayAnimation( TPS8( inground_dive ) );
				rPlayer.iHeldBall = -1;
			}

			ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::GOLF_GOLFHOLE, m_vHolePosition );
			pBallSteer->SetPlayHitSounds( TFALSE );

			AAnimatableObject* pGopherObject = pGopher->GetAnimatableObject();

			if ( pTerrain->FindLocator( "gopher_end" ) )
			{
				TVector4    vPosition;
				TQuaternion qRotation;
				pTerrain->GetLocatorPosAndRot( "gopher_end", vPosition, qRotation );

				// NOTE: GetLocatorPosAndRot outputs the direction of the locator as the rotation
				ASteer* pGopherSteer = pGopher->GetSteer();
				pGopherSteer->SetPosition( vPosition );
				pGopherSteer->SetDirection( TREINTERPRETCAST( const TVector4&, qRotation ) );
			}

			pGopherObject->PlayAnimation( TPS8( victory ) );

			if ( m_iNumPlayers == 1 )
			{
				// The end of the victory animation finishes the play
				m_oPlayListeners.Add( &pGopherObject->GetAnimationEmitter( TPS8( victory1 ) )->GetEmitter(), 0 );
				m_oPlayListeners.Add( &pGopherObject->GetAnimationEmitter( TPS8( victory2 ) )->GetEmitter(), 0 );
				m_oPlayListeners.Add( &pGopherObject->GetAnimationEmitter( TPS8( victory3 ) )->GetEmitter(), 0 );
				m_oPlayListeners.Add( &pGopherObject->GetAnimationEmitter( TPS8( victory4 ) )->GetEmitter(), 0 );

				if ( !m_pCourse->bStrokeLimit && ( m_oTimer.IsTimerWarning() || m_oTimer.IsTimerCritical() ) )
					m_oTimer.SetPaused( TTRUE );
			}
			else
			{
				GetMultiCameraHelper()->RemoveTarget( rPlayer.pGopher );
			}

			if ( m_iNumPlayers == 1 && rPlayer.iStrokes == 1 )
			{
				// NOTE: ShowPopup is inlined in retail
				ShowPopup( ALocaleManager::Interface()->GetString( LOCALE_HOLE_IN_ONE ) );
				m_iLives++;
			}
		}
	}
}

// Part of UpdatePlay (00476350)
// The swimming gopher dives back once it has thrown the ball out ("ballsave")
void AGolfMinigameState::UpdateBallSaves()
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		AAnimatableObject* pGopherGolf    = m_aPlayers[ i ].pGopherGolf;
		AModelInstance*    pModelInstance = pGopherGolf->GetModelInstance();

		if ( !pModelInstance || !pModelInstance->IsUpdatingSkeleton() || !pGopherGolf->IsAnimationFinished( TPS8( ballsave ) ) )
			continue;

		pGopherGolf->SetSkeletonUpdating( TFALSE, TTRUE );

		AModelInstance* pSnorkel = pGopherGolf->FindAttachedObject( TPS8( c_head ) )->GetModelInstance();

		if ( pSnorkel && pSnorkel->IsUpdatingSkeleton() )
		{
			// It was in the water
			TVector4 vPosition;
			pModelInstance->GetPosition( vPosition );
			ATerrainInterface::GetSingleton()->SetToGroundHeightBottom( vPosition, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );

			AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterSplashAEffect, vPosition, TVector4::VEC_NEGY );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterSplashBEffect, vPosition, TVector4::VEC_NEGY );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterRippleEffect, vPosition, TVector4::VEC_NEGY );
			ASplashManager::GetSingleton()->Spawn( vPosition, TVector4::VEC_NEGY, 0.3f, ASplashManager::SPLASHTYPE_WATER );
		}

		pGopherGolf->FindAttachedObject( TPS8( c_head ) )->SetSkeletonUpdating( TFALSE, TTRUE );
		pGopherGolf->FindAttachedObject( TPS8( c_l_upperarm ) )->SetSkeletonUpdating( TFALSE, TTRUE );
		pGopherGolf->FindAttachedObject( TPS8( c_r_upperarm ) )->SetSkeletonUpdating( TFALSE, TTRUE );
	}
}

// $Barnyard: FUNCTION 00475030
void AGolfMinigameState::UpdateBall( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		Player& rPlayer = m_aPlayers[ i ];

		if ( rPlayer.iBallState != BALLSTATE_IN_WATER )
			continue;

		if ( IsCourseOver() )
		{
			rPlayer.iBallState = BALLSTATE_ON_COURSE;
			continue;
		}

		// A ripple every 0.2 seconds
		TBOOL bRipple = a_fDeltaTime + fmodf( rPlayer.fWaterTime, 0.2f ) > 0.2f;
		rPlayer.fWaterTime += a_fDeltaTime;

		if ( rPlayer.fWaterTime > 2.0f )
		{
			ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

			if ( pTerrain->GetRayHitDistance( rPlayer.vBallPosition, rPlayer.vBallVelocity, 100.0f, IGNORE_DRIFT, TFALSE ) >= 2.0f )
			{
				// Drift towards the bank
				TVector4 vMove;
				vMove.Multiply( rPlayer.vBallVelocity, a_fDeltaTime );
				vMove.Multiply( 3.0f );
				rPlayer.vBallPosition.Add( vMove );

				GetAnimatedUnit( rPlayer.pBall )->GetSteer()->SetPosition( rPlayer.vBallPosition );
			}
			else
			{
				// At the bank, the swimming gopher throws the ball out
				pTerrain->SetToGroundHeightBottom( rPlayer.vBallPosition, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );

				AAnimatableObject* pGopherGolf = rPlayer.pGopherGolf;
				pGopherGolf->GetModelInstance()->SetTransform( rPlayer.vBallPosition, rPlayer.vBallVelocity );
				pGopherGolf->GetToshiAnimInterface().KillAnim( TPS8( ballsave ) );
				pGopherGolf->PlayAnimation( TPS8( ballsave ) );
				rPlayer.oAnimListener.Connect( pGopherGolf->GetAnimationEmitter( TPS8( ballsave ) )->GetEmitter(), this, TREINTERPRETCAST( TGenericListener::t_Callback, OnGopherAnimEventCallback ), 0 );
				pGopherGolf->SetSkeletonUpdating( TTRUE, TTRUE );

				// With the snorkel and the floaties
				pGopherGolf->FindAttachedObject( TPS8( c_head ) )->SetSkeletonUpdating( TTRUE, TTRUE );
				pGopherGolf->FindAttachedObject( TPS8( c_l_upperarm ) )->SetSkeletonUpdating( TTRUE, TTRUE );
				pGopherGolf->FindAttachedObject( TPS8( c_r_upperarm ) )->SetSkeletonUpdating( TTRUE, TTRUE );

				rPlayer.vBallVelocity.Multiply( 4.0f );
				rPlayer.vBallVelocity.y -= 4.0f;
				rPlayer.iBallState = BALLSTATE_SAVED;

				TVector4 vPosition = rPlayer.vBallPosition;
				vPosition.Add( TVector4::VEC_NEGY );
				GetAnimatedUnit( rPlayer.pBall )->GetSteer()->SetPosition( vPosition );
			}
		}

		if ( bRipple )
		{
			ATerrainInterface::GetSingleton()->SetToGroundHeightBottom( rPlayer.vBallPosition, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );
			AParticleEffectManager::GetSingleton()->PlayEffect( m_iWaterRippleEffect, rPlayer.vBallPosition, TVector4::VEC_NEGY );
		}
	}
}

// $Barnyard: FUNCTION 00475c30
void AGolfMinigameState::OnHoleFinished( TFLOAT a_fDeltaTime )
{
	TINT iNumHoled = 0;

	for ( TINT i = 0; i < m_iNumPlayers; i++ )
	{
		if ( m_aPlayers[ i ].iBallState == BALLSTATE_HOLED )
			iNumHoled++;
	}

	if ( iNumHoled < m_iNumPlayers )
	{
		if ( IsCourseOver() )
		{
			// The course is over, the balls that can't get to the hole give up
			m_fCourseOverTime += a_fDeltaTime;

			if ( !m_bCourseOverCamera )
			{
				m_bCourseOverCamera = TTRUE;

				AMultiCameraHelper* pCameraHelper = GetMultiCameraHelper();

				if ( m_iNumPlayers == 1 && m_aPlayers[ 0 ].iGopherState == GOPHERSTATE_FREE )
				{
					pCameraHelper->RemoveTarget( m_aPlayers[ 0 ].pBall );
					pCameraHelper->AddTarget( m_vHolePosition, TTRUE );
				}

				for ( TINT i = 0; i < m_iNumPlayers; i++ )
				{
					// NOTE: retail uses the gopher and the ball of the first player for every player
					pCameraHelper->RemoveTarget( m_aPlayers[ 0 ].pGopher );
					pCameraHelper->AddTarget( m_aPlayers[ 0 ].pBall, TFALSE );
				}
			}

			m_fElapsedTime = m_fTimeLimit;

			for ( TINT i = 0; i < m_iNumPlayers; i++ )
			{
				Player& rPlayer = m_aPlayers[ i ];
				TBOOL   bGiveUp = TFALSE;

				if ( rPlayer.iBallState == BALLSTATE_ON_COURSE )
				{
					ASteer* pBallSteer = GetAnimatedUnit( rPlayer.pBall )->GetSteer();
					TFLOAT  fSpeed     = pBallSteer->GetSpeed();

					if ( fSpeed < 0.02f )
					{
						bGiveUp = TTRUE;
					}
					else if ( m_fCourseOverTime > 5.0f )
					{
						if ( fSpeed < 1.0f )
						{
							bGiveUp = TTRUE;
						}
						else
						{
							// Slow and rolling away from the hole
							TVector4        vVelocity = pBallSteer->GetVelocity();
							const TVector4* pBallPos  = pBallSteer->GetPosition();
							TVector4        vToHole( m_vHolePosition.x - pBallPos->x, m_vHolePosition.y - pBallPos->y, m_vHolePosition.z - pBallPos->z, m_vHolePosition.w );

							if ( fSpeed < 4.0f && TVector4::DotProduct3( vVelocity, vToHole ) < 0.0f )
								bGiveUp = TTRUE;
						}
					}
				}

				if ( rPlayer.iBallState != BALLSTATE_HELD && m_fCourseOverTime <= 10.0f && !bGiveUp )
					continue;

				AAnimatedUnit*     pGopher  = GetAnimatedUnit( rPlayer.pGopher );
				ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

				if ( pTerrain->FindLocator( "gopher_end" ) )
				{
					TVector4    vPosition;
					TQuaternion qRotation;
					pTerrain->GetLocatorPosAndRot( "gopher_end", vPosition, qRotation );

					// NOTE: GetLocatorPosAndRot outputs the direction of the locator as the rotation
					ASteer* pGopherSteer = pGopher->GetSteer();
					pGopherSteer->SetPosition( vPosition );
					pGopherSteer->SetDirection( TREINTERPRETCAST( const TVector4&, qRotation ) );
				}

				AAnimatableObject* pGopherObject = pGopher->GetAnimatableObject();
				pGopherObject->PlayAnimation( TPS8( lose ) );

				if ( rPlayer.iGopherState != GOPHERSTATE_FREE )
				{
					AAnimatableObject* pBallModel = m_aPlayers[ rPlayer.iHeldBall ].pBallModel;
					pBallModel->SetSkeletonUpdating( TFALSE, TTRUE );
					AAnimatableObjectManager::GetSingleton()->DetachObject( pBallModel, pGopherObject );

					rPlayer.iGopherState = GOPHERSTATE_FREE;
					rPlayer.iHeldBall    = -1;
				}

				rPlayer.iBallState = BALLSTATE_HOLED;

				if ( m_iNumPlayers == 1 )
				{
					// NOTE: retail gets the animatable object of the first gopher again here
					AAnimatableObject* pFirstGopherObject = GetAnimatedUnit( m_aPlayers[ 0 ].pGopher )->GetAnimatableObject();
					m_oPlayListeners.Add( &pFirstGopherObject->GetAnimationEmitter( TPS8( victory2 ) )->GetEmitter(), 0 );
					m_oPlayListeners.Add( &pFirstGopherObject->GetAnimationEmitter( TPS8( victory4 ) )->GetEmitter(), 0 );
				}
			}
		}
	}
	else
	{
		// Every ball is holed, stop the clock
		m_fElapsedTime -= a_fDeltaTime;
		m_fHoleEndTime += a_fDeltaTime;

		if ( m_bLoseLife && !m_bHoleEndNotified )
		{
			// Nobody has holed out
			m_bHoleEndNotified = TTRUE;
			ShowPopup( ALocaleManager::Interface()->GetString( m_pCourse->bStrokeLimit ? LOCALE_OUT_OF_STROKES : LOCALE_TIMES_UP ) );
		}
	}

	if ( m_fHoleEndTime > 1.5f && !m_bHoleEndNotified )
	{
		ASoundManager::GetSingleton()->PlayCue( soundbank::GOLF_CLAP );
		m_bHoleEndNotified = TTRUE;
	}

	if ( m_fHoleEndTime > 2.5f && m_fHoleEndTime - a_fDeltaTime < 2.5f )
	{
		// NOTE: the original passes a fourth argument (TTRUE) that AFadeManager::StartFade doesn't have
		AFadeManager::GetSingleton()->StartFade( AFade::Color( 0, 0, 0, 0 ), AFade::Color( 255, 0, 0, 0 ), 0.5f );
	}

	if ( m_fHoleEndTime <= 3.0f )
		return;

	TBOOL bWon;

	if ( m_bLoseLife )
	{
		if ( m_iNumPlayers != 1 || m_iLives-- != 1 )
		{
			// Retry the hole
			if ( m_pCourse->bCharity )
			{
				APlayerProfileManager::GetSingleton()->GetMainProfile()->SetMiniGamePickups( m_iStartPickups );
				m_oPickUpHudElement.SetNumber( m_iStartPickups );
			}

			SetupPlayerTurn();
			return;
		}

		// No lives left
		bWon = TFALSE;
	}
	else
	{
		if ( m_iCurrentHole < m_pCourse->iNumHoles - 1 )
		{
			LoadHole();
			return;
		}

		// The course is finished
		m_bCourseFinished = TTRUE;
		bWon              = TTRUE;
	}

	m_eGameState = GAMESTATE_END;
	ACameraManager::GetSingleton()->DetachCameraHelpers();

	// The player turns to the camera
	AAnimatedUnit* pPlayerUnit  = GetAnimatedUnit( m_pPlayerUnit );
	ASteer*        pPlayerSteer = pPlayerUnit->GetSteer();

	TVector4 vDirection;
	pPlayerSteer->CopyDirection( vDirection );
	vDirection.AsVector3().RotateY( TMath::PI );
	pPlayerSteer->SetDirection( vDirection );

	pPlayerUnit->GetAnimatableObject()->PlayAnimation( bWon ? TPS8( win ) : TPS8( lose ) );

	// NOTE: the original passes a fourth argument (TTRUE) that AFadeManager::StartFade doesn't have
	AFadeManager::GetSingleton()->StartFade( AFade::Color( 255, 0, 0, 0 ), AFade::Color( 0, 0, 0, 0 ), 0.5f );
}

// $Barnyard: FUNCTION 004754f0
void AGolfMinigameState::LoadHole()
{
	m_iCurrentHole++;

	if ( m_iCurrentHole >= m_pCourse->iNumHoles )
		m_iCurrentHole = 0;

	TINT iHole = m_pCourse->aHoles[ m_iCurrentHole ];

	// Unload the previous hole
	AUnitManager::GetSingleton()->DeleteDyingUnits();
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	m_oFlagReticle.SetVisible( TFALSE );
	m_pTimerOrStrokes->Hide();

	if ( m_iNumPlayers == 1 )
		m_oLifeHud.SetVisible( TFALSE );

	if ( m_pCourse->bCharity )
		m_oPickUpHudElement.m_oBacking.Hide();

	ASoundManager::GetSingleton()->PauseAllCues( TTRUE );
	ACameraManager::GetSingleton()->GetCameraHelper( ACameraManager::CAMERAHELPER_BONEATTACH )->OnDetach();

	ASimAnimModelHelperManager::GetSingleton()->RemoveModelHelper( &m_oWindmillHelper );
	m_oWindmillHelper.OnDestroy();

	AParticleEffectLocatorManager::GetSingleton()->Reset();
	APositionalSoundSet::GetSingleton()->Unload();
	AGameLoader::DestroyLevel();

	// Load the terrain of the hole
	ARootTask::GetSingleton()->SetRenderWorld( TFALSE );
	g_oLoadScreen.StartLoading( 100, TTRUE );
	AGameLoader::Terrain_Set( AGameLoader::Terrain_Golf1 + iHole - 1, TFALSE, TTRUE, 0, 0, 0, 0 );

	TString8 strName;
	strName.Format( "Golf%d", iHole );

	AGameTimeFXManager* pGameTimeFX = AGameTimeFXManager::GetSingleton();
	pGameTimeFX->SetForcedFX( pGameTimeFX->FindFX( strName.GetString() ) );

	strName.Format( "Data/Sound/Golf%02d.trb", iHole );
	APositionalSoundSet::GetSingleton()->Load( strName.GetString() );
	AParticleEffectLocatorManager::GetSingleton()->LoadLocatorList( TPS8( golf_particles ) );

	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	// The tee
	strName.Format( "ball_%.2i", iHole );

	if ( !pTerrain->FindLocator( strName.GetString() ) )
		strName.Format( "ball_02" );

	m_vPlayerPosition = pTerrain->GetLocatorPos( strName.GetString() );

	// The hole
	strName.Format( "hole_%.2d", iHole );

	if ( !pTerrain->FindLocator( strName.GetString() ) )
		strName.Format( "hole_01" );

	if ( !pTerrain->FindLocator( strName.GetString() ) )
		strName.Format( "hole_02" );

	TQuaternion qRotation;
	pTerrain->GetLocatorPosAndRot( strName.GetString(), m_vHolePosition, qRotation );

	// The flag (the sock flag on the second 9 holes)
	AAnimatableObject* pFlag;

	if ( iHole > 9 )
	{
		m_pFlag->SetSkeletonUpdating( TFALSE, TTRUE );
		m_pSockFlag->SetSkeletonUpdating( TTRUE, TTRUE );
		pFlag = m_pSockFlag;
	}
	else
	{
		m_pFlag->SetSkeletonUpdating( TTRUE, TTRUE );
		m_pSockFlag->SetSkeletonUpdating( TFALSE, TTRUE );
		pFlag = m_pFlag;
	}

	// NOTE: GetLocatorPosAndRot outputs the direction of the locator as the rotation
	TVector4& rFlagDirection = TREINTERPRETCAST( TVector4&, qRotation );

	if ( iHole != 6 )
		rFlagDirection.AsVector3().RotateY( TMath::PI );

	pFlag->GetModelInstance()->SetTransform( m_vHolePosition, rFlagDirection );

	g_oLoadScreen.Reset();
	g_oLoadScreen.SetLoadingState( TFALSE, TTRUE );
	ARootTask::GetSingleton()->SetRenderWorld( TTRUE );
	ASoundManager::GetSingleton()->PauseAllCues( TFALSE );

	SetupPlayerTurn();

	if ( m_pCourse->bCharity )
	{
		// Everet and Daisy stand next to the tee
		TVector4 vSide;
		vSide.CrossProduct( m_vPlayerDirection, TVector4::VEC_NEGY );
		vSide.Normalise();

		TVector4 vOffset;
		vOffset.Multiply( vSide, 1.5f );

		TVector4 vEveretPos = m_vPlayerPosition;
		vEveretPos.Add( vOffset );
		vEveretPos.y = pTerrain->GetGroundHeight( vEveretPos, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );

		TVector4 vEveretDir( m_vPlayerPosition.x - vEveretPos.x, 0.0f, m_vPlayerPosition.z - vEveretPos.z, m_vPlayerPosition.w );
		vEveretDir.Normalise();

		TVector4 vDaisyPos = m_vPlayerPosition;
		vDaisyPos.Add( vOffset );
		vDaisyPos.Add( m_vPlayerDirection );
		vDaisyPos.y = pTerrain->GetGroundHeight( vDaisyPos, TCollisionCommon::TOSHICGROUPFLAG_CAMERA );

		TVector4 vDaisyDir( m_vPlayerPosition.x - vDaisyPos.x, 0.0f, m_vPlayerPosition.z - vDaisyPos.z, m_vPlayerPosition.w );
		vDaisyDir.Normalise();

		PlaceCharityActors( vEveretPos, vDaisyPos, vEveretDir, vDaisyDir );
		SetCharityActorsSkeletonUpdating( TTRUE );
		m_oPickUpHudElement.m_oBacking.Show();
	}

	// The windmills
	if ( iHole == 3 )
	{
		m_oWindmillHelper.AddWindmill( "wmill", 1.5f, 140.0f, 150.0f );
		ASimAnimModelHelperManager::GetSingleton()->AddModelHelper( &m_oWindmillHelper );
	}
	else if ( iHole == 13 )
	{
		m_oWindmillHelper.AddWindmill( "wmill_01", 1.0f, 140.0f, 150.0f );
		ASimAnimModelHelperManager::GetSingleton()->AddModelHelper( &m_oWindmillHelper );
	}

	if ( iHole == 18 )
		ATreeManager::GetSingleton()->SetDistances( m_fSavedTreeLODDistance, m_fSavedTreeRenderDistance, m_fSavedTreeBlendingDistance );
	else
		ATreeManager::GetSingleton()->SetDistances( 200.0f, 200.0f, 200.0f );
}

#ifdef BARNYARD_COMMUNITY_PATCH

// Strength of the strike that gets the ball about a_fDistance far (tuned by hand for -devautoplay)
static TFLOAT DevGetStrikeSpeed( TFLOAT a_fDistance )
{
	TFLOAT fSpeed = TMath::Sqrt( a_fDistance * 17.7f ) * 0.75f;
	TMath::Clip( fSpeed, 2.5f, 14.9f );
	return fSpeed;
}

// Development: "-devautoplay" runs
void AGolfMinigameState::DevAutoPlayGopher( TINT a_iPlayer )
{
	static const TBOOL s_bEnabled = T2String8::FindString( GetCommandLineA(), "-devautoplay" ) != TNULL;

	if ( !s_bEnabled )
		return;

	static TINT s_aLastStates[ MAX_PLAYERS ] = { -1, -1, -1, -1 };

	Player& rPlayer = m_aPlayers[ a_iPlayer ];
	TINT    iState  = rPlayer.iBallState * 10 + rPlayer.iGopherState;

	if ( iState != s_aLastStates[ a_iPlayer ] )
	{
		TINFO( "[devautoplay] player %d: ball state %d, gopher state %d, strokes %d, score %d\n", a_iPlayer, rPlayer.iBallState, rPlayer.iGopherState, rPlayer.iStrokes, rPlayer.iScore );
		s_aLastStates[ a_iPlayer ] = iState;
	}

	rPlayer.bSwingPressed  = TFALSE;
	rPlayer.bSwingReleased = TFALSE;

	if ( rPlayer.iBallState == BALLSTATE_HOLED || IsCourseOver() )
		return;

	const TVector4& rcGopherPos = *GetAnimatedUnit( rPlayer.pGopher )->GetSteer()->GetPosition();
	TVector4        vToHole( m_vHolePosition.x - rcGopherPos.x, 0.0f, m_vHolePosition.z - rcGopherPos.z, 1.0f );
	TFLOAT          fHoleDistance = vToHole.NormaliseXZ();

	if ( rPlayer.iGopherState == GOPHERSTATE_FREE )
	{
		// Run to the own ball
		if ( rPlayer.iBallState != BALLSTATE_ON_COURSE )
			return;

		const TVector4& rcBallPos = *GetAnimatedUnit( rPlayer.pBall )->GetSteer()->GetPosition();
		TVector4        vToBall( rcBallPos.x - rcGopherPos.x, 0.0f, rcBallPos.z - rcGopherPos.z, 1.0f );
		vToBall.NormaliseXZ();

		// Same basis as the movement in UpdateGophers
		const TMatrix44& rCameraMatrix = ACameraManager::GetSingleton()->GetCurrentCamera()->GetMatrix();
		TVector4         vForward( rCameraMatrix.m_f31, 0.0f, rCameraMatrix.m_f33, 1.0f );
		vForward.Normalise();

		TVector4 vRight;
		vRight.CrossProduct( vForward, TVector4::VEC_NEGY );

		rPlayer.fStickX = TVector4::DotProduct3( vToBall, vRight );
		rPlayer.fStickY = TVector4::DotProduct3( vToBall, vForward );
	}
	else if ( rPlayer.iGopherState == GOPHERSTATE_HOLDING )
	{
		// Turn to the hole, then wind up
		const TVector4& rcDirection = *GetAnimatedUnit( rPlayer.pGopher )->GetSteer()->GetDirection();
		TVector4        vDirection( rcDirection.x, 0.0f, rcDirection.z, 1.0f );
		vDirection.NormaliseXZ();

		// The turning can overshoot, so give up on a perfect aim after a while
		static TFLOAT s_afHoldTime[ MAX_PLAYERS ] = {};
		s_afHoldTime[ a_iPlayer ] += 1.0f / 60.0f;

		if ( TVector4::DotProduct3( vDirection, vToHole ) > 0.999f || ( s_afHoldTime[ a_iPlayer ] > 3.0f && TVector4::DotProduct3( vDirection, vToHole ) > 0.98f ) || s_afHoldTime[ a_iPlayer ] > 8.0f )
		{
			s_afHoldTime[ a_iPlayer ] = 0.0f;
			rPlayer.bSwingPressed     = TTRUE;
			return;
		}

		// Same basis as the aiming in UpdateGophers
		TVector4 vForward;
		ACameraManager::GetSingleton()->GetCurrentCamera()->GetFlatDirection( vForward );

		TVector4 vRight = vForward;
		vRight.AsVector3().RotateY( TMath::PI / 2 );

		rPlayer.fStickX = TVector4::DotProduct3( vToHole, vRight );
		rPlayer.fStickY = TVector4::DotProduct3( vToHole, vForward );
	}
	else if ( rPlayer.iGopherState == GOPHERSTATE_WINDUP )
	{
		// Strike with the strength (8.5 - cos( t * PI ) * 6.5) for the distance to the hole
		TFLOAT fSpeed      = DevGetStrikeSpeed( fHoleDistance );
		TFLOAT fStrikeTime = TMath::ACos( ( 8.5f - fSpeed ) / 6.5f ) / TMath::PI;

		if ( rPlayer.fWindupTime >= fStrikeTime )
		{
			TINFO( "[devautoplay] player %d strikes: hole distance %.2f, speed %.2f\n", a_iPlayer, fHoleDistance, fSpeed );
			rPlayer.bSwingReleased = TTRUE;
		}
	}
}

#endif // BARNYARD_COMMUNITY_PATCH
