#include "pch.h"
#include "APhysicsBike.h"
#include "AVehicleManager.h"
#include "ACollisionManager.h"
#include "ACollisionObjectSimple.h"
#include "ACollisionCommon.h"
#include "Terrain/ATerrainInterface.h"

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Tweakables (retail globals)
static TFLOAT s_fUprightAngle          = 1.0471976f; // 0x00779958, the bike can only stop when it's tilted less than this
static TFLOAT s_fRiderlessStopSpeed    = 0.7777778f; // 0x0077995c
static TFLOAT s_fBrakeStopSpeed        = 0.5f;       // 0x00779960, below it the brake stops the bike at once
static TFLOAT s_fWheelieStiffness      = 5000.0f;    // 0x00779964
static TFLOAT s_fStoppieStiffness      = 5000.0f;    // 0x00779968
static TFLOAT s_fHandBrakeTurnStrength = 4.0f;       // 0x0077996c
static TFLOAT s_fMaxLeanTorque         = 2000.0f;    // 0x00779970
static TFLOAT s_fLeanStiffness         = 1000.0f;    // 0x00779974
static TFLOAT s_fThrowRiderImpulse     = 1050.0f;    // 0x0077997c
static TFLOAT s_fAirPitchStrength      = 2.0f;       // 0x00779950
static TFLOAT s_fAirYawStrength        = 2.0f;       // 0x00779954
static TBOOL  s_bSweptBoxCollision     = TTRUE;      // 0x00779978
static TBOOL  s_bSimulate              = TTRUE;      // 0x00779980
static TBOOL  s_bHandBrakeTurn         = TFALSE;     // 0x007b5f64

// NOTE: retail initialises these with cos() of double constants (85 and 60 degrees) in dynamic initialisers (0070fe60, 0070fe70)
static TFLOAT s_fCos85 = TFLOAT( cos( 1.4835298527032137 ) ); // 0x007b5f70
static TFLOAT s_fCos60 = TFLOAT( cos( 1.0471975430846214 ) ); // 0x007b5f74

//-----------------------------------------------------------------------------
// ABikePhysicsEventHandler
//-----------------------------------------------------------------------------

ABikePhysicsEventHandler::ABikePhysicsEventHandler()
    : m_pBike( TNULL )
{
}

// NOTE: 00656540 is shared with the event handler of APhysicsCar (APhysicsCar+0xda0)
// $Barnyard: FUNCTION 00656540
// $Barnyard: FUNCTION 00655370
ABikePhysicsEventHandler::~ABikePhysicsEventHandler()
{
}

// $Barnyard: FUNCTION 00656550
void ABikePhysicsEventHandler::OnStaticContact( ARigidBodyContact* a_pContact )
{
	if ( m_pBike )
	{
		APhysicsVehicleContact* pContact = m_pBike->AddContact();

#ifdef BARNYARD_COMMUNITY_PATCH
		// Retail writes to TNULL when there are already 20 contacts
		if ( !pContact )
			return;
#endif // BARNYARD_COMMUNITY_PATCH

		pContact->fImpulse          = a_pContact->m_fImpulse;
		pContact->vNormal           = a_pContact->m_vNormal;
		pContact->vRelativeVelocity = m_pBike->m_oBody.m_vVelocity;
		pContact->pOtherBody        = TNULL;
		pContact->pCollisionObject  = a_pContact->m_pCollisionObject;
		pContact->iSurfaceType      = a_pContact->m_iSurfaceType;
		pContact->vPosition         = a_pContact->m_vPosition;
	}
}

// $Barnyard: FUNCTION 00653d20
void ABikePhysicsEventHandler::OnBodyContact( ARigidBodyContact* a_pContact )
{
	if ( m_pBike )
	{
		APhysicsVehicleContact* pContact = m_pBike->AddContact();

#ifdef BARNYARD_COMMUNITY_PATCH
		// Retail writes to TNULL when there are already 20 contacts
		if ( !pContact )
			return;
#endif // BARNYARD_COMMUNITY_PATCH

		pContact->fImpulse = a_pContact->m_fImpulse;
		pContact->vNormal  = a_pContact->m_vNormal;

		const TVector4& rcVelocity      = m_pBike->m_oBody.m_vVelocity;
		const TVector4& rcOtherVelocity = a_pContact->m_pOtherBody->m_vVelocity;
		pContact->vRelativeVelocity.Set( rcVelocity.x - rcOtherVelocity.x, rcVelocity.y - rcOtherVelocity.y, rcVelocity.z - rcOtherVelocity.z, rcVelocity.w );

		pContact->pOtherBody       = a_pContact->m_pOtherBody;
		pContact->pCollisionObject = a_pContact->m_pCollisionObject;
		pContact->iSurfaceType     = a_pContact->m_iSurfaceType;
		pContact->vPosition        = a_pContact->m_vPosition;
	}
}

//-----------------------------------------------------------------------------
// APhysicsBike
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00655390
APhysicsBike::APhysicsBike()
    : m_fLean( 0.0f )
    , m_bJump( TFALSE )
    , m_bJumping( TFALSE )
    , m_fLeanAngle( 0.0f )
    , m_fPitch( 0.0f )
    , m_fPrevPitch( 0.0f )
    , m_bRiderless( TFALSE )
    , m_bForceThrowRider( TFALSE )
    , m_bWheelie( TFALSE )
    , m_bStoppie( TFALSE )
    , m_vThrowVelocity( TVector4::VEC_ZERO )
    , m_vThrowDirection( TVector4::VEC_POSZ )
    , m_bThrowRider( TFALSE )
    , m_eThrowDirection( THROWDIRECTION_NONE )
    , m_bPowerSlide( TFALSE )
{
}

