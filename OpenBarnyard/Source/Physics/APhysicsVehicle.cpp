#include "pch.h"
#include "APhysicsVehicle.h"
#include "AVehicleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// Tweakables (retail globals)
static TFLOAT s_fSteeringDeadZone       = 0.0f;    // 0x007cd038
static TFLOAT s_fSteeringKnee           = 0.8f;    // 0x007799c4
static TFLOAT s_fSteeringKneeValue      = 0.2f;    // 0x007799c0
static TFLOAT s_fHandBrakeGripScale     = 0.5f;    // 0x007799b0
static TBOOL  s_bStopAirPitch           = TTRUE;   // 0x007799c8
static TBOOL  s_bGroundPitchDamping     = TTRUE;   // 0x007799c9
static TFLOAT s_fSlipAngleSmoothing     = 0.2f;    // 0x007799cc
static TFLOAT s_fMinSlipSpeed           = 0.0001f; // 0x007799d0
static TFLOAT s_fMaxSlipNormalCos       = 0.99f;   // 0x007799d4
static TFLOAT s_fMinSlipSpeedSq         = 0.1f;    // 0x007799d8
static TFLOAT s_fGroundNormalSmoothing  = 4.0f;    // 0x007799dc
static TBOOL  s_bClampCarTyreForce      = TTRUE;   // 0x007799e0
static TFLOAT s_fCarAirLateralSmoothing = 2.0f;    // 0x007799e4
static TFLOAT s_fBottomOutMaxTorque     = 0.04f;   // 0x007799e8
static TFLOAT s_fBottomOutMaxImpulse    = 0.25f;   // 0x007799ec
static TBOOL  s_bLimitBottomOut         = TTRUE;   // 0x007799f0
static TBOOL  s_bBottomOut              = TTRUE;   // 0x007799f1
static TBOOL  s_bClampLowSpeedLateral   = TTRUE;   // 0x007799f2
static TBOOL  s_bSoftSlipLimit          = TTRUE;   // 0x007799f3
static TFLOAT s_fAirLateralSmoothing    = 2.0f;    // 0x007799f4
static TFLOAT s_fSpringForceSmoothing   = 2.0f;    // 0x007799f8
static TBOOL  s_bSmoothSpringForce      = TTRUE;   // 0x007799fc
static TFLOAT s_fMaxSpringForceScale    = 1.0f;    // 0x00779a00
static TBOOL  s_bAirPitchUpright        = TFALSE;  // 0x007cd03c
static TBOOL  s_bMoveBottomedOutWheels  = TFALSE;  // 0x007cd03d

// NOTE: TVector4::Magnitude asserts on zero vectors, the original uses fsqrt (006c74f0)
static TFLOAT VectorLength( const TVector4& a_rcVector )
{
	return sqrtf( a_rcVector.x * a_rcVector.x + a_rcVector.y * a_rcVector.y + a_rcVector.z * a_rcVector.z );
}

//-----------------------------------------------------------------------------
// APhysicsWheel
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00660fa0
APhysicsWheel::APhysicsWheel()
{
	m_pVehicle          = TNULL;
	m_vForward          = TVector4::VEC_POSZ;
	m_vSide             = TVector4::VEC_NEGX;
	m_vGroundNormal     = TVector4::VEC_NEGY;
	m_vContactPosition  = TVector4::VEC_ZERO;
	m_vPosition         = TVector4::VEC_ZERO;
	m_vContactVelocity  = TVector4::VEC_ZERO;
	m_fSlipAngle        = 0.0f;
	m_fAngularVelocity  = 0.0f;
	m_fRotation         = 0.0f;
	m_bOnGround         = TTRUE;
	m_fGroundDistance   = 0.0f;
	m_fPrevSpringLength = 0.0f;
	m_fSpringForce      = 0.0f;
	m_bBottomedOut      = TFALSE;
	m_fLateralForce     = 0.0f;
	m_fDriveForce       = 0.0f;
	m_fEngineForce      = 0.0f;
	m_fDownForceLoad    = 0.0f;
	m_iSurfaceType      = AVehicleManager::SURFACE_NONE;
	m_bEnabled          = TTRUE;
}

// $Barnyard: FUNCTION 006610e0
void APhysicsWheel::Create( APhysicsVehicle* a_pVehicle, const APhysicsWheelParams& a_rcParams )
{
	m_pVehicle = a_pVehicle;
	SetParams( a_rcParams );
}

// $Barnyard: FUNCTION 0065f5b0
void APhysicsWheel::SetParams( const APhysicsWheelParams& a_rcParams )
{
	m_oParams = a_rcParams;
}

//-----------------------------------------------------------------------------
// APhysicsEngine
//-----------------------------------------------------------------------------

APhysicsEngine::APhysicsEngine()
{
	m_fRPM     = 0.0f;
	m_fUnk08   = 0.0f;
	m_fUnk10   = 0.0f;
	m_pVehicle = TNULL;
	m_pGearBox = TNULL;
	m_iUnk0C   = 2;
	m_pWheel1  = TNULL;
	m_pWheel2  = TNULL;
}

// $Barnyard: FUNCTION 006616f0
// APhysicsEngine::~APhysicsEngine (scalar deleting destructor)

// $Barnyard: FUNCTION 00660260
void APhysicsEngine::Create( APhysicsVehicle* a_pVehicle, APhysicsGearBox* a_pGearBox, const APhysicsEngineParams& a_rcParams )
{
	m_fPeakTorque    = a_rcParams.fPeakTorque;
	m_fPeakTorqueRPM = a_rcParams.fPeakTorqueRPM;
	m_fIdleTorque    = a_rcParams.fIdleTorque;
	m_fMaxRPMTorque  = a_rcParams.fMaxRPMTorque;
	m_fMaxRPM        = a_rcParams.fMaxRPM;
	m_fIdleRPM       = a_rcParams.fIdleRPM;
	m_fUnk3C         = a_rcParams.fUnk18;
	m_fInvUnk3C      = 1.0f / a_rcParams.fUnk18;
	m_fEngineBraking = a_rcParams.fEngineBraking;
	m_fNumGears      = a_rcParams.fNumGears;

	for ( TINT i = 0; i < m_fNumGears; i++ )
		m_afGearRatios[ i ] = a_rcParams.afGearRatios[ i ];

	m_fFinalDrive = a_rcParams.fFinalDrive;
	m_fShiftDelay = a_rcParams.fShiftDelay;
	m_fClutchTime = a_rcParams.fClutchTime;
	m_fShiftTime  = a_rcParams.fShiftTime;
	m_pVehicle    = a_pVehicle;
	m_pGearBox    = a_pGearBox;
	m_fRPM        = m_fIdleRPM / ( 60.0f / 6.2831855f );
}

//-----------------------------------------------------------------------------
// APhysicsEngine2
//-----------------------------------------------------------------------------

APhysicsEngine2::APhysicsEngine2()
{
	m_fRPM            = 0.0f;
	m_fPrevRPM        = 0.0f;
	m_iUnk0C          = 2;
	m_fThrottle       = 0.0f;
	m_fRevLimiterTime = 0.0f;
	m_fTorque         = 0.0f;
	m_fClutch         = 1.0f;
	m_fGearRatio      = 1.0f;
	m_fEfficiency     = 0.97f;
	m_fInertia        = 1.0f;
	m_fShiftTimer     = 0.0f;
	m_fReverseTimer   = 0.0f;
	m_bShiftingUp     = TFALSE;
	m_bShiftingDown   = TFALSE;
	m_pVehicle        = TNULL;
	m_pGearBox        = TNULL;
	m_pWheel1         = TNULL;
	m_pWheel2         = TNULL;
}

// $Barnyard: FUNCTION 00661710
// APhysicsEngine2::~APhysicsEngine2 (scalar deleting destructor)

