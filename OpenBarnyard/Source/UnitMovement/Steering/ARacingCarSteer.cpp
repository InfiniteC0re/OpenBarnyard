#include "pch.h"
#include "ARacingCarSteer.h"
#include "Helpers/APlayerProgress.h"
#include "FX/AParticleEffectManager.h"
#include "Units/AUnit.h"
#include "Units/AUnitCarChaseCar.h"
#include "Player/APlayerManager.h"
#include "Sound/ASoundManager.h"
#include "Input/AInputHandler.h"
#include "Helpers/ARegrowthManager.h"
#include "Physics/ACollisionObjectUnit.h"
#include "Physics/ACollisionObjectModel.h"
#include "Physics/AVehicleManager.h"
#include "Physics/APhysicsCar.h"

#include <Toshi/TScheduler.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0056e6e0, CreateObject 0056ef10, CreateObjectInPlace 0056ef70
TDEFINE_CLASS( ARacingCarSteer );

static TFLOAT s_fPedalSpeed    = 2.0f;  // 0x00774390, how fast the gradual pedal inputs change per second
static TFLOAT s_fBushEffectGap = 0.1f;  // 0x00774394, minimum time between two bushhit effects
static TFLOAT s_fTorqueScale   = 1.65f; // 0x00774398 (JP Wii 0x806b0488), scales the torque of the engine of every car

// Seat of the driver relative to the body of the car
static TVector4 s_vSeatOffset       = { 0.0f, -0.93f, -0.2f, 1.0f }; // 0x00774340
static TVector4 s_vPlayerSeatOffset = { 0.0f, -0.93f, -0.1f, 1.0f }; // 0x00774360, CARTYPE_PLAYER

// Knocks a regrowable object over (the car drives through it): the object is
// taken and its effects and sound are played where it was.
// NOTE: a free function of the original binary, also called by KnockOverCrate (0066eab0, ACoinSpawner.cpp)
// $Barnyard: FUNCTION 0066e950
void KnockOverGrowthObject( ARegrowthManager::GrowthObject* a_pObject, TINT a_iType )
{
	// NOTE: retail rotates VEC_POSZ by the orientation of the object here (006c9740, no side effects)
	// but doesn't use the result

	TVector4 vPosition = a_pObject->vecPosition;
	ARegrowthManager::GetSingleton()->TakeObject( a_iType, a_pObject );
	vPosition.y = vPosition.y + 0.2f;

	TINT iDustPuff = AParticleEffectManager::FindEffect( "dustpuff" );

	if ( iDustPuff != -1 )
		AParticleEffectManager::GetSingleton()->PlayEffect( iDustPuff, vPosition, TVector4::VEC_NEGY );

	const ARegrowthManager::GrowthMapping& rcMapping = ARegrowthManager::GetSingleton()->GetGrowthMapping( a_iType );

	if ( rcMapping.szEffect1 )
	{
		TINT iEffect = AParticleEffectManager::FindEffect( rcMapping.szEffect1 );

		if ( iEffect != -1 )
			AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, vPosition, TVector4::VEC_NEGY );
	}

	if ( rcMapping.szEffect2 )
	{
		TINT iEffect = AParticleEffectManager::FindEffect( rcMapping.szEffect2 );

		if ( iEffect != -1 )
			AParticleEffectManager::GetSingleton()->PlayEffect( iEffect, vPosition, TVector4::VEC_NEGY );
	}

	if ( rcMapping.iSound )
		ASoundManager::GetSingleton()->PlayCueAtPosition( rcMapping.iSound, vPosition );

	AInputHandler::RumbleParams oRumble = { { 0.5f, 0.0f, 0.0f, 0.25f, 0.0f, 1.0f, 1.0f }, 1 };
	AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
}

// $Barnyard: FUNCTION 0056e6f0
ARacingCarSteer::ARacingCarSteer()
    : m_iUnk21C( 0 )
    , m_pCar( TNULL )
    , m_vSeatOffset( TVector4::VEC_ZERO )
    , m_fStuckTime( 0.0f )
    , m_vLastGoodPosition( TVector4::VEC_ZERO )
    , m_vLastPosition( TVector4::VEC_ZERO )
    , m_qLastGoodOrientation( TQuaternion::IDENTITY )
    , m_iGrassEffect( -1 )
    , m_iBushEffect( -1 )
    , m_fBushEffectTime( 0.0f )
    , m_iBushCue( -1 )
{
}