// $Barnyard: FUNCTION 00653df0
// $Barnyard: FUNCTION 006554b0
APhysicsBike::~APhysicsBike()
{
	m_oBody.m_pEventHandler = TNULL;
	AVehicleManager::RemoveRigidBody( &m_oBody );
}

// $Barnyard: FUNCTION 00653ea0
void APhysicsBike::Setup( const APhysicsBikeParams& a_rcBikeParams, const APhysicsVehicleParams& a_rcVehicleParams, const APhysicsWheelParams& a_rcFrontWheel, const APhysicsWheelParams& a_rcRearWheel )
{
	m_fMass                = a_rcVehicleParams.fMass;
	m_vSize                = a_rcVehicleParams.vSize;
	m_fMaxSteeringAngle    = a_rcVehicleParams.fMaxSteeringAngle;
	m_fSteeringSpeedFactor = a_rcVehicleParams.fSteeringSpeedFactor;
	m_fLowSpeed            = a_rcVehicleParams.fLowSpeed;
	m_fGearShiftDelay      = a_rcVehicleParams.fGearShiftDelay;
	m_iNumWheels           = a_rcVehicleParams.iNumWheels;
	m_fRollingResistance   = a_rcVehicleParams.fRollingResistance;
	m_fSurfaceDrag         = a_rcVehicleParams.fSurfaceDrag;
	m_fSurfaceDrag7        = a_rcVehicleParams.fSurfaceDrag7;
	m_fSurfaceDrag0        = a_rcVehicleParams.fSurfaceDrag0;

	m_oParams = a_rcBikeParams;

	m_oBody.SetBoxInertia( m_fMass / ( m_vSize.z * m_vSize.y * m_vSize.x ), m_vSize );
	m_oBody.m_fMass     = m_fMass;
	m_oBody.m_vForce    = TVector4::VEC_ZERO;
	m_oBody.m_vTorque   = TVector4::VEC_ZERO;
	m_oBody.m_vVelocity = TVector4::VEC_ZERO;
	m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
	m_oBody.m_fAngularDamping = 0.1f;
	m_oBody.m_fLinearDamping  = 0.1f;
	m_oBody.SetPosition( TVector4::VEC_ZERO, TTRUE );
	m_oBody.SetOrientation( TQuaternion::IDENTITY, TTRUE );
	m_oBody.m_fAngularDamping = 0.2f;
	m_oBody.m_fLinearDamping  = 0.1f;
	m_oBody.m_fContactScale   = 0.2f;

	m_aWheels[ 0 ].Create( this, a_rcFrontWheel );
	m_aWheels[ 1 ].Create( this, a_rcRearWheel );

	m_oBody.m_oMaterial = ARigidBodyMaterial( 0.0f, 0.1f, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, 0x70000, 5 );
	m_oBody.m_oShape    = m_oParams.oShape;

	AVehicleManager::RemoveRigidBody( &m_oBody );
	AVehicleManager::AddRigidBody( &m_oBody );

	m_oEventHandler.m_pBike = this;
	m_oBody.m_pEventHandler = &m_oEventHandler;

	if ( m_oParams.oShape.m_eType == ARigidBodyShape::TYPE_BOX )
		m_vCentreOfMass2 = m_oParams.oShape.m_oBox.GetCentre();
	else
		m_vCentreOfMass2.Set( 0.0f, m_oParams.oShape.m_aPoints[ 0 ].y, 0.0f, 1.0f );

	m_bJump        = TFALSE;
	m_bJumping     = TFALSE;
	m_bPowerSlide  = TFALSE;
	m_fMaxThrottle = 2.0f;
}

// $Barnyard: FUNCTION 006541d0
void APhysicsBike::SetLean( TFLOAT a_fLean )
{
	TMath::Clip( a_fLean, -1.0f, 1.0f );
	m_fLean = a_fLean;
}

// $Barnyard: FUNCTION 00655190
TBOOL APhysicsBike::ResetThrowRider()
{
	m_bThrowRider     = TFALSE;
	m_eThrowDirection = THROWDIRECTION_NONE;
	return TFALSE;
}

// $Barnyard: FUNCTION 00653e60
TBOOL APhysicsBike::IsDriverless()
{
	return m_bRiderless;
}

// $Barnyard: FUNCTION 00653e70
TBOOL APhysicsBike::IsSliding()
{
	return m_bPowerSlide;
}

// $Barnyard: FUNCTION 00653e80
TBOOL APhysicsBike::IsWheelie()
{
	return m_bWheelie;
}

// $Barnyard: FUNCTION 00653e90
TBOOL APhysicsBike::IsStoppie()
{
	return m_bStoppie;
}

// NOTE: shared with APhysicsCar (same code)
// $Barnyard: FUNCTION 00657250
void APhysicsBike::VTable0x1C()
{
	if ( m_bUnkCD8 )
	{
		// DOES_NOTHING (005acd80 is called with this)
	}
}

