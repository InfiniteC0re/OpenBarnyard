#include "pch.h"
#include "APhysicsCar.h"
#include "ACollisionObject.h"
#include "AVehicleManager.h"

#include <cmath>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Tweakables (retail globals)
static TBOOL  s_bSimulate          = TTRUE;      // 0x0077999d
static TBOOL  s_bClampAntiRoll     = TTRUE;      // 0x0077999c
static TFLOAT s_fUprightAngle      = 1.0471976f; // 0x00779990, the car can only stop when it's tilted less than this
static TFLOAT s_fBrakeStopSpeed    = 0.5f;       // 0x00779994, below it the brake stops the car at once
static TFLOAT s_fAntiRollClamp     = 0.2f;       // 0x00779998, the anti-roll force is at most this part of the average spring force
static TFLOAT s_fFlipTime          = 2.5f;       // 0x007799a0, time on the roof before the car is put back on its wheels
static TBOOL  s_bSweptBoxCollision = TTRUE;      // 0x007b5f78

// $Barnyard: FUNCTION 00657690
APhysicsCar::APhysicsCar()
    : m_fFlipTimer( 0.0f )
    , m_bHandBrakeSlide( TFALSE )
{
	m_oEventHandler.m_pBike = TNULL;
}

// $Barnyard: FUNCTION 006565e0
// $Barnyard: FUNCTION 00657260
APhysicsCar::~APhysicsCar()
{
	m_oBody.m_pEventHandler = TNULL;
	AVehicleManager::RemoveRigidBody( &m_oBody );
}

// $Barnyard: FUNCTION 00656660
void APhysicsCar::Setup( const APhysicsCarParams& a_rcCarParams, const APhysicsVehicleParams& a_rcVehicleParams, const APhysicsEngineParams& a_rcEngineParams, const APhysicsWheelParams& a_rcWheel0, const APhysicsWheelParams& a_rcWheel1, const APhysicsWheelParams& a_rcWheel2, const APhysicsWheelParams& a_rcWheel3 )
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

	m_oParams.fUnk00         = a_rcCarParams.fUnk00;
	m_oParams.fBrakeForce    = a_rcCarParams.fBrakeForce;
	m_oParams.fRearBrakeBias = a_rcCarParams.fRearBrakeBias;
	TMath::Clip( m_oParams.fRearBrakeBias, 0.0f, 1.0f );
	m_oParams.fMaxSpeed   = a_rcCarParams.fMaxSpeed;
	m_oParams.fUnk10      = a_rcCarParams.fUnk10;
	m_oParams.iNumSpheres = a_rcCarParams.iNumSpheres;

	for ( TINT i = 0; i < m_oParams.iNumSpheres; i++ )
		m_oParams.avSpheres[ i ] = a_rcCarParams.avSpheres[ i ];

	m_oParams.fFrontAntiRoll         = a_rcCarParams.fFrontAntiRoll;
	m_oParams.fRearAntiRoll          = a_rcCarParams.fRearAntiRoll;
	m_oParams.fFriction              = a_rcCarParams.fFriction;
	m_oParams.fRestitution           = a_rcCarParams.fRestitution;
	m_oParams.fUpsideDownFriction    = a_rcCarParams.fUpsideDownFriction;
	m_oParams.fUpsideDownRestitution = a_rcCarParams.fUpsideDownRestitution;
	m_oParams.fDownForce             = a_rcCarParams.fDownForce;
	m_oParams.vBoxExtents            = a_rcCarParams.vBoxExtents;
	m_oParams.vBoxCentre             = a_rcCarParams.vBoxCentre;

	m_oBody.SetBoxInertia( m_fMass / ( m_vSize.z * m_vSize.y * m_vSize.x ), m_vSize );
	m_oBody.m_fMass     = m_fMass;
	m_oBody.m_vForce    = TVector4::VEC_ZERO;
	m_oBody.m_vTorque   = TVector4::VEC_ZERO;
	m_oBody.m_vVelocity = TVector4::VEC_ZERO;
	m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
	m_oBody.SetPosition( TVector4::VEC_ZERO, TTRUE );
	m_oBody.SetOrientation( TQuaternion::IDENTITY, TTRUE );
	m_oBody.m_fAngularDamping = 0.2f;
	m_oBody.m_fContactScale   = 0.2f;
	m_oBody.m_fLinearDamping  = 0.05f;

	m_aWheels[ 0 ].Create( this, a_rcWheel0 );
	m_aWheels[ 1 ].Create( this, a_rcWheel1 );
	m_aWheels[ 2 ].Create( this, a_rcWheel2 );
	m_aWheels[ 3 ].Create( this, a_rcWheel3 );

	// The engines drive the rear wheels
	m_oEngine.Create( this, &m_oGearBox, a_rcEngineParams );
	m_oEngine.m_pWheel1 = &m_aWheels[ 2 ];
	m_oEngine.m_pWheel2 = &m_aWheels[ 3 ];

	m_oEngine2.Create( this, &m_oGearBox, a_rcEngineParams );
	m_oEngine2.m_pWheel1 = &m_aWheels[ 2 ];
	m_oEngine2.m_pWheel2 = &m_aWheels[ 3 ];

	m_oBody.m_oMaterial = ARigidBodyMaterial( m_oParams.fFriction, m_oParams.fRestitution, ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, 0, 5 );

	// The shape of the body is a box
	ARigidBodyShape oShape;
	oShape.SetType( ARigidBodyShape::TYPE_BOX );

	TMatrix44 matBox        = TMatrix44::IDENTITY;
	matBox.GetTranslation() = m_oParams.vBoxCentre;
	oShape.SetBox( TOBBox( matBox, m_oParams.vBoxExtents ) );

	m_oBody.m_oShape = oShape;

	AVehicleManager::RemoveRigidBody( &m_oBody );
	AVehicleManager::AddRigidBody( &m_oBody );

	m_oBody.m_pEventHandler = &m_oEventHandler;
	m_vCentreOfMass2.Set( 0.0f, m_oParams.vBoxCentre.y, 0.0f, 1.0f );
	m_oEventHandler.m_pBike = this;
	m_fFlipTimer            = 0.0f;
}