// $Barnyard: FUNCTION 00661730
void APhysicsEngine2::Create( APhysicsVehicle* a_pVehicle, APhysicsGearBox* a_pGearBox, const APhysicsEngineParams& a_rcParams )
{
	m_fPeakTorque    = a_rcParams.fPeakTorque;
	m_fPeakTorqueRPM = a_rcParams.fPeakTorqueRPM;
	m_fIdleTorque    = a_rcParams.fIdleTorque;
	m_fMaxRPMTorque  = a_rcParams.fMaxRPMTorque;
	m_fMaxRPM        = a_rcParams.fMaxRPM;
	m_fIdleRPM       = a_rcParams.fIdleRPM;
	m_fUnk60         = a_rcParams.fUnk18;
	m_fInvUnk60      = 1.0f / a_rcParams.fUnk18;
	m_fEngineBraking = a_rcParams.fEngineBraking;
	m_fNumGears      = a_rcParams.fNumGears;

	for ( TINT i = 0; i < m_fNumGears; i++ )
		m_afGearRatios[ i ] = a_rcParams.afGearRatios[ i ];

	m_fFinalDrive  = a_rcParams.fFinalDrive;
	m_fShiftDelay  = a_rcParams.fShiftDelay;
	m_fClutchTime  = a_rcParams.fClutchTime;
	m_fShiftTime   = a_rcParams.fShiftTime;
	m_pVehicle     = a_pVehicle;
	m_pGearBox     = a_pGearBox;
	m_fRPM         = m_fIdleRPM / ( 60.0f / 6.2831855f );
	m_fPrevRPM     = m_fIdleRPM / ( 60.0f / 6.2831855f );
	m_fTorqueScale = a_rcParams.fTorqueScale;

	// NOTE: inlined in the original
	OnGearChanged();
}

// $Barnyard: FUNCTION 00660400
void APhysicsEngine2::OnGearChanged()
{
	TFLOAT fFinalDrive = m_fFinalDrive;
	m_fGearRatio       = fFinalDrive * m_afGearRatios[ m_pGearBox->iGear ];

	TFLOAT fWheelRadius = ( m_pWheel1 ) ? m_pWheel1->m_oParams.fRadius : 0.40000001f;

	m_fEfficiency = 0.97f;
	m_fInertia    = ( m_fGearRatio * m_fGearRatio * 0.1f + fFinalDrive * fFinalDrive * 0.15000001f + 1.24f ) / fWheelRadius;
}

//-----------------------------------------------------------------------------
// APhysicsVehicleBottomOut
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0065e800
APhysicsVehicleBottomOut::APhysicsVehicleBottomOut()
{
	iCount    = 0;
	fMaxDepth = 0.0f;
	vImpulse  = TVector4::VEC_ZERO;
	vTorque   = TVector4::VEC_ZERO;
}

// $Barnyard: FUNCTION 0065e860
void APhysicsVehicleBottomOut::Reset()
{
	iCount    = 0;
	fMaxDepth = 0.0f;
	vImpulse  = TVector4::VEC_ZERO;
	vTorque   = TVector4::VEC_ZERO;
}

//-----------------------------------------------------------------------------
// APhysicsVehicleContact
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0065e8c0
APhysicsVehicleContact::APhysicsVehicleContact()
{
	vNormal           = TVector4::VEC_NEGY;
	vPosition         = TVector4::VEC_ZERO;
	vRelativeVelocity = TVector4::VEC_ZERO;
	fImpulse          = 0.0f;
	iSurfaceType      = AVehicleManager::SURFACE_NONE;
	pOtherBody        = TNULL;
	pCollisionObject  = TNULL;
}

// $Barnyard: FUNCTION 0065e950
void APhysicsVehicleContact::Reset()
{
	vNormal           = TVector4::VEC_NEGY;
	vPosition         = TVector4::VEC_ZERO;
	vRelativeVelocity = TVector4::VEC_ZERO;
	fImpulse          = 0.0f;
	iSurfaceType      = AVehicleManager::SURFACE_NONE;
	pOtherBody        = TNULL;
	pCollisionObject  = TNULL;
}

//-----------------------------------------------------------------------------
// APhysicsVehicle
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00661ad0
APhysicsVehicle::APhysicsVehicle()
{
	m_oGearBox.fGearTimer    = 0.0f;
	m_oGearBox.iGear         = 1;
	m_oGearBox.iPrevGear     = 1;
	m_oGearBox.bAllowReverse = TTRUE;
	m_oGearBox.bAllowDrive   = TTRUE;

	m_iNumContacts       = 0;
	m_fSteering          = 0.0f;
	m_fThrottle          = 0.0f;
	m_fBrake             = 0.0f;
	m_fHandBrake         = 0.0f;
	m_fSteeringAngle     = 0.0f;
	m_fDeltaTime         = 0.033333335f;
	m_bUnkCC8            = TTRUE;
	m_fStopTimer         = 0.0f;
	m_bFrozen            = TFALSE;
	m_bFlipRecovery      = TTRUE;
	m_bUnkCD2            = TFALSE;
	m_iNumWheelsOnGround = 2;
	m_bUnkCD8            = TFALSE;
	m_bAirRollStopped    = TFALSE;
	m_bAirPitchStopped   = TFALSE;
	m_vLocalVelocity     = TVector4::VEC_ZERO;
	m_vPrevLocalVelocity = TVector4::VEC_ZERO;
	m_vPrevPosition      = TVector4::VEC_ZERO;
	m_qPrevOrientation   = TQuaternion::IDENTITY;
	m_vPrevVelocity      = TVector4::VEC_ZERO;
	m_vCentreOfMass1     = TVector4::VEC_ZERO;
	m_vCentreOfMass2     = TVector4::VEC_ZERO;
	m_bEnabled           = TTRUE;
	m_iUnkD50            = 0;
	m_bUseTorqueScale    = TFALSE;
	m_fMaxThrottle       = 1.0f;

	m_oBottomOut.Reset();

	for ( TINT i = 0; i < MAX_NUM_CONTACTS; i++ )
		m_aContacts[ i ].Reset();
}

// $Barnyard: FUNCTION 00661df0
// $Barnyard: FUNCTION 00660930
APhysicsVehicle::~APhysicsVehicle()
{
	m_oBody.Disable();
}

// $Barnyard: FUNCTION 0065e9e0
void APhysicsVehicle::SetSteering( TFLOAT a_fSteering )
{
	TMath::Clip( a_fSteering, -1.0f, 1.0f );
	TFLOAT fAbsSteering = TMath::Abs( a_fSteering );

	if ( fAbsSteering >= s_fSteeringDeadZone )
	{
		if ( fAbsSteering >= s_fSteeringKnee )
			m_fSteering = ( 1.0f - s_fSteeringKneeValue ) / ( 1.0f - s_fSteeringKnee ) * ( fAbsSteering - s_fSteeringKnee ) + s_fSteeringKneeValue;
		else
			m_fSteering = s_fSteeringKneeValue / ( s_fSteeringKnee - s_fSteeringDeadZone ) * ( fAbsSteering - s_fSteeringDeadZone );
	}
	else
	{
		m_fSteering = 0.0f;
	}

	if ( a_fSteering < 0.0f )
		m_fSteering = m_fSteering * -1.0f;

	TMath::Clip( m_fSteering, -1.0f, 1.0f );
}

// $Barnyard: FUNCTION 0065eaa0
void APhysicsVehicle::SetThrottle( TFLOAT a_fThrottle )
{
	TMath::Clip( a_fThrottle, 0.0f, m_fMaxThrottle );
	m_fThrottle = a_fThrottle;
}

// $Barnyard: FUNCTION 0065ead0
void APhysicsVehicle::SetBrake( TFLOAT a_fBrake )
{
	TMath::Clip( a_fBrake, 0.0f, 1.0f );
	m_fBrake = a_fBrake;
}

// $Barnyard: FUNCTION 0065eb00
void APhysicsVehicle::SetHandBrake( TFLOAT a_fHandBrake )
{
	TMath::Clip( a_fHandBrake, 0.0f, 1.0f );
	m_fHandBrake = a_fHandBrake;
}

// $Barnyard: FUNCTION 0065eb30
void APhysicsVehicle::PreUpdate( TFLOAT a_fDeltaTime )
{
	m_fDeltaTime = a_fDeltaTime;
	m_oBottomOut.Reset();
	m_iNumContacts = 0;

	TMatrix44 oInvTransform;
	oInvTransform.Transpose( m_oBody.m_oTransform );
	TMatrix44::RotateVector( m_vLocalVelocity, oInvTransform, m_oBody.m_vVelocity );
}

// $Barnyard: FUNCTION 0065eb90
void APhysicsVehicle::PostUpdate( TFLOAT a_fDeltaTime )
{
	m_vPrevPosition      = m_oBody.m_oTransform.GetTranslation();
	m_qPrevOrientation   = m_oBody.m_qOrientation;
	m_vPrevVelocity      = m_oBody.m_vVelocity;
	m_vPrevLocalVelocity = m_vLocalVelocity;
}

