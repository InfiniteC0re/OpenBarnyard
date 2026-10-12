#include "pch.h"
#include <Render/TCameraObject.h> // before the Physics headers (Opcode #defines PI)
#include "ACarChaseMiniGameState.h"
#include "Helpers/ASpline.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnitCarChaseCar.h"
#include "UnitMovement/Steering/ARacingCarSteer.h"
#include "Physics/APhysicsCar.h"

#ifdef PI
#  undef PI
#endif

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Steering is full lock when the car is facing further away than this from where it should go
static constexpr TFLOAT RACER_FULL_LOCK_ANGLE = TMath::DegToRad( 5.0f );

// The throttle is cut to a quarter when the rear wheels slip more than this
static constexpr TFLOAT RACER_MAX_SLIP_ANGLE = TMath::DegToRad( 10.0f );

// $Barnyard: FUNCTION 004ec4b0
void ACarChaseMiniGameState::Racer::Setup( ASteer* a_pSteer, TFLOAT a_fUnk10, TFLOAT a_fUnk14, TFLOAT a_fUnk18, TFLOAT a_fPointSpacing )
{
	fPointSpacing = a_fPointSpacing;
	fUnk10        = a_fUnk10;
	fUnk14        = a_fUnk14;
	fUnk18        = a_fUnk18;
	pSteer        = a_pSteer;
}

// $Barnyard: FUNCTION 004ec4e0
void ACarChaseMiniGameState::Racer::SetMode( TINT a_eMode, TINT a_iUnk1c )
{
	iUnk20 = 0;
	iMode  = a_eMode;
	iUnk1c = a_iUnk1c;

	if ( a_eMode == MODE_CHASE )
		fUnk674 = 10.0f;
}

// $Barnyard: FUNCTION 004ec920
void ACarChaseMiniGameState::Racer::ClearPoints()
{
	// NOTE: T2Vector::Clear (the destructors of TVector4 do nothing)
	iNumPoints = 0;
}

// $Barnyard: FUNCTION 004ec970
void ACarChaseMiniGameState::Racer::SetSpline( ASpline* a_pSpline )
{
	iUnk20 = 0;
	ClearPoints();
	pSpline = a_pSpline;
}

// $Barnyard: FUNCTION 004ec990
void ACarChaseMiniGameState::Racer::RemovePoint( TINT a_iIndex )
{
	// NOTE: T2Vector::Erase, keeps the order
	for ( TINT i = a_iIndex; i < iNumPoints - 1; i++ )
		aPoints[ i ] = aPoints[ i + 1 ];

	iNumPoints--;
}

// $Barnyard: FUNCTION 004eca10
TINT ACarChaseMiniGameState::Racer::GetLastPointIndex() const
{
	return ( iNumPoints != 0 ) ? iNumPoints - 1 : 0;
}

// $Barnyard: FUNCTION 004eca90
void ACarChaseMiniGameState::Racer::AddPoint( TVector4 a_vPoint )
{
	if ( iNumPoints <= 0 || TVector4::Distance( a_vPoint, aPoints[ GetLastPointIndex() ] ) >= fPointSpacing )
	{
		// Forget the oldest point when the trail is full
		if ( iNumPoints >= MAX_RACER_POINTS )
			RemovePoint( 0 );

		aPoints[ iNumPoints++ ] = a_vPoint;
	}
}