// $Barnyard: FUNCTION 0056e7f0
// $Barnyard: FUNCTION 0056efb0
ARacingCarSteer::~ARacingCarSteer()
{
	AVehicleManager::RemoveVehicle( m_pCar );
	AVehicleManager::DestroyCar( m_pCar );
	m_pCar = TNULL;
}

// $Barnyard: FUNCTION 0056fe80
TBOOL ARacingCarSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	const TVector4* pDirection = a_pOther->GetDirection();
	const TVector4* pPosition  = a_pOther->GetPosition();
	CreatePhysics( *pPosition, *pDirection );

	return TTRUE;
}

// $Barnyard: FUNCTION 0056fe40
TBOOL ARacingCarSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );
	CreatePhysics( a_rcPosition, a_rcDirection );

	return TTRUE;
}

// $Barnyard: FUNCTION 0056f210
// $BarnyardWiiJP: FUNCTION 802dddb4
void ARacingCarSteer::CreatePhysics( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	m_pCar = AVehicleManager::CreateCar();

	AUnitCarChaseCar::CARTYPE_ eCarType = TSTATICCAST( AUnitCarChaseCar, &GetParentUnit() )->GetCarType();

	TFLOAT fFrontAxle = 1.83f; // distance of the front wheels from the centre
	TFLOAT fRearAxle  = 1.83f; // distance of the rear wheels from the centre
	TFLOAT fHalfTrack = 0.8f;  // distance of the wheels from the middle

	TVector4 vBoxSize( 2.3f, 1.3f, 6.2f, 1.0f );
	TVector4 vBoxExtents;
	vBoxExtents.Multiply( vBoxSize, 0.5f );

	m_vSeatOffset = s_vSeatOffset;

	TVector4 vBoxCentre( 0.0f, -1.2f, -0.3f, 1.0f );

	if ( eCarType == AUnitCarChaseCar::CARTYPE_PLAYER )
	{
		// The player's truck is shorter
		fFrontAxle = 1.78f;
		fRearAxle  = 1.78f;
		fHalfTrack = 0.85f;

		TVector4 vPlayerBoxSize( 2.3f, 1.7f, 5.4f, 1.0f );
		vBoxExtents.Multiply( vPlayerBoxSize, 0.5f );

		m_vSeatOffset = s_vPlayerSeatOffset;
		vBoxCentre.Set( 0.0f, -1.35f, -0.1f, 1.0f );
	}

#ifdef BARNYARD_LATEST
	// The JP Wii build retunes the handling for the Wii Remote steering (more grip, slower steering, less torque). It
	// makes the trucks easier to drive, so here the Wii handling is only used on Easy; Normal keeps the PC retail handling.
	const TBOOL bWiiHandling = APlayerProgress::GetSingleton()->IsEasyDifficulty();
#endif

	// The vehicle
	APhysicsVehicleParams oVehicleParams;
	oVehicleParams.fMass = 1000.0f;
	oVehicleParams.vSize.Set( 2.6f, 2.0f, 5.0f, 1.0f );
	oVehicleParams.fGearShiftDelay   = 0.75f;
	oVehicleParams.fMaxSteeringAngle = 0.6981317f;
#ifdef BARNYARD_LATEST
	// The JP Wii build steers slower and the cops lose their faster steering
	if ( bWiiHandling )
		oVehicleParams.fSteeringSpeedFactor = ( eCarType == AUnitCarChaseCar::CARTYPE_PLAYER ) ? 0.0041f : 0.0037f;
	else
		oVehicleParams.fSteeringSpeedFactor = ( eCarType == AUnitCarChaseCar::CARTYPE_PLAYER || eCarType == AUnitCarChaseCar::CARTYPE_COP ) ? 0.005f : 0.0045f;
#else
	oVehicleParams.fSteeringSpeedFactor = ( eCarType == AUnitCarChaseCar::CARTYPE_PLAYER || eCarType == AUnitCarChaseCar::CARTYPE_COP ) ? 0.005f : 0.0045f;
#endif
	oVehicleParams.fLowSpeed          = 6.9444447f;
	oVehicleParams.iNumWheels         = 4;
	oVehicleParams.fRollingResistance = 0.01f;
	oVehicleParams.fSurfaceDrag       = 0.0f;
	oVehicleParams.fUnk30             = 0.005f;
	oVehicleParams.fSurfaceDrag7      = 0.1f;
	oVehicleParams.fSurfaceDrag0      = 0.002f;

	// The car
	APhysicsCarParams oCarParams;
	oCarParams.fUnk00         = 8000.0f;
	oCarParams.fBrakeForce    = 18000.0f;
	oCarParams.fRearBrakeBias = 0.5f;
	oCarParams.fMaxSpeed      = ( eCarType == AUnitCarChaseCar::CARTYPE_BEADY ) ? 50.0f : 55.555557f;
	oCarParams.fUnk10         = 11.111112f;
	oCarParams.avSpheres[ 0 ].Set( fHalfTrack, -0.8f, 2.5f, 0.4f );
	oCarParams.avSpheres[ 1 ].Set( -fHalfTrack, -0.8f, 2.5f, 0.4f );
	oCarParams.avSpheres[ 2 ].Set( fHalfTrack, -0.8f, -3.1f, 0.4f );
	oCarParams.avSpheres[ 3 ].Set( -fHalfTrack, -0.8f, -3.1f, 0.4f );
	oCarParams.iNumSpheres            = 4;
	oCarParams.fFrontAntiRoll         = 49000.0f;
	oCarParams.fRearAntiRoll          = 5880.0005f;
	oCarParams.fFriction              = 0.0f;
	oCarParams.fRestitution           = 0.01f;
	oCarParams.fUpsideDownFriction    = 0.2f;
	oCarParams.fUpsideDownRestitution = 0.01f;
	oCarParams.fDownForce             = 0.5f;
	oCarParams.vBoxExtents            = vBoxExtents;
	oCarParams.vBoxCentre             = vBoxCentre;

#ifdef BARNYARD_LATEST
	// NOTE: the JP Wii build lowers the torque of the player's truck by writing the shared
	// s_fTorqueScale, so every car created after it (the cops, Beady) also gets 1.5 instead
	// of 1.65 until the game restarts. Kept as in the JP Wii build (see Docs/RetailBugs.md), on Easy only.
	// NOTE: on Normal the player's truck sets it back to the PC value 1.65, which PC retail never changes, so a
	// Normal game after an Easy one plays like PC retail.
	if ( eCarType == AUnitCarChaseCar::CARTYPE_PLAYER )
		s_fTorqueScale = bWiiHandling ? 1.5f : 1.65f;
#endif // BARNYARD_LATEST

	// The engine
	APhysicsEngineParams oEngineParams;
	oEngineParams.fPeakTorque    = s_fTorqueScale * 600.0f;
	oEngineParams.fPeakTorqueRPM = 4000.0f;
	oEngineParams.fIdleTorque    = s_fTorqueScale * 400.0f;
	oEngineParams.fMaxRPMTorque  = s_fTorqueScale * 520.0f;
	oEngineParams.fMaxRPM        = 6000.0f;
	oEngineParams.fIdleRPM       = 1500.0f;
	oEngineParams.fUnk18         = 1.0f;
	// NOTE: retail leaves fUnk1C uninitialised (the engines don't use it)
	oEngineParams.fUnk1C            = 0.0f;
	oEngineParams.fEngineBraking    = 10.0f;
	oEngineParams.afGearRatios[ 0 ] = -9.5f;
	oEngineParams.afGearRatios[ 1 ] = 9.5f;
	oEngineParams.afGearRatios[ 2 ] = 9.5f;
	oEngineParams.afGearRatios[ 3 ] = 5.6f;
	oEngineParams.afGearRatios[ 4 ] = 4.0f;
	oEngineParams.afGearRatios[ 5 ] = 3.0f;
	oEngineParams.afGearRatios[ 6 ] = 2.5f;
	oEngineParams.fFinalDrive       = 1.6f;
	oEngineParams.fNumGears         = 7.0f;
	oEngineParams.fShiftDelay       = 0.5f;
	oEngineParams.fClutchTime       = 0.1f;
	oEngineParams.fShiftTime        = 1.0f;
	oEngineParams.fTorqueScale      = 2.0f;

	TFLOAT fDamping = TFLOAT( sqrt( 15000000.0 ) );
	TFLOAT fAxles   = fRearAxle + fFrontAxle;

	// The front wheels (steerable)
	APhysicsWheelParams oFrontWheel;
#ifdef BARNYARD_LATEST
	oFrontWheel.fGrip = bWiiHandling ? 2.4f : 2.0f;
#else
	oFrontWheel.fGrip                   = 2.0f;
#endif
	oFrontWheel.fRadius             = 0.4f;
	oFrontWheel.fSpringStiffness    = 15000.0f;
	oFrontWheel.fCompressionDamping = fDamping;
	oFrontWheel.fReboundDamping     = 1.5f * fDamping;
	oFrontWheel.fMaxExtension       = 0.3f;
	oFrontWheel.fMaxCompression     = 0.3f;
	oFrontWheel.fUnk1C              = fHalfTrack;
	oFrontWheel.fUnk20              = -0.4f;
	oFrontWheel.fUnk24              = fFrontAxle;
	oFrontWheel.fUnk28              = 1.0f;
	oFrontWheel.vPosition.Set( fHalfTrack, 0.0f, fFrontAxle, 1.0f );
	oFrontWheel.bSteerable = TTRUE;
	oFrontWheel.bUnk3D     = TFALSE;
	oFrontWheel.fDownForce = fRearAxle * 1000.0f / fAxles * 0.5f;
	oFrontWheel.eType      = APhysicsWheelParams::TYPE_CAR;
#ifdef BARNYARD_LATEST
	oFrontWheel.fBrakeGripFast = bWiiHandling ? 0.9f : 0.85f;
#else
	oFrontWheel.fBrakeGripFast          = 0.85f;
#endif
	oFrontWheel.fBrakeGripSlow = 1.0f;

	APhysicsWheelParams oFrontWheel2 = oFrontWheel;
	oFrontWheel.fUnk1C               = -fHalfTrack;
	oFrontWheel.vPosition.x          = -fHalfTrack;

	// The rear wheels (driven by the engine)
	APhysicsWheelParams oRearWheel;
#ifdef BARNYARD_LATEST
	oRearWheel.fGrip = bWiiHandling ? 2.4f : 2.0f;
#else
	oRearWheel.fGrip                    = 2.0f;
#endif
	oRearWheel.fRadius             = 0.4f;
	oRearWheel.fSpringStiffness    = 15000.0f;
	oRearWheel.fCompressionDamping = fDamping;
	oRearWheel.fReboundDamping     = oFrontWheel.fReboundDamping;
	oRearWheel.fMaxExtension       = 0.3f;
	oRearWheel.fMaxCompression     = 0.3f;
	oRearWheel.fUnk1C              = fHalfTrack;
	oRearWheel.fUnk20              = -0.4f;
	oRearWheel.fUnk24              = -fRearAxle;
	oRearWheel.fUnk28              = 1.0f;
	oRearWheel.vPosition.Set( fHalfTrack, 0.0f, -fRearAxle, 1.0f );
	oRearWheel.bSteerable = TFALSE;
	oRearWheel.bUnk3D     = TTRUE;
	oRearWheel.fDownForce = fFrontAxle * 1000.0f / fAxles * 0.5f;
	oRearWheel.eType      = APhysicsWheelParams::TYPE_CAR;
#ifdef BARNYARD_LATEST
	oRearWheel.fBrakeGripFast = bWiiHandling ? 0.9f : 0.85f;
	oRearWheel.fBrakeGripSlow = bWiiHandling ? 0.6f : 0.3f;
#else
	oRearWheel.fBrakeGripFast           = 0.85f;
	oRearWheel.fBrakeGripSlow           = 0.3f;
#endif

	APhysicsWheelParams oRearWheel2 = oRearWheel;
	oRearWheel2.fUnk1C              = -fHalfTrack;
	oRearWheel2.vPosition.x         = -fHalfTrack;

	m_pCar->Setup( oCarParams, oVehicleParams, oEngineParams, oFrontWheel, oFrontWheel2, oRearWheel, oRearWheel2 );

	// Put the car at the position, facing the direction
	TVector4 vDirection( a_rcDirection.x, a_rcDirection.y, a_rcDirection.z, 1.0f );

	TMatrix44 matTransform;
	matTransform.LookAtDirection( vDirection, TVector4::VEC_NEGY );
	matTransform.GetTranslation() = a_rcPosition;

	TQuaternion qOrientation;
	TMatrix44::MatToQuat( qOrientation, matTransform );

	m_pCar->Reset( matTransform.GetTranslation(), qOrientation );
	m_pCar->m_bFlipRecovery = TTRUE;
	m_pCar->m_bUnkCD2       = TTRUE;

	m_vLastPosition = m_pCar->GetBody().GetPosition();

	AVehicleManager::RemoveVehicle( m_pCar );
	AVehicleManager::AddVehicle( m_pCar );

	m_iGrassEffect = AParticleEffectManager::FindEffect( "grasswalk" );
	m_iBushEffect  = AParticleEffectManager::FindEffect( "bushhit" );
}