// $Barnyard: FUNCTION 0065f300
APhysicsVehicleContact* APhysicsVehicle::GetContact( TUINT a_uiIndex )
{
	if ( a_uiIndex >= MAX_NUM_CONTACTS )
		return TNULL;

	return &m_aContacts[ a_uiIndex ];
}

// $Barnyard: FUNCTION 0065f320
APhysicsVehicleContact* APhysicsVehicle::AddContact()
{
	if ( m_iNumContacts >= MAX_NUM_CONTACTS )
		return TNULL;

	APhysicsVehicleContact* pContact = &m_aContacts[ m_iNumContacts ];
	pContact->Reset();
	m_iNumContacts++;

	return pContact;
}

// $Barnyard: FUNCTION 0065f350
void APhysicsVehicle::UseCentreOfMass2()
{
	m_oBody.SetCentreOfMass( m_vCentreOfMass2 );
}

// $Barnyard: FUNCTION 0065f360
void APhysicsVehicle::UseCentreOfMass1()
{
	m_oBody.SetCentreOfMass( m_vCentreOfMass1 );
}

// $Barnyard: FUNCTION 0065f560
void APhysicsVehicle::SetFrozen( TBOOL a_bFrozen )
{
	m_fStopTimer = 0.0f;
	m_bFrozen    = a_bFrozen;

	if ( a_bFrozen == TTRUE )
	{
		for ( TINT i = 0; i < m_iNumWheels; i++ )
			m_aWheels[ i ].m_fAngularVelocity = 0.0f;
	}
}

// $Barnyard: FUNCTION 0065f260
TVector4* APhysicsVehicle::Freeze()
{
	if ( m_bFrozen == TFALSE )
	{
		m_oBody.m_vVelocity = TVector4::VEC_ZERO;
		m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
		m_oBody.m_vTorque = TVector4::VEC_ZERO;
	}

	m_oBody.m_vForce.x = TVector4::VEC_ZERO.x;
	m_oBody.m_vForce.y = TVector4::VEC_ZERO.y;
	m_oBody.m_vForce.z = TVector4::VEC_ZERO.z;
	m_bFrozen          = TTRUE;
	m_oBody.m_vForce.w = TVector4::VEC_ZERO.w;

	return &m_oBody.m_vForce;
}

// $Barnyard: FUNCTION 00661e10
void APhysicsVehicle::Reset( const TVector4& a_rcPosition, const TQuaternion& a_rcOrientation )
{
	m_oBody.SetPosition( a_rcPosition, TTRUE );
	m_oBody.SetOrientation( a_rcOrientation, TTRUE );
	m_oBody.m_vVelocity = TVector4::VEC_ZERO;
	m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
	m_oBody.m_vForce  = TVector4::VEC_ZERO;
	m_oBody.m_vTorque = TVector4::VEC_ZERO;

	m_vPrevPosition    = a_rcPosition;
	m_qPrevOrientation = a_rcOrientation;
	m_vPrevVelocity    = TVector4::VEC_ZERO;
	m_bFrozen          = TFALSE;

	UpdateWheelContacts();
}

// $Barnyard: FUNCTION 00661f40
void APhysicsVehicle::Stop()
{
	m_oBody.m_vVelocity = TVector4::VEC_ZERO;
	m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
	m_oBody.m_vForce  = TVector4::VEC_ZERO;
	m_oBody.m_vTorque = TVector4::VEC_ZERO;

	m_vPrevPosition    = m_oBody.m_oTransform.GetTranslation();
	m_qPrevOrientation = m_oBody.m_qOrientation;
	m_vPrevVelocity    = TVector4::VEC_ZERO;
	m_bFrozen          = TFALSE;

	UpdateWheelContacts();
}

// $Barnyard: FUNCTION 00660970
void APhysicsVehicle::UpdateWheelContacts()
{
	for ( TINT i = 0; i < m_iNumWheels; i++ )
		m_aWheels[ i ].UpdateContact();

	TINT iNumOnGround = 0;

	for ( TINT i = 0; i < m_iNumWheels; i++ )
	{
		if ( m_aWheels[ i ].m_bOnGround )
			iNumOnGround++;
	}

	m_iNumWheelsOnGround = iNumOnGround;
}

// $Barnyard: FUNCTION 00662330
void APhysicsVehicle::UpdateSuspension()
{
	for ( TINT i = 0; i < m_iNumWheels; i++ )
	{
		TVector4 vForce = m_aWheels[ i ].GetSuspensionForce( &m_oBottomOut );
		m_oBody.ApplyForce( vForce, m_aWheels[ i ].m_vPosition );
	}

	if ( m_oBottomOut.iCount > 0 )
		ApplyBottomOut( &m_oBottomOut );
}

// $Barnyard: FUNCTION 006622d0
void APhysicsVehicle::ApplyWheelForces()
{
	for ( TINT i = 0; i < m_iNumWheels; i++ )
	{
		TVector4 vForce = m_aWheels[ i ].GetTyreForce( m_fDeltaTime );
		m_oBody.ApplyForce( vForce, m_aWheels[ i ].m_vPosition );
	}
}


// $Barnyard: FUNCTION 006609d0
void APhysicsVehicle::ApplyBottomOut( APhysicsVehicleBottomOut* a_pBottomOut )
{
	if ( s_bBottomOut )
	{
		TVector4 vForce  = m_oBody.m_vForce;
		TVector4 vTorque = m_oBody.m_vTorque;

		m_oBody.m_vForce  = TVector4::VEC_ZERO;
		m_oBody.m_vTorque = TVector4::VEC_ZERO;

		TVector4 vAngularImpulse;

		if ( s_bLimitBottomOut )
		{
			// Move the body out of the ground
			TVector4 vMove;
			vMove.Multiply( m_oBody.m_oTransform.AsBasisVector4( 1 ), -1.0f );
			vMove.Multiply( vMove, a_pBottomOut->fMaxDepth );

			TVector4 vPosition;
			vPosition.Add( m_oBody.m_oTransform.GetTranslation(), vMove );
			m_oBody.SetPosition( vPosition, TFALSE );

			TFLOAT fScale = 1.0f / a_pBottomOut->iCount;

			// Average impulse, each component is limited
			TVector4 vImpulse;
			vImpulse.Multiply( a_pBottomOut->vImpulse, fScale );

			TFLOAT fMaxImpulse = s_fBottomOutMaxImpulse * m_oBody.m_fMass * 9.8000002f;
			vImpulse.x         = TMath::Sign( vImpulse.x ) * TMath::Min( TMath::Abs( vImpulse.x ), fMaxImpulse );
			vImpulse.y         = TMath::Sign( vImpulse.y ) * TMath::Min( TMath::Abs( vImpulse.y ), fMaxImpulse );
			vImpulse.z         = TMath::Sign( vImpulse.z ) * TMath::Min( TMath::Abs( vImpulse.z ), fMaxImpulse );

			m_oBody.m_vForce.x += vImpulse.x;
			m_oBody.m_vForce.y += vImpulse.y;
			m_oBody.m_vForce.z += vImpulse.z;

			// Average torque, limited by the inertia
			TFLOAT fContactScale = m_oBody.m_fContactScale;

			TVector4 vAverageTorque;
			vAverageTorque.Multiply( a_pBottomOut->vTorque, fScale );
			vAngularImpulse.Multiply( vAverageTorque, fContactScale );

			TVector4 vInertia = m_oBody.m_vInertia;

			TVector4 vMaxTorque;
			vMaxTorque.Multiply( vInertia, 19.6f );
			vMaxTorque.Multiply( vMaxTorque, 0.5f );
			vMaxTorque.Multiply( vMaxTorque, s_fBottomOutMaxTorque );

			TMath::Clip( vAngularImpulse.x, -vMaxTorque.x, vMaxTorque.x );
			TMath::Clip( vAngularImpulse.y, -vMaxTorque.y, vMaxTorque.y );
			TMath::Clip( vAngularImpulse.z, -vMaxTorque.z, vMaxTorque.z );
		}
		else
		{
			TVector4 vMove;
			vMove.Multiply( m_oBody.m_oTransform.AsBasisVector4( 1 ), -1.0f );
			vMove.Multiply( vMove, a_pBottomOut->fMaxDepth );

			TVector4 vPosition;
			vPosition.Add( m_oBody.m_oTransform.GetTranslation(), vMove );
			m_oBody.SetPosition( vPosition, TTRUE );

			m_oBody.m_vForce.x += a_pBottomOut->vImpulse.x;
			m_oBody.m_vForce.y += a_pBottomOut->vImpulse.y;
			m_oBody.m_vForce.z += a_pBottomOut->vImpulse.z;

			TFLOAT fContactScale = m_oBody.m_fContactScale;

			TVector4 vAverageTorque;
			vAverageTorque.Multiply( a_pBottomOut->vTorque, 1.0f / a_pBottomOut->iCount );
			vAngularImpulse.Multiply( vAverageTorque, fContactScale );
		}

		m_oBody.m_vTorque.x += vAngularImpulse.x;
		m_oBody.m_vTorque.y += vAngularImpulse.y;
		m_oBody.m_vTorque.z += vAngularImpulse.z;

		m_oBody.IntegrateVelocity( 1.0f );

		m_oBody.m_vForce  = vForce;
		m_oBody.m_vTorque = vTorque;
	}
}