// $Barnyard: FUNCTION 004ec510
void ACarChaseMiniGameState::Racer::UpdateChase( ARacingCarSteer* a_pSteer, ARacingCarSteer* a_pTarget, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fDeltaTime )
{
	TVector4 vPosition       = *a_pSteer->GetPosition();
	TVector4 vTargetPosition = *a_pTarget->GetPosition();
	a_pSteer->GetDirection();

	TVector4 vToTarget = vTargetPosition - vPosition;
	TFLOAT   fDistance = vToTarget.NormaliseMag();

	TVector4 vClosestPosition;
	TVector4 vClosestDirection;
	TFLOAT   fProgress;
	pSpline->GetClosestPositionAndDirection( *a_pSteer->GetPosition(), vClosestPosition, vClosestDirection, fProgress, 100.0f );

	TFLOAT fTargetProgress = pSpline->FindClosestProgress( *a_pTarget->GetPosition(), 300.0f );

	if ( fDistance <= 50.0f || fProgress >= fTargetProgress )
	{
		// Close to the target or ahead of it, follow its trail
		iMode = MODE_FOLLOW;
		return;
	}

	if ( fDistance > 300.0f )
	{
		iMode = MODE_LOST;
		return;
	}

	TVector4 vDirection = *a_pSteer->GetDirection();
	vDirection.y        = 0.0f;
	vDirection.Normalise();

	// Aim at a point of the track ahead, further away the faster the car goes
	TFLOAT fLookAhead = a_pSteer->GetSpeed() / 60.0f * 30.0f + 10.0f;

	TVector4 vAheadPosition;
	TVector4 vAheadDirection;
	pSpline->GetPositionAndDirection( fLookAhead / pSpline->GetLength() + fProgress, vAheadPosition, vAheadDirection );

	TVector4 vToAhead = vAheadPosition - vPosition;
	vToAhead.Normalise();

	// NOTE: computed and never used in the original
	TVector4 vToTrack       = vClosestPosition - vPosition;
	TFLOAT   fOffTrackRatio = vToTrack.MagnitudeXZ() * 0.5f;
	TMath::Clip( fOffTrackRatio, 0.0f, 1.0f );

	// NOTE: the original picks between VEC_POSZ and vToAhead with a global that is always TRUE
	TVector4 vWanted = vToAhead;
	vWanted.y        = 0.0f;
	vWanted.Normalise();

	*a_pOutSteering = vWanted.GetAngleBetweenVector( vDirection );

	APhysicsCar* pCar = a_pSteer->GetCar();

	if ( pCar->m_iNumWheelsOnGround > 2 )
	{
		// Lift off when the rear wheels slide
		if ( TMath::Abs( ( pCar->m_aWheels[ 2 ].m_fSlipAngle + pCar->m_aWheels[ 3 ].m_fSlipAngle ) * 0.5f ) > RACER_MAX_SLIP_ANGLE &&
		     pCar->m_oGearBox.iGear > 1 )
		{
			pCar->SetThrottle( 0.25f * pCar->m_fThrottle );
		}
	}
	else
	{
		pCar->SetThrottle( 0.0f );
		pCar->SetBrake( 0.0f );
	}

	if ( vWanted.z * vDirection.x - vDirection.z * vWanted.x > 0.0f )
		*a_pOutSteering = -*a_pOutSteering;

	if ( TMath::Abs( *a_pOutSteering ) > RACER_FULL_LOCK_ANGLE )
		*a_pOutSteering = ( *a_pOutSteering > 0.0f ) ? 1.0f : -1.0f;

	*a_pOutSteer = STEERCOMMAND_ANALOG;
}

