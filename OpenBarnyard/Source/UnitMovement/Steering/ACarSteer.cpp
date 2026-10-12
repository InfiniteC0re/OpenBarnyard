#include "pch.h"
#include "ACarSteer.h"
#include "FSM/AFunctionTable.h"
#include "Terrain/ATerrainInterface.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AModel.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0055cfe0, CreateObject 0055db00, CreateObjectInPlace 0055db60 (dynamic initialiser 0070a280)
TDEFINE_CLASS( ACarSteer, 3, 0 );

// Has no functions of its own (dynamic initialiser 0070a2c0)
AFunctionTable ACarSteer::ms_oFunctionTable( &TGetClass( ACarSteer ), &ASteer::ms_oFunctionTable );

static constexpr TFLOAT s_fDegToRad = 0.017453292f;

// $Barnyard: FUNCTION 0055d2e0
ACarSteer::ACarSteer()
{
	m_vPosition      = TVector4::VEC_ZERO;
	m_vVelocity      = TVector4::VEC_ZERO;
	m_vDirection     = TVector4::VEC_POSZ;
	m_vMoveDirection = TVector4::VEC_POSZ;
	m_vUp            = TVector4::VEC_NEGY;
	m_vRight         = TVector4::VEC_POSX;
	m_vUnk100        = TVector4::VEC_ZERO;
	m_vBodyOffset    = TVector4::VEC_ZERO;
	m_bEngineStarted = TFALSE;

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		m_avWheelPositions[ i ] = TVector4::VEC_ZERO;
		m_avWheelOffsets[ i ]   = TVector4::VEC_ZERO;
	}

	m_fFriction            = 0.9f;
	m_fMaxSteeringAngle    = 25.0f;
	m_fMaxSpeed            = 45.0f;
	m_fMinSpeed            = -10.0f;
	m_fAcceleration        = 14.0f;
	m_fRollingDeceleration = 9.0f;
	m_fBrakeDeceleration   = 50.0f;
	m_fSteerSpeed          = 15.0f;
	m_fSteerReturnScale    = 10.0f;
	m_fCollisionSpeedScale = 0.9f;
	m_fUnk48               = 1.0f;
	m_fSkidAmount          = 1.0f;
	m_fSlopeScale          = 1.0f;
	m_fWheelBase           = 1.0f;
	m_fCollisionTimer      = 0.0f;
	m_fUnk7C               = 0.0f;
	m_bCollided            = TFALSE;
	m_bTurning             = TFALSE;
	m_fWheelHeight         = 0.0f;
	m_fSpeed               = 0.0f;
	m_fSteeringAngle       = 0.0f;
	m_fSteerAmount         = 0.0f;
	m_fSlideAngle          = 87.0f;
	m_fSkidMinSpeed        = 30.0f;
	m_fSkidMinAngle        = 5.0f;
	m_fUnk5C               = 1.5f;
	m_fCollisionTurnSpeed  = 2.0f;
	m_fGravity             = 9.8f;
	m_fCollisionCooldown   = 2.0f;
	m_ePedal               = PEDAL_BRAKE_ALT;
	m_eSteer               = STEER_ABSOLUTE;
	m_fRadius              = 2.0f;
	m_oTransform           = TMatrix44::IDENTITY;
	m_iNumObstacles        = 0;
	m_fObstacleRadius      = 0.0f;
}

// $Barnyard: FUNCTION 0055cff0
// $Barnyard: FUNCTION 0055d670
ACarSteer::~ACarSteer()
{
}

// $Barnyard: FUNCTION 0055d000
AFunctionTable* ACarSteer::GetFunctionTable()
{
	return &ms_oFunctionTable;
}