// $Barnyard: FUNCTION 0065ec20
void APhysicsVehicle::StopAirRoll()
{
	if ( m_iNumWheelsOnGround != 0 )
	{
		m_bAirRollStopped = TFALSE;
	}
	else if ( m_bAirRollStopped == TFALSE && m_iNumContacts == 0 )
	{
		if ( TVector4::DotProduct3( m_oBody.m_vVelocity, TVector4::VEC_NEGY ) >= 0.0f )
		{
			const TVector4& rcAxis = m_oBody.m_oTransform.AsBasisVector4( 2 );

			TFLOAT   fAngularVelocity = TVector4::DotProduct3( rcAxis, m_oBody.m_vAngularVelocity );
			TVector4 vInertia         = m_oBody.m_vInertia;

			TVector4 vTorque;
			vTorque.Multiply( rcAxis, 1.0f / m_fDeltaTime * vInertia.z * fAngularVelocity * -0.80000001f );
			m_oBody.AddTorque( vTorque );
		}

		m_bAirRollStopped = TTRUE;
	}
}

// $Barnyard: FUNCTION 0065ed00
void APhysicsVehicle::StopAirPitch()
{
	if ( s_bStopAirPitch )
	{
		if ( m_iNumWheelsOnGround != 0 )
		{
			m_bAirPitchStopped = TFALSE;
		}
		else if ( m_bAirPitchStopped == TFALSE && m_iNumContacts == 0 )
		{
			if ( TVector4::DotProduct3( m_oBody.m_vVelocity, TVector4::VEC_NEGY ) >= 0.0f )
			{
				const TVector4& rcAxis = m_oBody.m_oTransform.AsBasisVector4( 0 );

				TFLOAT fAngularVelocity = -TVector4::DotProduct3( rcAxis, m_oBody.m_vAngularVelocity );
				TFLOAT fTorque          = 1.0f / m_fDeltaTime * fAngularVelocity * m_oBody.GetInertia().x * 0.5f;

				TVector4 vTorque;
				vTorque.Multiply( rcAxis, fTorque );
				m_oBody.AddTorque( vTorque );
			}

			m_bAirPitchStopped = TTRUE;
		}
	}
}

// $Barnyard: FUNCTION 0065ede0
void APhysicsVehicle::ApplyAirPitchUpright()
{
	if ( s_bAirPitchUpright )
	{
		TFLOAT fMinCos = (TFLOAT)cos( 1.483529852703214 );

		if ( TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( 1 ), TVector4::VEC_POSY ) >= fMinCos )
		{
			TVector4 vInertia = m_oBody.m_vInertia;
			TFLOAT   fScale   = 1.0f / m_fDeltaTime * vInertia.x;

			if ( m_iNumWheelsOnGround == 0 && m_iNumContacts == 0 )
			{
				TFLOAT fSinPitch = -m_oBody.m_oTransform.m_f32;
				TMath::Clip( fSinPitch, -1.0f, 1.0f );

				TVector4 vTorque;
				vTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( 0 ), ( 0.2617994f - TMath::ASin( fSinPitch ) ) * fScale );
				m_oBody.AddTorque( vTorque );
			}
		}
	}
}

// $Barnyard: FUNCTION 0065eed0
void APhysicsVehicle::ApplyGroundPitchDamping()
{
	if ( s_bGroundPitchDamping && m_iNumWheelsOnGround > 0 )
	{
		TFLOAT fPitchVelocity = TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( 0 ), m_oBody.m_vAngularVelocity );
		TFLOAT fRollVelocity  = TVector4::DotProduct3( m_oBody.m_oTransform.AsBasisVector4( 2 ), m_oBody.m_vAngularVelocity );
		TMath::Clip( fPitchVelocity, -1.5f, 1.5f );
		TMath::Clip( fRollVelocity, -1.5f, 1.5f );

		// NOTE: the roll damping is multiplied by zero in the original
		TVector4 vInertia = m_oBody.m_vInertia;
		fRollVelocity     = 0.0f;

		TVector4 vRollTorque;
		vRollTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( 2 ), TMath::Abs( 0.0f ) * vInertia.z * 0.0f );

		TVector4 vPitchTorque;
		vPitchTorque.Multiply( m_oBody.m_oTransform.AsBasisVector4( 0 ), -( TMath::Abs( fPitchVelocity ) * vInertia.x * fPitchVelocity ) );

		TVector4 vTorque;
		vTorque.Add( vPitchTorque, vRollTorque );
		m_oBody.m_vTorque.x += vTorque.x;
		m_oBody.m_vTorque.y += vTorque.y;
		m_oBody.m_vTorque.z += vTorque.z;
	}
}

// $Barnyard: FUNCTION 0065f050
void APhysicsVehicle::UpdateSteeringAngle()
{
	TFLOAT fSpeedFactor = m_fSteeringSpeedFactor;
	TFLOAT fSpeed       = VectorLength( m_oBody.m_vVelocity );
	TFLOAT fMaxAngle    = m_fMaxSteeringAngle / ( fSpeed * fSpeed * fSpeedFactor + 1.0f );
	TFLOAT fMinAngle    = -fMaxAngle;

	if ( m_fSteering <= 0.0f )
	{
		if ( m_fSteering >= 0.0f )
			m_fSteeringAngle = 0.0f;
		else
			m_fSteeringAngle = -( fMaxAngle * m_fSteering );

		TMath::Clip( m_fSteeringAngle, fMinAngle, fMaxAngle );
	}
	else
	{
		m_fSteeringAngle = fMinAngle * m_fSteering;
		TMath::Clip( m_fSteeringAngle, fMinAngle, fMaxAngle );
	}
}

// $Barnyard: FUNCTION 0065f120
void APhysicsVehicle::ApplyRollingResistance()
{
	if ( m_iNumWheelsOnGround > 0 )
	{
		const TVector4& rcAxis = m_oBody.m_oTransform.AsBasisVector4( 2 );

		TVector4 vForce;
		vForce.Multiply( rcAxis, -( TVector4::DotProduct3( rcAxis, m_oBody.m_vVelocity ) * m_fRollingResistance ) );

		m_oBody.m_vForce.x += vForce.x;
		m_oBody.m_vForce.y += vForce.y;
		m_oBody.m_vForce.z += vForce.z;
	}
}

// $Barnyard: FUNCTION 0065f1b0
void APhysicsVehicle::ApplyGravity()
{
	TFLOAT fMass = m_oBody.m_fMass;

	TVector4 vGravity;
	vGravity.Multiply( TVector4::VEC_POSY, 19.6f );

	TVector4 vForce;
	vForce.Multiply( vGravity, fMass );

	m_oBody.m_vForce.x += vForce.x;
	m_oBody.m_vForce.y += vForce.y;
	m_oBody.m_vForce.z += vForce.z;
}

// $Barnyard: FUNCTION 0065f370
void APhysicsVehicle::ApplySurfaceDrag()
{
	if ( m_iNumWheelsOnGround != 0 )
	{
		TFLOAT fMass  = m_oBody.m_fMass;
		TFLOAT fSpeed = VectorLength( m_oBody.m_vVelocity );

		TVector4 vDrag;
		vDrag.Multiply( m_oBody.GetVelocityDirection(), -fSpeed );
		vDrag.Multiply( vDrag, VectorLength( m_oBody.m_vVelocity ) );
		vDrag.Multiply( vDrag, fMass );

		for ( TINT i = 0; i < m_iNumWheels; i++ )
		{
			if ( m_aWheels[ i ].m_bOnGround )
			{
				TFLOAT fDrag;

				switch ( m_aWheels[ i ].m_iSurfaceType )
				{
					case AVehicleManager::SURFACE_GRASS: fDrag = m_fSurfaceDrag0; break;
					case AVehicleManager::SURFACE_DIRT: fDrag = m_fSurfaceDrag1; break;
					case AVehicleManager::SURFACE_MUD: fDrag = m_fSurfaceDrag7; break;
					default: fDrag = m_fSurfaceDrag; break;
				}

				if ( fDrag > 0.0f )
				{
					TVector4 vForce;
					vForce.Multiply( vDrag, fDrag );

					m_oBody.m_vForce.x += vForce.x;
					m_oBody.m_vForce.y += vForce.y;
					m_oBody.m_vForce.z += vForce.z;
				}
			}
		}
	}
}