// $Barnyard: FUNCTION 004ecb30
void ACarChaseMiniGameState::Racer::UpdateFollow( ARacingCarSteer* a_pSteer, ARacingCarSteer* a_pTarget, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fDeltaTime )
{
	TVector4 vPosition       = *a_pSteer->GetPosition();
	TVector4 vTargetPosition = *a_pTarget->GetPosition();
	TVector4 vDirection      = *a_pSteer->GetDirection();

	TVector4 vToTarget = vTargetPosition - vPosition;
	TFLOAT   fDistance = vToTarget.NormaliseMag();

	APhysicsCar* pCar = a_pSteer->GetCar();

	if ( fDistance < 50.0f )
	{
		fFarTime = 0.0f;
	}
	else
	{
		// Lost the target for 3 seconds, drive along the track again
		fFarTime += a_fDeltaTime;

		if ( fFarTime >= 3.0f )
		{
			iMode = MODE_CHASE;
			return;
		}
	}

	TFLOAT fThrottle    = pCar->m_fThrottle;
	TFLOAT fBrake       = pCar->m_fBrake;
	TFLOAT fTargetSpeed = a_pTarget->GetSpeed();

	if ( a_pSteer->GetSpeed() >= fTargetSpeed )
	{
		if ( a_pTarget->GetSpeed() < 5.0f || a_pSteer->GetSpeed() <= a_pTarget->GetSpeed() )
		{
			if ( fDistance >= 15.0f )
			{
				pCar->SetThrottle( 1.0f );
				pCar->SetBrake( 0.0f );
			}
			else
			{
				// Caught the target
				pCar->SetBrake( 0.0f );
				pCar->SetThrottle( 0.0f );
				a_pSteer->SetVelocity( TVector4::VEC_ZERO );
				iMode = MODE_CAUGHT;
			}
		}
		else if ( a_pSteer->GetSpeed() > 0.0f )
		{
			// Faster than the target, slow down
			if ( fThrottle <= 0.0f )
			{
				if ( fBrake <= 1.0f )
				{
					fBrake = a_fDeltaTime * 5.0f + fBrake;
					TMath::Clip( fBrake, 0.0f, 1.0f );
					pCar->SetBrake( fBrake );
				}
			}
			else
			{
				fThrottle = fThrottle - a_fDeltaTime * 5.0f;
				TMath::Clip( fThrottle, 0.0f, 1.0f );
				pCar->SetThrottle( fThrottle );
			}
		}
	}
	else if ( fBrake <= 0.0f )
	{
		if ( fThrottle < 1.0f )
		{
			fThrottle = a_fDeltaTime * 5.0f + fThrottle;
			TMath::Clip( fThrottle, 0.0f, 1.0f );
			pCar->SetThrottle( fThrottle );
		}
	}
	else
	{
		fBrake = fBrake - a_fDeltaTime * 5.0f;
		TMath::Clip( fBrake, 0.0f, 1.0f );
		pCar->SetBrake( fBrake );
	}

	// Can't see the target: steer towards the most recent point of its trail that can be seen
	TVector4 vRayStart( vPosition.x, vPosition.y - 2.0f, vPosition.z, vPosition.w );

	if ( ATerrainInterface::GetSingleton()->CollideRay( vRayStart, vToTarget, fDistance, TNULL, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) == TTRUE && iNumPoints > 0 )
	{
		TINT iPoint = GetLastPointIndex();

		vToTarget = aPoints[ iPoint ] - vRayStart;
		fDistance = vToTarget.NormaliseMag();

		if ( ATerrainInterface::GetSingleton()->CollideRay( vRayStart, vToTarget, fDistance, TNULL, TCollisionCommon::TOSHICGROUPFLAG_NONE, TTRUE ) )
		{
			TBOOL bFound = TFALSE;

			while ( iPoint != 0 )
			{
				iPoint--;

				vToTarget = aPoints[ iPoint ] - vRayStart;
				fDistance = vToTarget.NormaliseMag();

				if ( !ATerrainInterface::GetSingleton()->CollideRay( vRayStart, vToTarget, fDistance, TNULL, TCollisionCommon::TOSHICGROUPFLAG_NONE, TTRUE ) )
				{
					bFound = TTRUE;
					break;
				}
			}

			// No point of the trail can be seen, drive along the track again
			if ( !bFound )
				iMode = MODE_CHASE;
		}
	}

	vToTarget.y  = 0.0f;
	vDirection.y = 0.0f;
	vDirection.Normalise();
	vToTarget.Normalise();

	*a_pOutSteer    = STEERCOMMAND_ANALOG;
	TFLOAT fAngle   = vToTarget.GetAngleBetweenVector( vDirection );
	*a_pOutSteering = fAngle;

	if ( vToTarget.z * vDirection.x - vDirection.z * vToTarget.x > 0.0f )
		*a_pOutSteering = -fAngle;

	if ( TMath::Abs( *a_pOutSteering ) > TMath::DegToRad( 10.0f ) )
		*a_pOutSteering = ( *a_pOutSteering > 0.0f ) ? 1.0f : -1.0f;

	*a_pOutSteer = STEERCOMMAND_ANALOG;
}