// $Barnyard: FUNCTION 0055c910
TBOOL ACarSteer::SetPosition3( const TVector3& a_rcPosition )
{
	TVector4 vPosition( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	SetPosition( vPosition );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055d010
void ACarSteer::GetSeatMatrix( TMatrix44& a_rOutMatrix )
{
	a_rOutMatrix = m_oTransform;
}

// $Barnyard: FUNCTION 0055dc00
void ACarSteer::Reset( TVector4 a_vPosition, TVector4 a_vDirection )
{
	m_vPosition = a_vPosition;
	PlaceOnGround( a_vDirection );
}

// $Barnyard: FUNCTION 0055d280
void ACarSteer::SetPedal( PEDAL a_ePedal, TBOOL a_bGradual )
{
	m_ePedal = a_ePedal;
}

// $Barnyard: FUNCTION 0055d290
void ACarSteer::SetSteering( STEER a_eSteer, TFLOAT a_fSteering, TFLOAT a_fAngle )
{
	m_eSteer       = a_eSteer;
	m_fSteerAmount = a_fAngle;
}

// $Barnyard: FUNCTION 0055d690
void ACarSteer::PlaceOnGround( TVector4 a_vDirection )
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	TFLOAT fGroundHeight = pTerrain->GetGroundHeightTop( m_vPosition.x, m_vPosition.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );
	m_vDirection         = a_vDirection;
	m_vPosition.y        = fGroundHeight - ( m_fRadius + m_fRadius );
	m_vDirection.y       = 0.0f;
	m_vDirection.Normalise();

	m_vUp = TVector4::VEC_NEGY;
	m_vRight.CrossProduct( m_vDirection, m_vUp );

	// Put each wheel on the ground
	for ( TINT i = NUM_WHEELS - 1; i >= 0; i-- )
	{
		m_oTransform                                       = TMatrix44::IDENTITY;
		m_oTransform.AsBasisVector3( BASISVECTOR_RIGHT )   = m_vRight.AsVector3();
		m_oTransform.AsBasisVector3( BASISVECTOR_UP )      = m_vDirection.AsVector3();
		m_oTransform.AsBasisVector3( BASISVECTOR_FORWARD ) = m_vUp.AsVector3();
		m_oTransform.GetTranslation()                      = m_vPosition;

		TVector4 vWheelPosition;
		TMatrix44::TransformVector( vWheelPosition, m_oTransform, m_avWheelOffsets[ i ] );
		m_oTransform.GetTranslation() = vWheelPosition;

		m_avWheelPositions[ i ]   = vWheelPosition;
		m_avWheelPositions[ i ].y = pTerrain->GetGroundHeightTop( m_avWheelPositions[ i ].x, m_avWheelPositions[ i ].z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - m_fWheelHeight;
	}

	// Orient the car along its wheels
	m_vAxle1.Set( m_avWheelPositions[ 0 ].x + m_avWheelPositions[ 1 ].x, m_avWheelPositions[ 0 ].y + m_avWheelPositions[ 1 ].y, m_avWheelPositions[ 0 ].z + m_avWheelPositions[ 1 ].z, m_avWheelPositions[ 0 ].w );
	m_vAxle1.Multiply( 0.5f );

	m_vAxle2.Set( m_avWheelPositions[ 2 ].x + m_avWheelPositions[ 3 ].x, m_avWheelPositions[ 2 ].y + m_avWheelPositions[ 3 ].y, m_avWheelPositions[ 2 ].z + m_avWheelPositions[ 3 ].z, m_avWheelPositions[ 2 ].w );
	m_vAxle2.Multiply( 0.5f );

	TVector4 vForward( m_vAxle2.x - m_vAxle1.x, m_vAxle2.y - m_vAxle1.y, m_vAxle2.z - m_vAxle1.z, m_vAxle2.w );
	vForward.Normalise();

	TVector4 vToWheel3( m_avWheelPositions[ 3 ].x - m_vAxle1.x, m_avWheelPositions[ 3 ].y - m_vAxle1.y, m_avWheelPositions[ 3 ].z - m_vAxle1.z, m_avWheelPositions[ 3 ].w );

	m_vUp.CrossProduct( vForward, vToWheel3 );
	m_vUp.Normalise();
	m_vRight.CrossProduct( vForward, m_vUp );
	m_vRight.Normalise();

	m_oTransform                                       = TMatrix44::IDENTITY;
	m_oTransform.AsBasisVector3( BASISVECTOR_RIGHT )   = m_vRight.AsVector3();
	m_oTransform.AsBasisVector3( BASISVECTOR_UP )      = vForward.AsVector3();
	m_oTransform.AsBasisVector3( BASISVECTOR_FORWARD ) = m_vUp.AsVector3();
	m_oTransform.GetTranslation()                      = m_avWheelPositions[ 0 ];

	TVector4 vBodyPosition;
	TMatrix44::TransformVector( vBodyPosition, m_oTransform, m_vBodyOffset );
	m_oTransform.GetTranslation() = vBodyPosition;

	m_vBodyPosition = vBodyPosition;
}

// $Barnyard: FUNCTION 0055e7e0
TBOOL ACarSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );
	SetupWheels();

	a_pOther->CopyPosition( m_vPosition );
	a_pOther->CopyDirection( m_vDirection );
	a_pOther->CopyVelocity( m_vVelocity );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055e750
TBOOL ACarSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );
	SetupWheels();

	m_vPosition  = a_rcPosition;
	m_vDirection = a_rcDirection;
	m_vVelocity  = TVector4::VEC_ZERO;

	return TTRUE;
}