//-----------------------------------------------------------------------------
// APhysicsWheel
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 0065f670
void APhysicsWheel::UpdateContact()
{
	ARigidBody* pBody      = &m_pVehicle->m_oBody;
	TMatrix44   oTransform = m_pVehicle->m_oBody.m_oTransform;

	TMatrix44::TransformVector( m_vPosition, oTransform, m_oParams.vPosition );

	// Cast a ray along the Y axis of the body through the wheel
	TVector4 vAxisY = oTransform.AsBasisVector4( 1 );

	TVector4 vOffset;
	vOffset.Multiply( vAxisY, m_oParams.fRadius );
	vOffset.Multiply( vOffset, 2.0f );

	TVector4 vRayStart;
	vRayStart.Substract( m_vPosition, vOffset );

	vOffset.Multiply( vAxisY, 4.0f );

	TVector4 vRayEnd;
	vRayEnd.Add( vOffset, vRayStart );

	TUINT uiResponseTypes = m_pVehicle->m_oBody.m_oMaterial.m_uiResponseTypes;
	TUINT uiIgnoreGroups  = m_pVehicle->m_oBody.m_oMaterial.m_uiIgnoreGroups;

	ARigidBodyCollision oCollision;

	if ( AVehicleManager::CollideRay( vRayStart, vRayEnd, &oCollision, ~uiResponseTypes, (TCollisionCommon::TOSHICGROUPFLAG)uiIgnoreGroups ) == TTRUE )
	{
		TFLOAT fScale = 1.0f / ( s_fGroundNormalSmoothing + 1.0f );

		TVector4 vNormal;
		vNormal.Multiply( m_vGroundNormal, s_fGroundNormalSmoothing );
		vNormal.Add( oCollision.m_vNormal, vNormal );
		vNormal.w = m_vGroundNormal.w;

		m_vGroundNormal.Multiply( vNormal, fScale );
		m_vGroundNormal.Normalise();

		m_vContactPosition = oCollision.m_vPosition;

		TVector4 vToContact;
		vToContact.Substract( m_vContactPosition, m_vPosition );

		m_fGroundDistance = TVector4::DotProduct3( vToContact, oTransform.AsBasisVector4( 1 ) );
		m_bOnGround       = m_fGroundDistance < m_oParams.fMaxExtension;
		m_iSurfaceType    = oCollision.m_iSurfaceType;
	}
	else
	{
		m_vContactPosition = TVector4::VEC_ZERO;
		m_vGroundNormal    = TVector4::VEC_NEGY;
		m_bOnGround        = TFALSE;
		m_fGroundDistance  = m_oParams.fMaxExtension;
		m_iSurfaceType     = AVehicleManager::SURFACE_NONE;
	}

	// Axes of the wheel on the ground
	TVector4 vAxis1;
	vAxis1.CrossProduct( oTransform.AsBasisVector4( 0 ), m_vGroundNormal );
	vAxis1.Normalise();

	TVector4 vAxis2;
	vAxis2.CrossProduct( m_vGroundNormal, vAxis1 );

	TFLOAT fSteeringAngle = ( m_oParams.bSteerable ) ? m_pVehicle->m_fSteeringAngle : 0.0f;

	TFLOAT fSin;
	TFLOAT fCos;
	TMath::SinCos( fSteeringAngle, fSin, fCos );

	TVector4 vTemp1;
	vTemp1.Multiply( vAxis2, fSin );
	TVector4 vTemp2;
	vTemp2.Multiply( vAxis1, fCos );
	m_vForward.Set( vTemp1.x + vTemp2.x, vTemp1.y + vTemp2.y, vTemp1.z + vTemp2.z, vTemp2.w );

	vTemp1.Multiply( vAxis2, fCos );
	vTemp2.Multiply( vAxis1, -fSin );
	m_vSide.Set( vTemp1.x + vTemp2.x, vTemp1.y + vTemp2.y, vTemp1.z + vTemp2.z, vTemp2.w );

	// Slip angle
	m_vContactVelocity = pBody->GetPointVelocity( m_vPosition );

	TVector4 vVelocity  = m_vContactVelocity;
	TFLOAT   fNormalVel = TVector4::DotProduct3( vVelocity, m_vGroundNormal );
	TFLOAT   fSpeedSq   = vVelocity.MagnitudeSq();
	TFLOAT   fSlipAngle;

	if ( fSpeedSq <= s_fMinSlipSpeedSq || fNormalVel * fNormalVel / fSpeedSq >= s_fMaxSlipNormalCos )
	{
		fSlipAngle = 0.0f;
	}
	else
	{
		vVelocity.Normalise();

		TVector4 vScaled;
		vScaled.Multiply( vVelocity, 1000.0f );

		TFLOAT fSide       = TVector4::DotProduct3( vScaled, m_vSide );
		TFLOAT fForward    = TVector4::DotProduct3( vScaled, m_vForward );
		TFLOAT fAbsForward = TMath::Abs( fForward );

		if ( fAbsForward >= s_fMinSlipSpeed )
		{
			fSlipAngle = (TFLOAT)-atan2( fSide, fAbsForward );
		}
		else
		{
			fSlipAngle = 3.1415927f * 0.5f;

			if ( fForward < 0.0f )
				fSlipAngle = fSlipAngle * -1.0f;
		}
	}

	m_fSlipAngle = ( s_fSlipAngleSmoothing * m_fSlipAngle + fSlipAngle ) / ( s_fSlipAngleSmoothing + 1.0f );

	if ( m_bOnGround )
		m_fAngularVelocity = TVector4::DotProduct3( pBody->m_oTransform.AsBasisVector4( 2 ), pBody->m_vVelocity ) / m_oParams.fRadius;

	TFLOAT fRotation = m_pVehicle->m_fDeltaTime * m_fAngularVelocity + m_fRotation;
	m_fRotation      = fRotation;

	if ( fRotation <= 6.2831855f )
	{
		if ( -6.2831855f > fRotation )
			m_fRotation = fRotation + 6.2831855f;
	}
	else
	{
		m_fRotation = fRotation - 6.2831855f;
	}

	m_fDownForceLoad = 0.0f;
}

// $Barnyard: FUNCTION 006614f0
void APhysicsWheel::AddBottomOutImpulse( APhysicsVehicleBottomOut* a_pBottomOut )
{
	ARigidBody* pBody = &m_pVehicle->m_oBody;

	if ( TVector4::DotProduct3( m_vContactVelocity, m_vGroundNormal ) <= 0.0f )
	{
		TVector4 vImpulse;
		vImpulse.Multiply( m_vGroundNormal, pBody->CalculateCollisionImpulse( m_vPosition, m_vGroundNormal ) * 1.01f );

		TVector4 vOffset;
		vOffset.Substract( m_vPosition, pBody->m_vCentreOfMass );

		TVector4 vTorque;
		vTorque.CrossProduct( vOffset, vImpulse );

		a_pBottomOut->vTorque.x += vTorque.x;
		a_pBottomOut->vTorque.y += vTorque.y;
		a_pBottomOut->vTorque.z += vTorque.z;
		a_pBottomOut->vImpulse.x += vImpulse.x;
		a_pBottomOut->vImpulse.y += vImpulse.y;
		a_pBottomOut->vImpulse.z += vImpulse.z;
		a_pBottomOut->iCount++;

		TFLOAT fDepth           = m_fGroundDistance - m_oParams.fMaxCompression;
		a_pBottomOut->fMaxDepth = ( a_pBottomOut->fMaxDepth >= fDepth ) ? a_pBottomOut->fMaxDepth : fDepth;

		if ( s_bMoveBottomedOutWheels == TTRUE )
		{
			TVector4 vMove;
			vMove.Multiply( pBody->m_oTransform.AsBasisVector4( 1 ), -1.0f );
			vMove.Multiply( vMove, fDepth );

			m_vPosition.x += vMove.x;
			m_vPosition.y += vMove.y;
			m_vPosition.z += vMove.z;
		}
	}
}