// $Barnyard: FUNCTION 0056fb70
TBOOL ARacingCarSteer::Update( TFLOAT a_fDeltaTime )
{
	UpdateStuck( a_fDeltaTime );
	UpdateRespawn();
	m_pCar->VTable0x1C();

	// Keep the state of ACarSteer in sync with the physics
	const TMatrix44& rcTransform = m_pCar->GetBody().GetTransform();

	m_vBodyPosition = rcTransform.GetTranslation();
	m_vPosition     = rcTransform.GetTranslation();

	for ( TINT i = 0; i < NUM_WHEELS; i++ )
		m_avWheelPositions[ i ] = m_pCar->m_aWheels[ i ].m_vPosition;

	m_vDirection = rcTransform.AsBasisVector4( BASISVECTOR_FORWARD );

	m_vAxle2.Set( m_pCar->m_aWheels[ 1 ].m_vPosition.x + m_pCar->m_aWheels[ 0 ].m_vPosition.x, m_pCar->m_aWheels[ 1 ].m_vPosition.y + m_pCar->m_aWheels[ 0 ].m_vPosition.y, m_pCar->m_aWheels[ 1 ].m_vPosition.z + m_pCar->m_aWheels[ 0 ].m_vPosition.z, m_pCar->m_aWheels[ 0 ].m_vPosition.w );
	m_vAxle2.Multiply( 0.5f );

	m_vAxle1.Set( m_pCar->m_aWheels[ 3 ].m_vPosition.x + m_pCar->m_aWheels[ 2 ].m_vPosition.x, m_pCar->m_aWheels[ 3 ].m_vPosition.y + m_pCar->m_aWheels[ 2 ].m_vPosition.y, m_pCar->m_aWheels[ 3 ].m_vPosition.z + m_pCar->m_aWheels[ 2 ].m_vPosition.z, m_pCar->m_aWheels[ 2 ].m_vPosition.w );
	m_vAxle1.Multiply( 0.5f );

	m_vUp = rcTransform.AsBasisVector4( BASISVECTOR_UP );
	m_vRight.CrossProduct( m_vDirection, m_vUp );

	UpdateCollisions( a_fDeltaTime );
	m_vLastPosition = m_pCar->GetBody().GetPosition();

	return TTRUE;
}