// $Barnyard: FUNCTION 004ed150
TINT ACarChaseMiniGameState::Racer::Update( ARacingCarSteer* a_pSteer, TINT a_iNumPlayers, ARacingCarSteer** a_ppTargets, AUnitCarChaseCar** a_ppCars, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fDeltaTime )
{
	if ( iMode == MODE_LOST || iMode == MODE_CAUGHT || iMode == MODE_IDLE || iMode == MODE_FINISHED )
		return iMode;

	if ( iMode == MODE_CHASE )
	{
		UpdateChase( a_pSteer, a_ppTargets[ 0 ], a_pOutPedal, a_pOutSteer, a_pOutSteering, a_fDeltaTime );
	}
	else if ( iMode == MODE_FOLLOW )
	{
		AddPoint( *a_ppTargets[ 0 ]->GetPosition() );
		UpdateFollow( a_pSteer, a_ppTargets[ 0 ], a_pOutPedal, a_pOutSteer, a_pOutSteering, a_fDeltaTime );
	}

	return iMode;
}

// $Barnyard: FUNCTION 004f5940
TBOOL ACarChaseMiniGameState::SplineFollower::Update( ARacingCarSteer* a_pPlayerSteer, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fPlayerDistance, TFLOAT a_fDeltaTime )
{
	TVector4 vPosition  = *pSteer->GetPosition();
	TVector4 vDirection = *pSteer->GetDirection();
	vDirection.y        = 0.0f;
	vDirection.Normalise();

	TVector4 vClosestPosition;
	TVector4 vClosestDirection;
	pSpline->GetClosestPositionAndDirection( vPosition, vClosestPosition, vClosestDirection, fProgress, 100.0f );

	// Aim at a point of the track ahead, further away the faster the car goes
	TFLOAT fLookAhead = 30.0f * ( pSteer->GetSpeed() / 60.0f ) + 10.0f;

	TVector4 vAheadPosition;
	TVector4 vAheadDirection;
	pSpline->GetPositionAndDirection( fLookAhead / pSpline->GetLength() + fProgress, vAheadPosition, vAheadDirection );

	TVector4 vToAhead = vAheadPosition - vPosition;
	vToAhead.Normalise();

	// NOTE: computed and never used in the original
	TVector4 vToTrack       = vClosestPosition - vPosition;
	TFLOAT   fOffTrackRatio = vToTrack.MagnitudeXZ() * 0.5f;
	TMath::Clip( fOffTrackRatio, 0.0f, 1.0f );

	// NOTE: the original picks between VEC_POSZ and vToAhead with a global that is always TRUE
	TVector4 vWanted = vToAhead;
	vWanted.y        = 0.0f;
	vWanted.Normalise();

	*a_pOutSteering = vWanted.GetAngleBetweenVector( vDirection );

	APhysicsCar* pCar = pSteer->GetCar();

	if ( pCar->m_iNumWheelsOnGround > 2 )
	{
		// Lift off when the rear wheels slide
		if ( TMath::Abs( ( pCar->m_aWheels[ 2 ].m_fSlipAngle + pCar->m_aWheels[ 3 ].m_fSlipAngle ) * 0.5f ) > RACER_MAX_SLIP_ANGLE &&
		     pCar->m_oGearBox.iGear > 1 )
		{
			pCar->SetThrottle( 0.25f * pCar->m_fThrottle );
		}
	}
	else
	{
		pCar->SetThrottle( 0.0f );
		pCar->SetBrake( 0.0f );
	}

	if ( vWanted.z * vDirection.x - vDirection.z * vWanted.x > 0.0f )
		*a_pOutSteering = -*a_pOutSteering;

	if ( TMath::Abs( *a_pOutSteering ) > RACER_FULL_LOCK_ANGLE )
		*a_pOutSteering = ( *a_pOutSteering > 0.0f ) ? 1.0f : -1.0f;

	*a_pOutSteer = STEERCOMMAND_ANALOG;
	return TTRUE;
}