// $Barnyard: FUNCTION 00660120
TFLOAT APhysicsWheel::CalculateSpringForce( TFLOAT a_fLength, TFLOAT a_fMaxForce )
{
	TFLOAT fStiffness   = -m_oParams.fSpringStiffness;
	TFLOAT fCompression = -m_oParams.fCompressionDamping;
	TFLOAT fRebound     = -m_oParams.fReboundDamping;
	TFLOAT fMax         = m_oParams.fMaxCompression;
	TFLOAT fMin         = -m_oParams.fMaxExtension;
	TFLOAT fInvMass     = 1.0f / ( m_pVehicle->m_oBody.m_fMass * 0.5f );
	TFLOAT fStep        = 0.1f * m_pVehicle->m_fDeltaTime;
	TFLOAT fLength      = m_fPrevSpringLength;
	TFLOAT fVelocity    = ( a_fLength - fLength ) * ( 1.0f / m_pVehicle->m_fDeltaTime );
	TFLOAT fForce       = 0.0f;

	// Simulate the spring in 10 steps
	for ( TINT i = 0; i < 10; i++ )
	{
		TFLOAT fDamping = ( fVelocity > 0.0f ) ? fVelocity * fCompression : fVelocity * fRebound;
		TFLOAT fAccel   = fDamping + fLength * fStiffness;

		fVelocity = fAccel * fStep * fInvMass + fVelocity;
		fLength   = fVelocity * fStep + fLength;
		TMath::Clip( fLength, fMin, fMax );

		fForce = fForce - fAccel;
	}

	fForce *= 0.1f;
	m_fPrevSpringLength = a_fLength;
	TMath::Clip( fForce, 0.0f, a_fMaxForce );

	return fForce;
}

// $Barnyard: FUNCTION 00662150
TVector4 APhysicsWheel::GetSuspensionForce( APhysicsVehicleBottomOut* a_pBottomOut )
{
	TFLOAT fMaxForce  = s_fMaxSpringForceScale * m_pVehicle->m_oBody.m_fMass * 19.6f;
	TFLOAT fDownForce = m_oParams.fDownForce * 17.639999f;

	m_bBottomedOut = TFALSE;

	TFLOAT fCompression = m_fGroundDistance * -1.0f;
	m_fGroundDistance   = fCompression;

	TFLOAT fForce;

	if ( fCompression > m_oParams.fMaxCompression )
	{
		// The suspension bottomed out
		m_bBottomedOut = TTRUE;
		AddBottomOutImpulse( a_pBottomOut );
		fForce = CalculateSpringForce( m_oParams.fMaxCompression, fMaxForce ) + fDownForce;
	}
	else if ( -m_oParams.fMaxExtension >= fCompression )
	{
		// The wheel is in the air
		m_fPrevSpringLength = ( TVector4::DotProduct3( m_pVehicle->m_oBody.m_oTransform.AsBasisVector4( 1 ), TVector4::VEC_POSY ) <= 0.0f ) ?
		    m_oParams.fMaxExtension * -0.1f :
		    -m_oParams.fMaxExtension;

		if ( s_bSmoothSpringForce == TTRUE )
			fForce = 1.0f / ( s_fSpringForceSmoothing + 1.0f ) * ( s_fSpringForceSmoothing * m_fSpringForce );
		else
			fForce = 0.0f;
	}
	else
	{
		fForce = CalculateSpringForce( fCompression, fMaxForce ) + fDownForce;
	}

	m_fSpringForce = fForce;

	TVector4 vForce;
	vForce.Multiply( m_vGroundNormal, fForce );

	return vForce;
}

// $Barnyard: FUNCTION 00662050
TVector4 APhysicsWheel::GetTyreForce( TFLOAT a_fDeltaTime )
{
	TVector4 vForce = TVector4::VEC_ZERO;

	if ( m_oParams.eType == APhysicsWheelParams::TYPE_BIKE )
	{
		vForce = GetBikeTyreForce( a_fDeltaTime );

		if ( cos( 0.7853981573134661 ) > TVector4::DotProduct3( m_vGroundNormal, TVector4::VEC_NEGY ) )
			vForce = TVector4::VEC_ZERO;
	}
	else if ( m_oParams.eType == APhysicsWheelParams::TYPE_CAR )
	{
		vForce = GetCarTyreForce( a_fDeltaTime );

		if ( cos( 0.7853981573134661 ) > TVector4::DotProduct3( m_vGroundNormal, TVector4::VEC_NEGY ) )
			vForce = TVector4::VEC_ZERO;
	}

	return vForce;
}

// $Barnyard: FUNCTION 00661100
TVector4 APhysicsWheel::GetBikeTyreForce( TFLOAT a_fDeltaTime )
{
	APhysicsVehicle* pVehicle = m_pVehicle;
	TFLOAT           fGrip    = m_oParams.fGrip;

	if ( pVehicle->m_bUnkCD2 )
	{
		if ( pVehicle->IsSliding() && pVehicle->m_oGearBox.iGear == 2 )
		{
			TFLOAT fBrakeGrip = m_oParams.fBrakeGripFast;

			if ( pVehicle->m_fBrake > 0.0f )
			{
				if ( m_oParams.bUnk3D )
				{
					TFLOAT fBlend = ( VectorLength( pVehicle->m_oBody.m_vVelocity ) - 11.111112f ) * 0.35999998f;
					TMath::Clip( fBlend, 0.0f, 1.0f );

					fGrip = TMath::LERP( m_oParams.fBrakeGripSlow, m_oParams.fBrakeGripFast, fBlend ) * fGrip;
				}
				else
				{
					fGrip = m_oParams.fBrakeGripSlow * fGrip;
				}
			}
			else
			{
				fGrip = fBrakeGrip * fGrip;
			}
		}
	}
	else if ( m_oParams.bUnk3D && pVehicle->m_fHandBrake > 0.0f && pVehicle->m_oGearBox.iGear == 2 )
	{
		fGrip = fGrip * s_fHandBrakeGripScale;
	}

	ARigidBody* pBody       = &pVehicle->m_oBody;
	TFLOAT      fMaxLateral = pVehicle->m_oBody.m_fMass * 19.6f / pVehicle->m_iNumWheels * fGrip;
	TBOOL       bUnkState   = pVehicle->IsWheelie() || pVehicle->IsStoppie();

	if ( pVehicle->m_iNumWheelsOnGround > 0 || bUnkState )
	{
		if ( VectorLength( m_vContactVelocity ) <= pVehicle->m_fLowSpeed )
		{
			// Stop the sideways movement
			TFLOAT fImpulse = pBody->CalculateImpulse( m_vPosition, m_vSide, a_fDeltaTime, TVector4::VEC_ZERO );
			TFLOAT fForce   = ( fImpulse + fImpulse ) / pVehicle->m_iNumWheels;

			if ( s_bClampLowSpeedLateral == TTRUE )
				TMath::Clip( fForce, -fMaxLateral, fMaxLateral );

			m_fLateralForce = fForce;
		}
		else
		{
			TFLOAT fSlip = m_fSlipAngle * 11.459156f;

			if ( s_bSoftSlipLimit )
			{
				TFLOAT fZero   = 0.0f;
				TFLOAT fExcess = TMath::Abs( fSlip ) - 1.0f;

				TFLOAT* pExcess = ( fExcess <= 0.0f ) ? &fZero : &fExcess;
				m_fLateralForce = ( fSlip - TMath::Sign( fSlip ) * *pExcess * 0.85000002f ) * fMaxLateral;
			}
			else
			{
				TMath::Clip( fSlip, -1.0f, 1.0f );
				m_fLateralForce = fSlip * fMaxLateral;
			}
		}

		if ( bUnkState && !m_bOnGround )
			m_fLateralForce = m_fLateralForce * 0.1f;
	}
	else
	{
		m_fLateralForce = s_fAirLateralSmoothing * m_fLateralForce / ( s_fAirLateralSmoothing + 1.0f );

		if ( pVehicle->IsDriverless() == TTRUE )
			m_fLateralForce = 0.0f;
	}

	if ( !m_bEnabled )
		m_fLateralForce = 0.0f;

	TVector4 vForce;
	vForce.Multiply( m_vSide, m_fLateralForce );

	if ( m_bOnGround )
	{
		TVector4 vDrive;
		vDrive.Multiply( m_vForward, -m_fDriveForce );

		TVector4 vEngine;
		vEngine.Multiply( m_vForward, -m_fEngineForce );

		vForce.x += vDrive.x + vEngine.x;
		vForce.y += vDrive.y + vEngine.y;
		vForce.z += vDrive.z + vEngine.z;
	}

	return vForce;
}