// $Barnyard: FUNCTION 006560e0
void APhysicsBike::PreUpdate( TFLOAT a_fDeltaTime )
{
	APhysicsVehicle::PreUpdate( a_fDeltaTime );

	if ( s_bSimulate )
	{
		TVector4 vDown;
		vDown.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), -1.0f );

		// Is the bike tilted more than 60 degrees to the ground below the front wheel?
		TBOOL bOnSteepGround = TVector4::DotProduct3( m_aWheels[ 0 ].m_vGroundNormal, vDown ) < s_fCos60;

		UpdateGear();
		UpdateSteeringAngle();
		UpdateJump();

		if ( !m_bFrozen )
		{
			UpdateDriveForce();
			UpdateBrakeForce();

			if ( !m_bRiderless || ( m_bRiderless == TTRUE && !bOnSteepGround ) )
				ApplyWheelForces();

			ApplyGravity();

			if ( !m_bRiderless || ( m_bRiderless == TTRUE && !bOnSteepGround ) )
				UpdateSuspension();

			ApplyHandBrakeTurn();
			UpdateWheelie();
			UpdateStoppie();
			ApplyAirControl();
			ApplyRollingResistance();
			ApplySurfaceDrag();

			if ( !m_bRiderless )
			{
				StopAirPitch();
				ApplyAirPitchUpright();
				ApplyGroundPitchDamping();
			}

			ApplyPowerSlideTorque();
		}

		UpdatePowerSlide();

		if ( !m_bRiderless )
			UpdateLean();

		UpdateStopped();
	}
	else
	{
		m_oBody.m_vForce  = TVector4::VEC_ZERO;
		m_oBody.m_vTorque = TVector4::VEC_ZERO;
	}
}

// $Barnyard: FUNCTION 00656290
void APhysicsBike::PostUpdate( TFLOAT a_fDeltaTime )
{
	if ( !m_bRiderless )
	{
		CheckThrowRider();
		FlipUpright();
	}

	UpdateRiderlessStopped();
	UpdateWheelContacts();

	m_fPrevPitch = m_fPitch;

	TFLOAT fSin = -m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ).y;
	TMath::Clip( fSin, -1.0f, 1.0f );
	m_fPitch = TMath::ASin( fSin );

	if ( m_bFrozen == TTRUE && !m_bRiderless && m_iNumContacts > 0 )
		m_bFrozen = TFALSE;

	APhysicsVehicle::PostUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00656330
void APhysicsBike::FlipUpright()
{
	if ( m_bFlipRecovery == TTRUE && m_iNumContacts > 0 && TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) < s_fCos85 )
	{
		ACollisionModelRayHitList oHitList;
		TFLOAT                    fDistance = 10.0f;

		if ( ATerrainInterface::GetSingleton()->CollideRay( m_oBody.m_vCentreOfMass, TVector4::VEC_POSY, fDistance, &oHitList, 0, TFALSE ) && fDistance < 1.5f )
		{
			// Stand the bike up on the ground below it, keeping its direction
			TMatrix44 matTransform = TMatrix44::IDENTITY;

			TVector4 vUp;
			vUp.Set( TVector4::VEC_POSY );

			TVector4 vBack;
			vBack.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ), -1.0f );

			TVector4 vRight;
			vRight.CrossProduct( vUp, vBack );
			vRight.Normalise();

			TVector4 vForward;
			vForward.CrossProduct( vRight, vUp );

			matTransform.AsBasisVector4( BASISVECTOR_RIGHT )   = vRight;
			matTransform.AsBasisVector4( BASISVECTOR_UP )      = vUp;
			matTransform.AsBasisVector4( BASISVECTOR_FORWARD ) = vForward;

			TVector4 vOffset;
			vOffset.Multiply( TVector4::VEC_POSY, fDistance );

			TVector4 vPosition            = m_oBody.m_oTransform.GetTranslation() + vOffset;
			matTransform.GetTranslation() = vPosition;

			TQuaternion qOrientation;
			TMatrix44::MatToQuat( qOrientation, matTransform );

			m_oBody.SetVelocity( TVector4::VEC_ZERO );
			m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
			m_oBody.SetPosition( matTransform.GetTranslation(), TTRUE );
			m_oBody.SetOrientation( qOrientation, TTRUE );
		}
	}
}

// $Barnyard: FUNCTION 00655660
void APhysicsBike::Collide( TFLOAT a_fDeltaTime )
{
	TVector4 vCentreOfMass = m_oBody.m_vCentreOfMassLocal;

	if ( !m_bRiderless )
	{
		// Collide around the centre of the shape
		TVector4 vShapeCentre;

		if ( m_oBody.m_oShape.m_eType == ARigidBodyShape::TYPE_BOX )
			vShapeCentre = m_oBody.m_oShape.m_oBox.GetCentre();
		else
			vShapeCentre.Set( 0.0f, m_oBody.m_oShape.m_aPoints[ 0 ].y, 0.0f, 1.0f );

		m_oBody.SetCentreOfMass( vShapeCentre );
	}

	TINT iNumIterations = AVehicleManager::GetNumIterations();
	AVehicleManager::SetNumIterations( 2 );

	AVehicleManager::ContactSummary oSummary;
	TBOOL                           bCollided;

	if ( m_oBody.m_oShape.m_eType == ARigidBodyShape::TYPE_BOX )
		bCollided = AVehicleManager::CollideBoxAndResolve( &m_oBody, m_oBody.m_oShape.m_oBox, a_fDeltaTime, &oSummary, s_bSweptBoxCollision );
	else
		bCollided = AVehicleManager::CollidePointsAndResolve( &m_oBody, m_oBody.m_oShape.m_aPoints, m_oBody.m_oShape.m_iNumPoints, a_fDeltaTime, &oSummary );

	if ( bCollided )
	{
		TFLOAT fScale = 1.0f / oSummary.iNumContacts;

		for ( TINT i = 0; i < oSummary.iNumContacts; i++ )
		{
			APhysicsVehicleContact* pContact = AddContact();

#ifdef BARNYARD_COMMUNITY_PATCH
			// Retail writes to TNULL when there are already 20 contacts
			if ( !pContact )
				break;
#endif // BARNYARD_COMMUNITY_PATCH

			pContact->vNormal  = oSummary.vImpulse;
			pContact->fImpulse = pContact->vNormal.NormaliseMag() * fScale;
		}
	}

	AVehicleManager::SetNumIterations( iNumIterations );

	if ( !m_bRiderless )
		m_oBody.SetCentreOfMass( vCentreOfMass );
}