// $Barnyard: FUNCTION 0055de90
void ACarSteer::SetupWheels()
{
	AModelInstance* pModelInstance = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject()->GetModelInstance();

	// NOTE: the wheels 3 and 4 are swapped, so the wheels 0, 1 and 2, 3 are on the same axle
	static const TCHAR* s_aWheelBones[ NUM_WHEELS ] = { "wheel01", "wheel02", "wheel04", "wheel03" };

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
	{
		TINT iBone = pModelInstance->GetBoneID( TPString8( s_aWheelBones[ i ] ) );

		TMatrix44 oBoneTransform;
		pModelInstance->GetScaledBoneTransform( iBone, oBoneTransform );

		m_avWheelOffsets[ i ] = oBoneTransform.GetTranslation();
	}

	m_vBodyOffset.Set(
	    TVector4::VEC_ZERO.x - m_avWheelOffsets[ 0 ].x,
	    TVector4::VEC_ZERO.y - m_avWheelOffsets[ 0 ].y,
	    TVector4::VEC_ZERO.z - m_avWheelOffsets[ 0 ].z,
	    TVector4::VEC_ZERO.w
	);

	TVector4 vAxle( m_avWheelOffsets[ 1 ].x + m_avWheelOffsets[ 0 ].x, m_avWheelOffsets[ 1 ].y + m_avWheelOffsets[ 0 ].y, m_avWheelOffsets[ 1 ].z + m_avWheelOffsets[ 0 ].z, m_avWheelOffsets[ 0 ].w );
	m_vAxle1.Multiply( vAxle, 0.5f );

	vAxle.Set( m_avWheelOffsets[ 2 ].x + m_avWheelOffsets[ 3 ].x, m_avWheelOffsets[ 3 ].y + m_avWheelOffsets[ 2 ].y, m_avWheelOffsets[ 3 ].z + m_avWheelOffsets[ 2 ].z, m_avWheelOffsets[ 2 ].w );
	m_vAxle2.Multiply( vAxle, 0.5f );

	m_fWheelBase = TVector4::Distance( m_vAxle2, m_vAxle1 );
	m_fRadius    = TVector4::Distance( m_avWheelOffsets[ 0 ], m_avWheelOffsets[ 2 ] ) * 0.6f;
}