// $Barnyard: FUNCTION 0056ee50
void ARacingCarSteer::UpdateStuck( TFLOAT a_fDeltaTime )
{
	if ( m_pCar->m_iNumWheelsOnGround != 0 || m_pCar->m_bFrozen || m_pCar->IsDriverless() || m_pCar->m_iNumContacts != 0 )
	{
		m_fStuckTime = 0.0f;

		// Remember where the car stands on its 4 wheels without touching anything
		if ( m_pCar->m_iNumWheelsOnGround == 4 && m_pCar->m_iNumContacts == 0 )
		{
			m_vLastGoodPosition    = m_pCar->GetBody().GetPosition();
			m_qLastGoodOrientation = m_pCar->GetBody().m_qOrientation;
		}
	}
	else
	{
		// Neither on the ground nor touching anything
		m_fStuckTime = a_fDeltaTime + m_fStuckTime;
	}
}

// $Barnyard: FUNCTION 0056edb0
void ARacingCarSteer::UpdateRespawn()
{
	if ( m_fStuckTime > 5.0f )
	{
		m_pCar->GetBody().m_vVelocity = TVector4::VEC_ZERO;
		m_pCar->GetBody().SetAngularVelocity( TVector4::VEC_ZERO );
		m_pCar->Reset( m_vLastGoodPosition, m_qLastGoodOrientation );

		m_vLastPosition = m_vLastGoodPosition;
		m_fStuckTime    = 0.0f;
	}
}