// $Barnyard: FUNCTION 00654200
void APhysicsBike::UpdateJump()
{
	if ( m_bJump )
	{
		if ( m_iNumWheelsOnGround > 0 )
		{
			m_bFrozen = TFALSE;

			TVector4 vImpulse;
			vImpulse.Multiply( TVector4::VEC_NEGY, m_oParams.fJumpImpulse );

			m_oBody.m_vForce.x += vImpulse.x;
			m_oBody.m_vForce.y += vImpulse.y;
			m_oBody.m_vForce.z += vImpulse.z;
			m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );

			m_aWheels[ 0 ].m_fLateralForce = 0.0f;
			m_aWheels[ 0 ].m_bEnabled      = TFALSE;
			m_aWheels[ 1 ].m_fLateralForce = 0.0f;
			m_aWheels[ 1 ].m_bEnabled      = TFALSE;
			m_bJumping                     = TTRUE;
		}

		m_bJump = TFALSE;
	}

	if ( !m_bJumping )
	{
		for ( TINT i = 0; i < 2; i++ )
			m_aWheels[ i ].m_bEnabled = TTRUE;
	}
}

// $Barnyard: FUNCTION 006542f0
void APhysicsBike::UpdateGear()
{
	if ( m_iNumWheelsOnGround > 0 )
	{
		TINT iPrevGear;

		if ( m_vLocalVelocity.z > 0.0f && m_oGearBox.iGear == 0 )
		{
			// Moving forward in reverse gear
			iPrevGear             = m_oGearBox.iGear;
			m_oGearBox.iGear      = 2;
			m_oGearBox.iPrevGear  = iPrevGear;
			m_oGearBox.fGearTimer = 0.0f;
		}
		else if ( m_vLocalVelocity.z < 0.0f && m_oGearBox.iGear == 2 && !IsSliding() )
		{
			// Moving backwards in drive gear
			iPrevGear             = m_oGearBox.iGear;
			m_oGearBox.iGear      = 0;
			m_oGearBox.iPrevGear  = iPrevGear;
			m_oGearBox.fGearTimer = 0.0f;
		}
	}

	if ( m_oGearBox.iGear == 1 )
	{
		if ( m_fThrottle > 0.0f )
		{
			m_oGearBox.iPrevGear  = 1;
			m_oGearBox.iGear      = 2;
			m_bFrozen             = TFALSE;
			m_oGearBox.fGearTimer = 0.0f;
			return;
		}

		if ( m_oGearBox.bAllowReverse == TTRUE && m_fBrake > 0.0f && !IsSliding() )
		{
			// Holding the brake at a stop engages the reverse gear
			m_oGearBox.fGearTimer += m_fDeltaTime;

			if ( m_oGearBox.fGearTimer > m_fGearShiftDelay )
			{
				TINT iPrevGear        = m_oGearBox.iGear;
				m_oGearBox.iGear      = 0;
				m_oGearBox.iPrevGear  = iPrevGear;
				m_bFrozen             = TFALSE;
				m_oGearBox.fGearTimer = 0.0f;
			}

			return;
		}
	}

	if ( m_oGearBox.iGear == 1 )
		m_oGearBox.fGearTimer = 0.0f;
}

// $Barnyard: FUNCTION 00654430
void APhysicsBike::UpdateStopped()
{
	if ( m_bFrozen )
	{
		Freeze();
		m_fStopTimer = 0.0f;
		return;
	}

	if ( m_bRiderless == TTRUE )
		return;

	if ( m_vLocalVelocity.Magnitude() >= 1.0f ||
	     m_iNumWheelsOnGround != 2 ||
	     TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) <= TMath::Cos( s_fUprightAngle ) )
	{
		m_fStopTimer = 0.0f;
		return;
	}

	TINT iGear = m_oGearBox.iGear;

	if ( iGear == 2 && m_fBrake > 0.0f || iGear == 0 && m_fThrottle > 0.0f )
	{
		// Braking against the gear: stop at once
		Freeze();
		m_oGearBox.iPrevGear = m_oGearBox.iGear;
		m_oGearBox.iGear     = 1;
		m_fStopTimer         = 0.0f;
		return;
	}

	if ( iGear == 2 && m_fThrottle <= 0.0f || iGear == 0 && m_fBrake <= 0.0f )
	{
		// Not accelerating: stop after a second
		m_fStopTimer += m_fDeltaTime;

		if ( m_fStopTimer <= 1.0f )
			return;

		Freeze();
		m_oGearBox.iPrevGear = m_oGearBox.iGear;
		m_oGearBox.iGear     = 1;
		m_fStopTimer         = 0.0f;
		return;
	}

	if ( iGear == 1 )
	{
		m_fStopTimer += m_fDeltaTime;

		if ( m_fStopTimer > 1.0f )
		{
			Freeze();
			m_fStopTimer = 0.0f;
		}
	}
}