// $Barnyard: FUNCTION 0055f4b0
TBOOL ACarSteer::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bEngineStarted == TTRUE )
	{
		// Rolls down slopes
		TFLOAT fSpeed = m_vDirection.y * m_fGravity * m_fSlopeScale * a_fDeltaTime + m_fSpeed;
		TMath::Clip( fSpeed, m_fMinSpeed, m_fMaxSpeed );
		m_fSpeed = fSpeed;

		// Friction
		TFLOAT fFriction = a_fDeltaTime * m_fFriction;

		m_fUnk7C = m_fUnk7C - 4.0f * fFriction;
		if ( m_fUnk7C < 0.0f )
			m_fUnk7C = 0.0f;

		if ( fSpeed > 0.0f )
			fSpeed = fSpeed - fFriction;
		else
			fSpeed = fFriction + fSpeed;

		TMath::Clip( fSpeed, m_fMinSpeed, m_fMaxSpeed );
		m_fSpeed = fSpeed;

		// Pedals
		switch ( m_ePedal )
		{
			case PEDAL_NONE:
			{
				// Rolling, slows down to a stop
				TFLOAT fDeceleration = a_fDeltaTime * m_fRollingDeceleration;

				if ( m_fSpeed < 0.0f )
				{
					Accelerate( fDeceleration );
					if ( m_fSpeed > 0.0f ) m_fSpeed = 0.0f;
				}
				else
				{
					Accelerate( -fDeceleration );
					if ( m_fSpeed < 0.0f ) m_fSpeed = 0.0f;
				}

				break;
			}
			case PEDAL_THROTTLE:
				Accelerate( a_fDeltaTime * m_fAcceleration );
				m_ePedal = PEDAL_NONE;
				break;
			case PEDAL_BRAKE:
				Accelerate( -( a_fDeltaTime * m_fAcceleration ) );
				m_ePedal = PEDAL_NONE;
				break;
			case PEDAL_BRAKE_ALT:
			{
				// Braking
				TFLOAT fDeceleration = a_fDeltaTime * m_fBrakeDeceleration;

				if ( m_fSpeed < 0.0f )
				{
					Accelerate( fDeceleration );
					if ( m_fSpeed > 0.0f ) m_fSpeed = 0.0f;
				}
				else
				{
					Accelerate( -fDeceleration );
					if ( m_fSpeed < 0.0f ) m_fSpeed = 0.0f;
				}

				break;
			}
		}

		// Steering
		switch ( m_eSteer )
		{
			case STEER_LEFT:
			{
				TFLOAT fAmount = a_fDeltaTime * m_fSteerSpeed;

				if ( m_fSteerAmount - fAmount < 0.0f )
				{
					SetSteeringAngle( m_fSteeringAngle - m_fSteerAmount );
					m_fSteerAmount = 0.0f;
					m_eSteer       = STEER_ABSOLUTE;
				}
				else
				{
					m_fSteerAmount = m_fSteerAmount - fAmount;
					SetSteeringAngle( -fAmount + m_fSteeringAngle );
				}

				break;
			}
			case STEER_RIGHT:
			{
				TFLOAT fAmount = a_fDeltaTime * m_fSteerSpeed;

				if ( m_fSteerAmount - fAmount < 0.0f )
				{
					SetSteeringAngle( m_fSteeringAngle + m_fSteerAmount );
					m_fSteerAmount = 0.0f;
					m_eSteer       = STEER_ABSOLUTE;
				}
				else
				{
					m_fSteerAmount = m_fSteerAmount - fAmount;
					SetSteeringAngle( fAmount + m_fSteeringAngle );
				}

				break;
			}
			case STEER_ABSOLUTE:
				// Straighten the wheels
				if ( m_fSteeringAngle != 0.0f )
				{
					TFLOAT fAmount = m_fSteerReturnScale * m_fSteerSpeed * a_fDeltaTime;

					if ( 0.0f > m_fSteeringAngle )
					{
						AddSteeringAngle( fAmount );

						if ( 0.0f < m_fSteeringAngle )
						{
							m_fSteeringAngle = 0.0f;
							m_bTurning       = TFALSE;
						}
					}
					else
					{
						AddSteeringAngle( -fAmount );

						if ( 0.0f > m_fSteeringAngle )
						{
							m_fSteeringAngle = 0.0f;
							m_bTurning       = TFALSE;
						}
					}
				}

				break;
		}

		m_fCollisionTimer -= ( m_fCollisionTimer - a_fDeltaTime >= 0.0f ) ? a_fDeltaTime : m_fCollisionTimer;
	}

	UpdateMovement( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 0055d2b0
void ACarSteer::Accelerate( TFLOAT a_fAmount )
{
	TFLOAT fSpeed = a_fAmount + m_fSpeed;
	TMath::Clip( fSpeed, m_fMinSpeed, m_fMaxSpeed );
	m_fSpeed = fSpeed;
}

// $Barnyard: FUNCTION 0055d1f0
void ACarSteer::SetSteeringAngle( TFLOAT a_fAngle )
{
	TMath::Clip( a_fAngle, -m_fMaxSteeringAngle, m_fMaxSteeringAngle );

	m_fSteeringAngle = a_fAngle;
	m_bTurning       = TFALSE;

	if ( a_fAngle != 0.0f )
	{
		// Turning radius from the wheel base
		m_fTurningRadius = m_fWheelBase / TMath::Tan( a_fAngle * s_fDegToRad );
		TMath::Clip( m_fTurningRadius, -300.0f, 300.0f );

		m_bTurning = TTRUE;
	}
}

// $Barnyard: FUNCTION 0055d260
void ACarSteer::AddSteeringAngle( TFLOAT a_fAmount )
{
	SetSteeringAngle( a_fAmount + m_fSteeringAngle );
}

// $Barnyard: FUNCTION 0055e840
void ACarSteer::UpdateMovement( TFLOAT a_fDeltaTime )
{
	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	TFLOAT   fDistance = a_fDeltaTime * m_fSpeed;
	TVector4 vStart    = m_vPosition;

	// Where the car would go along its movement direction (used when skidding)
	TVector4 vTarget;
	vTarget.Multiply( m_vMoveDirection, fDistance );
	vTarget.Set( vTarget.x + m_vPosition.x, vTarget.y + m_vPosition.y, vTarget.z + m_vPosition.z, m_vPosition.w );
	vTarget.y = pTerrain->GetGroundHeightTop( vTarget.x, vTarget.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - m_fWheelHeight;

	if ( m_bTurning == TTRUE )
	{
		// Drive along the turning circle
		TFLOAT fAngle = ( fDistance * 360.0f ) / ( TMath::TWO_PI * m_fTurningRadius );

		TVector4 vCentre;
		vCentre.Multiply( m_vRight, m_fTurningRadius );
		vCentre.Set( vCentre.x + m_vPosition.x, m_vPosition.y, vCentre.z + m_vPosition.z, m_vPosition.w );

		TVector4 vOffset( m_vPosition.x - vCentre.x, m_vPosition.y - vCentre.y, m_vPosition.z - vCentre.z, m_vPosition.w );
		vOffset.AsVector3().RotateY( fAngle * s_fDegToRad );

		m_vPosition.Set( vOffset.x + vCentre.x, vOffset.y + vCentre.y, vOffset.z + vCentre.z, vCentre.w );
		m_vDirection.Set( m_vPosition.x - vCentre.x, m_vPosition.y - vCentre.y, m_vPosition.z - vCentre.z, m_vPosition.w );

		if ( m_fTurningRadius > 0.0f )
			m_vDirection.AsVector3().RotateY( TMath::HALF_PI );
		else
			m_vDirection.AsVector3().RotateY( -TMath::HALF_PI );

		m_vDirection.Normalise();
	}
	else
	{
		TVector4 vMove;
		vMove.Multiply( m_vDirection, fDistance );

		m_vPosition.x += vMove.x;
		m_vPosition.y += vMove.y;
		m_vPosition.z += vMove.z;
	}

	if ( m_fSpeed > m_fSkidMinSpeed && TMath::Abs( m_fSteeringAngle ) > m_fSkidMinAngle )
	{
		// Skidding: drift towards the previous movement direction
		TVector4 vSkid;
		vSkid.Substract( vTarget, m_vPosition );

		TVector4 vSkidScaled;
		vSkidScaled.Multiply( vSkid, m_fSkidAmount );

		TVector4 vSkidMove;
		vSkidMove.Multiply( vSkidScaled, m_fSteeringAngle / m_fMaxSteeringAngle );

		m_vPosition.x += vSkidMove.x;
		m_vPosition.y += vSkidMove.y;
		m_vPosition.z += vSkidMove.z;

		if ( m_fSpeed > 0.0f )
			Accelerate( a_fDeltaTime * m_fAcceleration * -1.5f );
	}
	else
	{
		m_vMoveDirection = m_vDirection;
	}

	// Sweep from the old position to the new one on the ground
	TFLOAT   fGroundHeight = pTerrain->GetGroundHeightTop( m_vPosition.x, m_vPosition.z, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - ( m_fRadius + m_fRadius );
	TVector4 vMoveDir( m_vPosition.x - vStart.x, fGroundHeight - vStart.y, m_vPosition.z - vStart.z, m_vPosition.w );

	TFLOAT fRemaining     = vMoveDir.NormaliseMag();
	TBOOL  bCollided      = TFALSE;
	AUnit* pHitUnit       = TNULL;
	TINT   iNumCollisions = 0;

	if ( fRemaining > 0.0f )
	{
		do
		{
			TFLOAT   fStep = fRemaining;
			TVector4 vNormal;
			TFLOAT   fStepHeight;
			TFLOAT   fPenetration;

			TBOOL bHitTerrain = pTerrain->CollideSweptSphere( vStart, m_fRadius, vMoveDir, fStep, vNormal, 0.0f, fStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG( 0 ) );

			// NOTE: the velocities after the impact aren't used
			TVector4 vVelocity;
			TVector4 vOtherVelocity;
			TBOOL    bHitCar = CollideCars( vStart, vMoveDir, fStep, vNormal, vVelocity, vOtherVelocity, pHitUnit );

			if ( bHitCar == TTRUE )
				bHitTerrain = TFALSE;

			if ( CollideObstacles( vStart, vMoveDir, fStep, vNormal ) == TTRUE )
			{
				bHitTerrain = TTRUE;
				bHitCar     = TFALSE;
			}

			TVector4 vStep;
			vStep.Multiply( vMoveDir, fStep );

			vStart.x += vStep.x;
			vStart.y += vStep.y;
			vStart.z += vStep.z;
			fRemaining = fRemaining - fStep;

			if ( bHitTerrain == TTRUE || bHitCar == TTRUE )
			{
				iNumCollisions++;
				bCollided = TTRUE;

				TVector4 vSlide = vNormal;

				if ( iNumCollisions >= 10 )
				{
					vMoveDir = vNormal;
					break;
				}

				if ( iNumCollisions > 5 )
				{
					vMoveDir = vNormal;
					continue;
				}

				// NOTE: retail has the same code twice, for bHitTerrain and for bHitCar
				// Slide along what was hit
				TFLOAT fNumCollisions = TFLOAT( iNumCollisions );
				vSlide.AsVector3().RotateY( ( m_fSlideAngle - fNumCollisions ) * s_fDegToRad );

				if ( TVector4::DotProduct3( vSlide, vMoveDir ) < 0.0f )
				{
					vSlide = vNormal;
					vSlide.AsVector3().RotateY( ( fNumCollisions - m_fSlideAngle ) * s_fDegToRad );
				}

				TVector4 vSlideMove;
				vSlideMove.Multiply( vSlide, fRemaining );

				TFLOAT fSlideX = vSlideMove.x + vStart.x;
				TFLOAT fSlideZ = vSlideMove.z + vStart.z;
				TFLOAT fSlideY = pTerrain->GetGroundHeightTop( fSlideX, fSlideZ, TCollisionCommon::TOSHICGROUPFLAG( 0 ) ) - ( m_fRadius + m_fRadius );

				vMoveDir.Set( fSlideX - vStart.x, fSlideY - vStart.y, fSlideZ - vStart.z, vStart.w );
				vMoveDir.Normalise();
			}
		} while ( fRemaining > 0.0f );

		if ( bCollided == TTRUE )
		{
			if ( m_fCollisionTimer < 0.001f )
			{
				m_fCollisionTimer = m_fCollisionCooldown;
				m_bCollided       = TTRUE;
			}

			TVector4 vFlatDirection = m_vDirection;
			vFlatDirection.y        = 0.0f;
			vFlatDirection.Normalise();

			// Lose speed
			TFLOAT fSpeed = m_fCollisionSpeedScale * m_fSpeed;
			TMath::Clip( fSpeed, m_fMinSpeed, m_fMaxSpeed );
			m_fSpeed = fSpeed;

			// Turn towards the direction of the slide
			TFLOAT fTurn = a_fDeltaTime * m_fCollisionTurnSpeed;

			if ( m_eSteer == STEER_ABSOLUTE )
			{
				if ( !vFlatDirection.IsEqual( vMoveDir ) )
				{
					TFLOAT fAngle = m_vDirection.GetAngleBetweenVector( vMoveDir );

					TVector4 vAxis;
					vAxis.CrossProduct( vFlatDirection, vMoveDir );
					vAxis.x = 0.0f;
					vAxis.z = 0.0f;
					vAxis.Normalise();

					if ( fTurn > fAngle )
						fTurn = fAngle;

					if ( vAxis.IsEqual( TVector4::VEC_NEGY ) )
						fTurn = -fTurn;

					if ( m_fSpeed < 0.0f )
						fTurn = -fTurn;

					m_vDirection.AsVector3().RotateY( fTurn );
				}
			}
			else if ( m_eSteer == STEER_LEFT )
			{
				m_vDirection.AsVector3().RotateY( -fTurn );
			}
			else if ( m_eSteer == STEER_RIGHT )
			{
				m_vDirection.AsVector3().RotateY( fTurn );
			}
		}
	}

	Reset( vStart, m_vDirection );
}

// $Barnyard: FUNCTION 0055e2b0
TBOOL ACarSteer::CollideCars( TVector4 a_vPosition, TVector4 a_vDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal, TVector4& a_rOutVelocity, TVector4& a_rOutOtherVelocity, AUnit*& a_rpHitUnit )
{
	AUnit* pOwner = m_oOwner->GetUnit();
	TBOOL  bHit   = TFALSE;

	auto& llUnits = AUnitManager::GetSingleton()->GetUnits();

	for ( auto it = llUnits.Begin(); it != llUnits.End(); it++ )
	{
		AUnit* pUnit = it;

		if ( pOwner == pUnit || pUnit->GetSteer()->IsA( &TGetClass( ACarSteer ) ) != TTRUE )
			continue;

		ASteer*  pSteer       = pUnit->GetSteer();
		TFLOAT   fOtherRadius = pSteer->GetRadius();
		TVector4 vOtherCenter = *pSteer->GetPosition();
		vOtherCenter.y        = a_vPosition.y;

		if ( TMath::SphereVsSphere( a_vPosition, m_fRadius, vOtherCenter, fOtherRadius ) )
		{
			// Already touching: stop if moving towards the other car
			a_rpHitUnit = pUnit;
			bHit        = TTRUE;

			TVector4 vToOther( vOtherCenter.x - a_vPosition.x, vOtherCenter.y - a_vPosition.y, vOtherCenter.z - a_vPosition.z, vOtherCenter.w );

			if ( TVector4::DotProduct3( a_vDirection, vToOther ) > 0.0f )
				a_rfDistance = 0.0f;
		}
		else if ( TMath::SweptSphereVsSphere( a_vPosition, m_fRadius, a_vDirection, vOtherCenter, fOtherRadius, a_rfDistance, a_rNormal, 0.001f ) )
		{
			bHit        = TTRUE;
			a_rpHitUnit = pUnit;
		}
	}

	if ( bHit )
	{
		ASteer* pOtherSteer = a_rpHitUnit->GetSteer();

		TVector4 vMove;
		vMove.Multiply( a_vDirection, a_rfDistance );

		TVector4 vContact( vMove.x + a_vPosition.x, vMove.y + a_vPosition.y, vMove.z + a_vPosition.z, a_vPosition.w );
		TVector4 vOtherPosition = *pOtherSteer->GetPosition();
		TVector4 vOtherVelocity = pOtherSteer->GetVelocity();
		TVector4 vVelocity      = GetVelocity();

		TVector4 vImpactDir( vOtherPosition.x - vContact.x, vOtherPosition.y - vContact.y, vOtherPosition.z - vContact.z, vOtherPosition.w );
		vImpactDir.Normalise();

		// Exchange the velocities along the impact direction
		TFLOAT fImpulse = TVector4::DotProduct3( vVelocity, a_rNormal ) - TVector4::DotProduct3( vOtherVelocity, a_rNormal );

		TVector4 vImpulse;
		vImpulse.Multiply( vImpactDir, fImpulse );
		a_rOutVelocity.Set( vVelocity.x - vImpulse.x, vVelocity.y - vImpulse.y, vVelocity.z - vImpulse.z, vVelocity.w );

		vImpulse.Multiply( vImpactDir, fImpulse );
		a_rOutOtherVelocity.Set( vImpulse.x + vOtherVelocity.x, vImpulse.y + vOtherVelocity.y, vImpulse.z + vOtherVelocity.z, vOtherVelocity.w );

		bHit = TTRUE;

		a_rNormal.Set( a_vPosition.x - vOtherPosition.x, a_vPosition.y - vOtherPosition.y, a_vPosition.z - vOtherPosition.z, a_vPosition.w );
		a_rNormal.Normalise();
	}

	return bHit;
}

// $Barnyard: FUNCTION 0055dc80
TBOOL ACarSteer::CollideObstacles( const TVector4& a_rcPosition, const TVector4& a_rcDirection, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	TFLOAT fDistance = a_rfDistance;
	TBOOL  bHit      = TFALSE;

	for ( TINT i = m_iNumObstacles; i != 0; )
	{
		i--;

		// The obstacle is a plane along its direction
		TVector4 vPlanePoint  = *m_ppObstacles[ i ]->GetUnit()->GetSteer()->GetPosition();
		TVector4 vPlaneNormal = *m_ppObstacles[ i ]->GetUnit()->GetSteer()->GetDirection();
		vPlaneNormal.AsVector3().RotateY( TMath::HALF_PI );

		vPlanePoint.y = a_rcPosition.y;

		if ( TMath::SweptSphereVsPlane( a_rcPosition, a_rcDirection, m_fRadius, a_rfDistance, vPlanePoint, vPlaneNormal ) )
		{
			TVector4 vMove;
			vMove.Multiply( a_rcDirection, a_rfDistance );

			TVector4 vContact( vMove.x + a_rcPosition.x, vMove.y + a_rcPosition.y, vMove.z + a_rcPosition.z, a_rcPosition.w );

			// Only within its radius
			if ( TMath::SphereVsSphere( vContact, m_fRadius, vPlanePoint, m_fObstacleRadius ) )
			{
				fDistance = a_rfDistance;
				a_rNormal = *m_ppObstacles[ i ]->GetUnit()->GetSteer()->GetDirection();
				bHit      = TTRUE;
			}
			else
			{
				a_rfDistance = fDistance;
			}
		}
	}

	return bHit;
}

// $Barnyard: FUNCTION 0055dba0
TBOOL ACarSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vPosition = a_rcPosition;
	PlaceOnGround( TVector4::VEC_POSZ );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055dc50
void ACarSteer::SetDirection( const TVector4& a_rcDirection )
{
	// NOTE: retail ignores a_rcDirection and keeps the current direction
	PlaceOnGround( m_vDirection );
}

// $Barnyard: FUNCTION 0055d050
void ACarSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vPosition;
}

// $Barnyard: FUNCTION 0055d030
void ACarSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vPosition.AsVector3();
}

// $Barnyard: FUNCTION 0055d080
const TVector4* ACarSteer::GetPosition()
{
	return &m_vPosition;
}

// $Barnyard: FUNCTION 0055d0d0
void ACarSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vDirection;
}

// $Barnyard: FUNCTION 0055d0b0
void ACarSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vDirection.AsVector3();
}

// $Barnyard: FUNCTION 0055d0a0
const TVector4* ACarSteer::GetDirection()
{
	return &m_vDirection;
}

// $Barnyard: FUNCTION 0055d160
void ACarSteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	TVector4 vVelocity = m_vDirection;
	vVelocity.Multiply( m_fSpeed );

	a_rOutVelocity = vVelocity;
}

// $Barnyard: FUNCTION 0055d100
TVector4 ACarSteer::GetVelocity()
{
	TVector4 vVelocity = m_vDirection;
	vVelocity.Multiply( m_fSpeed );

	return vVelocity;
}

// $Barnyard: FUNCTION 0055d1c0
void ACarSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vVelocity = a_rcVelocity;
}