// $Barnyard: FUNCTION 004ed230
void ACarChaseMiniGameState::DriveCar( ARacingCarSteer* a_pSteer, TINT a_ePedalCommand, STEERCOMMAND a_eSteerCommand, TFLOAT a_fSteering, TFLOAT a_fUnk, TINT a_iUnk )
{
	switch ( a_ePedalCommand )
	{
		case AUnitCarChaseCar::DRIVESTATE_IDLE: a_pSteer->SetPedal( ARacingCarSteer::PEDAL_NONE, a_iUnk ); break;
		case AUnitCarChaseCar::DRIVESTATE_ACCELERATE: a_pSteer->SetPedal( ARacingCarSteer::PEDAL_THROTTLE, a_iUnk ); break;
		case AUnitCarChaseCar::DRIVESTATE_BRAKE: a_pSteer->SetPedal( ARacingCarSteer::PEDAL_BRAKE, a_iUnk ); break;
		case AUnitCarChaseCar::DRIVESTATE_UNK7: a_pSteer->SetPedal( ARacingCarSteer::PEDAL_BRAKE_ALT, a_iUnk ); break;
	}

	if ( a_eSteerCommand == STEERCOMMAND_ANALOG )
		a_pSteer->SetSteering( ARacingCarSteer::STEER_ABSOLUTE, a_fSteering, a_fUnk );
	else if ( a_eSteerCommand == STEERCOMMAND_LEFT )
		a_pSteer->SetSteering( ARacingCarSteer::STEER_LEFT, a_fSteering, a_fUnk );
	else if ( a_eSteerCommand == STEERCOMMAND_RIGHT )
		a_pSteer->SetSteering( ARacingCarSteer::STEER_RIGHT, a_fSteering, a_fUnk );
}

// $Barnyard: FUNCTION 004ef790
void ACarChaseMiniGameState::RespawnRacer1()
{
	if ( m_apCars[ CAR_RACER1 ]->IsVisible() == TTRUE )
		return;

	ASpline& rSpline = m_oRaceTrack.oSpline;

	// 75 units behind the player
	TFLOAT fProgress = rSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );
	fProgress        = fProgress - 75.0f / rSpline.GetLength();
	TMath::Clip( fProgress, 0.0f, 1.0f );

	TVector4 vPosition;
	TVector4 vDirection;
	rSpline.GetPositionAndDirection( fProgress, vPosition, vDirection );

	// On the ground
	TFLOAT fDistance = 200.0f;

	if ( ATerrainInterface::GetSingleton()->CollideRay( vPosition, TVector4::VEC_POSY, fDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) == TTRUE )
	{
		vPosition.y += fDistance;
	}
	else
	{
		if ( ATerrainInterface::GetSingleton()->CollideRay( vPosition, TVector4::VEC_NEGY, fDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) != TTRUE )
			return;

		vPosition.y -= fDistance;
	}

	vPosition.y -= 0.2f;

	// Horizontal direction
	TVector4 vDown = TVector4::VEC_NEGY;
	TVector4 vRight;
	vRight.CrossProduct( vDown, vDirection );
	vDirection.CrossProduct( vRight, vDown );
	vDirection.Normalise();

	m_apSteers[ CAR_RACER1 ]->Reset( vPosition, vDirection );

	// The "cop is chasing you" light
	if ( !m_oIconElement.IsVisible() )
	{
		m_oIconElement.Show();
		m_oIconRectangle.GetTransform().SetIdentity();
	}

	m_apCars[ CAR_RACER1 ]->SetVisible( TTRUE );

	// Let the car settle on the ground
	for ( TINT i = 10; i != 0; i-- )
	{
		m_apSteers[ CAR_RACER1 ]->Update( 0.04f );
		m_apSteers[ CAR_RACER1 ]->GetCar()->m_oBody.Integrate( 0.04f );
	}

	TVector4 vVelocity;
	vVelocity.Multiply( vDirection, m_apSteers[ CAR_RACER1 ]->GetMaxSpeed() );

	TVector4 vStartVelocity;
	vStartVelocity.Multiply( vVelocity, 0.25f );
	m_apSteers[ CAR_RACER1 ]->SetVelocity( vStartVelocity );

	m_aMiniMaps[ 0 ].SetIconVisible( m_apCars[ CAR_RACER1 ]->m_iUnk168, TTRUE );
	m_apCars[ CAR_RACER1 ]->SetSirenOn( TTRUE );
	m_aRacers[ 0 ].SetMode( Racer::MODE_CHASE, 0 );

	APhysicsCar* pCar = m_apSteers[ CAR_RACER1 ]->GetCar();
	pCar->SetThrottle( 1.0f );
	pCar->SetBrake( 0.0f );
	m_apSteers[ CAR_RACER1 ]->GetCar()->m_oGearBox.bAllowReverse = TFALSE;
}