// $Barnyard: FUNCTION 006545e0
void APhysicsBike::UpdateRiderlessStopped()
{
	if ( !m_bFrozen && m_bRiderless == TTRUE )
	{
		static TFLOAT s_fMaxAngularSpeed = 3.0f / ( 60.0f / 6.2831855f );

		// NOTE: unused
		TVector4 vMovement = m_oBody.m_oTransform.GetTranslation() - m_vPrevPosition;
		TVector4 vVelocity;
		vVelocity.Divide( vMovement, m_fDeltaTime );

		if ( m_oBody.m_vVelocity.Magnitude() < s_fRiderlessStopSpeed && m_oBody.m_vAngularVelocity.Magnitude() < s_fMaxAngularSpeed && m_iNumContacts > 0 )
		{
			m_fStopTimer += m_fDeltaTime;

			if ( m_fStopTimer > 1.0f )
			{
				Freeze();
				m_oGearBox.iPrevGear = m_oGearBox.iGear;
				m_oGearBox.iGear     = 1;
				m_fStopTimer         = 0.0f;
			}
		}
	}
}

// $Barnyard: FUNCTION 006546e0
void APhysicsBike::UpdateBrakeForce()
{
	TINT   iGear  = m_oGearBox.iGear;
	TFLOAT fInput = ( iGear == 0 ) ? m_fThrottle : m_fBrake;
	TFLOAT fSpeed = m_vLocalVelocity.z;
	TFLOAT fForce = 0.0f;

	if ( m_iNumWheelsOnGround > 0 )
	{
		if ( fInput <= 0.0f || TMath::Abs( fSpeed ) >= s_fBrakeStopSpeed )
		{
			fForce = -m_oParams.fBrakeForce * fInput;

			if ( iGear == 0 || m_vLocalVelocity.z < 0.0f )
				fForce = fForce * -1.0f;
		}
		else
		{
			// Slow enough to stop at once
			TVector4 vForwardVelocity;
			vForwardVelocity.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ), fSpeed );

			m_oBody.m_vVelocity.x -= vForwardVelocity.x;
			m_oBody.m_vVelocity.y -= vForwardVelocity.y;
			m_oBody.m_vVelocity.z -= vForwardVelocity.z;
		}
	}

	TFLOAT fFront = 1.0f;
	TFLOAT fRear  = 1.0f;

	if ( m_bUnkCD2 == TTRUE && IsSliding() == TTRUE )
	{
		// Power slide: the rear wheel slides
		fFront = 0.35f;
		fRear  = 0.0f;
	}

	m_aWheels[ 0 ].m_fDriveForce = fFront * fForce * 0.5f;
	m_aWheels[ 1 ].m_fDriveForce = fRear * fForce * 0.5f;
}

// $Barnyard: FUNCTION 00654850
void APhysicsBike::UpdateWheelie()
{
	if ( m_bStoppie )
		return;

	if ( !m_bWheelie )
	{
		if ( m_iNumWheelsOnGround != 2 || m_oGearBox.iGear != 2 || m_fThrottle <= 0.0f || m_fLean >= -0.99000001f || m_vLocalVelocity.z <= m_oParams.fWheelieMinSpeed )
			return;

		m_bWheelie = TTRUE;
	}

	if ( m_fLean > -0.1f && m_iNumWheelsOnGround == 2 || m_vLocalVelocity.z < m_oParams.fWheelieMinSpeed && m_iNumWheelsOnGround == 2 )
	{
		m_bWheelie = TFALSE;
	}
	else
	{
		static TFLOAT s_fWheelieDamping = TMath::Sqrt( s_fWheelieStiffness * m_oBody.GetInertia().x * 4.0f );

		TFLOAT fSpeedFactor = m_vLocalVelocity.z / m_oParams.fWheelieFullSpeed;
		TMath::Clip( fSpeedFactor, 0.0f, 1.0f );

		TFLOAT fLean = m_fLean + m_fLean;
		TMath::Clip( fLean, -1.0f, 1.0f );

		TFLOAT fTorque = ( -( fLean * m_oParams.fWheelieAngle * fSpeedFactor ) - m_fPitch ) * s_fWheelieStiffness -
		    ( m_fPitch - m_fPrevPitch ) / m_fDeltaTime * s_fWheelieDamping;

		TVector4 vTorque;
		vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ), fTorque );

		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;
	}
}

// $Barnyard: FUNCTION 00654a50
void APhysicsBike::UpdateStoppie()
{
	if ( m_bWheelie )
		return;

	if ( !m_bStoppie )
	{
		if ( m_iNumWheelsOnGround != 2 || m_oGearBox.iGear != 2 || m_fBrake <= 0.0f || m_fLean <= 0.99000001f || m_vLocalVelocity.z <= m_oParams.fWheelieMinSpeed )
			return;

		m_bStoppie = TTRUE;
	}

	if ( m_fLean < 0.1f && m_iNumWheelsOnGround == 2 || m_vLocalVelocity.z < m_oParams.fWheelieMinSpeed && m_iNumWheelsOnGround == 2 )
	{
		m_bStoppie = TFALSE;
	}
	else
	{
		static TFLOAT s_fStoppieDamping = TMath::Sqrt( s_fStoppieStiffness * m_oBody.GetInertia().x * 4.0f );

		TFLOAT fSpeedFactor = m_vLocalVelocity.z / m_oParams.fWheelieFullSpeed;
		TMath::Clip( fSpeedFactor, 0.0f, 1.0f );

		TFLOAT fTorque = ( -( fSpeedFactor * m_oParams.fStoppieAngle * m_fLean ) - m_fPitch ) * s_fStoppieStiffness -
		    ( m_fPitch - m_fPrevPitch ) / m_fDeltaTime * s_fStoppieDamping;

		TVector4 vTorque;
		vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ), fTorque );

		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;
	}
}