// $Barnyard: FUNCTION 00656650
TBOOL APhysicsCar::IsSliding()
{
	return m_bHandBrakeSlide;
}

// NOTE: 00657250 is shared with APhysicsBike (same code)
void APhysicsCar::VTable0x1C()
{
	if ( m_bUnkCD8 )
	{
		// DOES_NOTHING (005acd80 is called with this)
	}
}

// $Barnyard: FUNCTION 006576c0
void APhysicsCar::PreUpdate( TFLOAT a_fDeltaTime )
{
	APhysicsVehicle::PreUpdate( a_fDeltaTime );

	if ( s_bSimulate )
	{
		m_oEngine2.UpdateGears( m_fDeltaTime );
		UpdateSteeringAngle();
		m_oEngine2.UpdateTorque( m_fDeltaTime );

		if ( !m_bFrozen )
		{
			UpdateBrakeForce();
			ApplyWheelForces();
			ApplyGravity();
			UpdateSuspension();
			ApplyRollingResistance();
			ApplySurfaceDrag();
			ApplyDownForce();
			StopAirRoll();
			StopAirPitch();
			ApplyAirPitchUpright();
			ApplyGroundPitchDamping();
			ApplyHandBrakeTurn();
		}

		UpdateHandBrakeSlide();
		UpdateMaterial();
		m_oEngine2.UpdateRPM( m_fDeltaTime );
		UpdateStopped();
	}
	else
	{
		m_oBody.m_vForce  = TVector4::VEC_ZERO;
		m_oBody.m_vTorque = TVector4::VEC_ZERO;
	}
}

// $Barnyard: FUNCTION 00656ad0
void APhysicsCar::PostUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bFrozen == TTRUE )
	{
		// Unfreeze the car if something has moved it
		TVector4 vMovement = m_vPrevPosition - m_oBody.m_oTransform.GetTranslation();

		if ( vMovement.MagnitudeSq() > 0.001f )
			m_bFrozen = TFALSE;
	}

	FlipUpright();
	UpdateWheelContacts();
	APhysicsVehicle::PostUpdate( a_fDeltaTime );
}

// $Barnyard: FUNCTION 006577e0
void APhysicsCar::UpdateSuspension()
{
	// NOTE: the force returned by the wheels is ignored, the spring forces are applied after the anti-roll bars changed them
	for ( TINT i = 0; i < m_iNumWheels; i++ )
		m_aWheels[ i ].GetSuspensionForce( &m_oBottomOut );

	if ( m_oBottomOut.iCount > 0 )
		ApplyBottomOut( &m_oBottomOut );

	ApplyAntiRoll();

	for ( TINT i = 0; i < m_iNumWheels; i++ )
	{
		TVector4 vForce;
		vForce.Multiply( m_aWheels[ i ].m_vGroundNormal, m_aWheels[ i ].m_fSpringForce );

		TVector4 vForceCopy = vForce;
		m_oBody.ApplyForce( vForceCopy, m_aWheels[ i ].m_vPosition );
	}
}