// $Barnyard: FUNCTION 004efad0
void ACarChaseMiniGameState::RespawnRacer2()
{
	if ( m_apCars[ CAR_RACER2 ]->IsVisible() == TTRUE )
		return;

	ASpline& rSpline = m_oRaceTrack.oSpline;

	// 200 units ahead of the player
	TFLOAT fProgress = rSpline.FindClosestProgress( *m_apSteers[ CAR_PLAYER ]->GetPosition(), 300.0f );
	fProgress        = 200.0f / rSpline.GetLength() + fProgress;
	TMath::Clip( fProgress, 0.0f, 1.0f );

	TVector4 vPosition;
	TVector4 vDirection;
	rSpline.GetPositionAndDirection( fProgress, vPosition, vDirection );

	// 3 units to the side of the track
	TVector4 vSide;
	vSide.CrossProduct( vDirection, TVector4::VEC_NEGY );
	vSide.Normalise();

	TVector4 vOffset;
	vOffset.Multiply( vSide, 3.0f );
	vPosition.x += vOffset.x;
	vPosition.y += vOffset.y;
	vPosition.z += vOffset.z;

	// On the ground
	TFLOAT fDistance = 200.0f;
	TFLOAT fGroundY;

	if ( ATerrainInterface::GetSingleton()->CollideRay( vPosition, TVector4::VEC_POSY, fDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) == TTRUE )
	{
		fGroundY = vPosition.y + fDistance;
	}
	else
	{
		if ( ATerrainInterface::GetSingleton()->CollideRay( vPosition, TVector4::VEC_NEGY, fDistance, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE ) != TTRUE )
			return;

		fGroundY = vPosition.y - fDistance;
	}

	// The "cop is chasing you" light
	if ( !m_oIconElement.IsVisible() )
	{
		m_oIconElement.Show();
		m_oIconRectangle.GetTransform().SetIdentity();
	}

	vPosition.y = fGroundY - 0.2f;

	// Horizontal direction
	TVector4 vDown = TVector4::VEC_NEGY;
	TVector4 vRight;
	vRight.CrossProduct( vDown, vDirection );
	vDirection.CrossProduct( vRight, vDown );
	vDirection.Normalise();

	m_apSteers[ CAR_RACER2 ]->Reset( vPosition, vDirection );
	m_apSteers[ CAR_RACER2 ]->GetCar()->m_oGearBox.bAllowReverse = TFALSE;
	m_apSteers[ CAR_RACER2 ]->GetCar()->SetBrake( 1.0f );

	// Waits for the player (Racer::MODE_IDLE) until it gets passed (see UpdateRacers)
	m_apCars[ CAR_RACER2 ]->SetVisible( TTRUE );
	m_aMiniMaps[ 0 ].SetIconVisible( m_apCars[ CAR_RACER2 ]->m_iUnk168, TTRUE );
	m_aRacers[ 1 ].SetMode( Racer::MODE_IDLE, 0 );

	// Let the car settle on the ground
	for ( TINT i = 10; i != 0; i-- )
	{
		m_apSteers[ CAR_RACER2 ]->Update( 0.04f );
		m_apSteers[ CAR_RACER2 ]->GetCar()->m_oBody.Integrate( 0.04f );
	}

	m_apSteers[ CAR_RACER2 ]->SetVelocity( TVector4::VEC_ZERO );
}