// $Barnyard: FUNCTION 0056efd0
void ARacingCarSteer::UpdateCollisions( TFLOAT a_fDeltaTime )
{
	m_fBushEffectTime = TMath::Max( m_fBushEffectTime - a_fDeltaTime, 0.0f );

	for ( TINT i = 0; i < m_pCar->m_iNumContacts; i++ )
	{
		APhysicsVehicleContact* pContact = m_pCar->GetContact( i );
		ACollisionObject*       pObject  = pContact->pCollisionObject;

		if ( !pObject || pObject->IsA( &TGetClass( ACollisionObjectUnit ) ) )
			continue;

		if ( pObject->GetType() & ACollisionObject::TYPE_BUSH )
		{
			// Drove through a bush
			if ( m_fBushEffectTime <= 0.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_iBushEffect, pContact->vPosition, pContact->vNormal );
				m_fBushEffectTime = s_fBushEffectGap;
			}
		}
		else if ( !( pObject->GetType() & ACollisionObject::TYPE_TRUNK ) && pObject->IsA( &TGetClass( ACollisionObjectModel ) ) )
		{
			ARegrowthManager::GrowthObject* pGrowthObject = TSTATICCAST( ARegrowthManager::ACollisionObjectModelNode, pObject )->m_pObject;
			TINT                            iType         = ARegrowthManager::GetSingleton()->FindTypeOfObject( pGrowthObject );

			if ( ARegrowthManager::HasTypeFlag( iType, ARegrowthManager::TYPEFLAG_CRATE ) )
			{
				// Knock the object over
				KnockOverGrowthObject( pGrowthObject, iType );

				TFLOAT fVolume = GetSpeed() / GetMaxSpeed();
				TMath::Clip( fVolume, 0.25f, 1.0f );

				if ( m_iBushCue == -1 || !ASoundManager::GetSingleton()->IsCueUsed( m_iBushCue ) )
					m_iBushCue = ASoundManager::GetSingleton()->PlayCueAtPositionEx( 407, pContact->vPosition, fVolume * 0.5f, TFALSE, 0.0f, -1 );

				if ( !APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer() )
				{
					AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
					AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
				}
			}
		}
	}
}