// $Barnyard: FUNCTION 006578b0
void APhysicsCar::FlipUpright()
{
	if ( !m_bFlipRecovery )
		return;

	if ( !( m_fFlipTimer > 0.0f ) && m_iNumContacts <= 0 )
		return;

	if ( TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) >= 0.17f )
		m_fFlipTimer = 0.0f;
	else
		m_fFlipTimer = m_fDeltaTime + m_fFlipTimer;

	if ( m_fFlipTimer > s_fFlipTime )
	{
		// The car lies on its roof for too long, put it back on its wheels on the ground below it
		ARigidBodyCollision oCollision;

		TVector4 vOffset;
		vOffset.Multiply( TVector4::VEC_POSY, 10.0f );

		TVector4 vEnd = m_oBody.m_vCentreOfMass + vOffset;

		if ( AVehicleManager::CollideRay( m_oBody.m_vCentreOfMass, vEnd, &oCollision, ~m_oBody.m_oMaterial.m_uiResponseTypes, TCollisionCommon::TOSHICGROUPFLAG( m_oBody.m_oMaterial.m_uiIgnoreGroups ) ) )
		{
			if ( ( m_oBody.m_vCentreOfMass - oCollision.m_vPosition ).Magnitude() < 5.0f )
			{
				TMatrix44 matTransform = TMatrix44::IDENTITY;

				TVector4 vUp;
				vUp.Multiply( oCollision.m_vNormal, -1.0f );

				TVector4 vRight;
				vRight.CrossProduct( vUp, m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_FORWARD ) );
				vRight.Normalise();

				TVector4 vForward;
				vForward.CrossProduct( vRight, vUp );

				matTransform.AsBasisVector4( BASISVECTOR_RIGHT )       = vRight;
				matTransform.AsBasisVector4( BASISVECTOR_UP )          = vUp;
				matTransform.AsBasisVector4( BASISVECTOR_FORWARD )     = vForward;
				matTransform.AsBasisVector4( BASISVECTOR_TRANSLATION ) = oCollision.m_vPosition;

				TQuaternion qOrientation;
				TMatrix44::MatToQuat( qOrientation, matTransform );

				m_oBody.m_vVelocity = TVector4::VEC_ZERO;
				m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
				m_oBody.SetPosition( matTransform.GetTranslation(), TTRUE );
				m_oBody.SetOrientation( qOrientation, TTRUE );

				m_vPrevPosition    = m_oBody.m_oTransform.GetTranslation();
				m_qPrevOrientation = m_oBody.m_qOrientation;
				m_vPrevVelocity    = TVector4::VEC_ZERO;

				UpdateWheelContacts();
			}

			m_fFlipTimer = 0.0f;
		}
	}
}

// $Barnyard: FUNCTION 00657530
void APhysicsCar::Collide( TFLOAT a_fDeltaTime )
{
	TVector4 vCentreOfMass = m_oBody.m_vCentreOfMassLocal;

	// Collide around the centre of the box
	m_oBody.SetCentreOfMass( TVector4( 0.0f, m_oParams.vBoxCentre.y, 0.0f, 1.0f ) );

	TINT iNumIterations = AVehicleManager::GetNumIterations();
	AVehicleManager::SetNumIterations( 1 );

	TMatrix44 matBox        = TMatrix44::IDENTITY;
	matBox.GetTranslation() = m_oParams.vBoxCentre;
	TOBBox oBox( matBox, m_oParams.vBoxExtents );

	AVehicleManager::ContactSummary oSummary;

	if ( AVehicleManager::CollideBoxAndResolve( &m_oBody, oBox, a_fDeltaTime, &oSummary, s_bSweptBoxCollision ) == TTRUE )
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
	m_oBody.SetCentreOfMass( vCentreOfMass );
}

// $Barnyard: FUNCTION 00656b80
void APhysicsCar::UpdateStopped()
{
	if ( m_bFrozen )
	{
		if ( m_oGearBox.iGear != 1 )
		{
			m_bFrozen = TFALSE;
			return;
		}

		Freeze();
		m_fStopTimer = 0.0f;
		return;
	}

	if ( m_vLocalVelocity.Magnitude() >= 0.1388889f || m_iNumWheelsOnGround != 4 ||
	     TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) <= TFLOAT( cos( s_fUprightAngle ) ) )
	{
		m_fStopTimer = 0.0f;
		return;
	}

	TINT iGear = m_oGearBox.iGear;

	if ( ( iGear == 2 && m_fBrake > 0.0f ) || ( iGear == 0 && m_fThrottle > 0.0f ) )
	{
		// Braking at a stop shifts into neutral
		Freeze();
		m_oGearBox.iPrevGear = m_oGearBox.iGear;
		m_oGearBox.iGear     = 1;
		m_fStopTimer         = 0.0f;
		return;
	}

	if ( ( iGear != 2 || m_fThrottle > 0.0f ) && ( iGear != 0 || m_fBrake > 0.0f ) )
	{
		if ( iGear != 1 )
			return;

		Freeze();
		m_fStopTimer = 0.0f;
		return;
	}

	// No input: shift into neutral after a second
	m_fStopTimer = m_fDeltaTime + m_fStopTimer;

	if ( m_fStopTimer > 1.0f )
	{
		Freeze();
		m_oGearBox.iPrevGear = m_oGearBox.iGear;
		m_oGearBox.iGear     = 1;
		m_fStopTimer         = 0.0f;
	}
}