// $Barnyard: FUNCTION 0065fe10
TVector4 APhysicsWheel::GetCarTyreForce( TFLOAT a_fDeltaTime )
{
	APhysicsVehicle* pVehicle = m_pVehicle;
	ARigidBody*      pBody    = &pVehicle->m_oBody;
	TFLOAT           fGrip    = m_oParams.fGrip;
	TFLOAT           fLoad    = pVehicle->m_oBody.m_fMass * 19.6f / pVehicle->m_iNumWheels;

	if ( pVehicle->m_bUnkCD2 && pVehicle->IsSliding() )
	{
		if ( pVehicle->m_oGearBox.iGear >= 2 )
		{
			TFLOAT fBrakeGrip = m_oParams.fBrakeGripFast;

			if ( pVehicle->m_fBrake > 0.0f )
			{
				if ( m_oParams.bUnk3D )
				{
					TFLOAT fBlend = ( VectorLength( pVehicle->m_oBody.m_vVelocity ) - 11.111112f ) * 0.35999998f;
					TMath::Clip( fBlend, 0.0f, 1.0f );
					fBrakeGrip = TMath::LERP( m_oParams.fBrakeGripSlow, m_oParams.fBrakeGripFast, fBlend );
				}
				else
				{
					fBrakeGrip = m_oParams.fBrakeGripSlow;
				}
			}

			fGrip = fBrakeGrip * fGrip;
		}
	}

	TFLOAT fMaxLateral = fGrip * m_fDownForceLoad + fGrip * fLoad;

	if ( pVehicle->m_iNumWheelsOnGround <= 0 )
	{
		m_fEngineForce  = 0.0f;
		m_fDriveForce   = 0.0f;
		m_fLateralForce = s_fCarAirLateralSmoothing * m_fLateralForce / ( s_fCarAirLateralSmoothing + 1.0f );
	}
	else if ( VectorLength( m_vContactVelocity ) <= pVehicle->m_fLowSpeed )
	{
		TFLOAT fImpulse = pBody->CalculateImpulse( m_vPosition, m_vSide, a_fDeltaTime, TVector4::VEC_ZERO );
		m_fLateralForce = ( fImpulse + fImpulse ) / pVehicle->m_iNumWheels;
	}
	else
	{
		m_fLateralForce = m_fSlipAngle * 11.459156f * fMaxLateral;
	}

	TVector4 vForce;
	vForce.Multiply( m_vSide, m_fLateralForce );

	if ( s_bClampCarTyreForce == TTRUE )
	{
		TFLOAT fForce = VectorLength( vForce );

		if ( fForce > fMaxLateral )
			vForce.Multiply( fMaxLateral / fForce );
	}

	if ( m_bOnGround )
	{
		TVector4 vDrive;
		vDrive.Multiply( m_vForward, -m_fDriveForce );

		TVector4 vEngine;
		vEngine.Multiply( m_vForward, -m_fEngineForce );

		vForce.x += vDrive.x + vEngine.x;
		vForce.y += vDrive.y + vEngine.y;
		vForce.z += vDrive.z + vEngine.z;
	}

	return vForce;
}

//-----------------------------------------------------------------------------
// APhysicsEngine2
//-----------------------------------------------------------------------------

// $Barnyard: FUNCTION 00660320
void APhysicsEngine2::UpdateRPM( TFLOAT a_fDeltaTime )
{
	TFLOAT fGearRatio = m_fGearRatio;
	TFLOAT fRPM       = m_fRPM;
	TFLOAT fNewRPM;

	if ( m_pGearBox->iGear == 1 )
	{
		// Neutral
		fNewRPM = a_fDeltaTime * m_fTorque * 20.0f + fRPM;
	}
	else if ( m_bShiftingDown == TTRUE )
	{
		TFLOAT fMaxRPM = m_fMaxRPM / ( 60.0f / 6.2831855f );
		fNewRPM        = m_fRPM - ( m_fRPM - 0.60000002f * fMaxRPM ) * ( fMaxRPM * 5.0f ) * a_fDeltaTime;
	}
	else if ( m_fClutch >= 1.0f )
	{
		fNewRPM = fGearRatio * m_pWheel1->m_fAngularVelocity;
	}
	else
	{
		fNewRPM = ( fGearRatio * m_pWheel1->m_fAngularVelocity - fRPM ) * ( 1.0f - m_fClutch ) * a_fDeltaTime +
		    ( 1.0f - m_fClutch ) * m_fTorque * a_fDeltaTime * 20.0f +
		    fRPM;
	}

	m_fPrevRPM = m_fRPM;
	m_fRPM     = fNewRPM;

	TFLOAT fScale = 1.0f / ( 60.0f / 6.2831855f );
	TMath::Clip( m_fRPM, fScale * m_fIdleRPM, fScale * m_fMaxRPM );
}

// $Barnyard: FUNCTION 00660460
TFLOAT APhysicsEngine2::GetTorque() const
{
	TFLOAT fRPMScale    = 60.0f / 6.2831855f;
	TFLOAT fInvRPMScale = 1.0f / fRPMScale;

	if ( fInvRPMScale * m_fMaxRPM <= m_fRPM )
		return 0.0f;

	TFLOAT fPeakRPM = fInvRPMScale * m_fPeakTorqueRPM;
	TFLOAT fProgress;
	TFLOAT fStart;
	TFLOAT fRange;

	if ( fPeakRPM <= m_fRPM )
	{
		fProgress = ( m_fRPM - fInvRPMScale * m_fPeakTorqueRPM ) / ( ( m_fMaxRPM - m_fPeakTorqueRPM ) / fRPMScale );
		fStart    = m_fPeakTorque;
		fRange    = m_fMaxRPMTorque - fStart;
	}
	else
	{
		fProgress = m_fRPM / fPeakRPM;
		fStart    = m_fIdleTorque;
		fRange    = m_fPeakTorque - fStart;
	}

	TFLOAT fTorque = fProgress * fRange + fStart;

	if ( m_pVehicle->m_bUseTorqueScale == TTRUE )
		return fTorque * m_fTorqueScale;

	return fTorque;
}