// $Barnyard: FUNCTION 0056e860
TBOOL ARacingCarSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_pCar->Reset( a_rcPosition, m_pCar->GetBody().m_qOrientation );
	m_vLastPosition = a_rcPosition;

	return TTRUE;
}

// $Barnyard: FUNCTION 0056ea50
void ARacingCarSteer::SetDirection( const TVector4& a_rcDirection )
{
	TMatrix44 matTransform;
	matTransform.LookAtDirection( a_rcDirection, TVector4::VEC_NEGY );

	TQuaternion qOrientation;
	TMatrix44::MatToQuat( qOrientation, matTransform );

	m_pCar->Reset( m_pCar->GetBody().GetPosition(), qOrientation );
	m_vLastPosition = m_pCar->GetBody().GetPosition();
}

// $Barnyard: FUNCTION 0056e8d0
void ARacingCarSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_pCar->GetBody().GetPosition();
}

// $Barnyard: FUNCTION 0056e8a0
void ARacingCarSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_pCar->GetBody().GetPosition().AsVector3();
}

// $Barnyard: FUNCTION 0056e900
const TVector4* ARacingCarSteer::GetPosition()
{
	return &m_pCar->GetBody().GetPosition();
}

// $Barnyard: FUNCTION 0056e950
void ARacingCarSteer::CopyDirection( TVector4& a_rOutDirection )
{
	const TVector4& rcForward = m_pCar->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD );
	a_rOutDirection.Set( rcForward.x, rcForward.y, rcForward.z, 1.0f );
}

// $Barnyard: FUNCTION 0056e920
void ARacingCarSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	// NOTE: retail always returns (0, 0, 1) here
	a_rOutDirection = TVector4::VEC_POSZ.AsVector3();
}

// $Barnyard: FUNCTION 0056e910
const TVector4* ARacingCarSteer::GetDirection()
{
	return &m_pCar->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD );
}