// $Barnyard: FUNCTION 00656d10
void APhysicsCar::UpdateBrakeForce()
{
	TINT   iGear  = m_oGearBox.iGear;
	TFLOAT fBrake = m_fBrake;
	TFLOAT fSpeed = m_vLocalVelocity.z;
	TFLOAT fForce = 0.0f;

	// In reverse the throttle brakes
	if ( iGear == 0 )
		fBrake = m_fThrottle;

	if ( m_iNumWheelsOnGround > 0 )
	{
		if ( fBrake <= 0.0f || TMath::Abs( fSpeed ) >= s_fBrakeStopSpeed )
		{
			if ( iGear < 1 )
				fForce = m_oParams.fBrakeForce * m_fThrottle;
			else
				fForce = -m_oParams.fBrakeForce * m_fBrake;
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

	TFLOAT fFrontScale = 1.0f;
	TFLOAT fRearScale  = 1.0f;

	if ( m_bUnkCD2 == TTRUE && IsSliding() == TTRUE )
	{
		// The hand brake slide locks the rear wheels
		fFrontScale = 0.35f;
		fRearScale  = 0.0f;
	}

	TFLOAT fRearBias             = m_oParams.fRearBrakeBias;
	TFLOAT fFrontForce           = ( 1.0f - fRearBias ) * fFrontScale * fForce * 0.5f;
	m_aWheels[ 0 ].m_fDriveForce = fFrontForce;
	m_aWheels[ 1 ].m_fDriveForce = fFrontForce;

	TFLOAT fRearForce            = fRearBias * fRearScale * fForce * 0.5f;
	m_aWheels[ 2 ].m_fDriveForce = fRearForce;
	m_aWheels[ 3 ].m_fDriveForce = fRearForce;
}

// $Barnyard: FUNCTION 00656ea0
void APhysicsCar::ApplyHandBrakeTurn()
{
	if ( m_bHandBrakeSlide && m_iNumWheelsOnGround > 0 &&
	     TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), m_oBody.m_vAngularVelocity ) * m_fSteering < 0.0f )
	{
		// Help the car turn into the slide
		TFLOAT fStrength = m_oBody.m_vInertia.y * m_fDeltaTime * 240.0f;

		TVector4 vTorque;
		vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), fStrength );
		vTorque.Multiply( m_fSteering );

		m_oBody.m_vTorque.x = vTorque.x + m_oBody.m_vTorque.x;
		m_oBody.m_vTorque.y = vTorque.y + m_oBody.m_vTorque.y;
		m_oBody.m_vTorque.z = vTorque.z + m_oBody.m_vTorque.z;
	}
}