// $Barnyard: FUNCTION 00661870
void APhysicsEngine2::UpdateTorque( TFLOAT a_fDeltaTime )
{
	if ( m_iUnk0C == 0 )
		return;

	APhysicsGearBox* pGearBox = m_pGearBox;
	APhysicsVehicle* pVehicle = m_pVehicle;

	TFLOAT fRevLimit = m_fMaxRPM / ( 60.0f / 6.2831855f ) * 0.94999999f;
	TFLOAT fThrottle = ( pGearBox->iGear != 0 ) ? pVehicle->m_fThrottle : pVehicle->m_fBrake;
	m_fThrottle      = fThrottle;

	if ( pGearBox->iGear == 1 )
	{
		// Neutral, the engine only revs
		TFLOAT fEngineTorque = GetTorque();
		TFLOAT fBraking      = a_fDeltaTime * m_fEngineBraking * m_fRPM;
		TFLOAT fTorque       = 0.0f;

		if ( m_fRevLimiterTime <= 0.0f )
		{
			if ( m_fRPM <= fRevLimit )
			{
				if ( fThrottle > 0.0f )
					fTorque = fEngineTorque * fThrottle;
			}
			else
			{
				m_fRevLimiterTime = a_fDeltaTime;
			}
		}
		else
		{
			m_fRevLimiterTime = m_fRevLimiterTime - a_fDeltaTime;

			if ( m_fRevLimiterTime < 0.0f )
				m_fRevLimiterTime = 0.0f;
		}

		m_fTorque                 = fTorque - fBraking;
		m_pWheel1->m_fEngineForce = 0.0f;
		m_pWheel2->m_fEngineForce = 0.0f;
		return;
	}

	TFLOAT fAcceleration = pVehicle->m_vLocalVelocity.z - pVehicle->m_vPrevLocalVelocity.z;

	if ( pGearBox->iGear == 0 )
		fAcceleration = fAcceleration * -1.0f;

	TFLOAT fInvDeltaTime = 1.0f / a_fDeltaTime;
	TFLOAT fEngineTorque = GetTorque();
	TFLOAT fLoss         = TMath::Abs( ( 1.0f - m_fEfficiency ) * m_fGearRatio * fEngineTorque );
	TFLOAT fDrive;
	TFLOAT fInertia;

	if ( m_fClutch >= 1.0f )
	{
		if ( fThrottle > 0.0f )
		{
			fDrive   = TMath::Abs( fEngineTorque * m_fGearRatio * fThrottle );
			fInertia = fInvDeltaTime * m_fInertia * fAcceleration;
		}
		else
		{
			fInertia = fInvDeltaTime * m_fInertia * fAcceleration;
			fDrive   = 0.0f;
		}
	}
	else if ( pGearBox->iGear == 2 && pGearBox->iPrevGear == 1 )
	{
		fDrive   = TMath::Abs( fEngineTorque * m_fGearRatio * fThrottle );
		fInertia = fInvDeltaTime * m_fInertia * fAcceleration * m_fClutch;
	}
	else
	{
		fInertia = fInvDeltaTime * m_fInertia * fAcceleration * m_fClutch;
		fDrive   = 0.0f;
	}

	if ( m_fRevLimiterTime <= 0.0f )
	{
		if ( m_fRPM <= fRevLimit )
		{
			m_fTorque = fDrive - fLoss - fInertia;
		}
		else
		{
			m_fRevLimiterTime = a_fDeltaTime;
			m_fTorque         = -fInertia - fLoss;
		}
	}
	else
	{
		m_fRevLimiterTime = m_fRevLimiterTime - a_fDeltaTime;

		if ( m_fRevLimiterTime < 0.0f )
			m_fRevLimiterTime = 0.0f;

		m_fTorque = -fInertia - fLoss;
	}

	if ( pGearBox->iGear == 0 )
		m_fTorque = m_fTorque * -1.0f;

	m_pWheel1->m_fEngineForce = m_fClutch / m_pWheel1->m_oParams.fRadius * m_fTorque * 0.5f;
	m_pWheel2->m_fEngineForce = m_pWheel1->m_fEngineForce;
}

// $Barnyard: FUNCTION 006604f0
void APhysicsEngine2::UpdateGears( TFLOAT a_fDeltaTime )
{
	APhysicsVehicle* pVehicle = m_pVehicle;
	TFLOAT           fSpeed   = pVehicle->m_vLocalVelocity.z;

	if ( m_fShiftTimer > 0.0f )
	{
		if ( m_bShiftingDown == TFALSE )
			m_fShiftTimer = m_fShiftTimer - a_fDeltaTime;

		if ( m_fShiftTimer >= 0.0f )
		{
			// Still changing the gear
			if ( m_bShiftingUp == TFALSE && m_bShiftingDown == TFALSE )
			{
				m_fClutch = 1.0f - m_fShiftTimer / m_fClutchTime;
			}
			else if ( m_bShiftingUp == TTRUE )
			{
				if ( pVehicle->m_fThrottle == 0.0f )
				{
					m_fShiftTimer = 0.0f;
					m_bShiftingUp = TFALSE;
					m_fClutch     = 1.0f;
				}
				else
				{
					m_fClutch = m_fShiftTimer / m_fShiftTime * 0.0f + 1.0f;
				}
			}
			else
			{
				m_fClutch = 1.0f;

				if ( m_fMaxRPM / ( 60.0f / 6.2831855f ) * 0.9f < fSpeed / m_pWheel1->m_oParams.fRadius / m_fRPM * m_fGearRatio || pVehicle->m_fThrottle == 0.0f )
				{
					m_fShiftTimer   = 0.0f;
					m_bShiftingDown = TFALSE;
					m_fClutch       = 1.0f;
				}
			}

			return;
		}

		m_fShiftTimer   = 0.0f;
		m_fClutch       = 1.0f;
		m_bShiftingUp   = TFALSE;
		m_bShiftingDown = TFALSE;
	}

	TFLOAT           fWheelRadius = m_pWheel1->m_oParams.fRadius;
	TFLOAT           fThrottle    = pVehicle->m_fThrottle;
	TFLOAT           fBrake       = pVehicle->m_fBrake;
	APhysicsGearBox* pGearBox     = m_pGearBox;
	TINT             iGear        = pGearBox->iGear;
	TFLOAT           fInvRPMScale = 1.0f / ( 60.0f / 6.2831855f );
	TFLOAT           fMaxRPM      = fInvRPMScale * m_fMaxRPM;
	TFLOAT           fHalfMaxRPM  = 0.5f * fMaxRPM;
	TFLOAT           fInvRatio    = 1.0f / m_fGearRatio;
	TFLOAT           fDownSpeed   = fInvRatio * fHalfMaxRPM * fWheelRadius;
	TFLOAT           fFirstSpeed  = fHalfMaxRPM / ( m_fFinalDrive * m_afGearRatios[ 2 ] ) * fWheelRadius;

	if ( fSpeed > fInvRatio * fMaxRPM * fWheelRadius * 0.9f && iGear > 1 )
	{
		// Shift up
		if ( iGear >= m_fNumGears )
			return;

		pGearBox->iPrevGear = iGear;
		pGearBox->iGear     = iGear + 1;
	}
	else if ( fSpeed < fDownSpeed && iGear > 2 )
	{
		// Shift down
		pGearBox->iGear     = iGear - 1;
		pGearBox->iPrevGear = iGear;
	}
	else
	{
		if ( fBrake > 0.0f && fThrottle > 0.0f )
		{
			if ( iGear == 1 )
			{
				m_fReverseTimer = 0.0f;
				m_fClutch       = 0.0f;
				return;
			}
		}
		else if ( iGear == 1 && pVehicle->m_fThrottle > 0.0f && pGearBox->bAllowDrive == TTRUE )
		{
			// Neutral to first gear
			pGearBox->iPrevGear = pGearBox->iGear;
			pGearBox->iGear     = 2;
			m_bShiftingDown     = TFALSE;
			m_bShiftingUp       = TTRUE;
			goto ON_GEAR_CHANGED;
		}

		if ( fInvRPMScale * m_fIdleRPM < m_fRPM )
		{
			if ( fThrottle > 0.0f && pGearBox->bAllowDrive == TTRUE )
			{
				if ( iGear != 1 )
					return;

				pGearBox->iPrevGear = pGearBox->iGear;
				pGearBox->iGear     = 2;
			}
			else
			{
				if ( fSpeed >= fFirstSpeed )
					return;

				if ( iGear <= 2 && fThrottle != 0.0f )
					return;

				if ( iGear == 0 )
					return;

				// To neutral
				pGearBox->iGear     = 1;
				pGearBox->iPrevGear = iGear;
			}
		}
		else
		{
			if ( iGear == 2 )
			{
				if ( fThrottle != 0.0f )
					return;

				pGearBox->iGear     = 1;
				pGearBox->iPrevGear = iGear;
			}
			else if ( iGear == 0 )
			{
				if ( fBrake != 0.0f )
					return;

				pGearBox->iPrevGear = pGearBox->iGear;
				pGearBox->iGear     = 1;
			}
			else
			{
				if ( iGear != 1 )
					return;

				if ( fBrake == 0.0f )
					return;

				if ( pGearBox->bAllowReverse != TTRUE )
					return;

				if ( pVehicle->IsSliding() )
					return;

				// Hold the brake in neutral to reverse
				m_fReverseTimer = a_fDeltaTime + m_fReverseTimer;

				if ( m_fReverseTimer <= m_fShiftDelay )
					return;

				m_pGearBox->iPrevGear = m_pGearBox->iGear;
				m_pGearBox->iGear     = 0;
			}
		}
	}

ON_GEAR_CHANGED:
	m_fReverseTimer = 0.0f;

	if ( m_bShiftingUp != TFALSE )
	{
		m_fClutch     = 0.99f;
		m_fShiftTimer = m_fShiftTime;
		OnGearChanged();
	}
	else if ( m_bShiftingDown != TFALSE )
	{
		m_fClutch     = 0.5f;
		m_fShiftTimer = m_fShiftTime;
		OnGearChanged();
	}
	else
	{
		m_fClutch = 0.0f;

		if ( m_pGearBox->iGear == 1 )
			m_fShiftTimer = 0.0f;
		else
			m_fShiftTimer = m_fClutchTime;

		OnGearChanged();
	}
}