// $Barnyard: FUNCTION 0056e9e0
void ARacingCarSteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	a_rOutVelocity = m_pCar->GetBody().GetVelocity();
}

// $Barnyard: FUNCTION 0056e9b0
TVector4 ARacingCarSteer::GetVelocity()
{
	return m_pCar->GetBody().GetVelocity();
}

// $Barnyard: FUNCTION 0056e990
TFLOAT ARacingCarSteer::GetSpeed()
{
	return m_pCar->GetBody().GetVelocity().Magnitude();
}

// $Barnyard: FUNCTION 0056ea10
void ARacingCarSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_pCar->GetBody().SetVelocity( a_rcVelocity );
	m_pCar->GetBody().SetAngularVelocity( TVector4::VEC_ZERO );
}

// $Barnyard: FUNCTION 0056eb30
void ARacingCarSteer::GetSeatMatrix( TMatrix44& a_rOutMatrix )
{
	a_rOutMatrix = m_pCar->GetBody().GetTransform();

	TVector4 vOffset;
	TMatrix44::RotateVector( vOffset, a_rOutMatrix, m_vSeatOffset );

	a_rOutMatrix.m_f41 = vOffset.x + a_rOutMatrix.m_f41;
	a_rOutMatrix.m_f42 = vOffset.y + a_rOutMatrix.m_f42;
	a_rOutMatrix.m_f43 = vOffset.z + a_rOutMatrix.m_f43;

	a_rOutMatrix.RotateX( 1.5707964f );
}

// $Barnyard: FUNCTION 0056eac0
void ARacingCarSteer::Reset( TVector4 a_vPosition, TVector4 a_vDirection )
{
	SetPosition( a_vPosition );
	SetDirection( a_vDirection );
	ACarSteer::Reset( a_vPosition, a_vDirection );
}

// $Barnyard: FUNCTION 0056ec00
void ARacingCarSteer::SetPedal( PEDAL a_ePedal, TBOOL a_bGradual )
{
	switch ( a_ePedal )
	{
		case PEDAL_NONE:
			if ( a_bGradual == TTRUE )
			{
				TFLOAT fThrottle = m_pCar->m_fThrottle;
				TFLOAT fBrake    = m_pCar->m_fBrake;
				TFLOAT fChange   = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime() * s_fPedalSpeed;
				m_pCar->SetThrottle( fThrottle - fChange );
				m_pCar->SetBrake( fBrake - fChange );
			}
			else
			{
				m_pCar->SetThrottle( 0.0f );
				m_pCar->SetBrake( 0.0f );
			}
			break;
		case PEDAL_THROTTLE:
			if ( a_bGradual == TTRUE )
			{
				TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
				m_pCar->SetThrottle( s_fPedalSpeed * fDeltaTime + m_pCar->m_fThrottle );
				m_pCar->SetBrake( m_pCar->m_fBrake - s_fPedalSpeed * fDeltaTime );
			}
			else
			{
				m_pCar->SetThrottle( 1.0f );
			}
			break;
		case PEDAL_BRAKE:
		case PEDAL_BRAKE_ALT:
			if ( a_bGradual == TTRUE )
			{
				TFLOAT fDeltaTime = g_oSystemManager.GetScheduler()->GetCurrentDeltaTime();
				m_pCar->SetThrottle( m_pCar->m_fThrottle - s_fPedalSpeed * fDeltaTime );
				m_pCar->SetBrake( s_fPedalSpeed * fDeltaTime + m_pCar->m_fBrake );
			}
			else
			{
				m_pCar->SetBrake( 1.0f );
			}
			break;
	}
}

// $Barnyard: FUNCTION 0056eba0
void ARacingCarSteer::SetSteering( STEER a_eSteer, TFLOAT a_fSteering, TFLOAT a_fAngle )
{
	TFLOAT fSteering = a_fSteering;

	if ( a_eSteer == STEER_LEFT )
		fSteering = a_fSteering - 1.0f;
	else if ( a_eSteer == STEER_RIGHT )
		fSteering = a_fSteering + 1.0f;

	TMath::Clip( fSteering, -1.0f, 1.0f );
	m_pCar->SetSteering( fSteering );
}

// $Barnyard: FUNCTION 0056e7e0
TFLOAT ARacingCarSteer::GetMaxSpeed()
{
	return m_pCar->GetMaxSpeed();
}