// $Barnyard: FUNCTION 00656f80
void APhysicsCar::ApplyDownForce()
{
	TVector4 vVelocity;
	vVelocity.Multiply( m_vLocalVelocity, -1.0f );

	TFLOAT fSpeedSq = vVelocity.z * vVelocity.z;

	if ( fSpeedSq != 0.0f && 0.0f != vVelocity.x * vVelocity.x )
	{
		TVector4 vBack;
		vBack.Multiply( m_oBody.GetVelocityDirection(), -1.0f );

		if ( !vBack.IsEqual( TVector4::VEC_ZERO ) )
		{
			TVector4 vForward = vBack;
			vForward.Normalise();

			TVector4 vRight;
			vRight.CrossProduct( vForward, m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ) );
			vRight.Multiply( -1.0f );

			if ( !vRight.IsEqual( TVector4::VEC_ZERO ) )
			{
				vRight.Normalise();

				TVector4 vDown;
				vDown.CrossProduct( vForward, vRight );

				if ( !vDown.IsEqual( TVector4::VEC_ZERO ) )
				{
					vDown.Normalise();

					// The down force grows with the square of the speed
					TFLOAT fScale = fSpeedSq / ( m_oParams.fMaxSpeed * m_oParams.fMaxSpeed );
					TMath::Clip( fScale, 0.0f, 1.0f );

					TFLOAT fForce = m_oParams.fDownForce * m_oBody.m_fMass * fScale * 19.6f;

					TVector4 vForce;
					vForce.Multiply( vDown, fForce );

					TFLOAT fWheelForce              = fForce * 0.25f;
					m_aWheels[ 0 ].m_fDownForceLoad = fWheelForce;
					m_aWheels[ 1 ].m_fDownForceLoad = fWheelForce;
					m_aWheels[ 2 ].m_fDownForceLoad = fWheelForce;
					m_aWheels[ 3 ].m_fDownForceLoad = fWheelForce;

					m_oBody.m_vForce.x = vForce.x + m_oBody.m_vForce.x;
					m_oBody.m_vForce.y = vForce.y + m_oBody.m_vForce.y;
					m_oBody.m_vForce.z = vForce.z + m_oBody.m_vForce.z;
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 006571d0
void APhysicsCar::UpdateMaterial()
{
	if ( TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), TVector4::VEC_POSY ) > 0.0f )
	{
		m_oBody.m_oMaterial.m_fFriction    = m_oParams.fFriction;
		m_oBody.m_oMaterial.m_fRestitution = m_oParams.fRestitution;
	}
	else
	{
		m_oBody.m_oMaterial.m_fFriction    = m_oParams.fUpsideDownFriction;
		m_oBody.m_oMaterial.m_fRestitution = m_oParams.fUpsideDownRestitution;
	}
}

// $Barnyard: FUNCTION 00657280
void APhysicsCar::ApplyAntiRoll()
{
	for ( TINT i = 0; i < 2; i++ )
	{
		TFLOAT         fStiffness;
		APhysicsWheel* pWheel1;
		APhysicsWheel* pWheel2;

		if ( i != 0 )
		{
			fStiffness = m_oParams.fRearAntiRoll;
			pWheel1    = &m_aWheels[ 2 ];
			pWheel2    = &m_aWheels[ 3 ];
		}
		else
		{
			fStiffness = m_oParams.fFrontAntiRoll;
			pWheel1    = &m_aWheels[ 0 ];
			pWheel2    = &m_aWheels[ 1 ];
		}

		TFLOAT fForce = TMath::Abs( pWheel1->m_fPrevSpringLength - pWheel2->m_fPrevSpringLength ) * fStiffness;

		if ( s_bClampAntiRoll )
			fForce = TMath::Min( ( pWheel2->m_fSpringForce + pWheel1->m_fSpringForce ) * 0.5f * s_fAntiRollClamp, fForce );

		if ( pWheel1->m_fPrevSpringLength > pWheel2->m_fPrevSpringLength )
			fForce *= -1.0f;

		// Move the spring force from one wheel to the other
		pWheel1->m_fSpringForce = TMath::Max( pWheel1->m_fSpringForce - fForce, 0.0f );
		pWheel2->m_fSpringForce = TMath::Max( fForce + pWheel2->m_fSpringForce, 0.0f );

		TVector4 vForce;
		vForce.Multiply( m_oBody.m_oTransform.AsBasisVector4( BASISVECTOR_UP ), fForce );

		TVector4 vHalfForce;
		vHalfForce.Multiply( vForce, 0.5f );

		TVector4 vNegHalfForce;
		vNegHalfForce.Multiply( vHalfForce, -1.0f );

		m_oBody.ApplyForce( vHalfForce, pWheel1->m_vPosition );
		m_oBody.ApplyForce( vNegHalfForce, pWheel2->m_vPosition );
	}
}

// $Barnyard: FUNCTION 00657450
void APhysicsCar::UpdateHandBrakeSlide()
{
	if ( !m_bUnkCD2 )
		return;

	if ( m_bHandBrakeSlide == TTRUE )
	{
		// The slide ends when the car is slow or the rear wheels don't slip anymore
		if ( m_oBody.m_vVelocity.Magnitude() < 4.0f || TMath::Max( TMath::Abs( m_aWheels[ 2 ].m_fSlipAngle ), 0.0f ) < 0.0087266462f )
			m_bHandBrakeSlide = TFALSE;
	}
	else if ( m_iNumWheelsOnGround == 4 && m_oGearBox.iGear >= 2 && m_fBrake >= 0.99f && TMath::Abs( m_fSteering ) >= 0.9f && m_oBody.m_vVelocity.Magnitude() > 5.5555558f )
	{
		// Full brake and full steering at speed
		m_bHandBrakeSlide = TTRUE;
	}
}