// $Barnyard: FUNCTION 00654c30
void APhysicsBike::ApplyAirControl()
{
	if ( m_iNumWheelsOnGround != 0 )
		return;

	if ( TMath::Abs( m_fLean ) > 0.89999998f )
	{
		// Pitch
		TVector4 vInertia = m_oBody.GetInertia();
		TFLOAT   fScale   = s_fAirPitchStrength * vInertia.x;

		TVector4 vAxis;
		vAxis.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ), -TMath::Sign( m_fLean ) );

		TVector4 vTorque;
		vTorque.Multiply( vAxis, fScale );

		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;
	}

	if ( TMath::Abs( m_fSteering ) > 0.89999998f )
	{
		// Yaw
		TVector4 vInertia = m_oBody.GetInertia();
		TFLOAT   fScale   = vInertia.y * s_fAirYawStrength;

		TVector4 vAxis;
		vAxis.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TMath::Sign( m_fSteering ) );

		TVector4 vTorque;
		vTorque.Multiply( vAxis, fScale );

		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;
	}
}

// $Barnyard: FUNCTION 00654e30
void APhysicsBike::ApplyHandBrakeTurn()
{
	if ( s_bHandBrakeTurn && m_fHandBrake > 0.0f && m_oGearBox.iGear == 2 && m_iNumWheelsOnGround > 0 )
	{
		TVector4 vAxis = m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP );
		vAxis.x        = 0.0f;
		vAxis.Normalise();

		TFLOAT fTargetYawVelocity = m_oParams.fHandBrakeTurnRate * m_fSteering;
		TFLOAT fYawVelocity       = TVector4::DotProduct3( m_oBody.m_vAngularVelocity, vAxis );

		TVector4 vInertia = m_oBody.GetInertia();

		TVector4 vTorque1;
		vTorque1.Multiply( vAxis, fTargetYawVelocity - fYawVelocity );
		TVector4 vTorque2;
		vTorque2.Multiply( vTorque1, vInertia.y );
		TVector4 vTorque;
		vTorque.Multiply( vTorque2, s_fHandBrakeTurnStrength );

		m_oBody.AddTorque( vTorque );
	}
}

// $Barnyard: FUNCTION 00654f30
void APhysicsBike::UpdateLean()
{
	TFLOAT fCos85 = TFLOAT( cos( 1.483529852703214 ) );

	if ( TMath::Abs( TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) ) >= fCos85 )
	{
		static TFLOAT s_fLeanDamping = TMath::Sqrt( s_fLeanStiffness * m_oBody.GetInertia().z * 4.0f );

		// Target lean angle from the lateral force of the rear wheel
		TFLOAT fMaxLateralForce = m_oBody.m_fMass * 1.96f;
		TFLOAT fLateralForce    = m_aWheels[ 1 ].m_fLateralForce;
		TMath::Clip( fLateralForce, -fMaxLateralForce, fMaxLateralForce );

		TFLOAT fTargetLean;

		if ( m_oBody.m_vVelocity.Magnitude() < 1.0f || m_oGearBox.iGear == 1 || m_iNumWheelsOnGround < 2 )
			fTargetLean = 0.0f;
		else
			fTargetLean = TMath::Abs( m_fSteering ) * m_oParams.fMaxLeanAngle * ( -1.0f / fMaxLateralForce ) * fLateralForce;

		// Current lean angle
		TFLOAT fSin = -m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ).y;

		if ( TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) < 0.0f )
			fSin = fSin * -1.0f;

		TMath::Clip( fSin, -1.0f, 1.0f );
		TFLOAT fLeanAngle = TMath::ASin( fSin );

		TFLOAT fTorque = ( fLeanAngle - m_fLeanAngle ) / m_fDeltaTime * s_fLeanDamping - ( fTargetLean - fLeanAngle ) * s_fLeanStiffness;

		if ( fTorque <= s_fMaxLeanTorque || fSin >= 0.0f )
		{
			if ( fTorque < -s_fMaxLeanTorque && fSin > 0.0f )
				fTorque = -s_fMaxLeanTorque;
		}
		else
		{
			fTorque = s_fMaxLeanTorque;
		}

		TVector4 vTorque;
		vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ), fTorque );

		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;

		m_fLeanAngle = fLeanAngle;
	}
}

// $Barnyard: FUNCTION 006551a0
void APhysicsBike::UpdatePowerSlide()
{
	if ( !m_bUnkCD2 )
		return;

	if ( m_bPowerSlide == TTRUE )
	{
		if ( m_oBody.m_vVelocity.Magnitude() < 0.5f )
		{
			m_bPowerSlide = TFALSE;
		}
		else
		{
			TFLOAT fSlipAngle    = TMath::Abs( m_aWheels[ 1 ].m_fSlipAngle );
			TFLOAT fMinSlipAngle = ( m_oBody.m_vVelocity.Magnitude() >= 1.388889f ) ? 0.034906585f : 0.017453292f;

			if ( fSlipAngle < fMinSlipAngle )
				m_bPowerSlide = TFALSE;
		}
	}
	else if ( m_iNumWheelsOnGround == 2 && m_oGearBox.iGear == 2 && m_fBrake >= 0.99000001f && TMath::Abs( m_fSteering ) >= 0.99000001f && m_oBody.m_vVelocity.Magnitude() > 5.5555558f )
	{
		m_bPowerSlide = TTRUE;
	}
}

// $Barnyard: FUNCTION 00655290
void APhysicsBike::ApplyPowerSlideTorque()
{
	if ( m_bPowerSlide &&
	     m_iNumWheelsOnGround > 0 &&
	     TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), m_oBody.m_vAngularVelocity ) * m_fSteering < 0.0f )
	{
		TVector4 vInertia = m_oBody.GetInertia();
		TFLOAT   fScale   = vInertia.y * m_fDeltaTime * 80.0f;

		TVector4 vTorque1;
		vTorque1.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), fScale );
		TVector4 vTorque;
		vTorque.Multiply( vTorque1, m_fSteering );

		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;
	}
}

// $Barnyard: FUNCTION 006554d0
TFLOAT APhysicsBike::GetDriveForce()
{
	// Less force at higher speeds
	TFLOAT fSpeedFactor = TMath::Abs( TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ), m_oBody.m_vVelocity ) ) / m_oParams.fMaxBoostSpeed;
	TFLOAT fZero        = 0.0f;
	TFLOAT fFactor      = 1.0f - fSpeedFactor + 1.0f;
	fFactor             = TMath::Max( fZero, fFactor );

	if ( m_bWheelie == TTRUE )
		return fFactor * m_oParams.fDriveForce * 0.25f;

	if ( m_fThrottle <= 1.0f )
		return fFactor * m_oParams.fDriveForce;

	return fFactor * m_oParams.fBoostDriveForce;
}

// $Barnyard: FUNCTION 00655570
void APhysicsBike::UpdateDriveForce()
{
	TFLOAT fForce = 0.0f;
	TFLOAT fInput = 0.0f;
	TINT   iGear  = m_oGearBox.iGear;

	if ( iGear != 1 && m_iNumWheelsOnGround > 0 && !m_bStoppie )
	{
		TFLOAT fMaxSpeed = m_oParams.fMaxSpeed;

		if ( m_bWheelie == TTRUE )
			fMaxSpeed = m_oParams.fMaxWheelieSpeed;
		else if ( m_fThrottle > 1.0f && !m_bWheelie )
			fMaxSpeed = m_oParams.fMaxBoostSpeed;

		if ( iGear == 2 )
		{
			if ( fMaxSpeed > m_vLocalVelocity.z )
			{
				TFLOAT fOne = 1.0f;
				fInput      = TMath::Min( m_fThrottle, fOne );
			}
		}
		else if ( -m_oParams.fMaxReverseSpeed < m_vLocalVelocity.z )
		{
			fInput = -m_fBrake;
		}

		fForce = GetDriveForce() * fInput;
	}

	m_aWheels[ 1 ].m_fEngineForce = fForce;
}

// $Barnyard: FUNCTION 006557f0
void APhysicsBike::CheckThrowRider()
{
	if ( m_bThrowRider )
		return;

	TVector4 vUp          = TVector4::VEC_POSY;
	TINT     iNumContacts = m_iNumContacts;
	TFLOAT   fCos45       = TFLOAT( cos( 0.7853981573134661 ) );

	if ( m_bForceThrowRider && ( iNumContacts > 0 || m_iNumWheelsOnGround > 0 ) )
	{
		m_vThrowVelocity  = m_vPrevVelocity;
		m_bThrowRider     = TTRUE;
		m_eThrowDirection = THROWDIRECTION_FORWARD;

		m_vThrowDirection   = m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD );
		m_vThrowDirection.y = 0.0f;
		m_vThrowDirection.w = 1.0f;
		m_vThrowDirection.Normalise();
		return;
	}

	if ( iNumContacts > 0 )
	{
		// Sum of the impulses of the contacts
		TVector4 vImpulse = TVector4::VEC_ZERO;

		for ( TINT i = 0; i < iNumContacts; i++ )
		{
			APhysicsVehicleContact* pContact = GetContact( i );

			TVector4 vContactImpulse;
			vContactImpulse.Multiply( pContact->vNormal, pContact->fImpulse );

			vImpulse.x = vContactImpulse.x + vImpulse.x;
			vImpulse.y = vContactImpulse.y + vImpulse.y;
			vImpulse.z = vContactImpulse.z + vImpulse.z;
		}

		// Is the impulse coming from the ground the wheels are on?
		TBOOL bFromGround     = TFALSE;
		TINT  iWheelsOnGround = m_iNumWheelsOnGround;

		if ( iWheelsOnGround > 0 )
		{
			TVector4 vGroundNormal = TVector4::VEC_ZERO;

			if ( m_aWheels[ 0 ].m_bOnGround )
			{
				vGroundNormal.x = vGroundNormal.x + m_aWheels[ 0 ].m_vGroundNormal.x;
				vGroundNormal.y = vGroundNormal.y + m_aWheels[ 0 ].m_vGroundNormal.y;
				vGroundNormal.z = vGroundNormal.z + m_aWheels[ 0 ].m_vGroundNormal.z;
			}

			if ( m_aWheels[ 1 ].m_bOnGround )
			{
				vGroundNormal.x = vGroundNormal.x + m_aWheels[ 1 ].m_vGroundNormal.x;
				vGroundNormal.y = vGroundNormal.y + m_aWheels[ 1 ].m_vGroundNormal.y;
				vGroundNormal.z = vGroundNormal.z + m_aWheels[ 1 ].m_vGroundNormal.z;
			}

			vGroundNormal.Multiply( 1.0f / iWheelsOnGround );

			TVector4 vImpulseDir = vImpulse;
			vImpulseDir.Normalise();

			bFromGround = TVector4::DotProduct3( vGroundNormal, vImpulseDir ) > fCos45;
		}

		// Signed angle between the up axis of the bike and the world
		const TVector4& rcBikeUp = m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP );

		TFLOAT fAngle = TMath::ACos( TVector4::DotProduct3( rcBikeUp, vUp ) );

		TVector4 vCross;
		vCross.CrossProduct( rcBikeUp, vUp );

		if ( TVector4::DotProduct3( vCross, m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ) ) < 0.0f )
			fAngle = fAngle * -1.0f;

		TINT iQuadrant;

		if ( fAngle >= 0.0f )
			iQuadrant = ( fAngle >= 1.5707964f ) ? 2 : 0;
		else
			iQuadrant = ( fAngle <= -1.5707964f ) ? 3 : 1;

		if ( s_fThrowRiderImpulse * s_fThrowRiderImpulse < vImpulse.MagnitudeSq() && !bFromGround )
		{
			// Hard impact
			m_bThrowRider     = TTRUE;
			m_eThrowDirection = ( iQuadrant == 0 || iQuadrant == 1 ) ? THROWDIRECTION_FORWARD : THROWDIRECTION_BACKWARD;
			m_vThrowVelocity  = m_vPrevVelocity;

			if ( m_iNumWheelsOnGround > 0 )
			{
				TFLOAT fSpeed = m_vThrowVelocity.Magnitude();

				TVector4 vUpVelocity;
				vUpVelocity.Multiply( TVector4::VEC_NEGY, fSpeed );
				TVector4 vHalfUpVelocity;
				vHalfUpVelocity.Multiply( vUpVelocity, 0.5f );

				m_vThrowVelocity.x = vHalfUpVelocity.x + m_vThrowVelocity.x;
				m_vThrowVelocity.y = vHalfUpVelocity.y + m_vThrowVelocity.y;
				m_vThrowVelocity.z = vHalfUpVelocity.z + m_vThrowVelocity.z;
			}

			m_vThrowDirection   = m_vThrowVelocity;
			m_vThrowDirection.y = 0.0f;
			m_vThrowDirection.w = 1.0f;
			m_vThrowDirection.Normalise();
		}
		else
		{
			TBOOL bForward;

			if ( fAngle > 1.0471976f )
			{
				// Fell over to one side
				m_vThrowVelocity  = m_oBody.m_vVelocity;
				m_bThrowRider     = TTRUE;
				m_eThrowDirection = THROWDIRECTION_FORWARD;

				TFLOAT fZero       = 0.0f;
				m_vThrowVelocity.y = TMath::Max( fZero, m_vThrowVelocity.y );

				m_vThrowDirection   = m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD );
				m_vThrowDirection.y = 0.0f;
				m_vThrowDirection.w = 1.0f;
				m_vThrowDirection.Normalise();

				bForward = TTRUE;
			}
			else if ( fAngle < -1.3089969f )
			{
				// Fell over to the other side
				m_vThrowVelocity  = m_oBody.m_vVelocity;
				m_bThrowRider     = TTRUE;
				m_eThrowDirection = THROWDIRECTION_BACKWARD;

				TFLOAT fZero       = 0.0f;
				m_vThrowVelocity.y = TMath::Max( fZero, m_vThrowVelocity.y );

				m_vThrowDirection = m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD );

				if ( -1.5707964f > fAngle )
					m_vThrowDirection.Multiply( -1.0f );

				m_vThrowDirection.y = 0.0f;
				m_vThrowDirection.w = 1.0f;
				m_vThrowDirection.Normalise();

				bForward = TFALSE;
			}
			else
			{
				goto CHECK_ABOVE;
			}

			if ( !IsDriverless() )
			{
				// Push the bike over
				TFLOAT fTorque     = m_oBody.m_fMass * 30.0f;
				TFLOAT fRollTorque = m_oBody.m_fMass * 30.0f;

				if ( !bForward )
					fRollTorque = fRollTorque * -1.0f;

				TVector4 vTorque;
				vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ), fTorque );

				m_oBody.m_vTorque.x += vTorque.x;
				m_oBody.m_vTorque.y += vTorque.y;
				m_oBody.m_vTorque.z += vTorque.z;

				vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_RIGHT ), -fRollTorque );

				m_oBody.m_vTorque.x += vTorque.x;
				m_oBody.m_vTorque.y += vTorque.y;
				m_oBody.m_vTorque.z += vTorque.z;
			}
		}
	}

CHECK_ABOVE:
	// Is there something right above the bike while it is upside down?
	TVector4 vOffset;
	vOffset.Multiply( TVector4::VEC_NEGY, 1.5f );

	ACollisionObjectSimple oSphere;

	TVector4 vPosition;
	TMatrix44::TransformVector( vPosition, m_oBody.m_oTransform, vOffset );
	oSphere.UpdateHash( 0.30000001f, vPosition.AsVector3() );

	TVector4 vNormal   = TVector4::VEC_NEGY;
	TFLOAT   fDistance = 0.3f;

	if ( ACollisionManager::CollideSphereWithWorld( vPosition, &oSphere, fDistance, vNormal, ~m_oBody.m_oMaterial.m_uiResponseTypes, TCollisionCommon::TOSHICGROUPFLAG( m_oBody.m_oMaterial.m_uiIgnoreGroups ) ) &&
	     TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), vUp ) < 0.0f )
	{
		m_vThrowVelocity  = m_vPrevVelocity;
		m_bThrowRider     = TTRUE;
		m_eThrowDirection = THROWDIRECTION_BACKWARD;

		m_vThrowDirection.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ), -1.0f );
		m_vThrowDirection.y = 0.0f;
		m_vThrowDirection.w = 1.0f;
		m_vThrowDirection.Normalise();
	}
}
