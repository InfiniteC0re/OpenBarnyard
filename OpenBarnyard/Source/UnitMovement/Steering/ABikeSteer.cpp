#include "pch.h"
#include "ABikeSteer.h"
#include "FX/AParticleEffectManager.h"
#include "FX/AParticleSystem.h"
#include "FX/ASplashManager.h"
#include "Units/AUnit.h"
#include "Units/AAnimatedUnit.h"
#include "Units/ACharacterUnit.h"
#include "Units/AUnitType.h"
#include "Units/AInteractionManager.h"
#include "Units/AKinematicsObstructionManager.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"
#include "Animation/ANamedAnimationSet.h"
#include "Animation/AModel.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "Sound/AVOStream.h"
#include "Input/AInputHandler.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/AGameTimeManager.h"
#include "GameInterface/ASSeqState.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionObjectSimple.h"
#include "Physics/ACollisionObjectUnit.h"
#include "Physics/ACollisionObjectModel.h"
#include "Physics/ACollisionManager.h"
#include "Physics/ACollisionModelInstance.h"
#include "Physics/AVehicleSkidManager.h"
#include "Physics/AVehicleManager.h"
#include "Physics/APhysicsBike.h"
#include "Helpers/ABikeHelpers.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( body_loc );
TPSTRING8_DECLARE( bikeimpact );
TPSTRING8_DECLARE( yuk );
TPSTRING8_DECLARE( trigger_onbikehit_impact );
TPSTRING8_DECLARE( trigger_onbikehit );

// NOTE: GetClass 00558280, CreateObject 00558f90, CreateObjectInPlace 00558ff0
TDEFINE_CLASS( ABikeSteer );

// The stamina the boost uses per second for each bike type (dynamic initialisers 0070a1d0..0070a210)
static TFLOAT s_fStaminaUseBike1   = 2.0f / 15.0f; // 0x00788ba8
static TFLOAT s_fStaminaUseBike2   = 2.0f / 15.0f; // 0x00788b98
static TFLOAT s_fStaminaUseBike3   = 0.1f;         // 0x00788b9c
static TFLOAT s_fStaminaUseBike4   = 1.0f / 7.0f;  // 0x00788ba0
static TFLOAT s_fStaminaUseDefault = 0.2f;         // 0x00788ba4

// Normalises the vector on the XZ plane (y = 0), (0, 0, 1) if it's zero.
// NOTE: a TVector4 method of the original binary (006c7540), a copy of it lives in AGTAStyleCameraHelper.cpp
static void NormaliseXZ( TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// Makes a plane (xyz is the normal, w the distance) through a_rcPoint
// $Barnyard: FUNCTION 00558240
static void MakePlane( TVector4& a_rOutPlane, const TVector4& a_rcNormal, const TVector4& a_rcPoint )
{
	a_rOutPlane.x = a_rcNormal.x;
	a_rOutPlane.y = a_rcNormal.y;
	a_rOutPlane.z = a_rcNormal.z;
	a_rOutPlane.w = a_rcNormal.z * a_rcPoint.z + a_rcNormal.y * a_rcPoint.y + a_rcNormal.x * a_rcPoint.x;
}

// Builds a matrix that has the up axis of the world and faces the forward axis of a_rcMatrix
// $Barnyard: FUNCTION 00558e60
static void MakeUprightMatrix( TMatrix44& a_rOutMatrix, const TMatrix44& a_rcMatrix )
{
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_UP ) = TVector4( 0.0f, 1.0f, 0.0f, 0.0f );
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_RIGHT ).CrossProduct( a_rOutMatrix.AsBasisVector4( BASISVECTOR_UP ), a_rcMatrix.AsBasisVector4( BASISVECTOR_FORWARD ) );
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_RIGHT ).Normalise();
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_FORWARD ).CrossProduct( a_rOutMatrix.AsBasisVector4( BASISVECTOR_RIGHT ), a_rOutMatrix.AsBasisVector4( BASISVECTOR_UP ) );
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_FORWARD ).Normalise();
	a_rOutMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION ) = a_rcMatrix.AsBasisVector4( BASISVECTOR_TRANSLATION );
}

// Packs the colour of a tyre track
// $Barnyard: FUNCTION 004083c0
static TUINT32 PackColour( TFLOAT a_fRed, TFLOAT a_fGreen, TFLOAT a_fBlue, TFLOAT a_fAlpha )
{
	return ( ( ( TUINT32( a_fGreen * 255.0f ) | ( ( TUINT32( a_fRed * 255.0f ) | ( TUINT32( a_fAlpha * 255.0f ) << 8 ) ) << 8 ) ) << 8 ) | TUINT32( a_fBlue * 255.0f ) );
}

// $Barnyard: FUNCTION 00558290
ABikeSteer::ABikeSteer()
{
	m_vecSeekTarget         = TVector4::VEC_ZERO;
	m_fSpeed                = 0.0f;
	m_fAirTime              = 0.0f;
	m_vecLastGroundPosition = TVector4::VEC_ZERO;
	m_vecLastPosition       = TVector4::VEC_ZERO;
	m_fSeekSpeed            = 1.0f;
	m_fStamina              = 1.0f;
	m_fHornTime             = 0.0f;
	m_fStaminaTime          = 0.0f;
	m_bExhausted            = TFALSE;
	m_bTrickHeld            = TFALSE;
	m_iTrickDirection       = 0;
	m_iTrickState           = TRICKSTATE_NONE;
	m_bAirborne             = TFALSE;
	m_bSeeking              = TFALSE;
	m_bStopped              = TFALSE;
	m_bUnk77                = TFALSE;
	m_bInWater              = TFALSE;
	m_fInWaterTime          = 0.0f;
	m_bStoppingToGetOff     = TFALSE;
	m_bJump                 = TFALSE;
	m_bSkidding             = TFALSE;
	m_bHasCrashAnim         = TFALSE;
	m_bAIControlled         = TFALSE;
	m_bOffTerrain           = TFALSE;

	for ( TINT i = 0; i < CUE_NUMOF; i++ )
		m_aCues[ i ] = -1;

	m_iUnkA4 = 0;
	m_iUnkA8 = 0;

	for ( TINT i = 0; i < TARRAYSIZE( m_afEffectTimers ); i++ )
		m_afEffectTimers[ i ] = 0.0f;

	m_pPhysics    = TNULL;
	m_pTailLight  = TNULL;
	m_pFrontLight = TNULL;
	m_iBikeType   = 0;

	m_aTyreTrackPositions[ 0 ] = TVector4::VEC_ZERO;
	m_aTyreTrackPositions[ 1 ] = TVector4::VEC_ZERO;
	m_afTyreTrackOffsets[ 0 ]  = 0.0f;
	m_afTyreTrackOffsets[ 1 ]  = 0.0f;
	m_afTyreTrackWidths[ 0 ]   = 0.1f;
	m_afTyreTrackWidths[ 1 ]   = 0.1f;
	m_aiGroundMaterials[ 0 ]   = -1;
	m_aiGroundMaterials[ 1 ]   = -1;

	m_aiEffects[ EFFECT_WATERSPLASH2A ]   = AParticleEffectManager::FindEffect( "watersplash2a" );
	m_aiEffects[ EFFECT_WATERSPLASH2B ]   = AParticleEffectManager::FindEffect( "watersplash2b" );
	m_aiEffects[ EFFECT_WATERRIPPLE ]     = AParticleEffectManager::FindEffect( "waterripple" );
	m_aiEffects[ EFFECT_MUDRIPPLES ]      = AParticleEffectManager::FindEffect( "mudripples" );
	m_aiEffects[ EFFECT_MUDSPLASH ]       = AParticleEffectManager::FindEffect( "mudsplash" );
	m_aiEffects[ EFFECT_DUSTPUFFSMALL ]   = AParticleEffectManager::FindEffect( "dustpuffsmall" );
	m_aiEffects[ EFFECT_GRASSWALK ]       = AParticleEffectManager::FindEffect( "grasswalk" );
	m_aiEffects[ EFFECT_BIKE_TAILLIGHT ]  = AParticleEffectManager::FindEffect( "bike_taillight" );
	m_aiEffects[ EFFECT_BIKE_FRONTLIGHT ] = AParticleEffectManager::FindEffect( "bike_frontlight" );
	m_aiEffects[ EFFECT_BUSHHIT ]         = AParticleEffectManager::FindEffect( "bushhit" );
}

// $Barnyard: FUNCTION 0055b2c0
// $Barnyard: FUNCTION 0055a830
ABikeSteer::~ABikeSteer()
{
	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, &GetParentUnit() );

	if ( pUnit->GetCollisionObject() )
		pUnit->GetCollisionObject()->UpdateHash( 0.5f );

	AVehicleManager::RemoveVehicle( m_pPhysics );
	AKinematicsObstructionManager::GetSingleton()->RemoveObstruction( TSTATICCAST( AKinematicsObstruction, GetParentUnit().GetObstruction() ) );

	if ( m_aCues[ CUE_ENGINE ] != -1 )
	{
		ASoundManager::GetSingleton()->StopCue( m_aCues[ CUE_ENGINE ] );
		ASoundManager::GetSingleton()->StopCue( m_aCues[ CUE_SKID ] );
	}

	m_aCues[ CUE_ENGINE ] = -1;
	m_aCues[ CUE_LAND ]   = -1;
	m_aCues[ CUE_CRASH ]  = -1;
	m_aCues[ CUE_SKID ]   = -1;
	m_aCues[ CUE_UNK4 ]   = -1;
	m_aCues[ CUE_BUSH ]   = -1;
	m_iUnkA4              = 0;
	m_iUnkA8              = 0;

	if ( m_pTailLight )
	{
		m_pTailLight->GetInstance()->Kill();
		m_pTailLight = TNULL;
	}

	if ( m_pFrontLight )
	{
		m_pFrontLight->GetInstance()->Kill();
		m_pFrontLight = TNULL;
	}

	AVehicleManager::DestroyBike( m_pPhysics );
	m_pPhysics = TNULL;
}

// $Barnyard: FUNCTION 0055b320
TBOOL ABikeSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ACommonSteer::CreateFrom( a_pOther, a_iLOD );
	Initialise( *a_pOther->GetPosition(), *a_pOther->GetDirection() );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055b2e0
TBOOL ABikeSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ACommonSteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );
	Initialise( a_rcPosition, a_rcDirection );

	return TTRUE;
}

// $Barnyard: FUNCTION 0055c630
TBOOL ABikeSteer::Update( TFLOAT a_fDeltaTime )
{
	ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject();
	pCollisionObject->SetType( pCollisionObject->GetType() | ACollisionObject::TYPE_IGNORED );

	if ( !m_pPhysics->IsDriverless() )
		TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject()->UpdateHash( 0.5f );

	m_fHornTime -= a_fDeltaTime;
	m_fHornTime = TMath::Max( m_fHornTime, 0.0f );

	UpdateAIControl( a_fDeltaTime );

	TBOOL bResult = TTRUE;

	if ( !m_bOffTerrain )
		bResult = UpdatePlayerControl( a_fDeltaTime );

	UpdateSounds( a_fDeltaTime );
	UpdateEffects( a_fDeltaTime );
	UpdateCollisions( a_fDeltaTime );

	return bResult;
}

// $Barnyard: FUNCTION 0055ab00
TBOOL ABikeSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_fSpeed = 0.0f;
	TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject()->Update( TNULL );

	m_pPhysics->GetBody().SetPosition( a_rcPosition, TTRUE );
	m_pPhysics->Stop();

	m_vecLastPosition = a_rcPosition;
	m_bInWater        = TFALSE;
	m_fInWaterTime    = 0.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 0055aa50
TBOOL ABikeSteer::SetPosition3( const TVector3& a_rcPosition )
{
	m_fSpeed = 0.0f;
	TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject()->Update( TNULL );

	TVector4 vecPosition( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	m_pPhysics->GetBody().SetPosition( vecPosition, TTRUE );
	m_pPhysics->Stop();

	m_vecLastPosition = TVector4( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	m_bInWater        = TFALSE;
	m_fInWaterTime    = 0.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 00558590
void ABikeSteer::SetDirection( const TVector4& a_rcDirection )
{
	TMatrix44 mRotation;
	mRotation.LookAtDirection( a_rcDirection, TVector4::VEC_NEGY );

	TQuaternion qRotation;
	TMatrix44::MatToQuat( qRotation, mRotation );

	m_pPhysics->GetBody().SetOrientation( qRotation, TTRUE );
	m_pPhysics->Stop();

	m_vecLastPosition = m_pPhysics->GetBody().GetPosition();
}

// $Barnyard: FUNCTION 00558640
void ABikeSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_pPhysics->GetBody().GetPosition();
}

// $Barnyard: FUNCTION 00558610
void ABikeSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_pPhysics->GetBody().GetPosition().AsVector3();
}

// $Barnyard: FUNCTION 00558670
const TVector4* ABikeSteer::GetPosition()
{
	return &m_pPhysics->GetBody().GetPosition();
}

// $Barnyard: FUNCTION 005586c0
void ABikeSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection   = m_pPhysics->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD );
	a_rOutDirection.w = 1.0f;
}

// $Barnyard: FUNCTION 00558690
void ABikeSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_pPhysics->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD ).AsVector3();
}

// $Barnyard: FUNCTION 00558680
const TVector4* ABikeSteer::GetDirection()
{
	return &m_pPhysics->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD );
}

// $Barnyard: FUNCTION 00558700
TVector4 ABikeSteer::GetVelocity()
{
	return m_pPhysics->GetBody().GetVelocity();
}

// $Barnyard: FUNCTION 00558730
TBOOL ABikeSteer::SetDesiredVelocity( const TVector4& a_rcVelocity )
{
	if ( m_bStopped == TTRUE )
	{
		m_pPhysics->SetBrake( 1.0f );
		return TTRUE;
	}

	m_pPhysics->SetThrottle( ( a_rcVelocity.z <= 0.0f ) ? 0.0f : a_rcVelocity.z );
	m_pPhysics->SetBrake( ( a_rcVelocity.z >= 0.0f ) ? 0.0f : 1.0f );
	m_pPhysics->SetSteering( a_rcVelocity.x );
	m_pPhysics->SetLean( a_rcVelocity.y );
	m_pPhysics->SetHandBrake( ( a_rcVelocity.w < 2.0f ) ? 0.0f : 1.0f );

	if ( a_rcVelocity.w - 2.0f > 0.0f || a_rcVelocity.w == 1.0f )
	{
		if ( !m_pPhysics->m_bJump )
			m_pPhysics->m_bJump = TTRUE;

		m_bJump = TTRUE;
	}
	else
	{
		// NOTE: retail only clears the flag when it's already cleared
		if ( !m_pPhysics->m_bJump )
			m_pPhysics->m_bJump = TFALSE;

		m_bJump = TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00558530
void ABikeSteer::SetAIControlled( TBOOL a_bAIControlled )
{
	m_bAIControlled = a_bAIControlled;

	if ( a_bAIControlled )
	{
		m_pPhysics->m_aWheels[ 0 ].m_oParams.fGrip = 2.6f;
		m_pPhysics->m_aWheels[ 0 ].m_oParams.fGrip = 2.6f;
		m_pPhysics->m_fMaxSteeringAngle            = 0.6981317f;
		m_pPhysics->m_fSteeringSpeedFactor         = 0.028f;
		m_pPhysics->m_oParams.fJumpImpulse         = 30000.0f;
	}
}

// $Barnyard: FUNCTION 005588e0
void ABikeSteer::SetStamina( TFLOAT a_fStamina )
{
	TMath::Clip( a_fStamina, 0.0f, 1.0f );

	m_fStamina   = a_fStamina;
	m_bExhausted = a_fStamina <= 0.0f;
}

// $Barnyard: FUNCTION 00558c00
TBOOL ABikeSteer::IsSunk() const
{
	return m_fInWaterTime > 1.0f;
}

// $Barnyard: FUNCTION 00558c20
TBOOL ABikeSteer::SetSeekTarget( const TVector4& a_rcTarget )
{
	m_bSeeking      = TTRUE;
	m_vecSeekTarget = a_rcTarget;
	return TTRUE;
}

// $Barnyard: FUNCTION 00558c50
void ABikeSteer::SetTrick( TBOOL a_bHeld, TINT a_iDirection )
{
	m_bTrickHeld = a_bHeld;

	if ( a_bHeld == TTRUE )
		m_iTrickDirection = a_iDirection;
}

// $Barnyard: FUNCTION 00558c70
TBOOL ABikeSteer::FacePosition( TFLOAT a_fX, TFLOAT a_fY, TFLOAT a_fZ, TFLOAT a_fW )
{
	const TVector4* pPosition = GetPosition();

	TVector4 vecDirection( a_fX - pPosition->x, a_fY - pPosition->y, a_fZ - pPosition->z, a_fW );
	NormaliseXZ( vecDirection );

	SetDirection( vecDirection );
	return TTRUE;
}

// $Barnyard: FUNCTION 00558cf0
void ABikeSteer::Jump()
{
	if ( !m_pPhysics->m_bJump )
		m_pPhysics->m_bJump = TTRUE;
}

// $Barnyard: FUNCTION 00558d10
void ABikeSteer::Stop( TBOOL a_bFreeze )
{
	m_bStopped = TTRUE;

	// NOTE: retail only clears the flag when it's already cleared
	if ( !m_pPhysics->m_bJump )
		m_pPhysics->m_bJump = TFALSE;

	m_pPhysics->SetBrake( 1.0f );
	m_pPhysics->SetSteering( 0.0f );
	m_pPhysics->SetThrottle( 0.0f );
	m_pPhysics->SetHandBrake( 0.0f );
	m_pPhysics->m_oGearBox.bAllowReverse = TFALSE;

	if ( a_bFreeze )
	{
		m_pPhysics->SetFrozen( TTRUE );
		m_pPhysics->GetBody().m_bEnabled = TFALSE;
	}
}

// $Barnyard: FUNCTION 00558da0
void ABikeSteer::Start()
{
	m_bStopped = TFALSE;
	m_pPhysics->SetFrozen( TFALSE );

	// NOTE: retail only clears the flag when it's already cleared
	if ( !m_pPhysics->m_bJump )
		m_pPhysics->m_bJump = TFALSE;

	m_pPhysics->SetBrake( 0.0f );
	m_pPhysics->SetSteering( 0.0f );
	m_pPhysics->SetThrottle( 0.0f );
	m_pPhysics->SetHandBrake( 0.0f );
	m_pPhysics->m_oGearBox.bAllowReverse = TTRUE;
	m_pPhysics->GetBody().m_bEnabled     = TTRUE;
	m_pPhysics->GetBody().m_vVelocity    = TVector4::VEC_ZERO;
	m_pPhysics->GetBody().SetAngularVelocity( TVector4::VEC_ZERO );
}

// $Barnyard: FUNCTION 00558f00
TBOOL ABikeSteer::IsAirborne() const
{
	return m_pPhysics->m_iNumWheelsOnGround == 0;
}

// $Barnyard: FUNCTION 005595f0
void ABikeSteer::SetStoppingToGetOff( TBOOL a_bUnk )
{
	m_bStoppingToGetOff = a_bUnk;

	if ( !m_bAIControlled )
	{
		if ( a_bUnk == TTRUE )
			m_pPhysics->m_bUnkCD2 = TFALSE;
		else
			m_pPhysics->m_bUnkCD2 = TTRUE;
	}
}

// $Barnyard: FUNCTION 00559c60
TBOOL ABikeSteer::CanGetOff( TBOOL a_bRight )
{
	ARigidBody& rBody = m_pPhysics->GetBody();

	TVector4 vecPosition = rBody.GetPosition();
	TVector4 vecRight    = rBody.GetTransform().AsBasisVector4( BASISVECTOR_RIGHT );
	TVector4 vecUp       = rBody.GetTransform().AsBasisVector4( BASISVECTOR_UP );
	TVector4 vecForward  = rBody.GetTransform().AsBasisVector4( BASISVECTOR_FORWARD );

	if ( !a_bRight )
		vecRight.Multiply( -1.0f );

	TVector4 vecSide;
	vecSide.Multiply( vecRight, 0.4f );
	TVector4 vecSideOffset;
	vecSideOffset.Multiply( vecSide, 1.5f );

	TVector4 vecUpOffset;
	vecUpOffset.Multiply( vecUp, -0.6f );

	TVector4 vecForwardOffset;
	vecForwardOffset.Multiply( vecForward, 0.58f );

	// The three points where the rider would stand: next to the front, the middle and the back of the bike
	TVector4 aPoints[ 3 ];
	aPoints[ 0 ].x = vecPosition.x - vecSideOffset.x + vecUpOffset.x + vecForwardOffset.x;
	aPoints[ 0 ].y = vecPosition.y - vecSideOffset.y + vecUpOffset.y + vecForwardOffset.y;
	aPoints[ 0 ].z = vecPosition.z - vecSideOffset.z + vecUpOffset.z + vecForwardOffset.z;
	aPoints[ 0 ].w = vecPosition.w;
	aPoints[ 1 ].x = vecPosition.x - vecSideOffset.x + vecUpOffset.x;
	aPoints[ 1 ].y = vecPosition.y - vecSideOffset.y + vecUpOffset.y;
	aPoints[ 1 ].z = vecPosition.z - vecSideOffset.z + vecUpOffset.z;
	aPoints[ 1 ].w = vecPosition.w;
	aPoints[ 2 ].x = vecPosition.x - vecSideOffset.x + vecUpOffset.x - vecForwardOffset.x;
	aPoints[ 2 ].y = vecPosition.y - vecSideOffset.y + vecUpOffset.y - vecForwardOffset.y;
	aPoints[ 2 ].z = vecPosition.z - vecSideOffset.z + vecUpOffset.z - vecForwardOffset.z;
	aPoints[ 2 ].w = vecPosition.w;

	TBOOL bBlocked = TFALSE;

	for ( TINT i = 0; i < 3; i++ )
	{
		ACollisionObjectSimple oSphere;
		TVector4               vecPoint = aPoints[ i ];
		oSphere.UpdateHash( 0.4f, vecPoint.AsVector3() );

		TVector4 vecNormal = TVector4::VEC_NEGY;
		TFLOAT   fDistance = 0.4f;
		bBlocked |= ACollisionManager::CollideSphereWithWorld( vecPoint, &oSphere, fDistance, vecNormal, ~rBody.m_oMaterial.m_uiResponseTypes, rBody.m_oMaterial.m_uiIgnoreGroups );

		if ( i == 0 || i == 2 )
		{
			// NOTE: retail computes these offsets from an upright matrix of the bike but never uses them
			TMatrix44 mUpright;
			MakeUprightMatrix( mUpright, rBody.GetTransform() );

			TVector4 vecUnused1;
			vecUnused1.Multiply( mUpright.AsBasisVector4( BASISVECTOR_RIGHT ), 0.4f );
			TVector4 vecUnused2;
			vecUnused2.Multiply( vecUnused1, 1.5f );
			TVector4 vecUnused3;
			vecUnused3.Multiply( mUpright.AsBasisVector4( BASISVECTOR_UP ), -0.6f );
			TVector4 vecUnused4;
			vecUnused4.Multiply( mUpright.AsBasisVector4( BASISVECTOR_FORWARD ), 0.58f );

			if ( !a_bRight )
				vecUnused2.Multiply( -1.0f );

			// The ground below the point, a bit further out
			TVector4 vecForwardOut;
			vecForwardOut.Multiply( vecForwardOffset, 1.4f );
			TVector4 vecSideOut;
			vecSideOut.Multiply( vecSideOffset, 0.5f );

			TVector4 vecStart;
			vecStart.x = vecForwardOut.x + ( ( vecPosition.x - vecSideOut.x ) + vecUpOffset.x );
			vecStart.y = vecForwardOut.y + ( ( vecPosition.y - vecSideOut.y ) + vecUpOffset.y );
			vecStart.z = vecForwardOut.z + ( ( vecPosition.z - vecSideOut.z ) + vecUpOffset.z );
			vecStart.w = vecPosition.w;

			if ( i == 2 )
			{
				TVector4 vecBackOut;
				vecBackOut.Multiply( vecForwardOffset, 0.1f );
				TVector4 vecSideFar;
				vecSideFar.Multiply( vecSideOffset, 1.5f );

				vecStart.x = ( ( vecPosition.x - vecSideFar.x ) + vecUpOffset.x ) - vecBackOut.x;
				vecStart.y = ( ( vecPosition.y - vecSideFar.y ) + vecUpOffset.y ) - vecBackOut.y;
				vecStart.z = ( ( vecPosition.z - vecSideFar.z ) + vecUpOffset.z ) - vecBackOut.z;
				vecStart.w = vecPosition.w;
			}

			TVector4 vecDown;
			vecDown.Multiply( TVector4::VEC_POSY, 1.0f );

			TVector4 vecEnd;
			vecEnd.x = vecDown.x + vecStart.x;
			vecEnd.y = vecDown.y + vecStart.y;
			vecEnd.z = vecDown.z + vecStart.z;
			vecEnd.w = vecStart.w;

			ARigidBodyCollision oCollision;

			if ( !AVehicleManager::CollideRay( vecStart, vecEnd, &oCollision, ~rBody.m_oMaterial.m_uiResponseTypes, rBody.m_oMaterial.m_uiIgnoreGroups ) )
			{
				// No ground to stand on
				bBlocked = TTRUE;
				break;
			}
		}

		if ( bBlocked == TTRUE )
			break;
	}

	return bBlocked == TFALSE;
}

// $Barnyard: FUNCTION 0055a810
void ABikeSteer::SetBikeType( TINT a_iType )
{
	m_iBikeType = a_iType;
	UpdateTyreTrackSizes();
}

// $Barnyard: FUNCTION 0055ab70
void ABikeSteer::PlayHorn()
{
	if ( m_bStopped != TTRUE && 0.0f == m_fHornTime )
	{
		ASoundId eSound;

		switch ( m_iBikeType )
		{
			case 1: eSound = 277; break;
			case 2: eSound = 278; break;
			case 3: eSound = 279; break;
			case 4: eSound = 268; break;
			default: eSound = 243; break;
		}

		ASoundManager::GetSingleton()->PlayCueAtUnit( eSound, &GetParentUnit() );
		m_fHornTime = 1.5f;
	}
}

// $Barnyard: FUNCTION 0055af40
void ABikeSteer::PlayCrashSound()
{
	AUnit* pUnit   = &GetParentUnit();
	TBOOL  bPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == pUnit;

	if ( m_aCues[ CUE_CRASH ] == -1 || !ASoundManager::GetSingleton()->IsCueUsed( m_aCues[ CUE_CRASH ] ) )
	{
		m_aCues[ CUE_CRASH ] = ASoundManager::GetSingleton()->PlayCueAtUnit( 245, &GetParentUnit() );

		if ( bPlayer )
		{
			AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
			AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
		}
	}
}

// $Barnyard: FUNCTION 0055b000
void ABikeSteer::PlayLandSound()
{
	AUnit* pUnit   = &GetParentUnit();
	TBOOL  bPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == pUnit;

	if ( m_aCues[ CUE_LAND ] == -1 || !ASoundManager::GetSingleton()->IsCueUsed( m_aCues[ CUE_LAND ] ) )
	{
		m_aCues[ CUE_LAND ] = ASoundManager::GetSingleton()->PlayCueAtUnit( 246, &GetParentUnit() );

		if ( bPlayer )
		{
			AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
			AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
		}
	}
}

// $Barnyard: FUNCTION 0055b0c0
void ABikeSteer::Park( TINT a_iUnk )
{
	m_pPhysics->m_bRiderless = TTRUE;

	ARigidBody& rBody = m_pPhysics->GetBody();
	rBody.m_oShape    = m_pPhysics->m_oParams.oRiderlessShape;

	m_pPhysics->m_vCentreOfMass1 = TVector4( 0.0f, -0.3f, 0.0f, 1.0f );
	rBody.m_fContactScale        = 0.2f;
	m_pPhysics->m_vCentreOfMass1 = TVector4( 0.05f, -0.3f, 0.0f, 1.0f );

	rBody.m_oMaterial.m_fRestitution    = 0.5f;
	rBody.m_oMaterial.m_fFriction       = 0.1f;
	rBody.m_oMaterial.m_uiResponseTypes = rBody.m_oMaterial.m_uiResponseTypes & ~ACollisionObject::TYPE_PLAYER;

	TFLOAT   fSize = m_pPhysics->m_vSize.x;
	TVector4 vecSize( fSize, fSize, fSize, 1.0f );
	rBody.SetBoxInertia( m_pPhysics->m_fMass / ( fSize * fSize * vecSize.x ), vecSize );

	TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject()->UpdateHash( 0.01f );
}

// $Barnyard: FUNCTION 0055b220
void ABikeSteer::Reset( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	SetPosition( a_rcPosition );
	SetDirection( a_rcDirection );

	m_iTrickState = TRICKSTATE_NONE;

	m_pPhysics->GetBody().m_vVelocity = TVector4::VEC_ZERO;
	m_pPhysics->GetBody().SetAngularVelocity( TVector4::VEC_ZERO );
	m_pPhysics->ResetThrowRider();

	AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject();
	pAnimatableObject->KillAllAnimations();
	pAnimatableObject->Update( 0.0f );
	pAnimatableObject->UpdateAttachedObjectTransforms();
}

// $Barnyard: FUNCTION 0055a960
void ABikeSteer::Initialise( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	m_bStoppingToGetOff        = TFALSE;
	m_iUnkA4                   = 0;
	m_iUnkA8                   = 0;
	m_bInWater                 = TFALSE;
	m_fInWaterTime             = 0.0f;
	m_aCues[ CUE_ENGINE ]      = -1;
	m_aCues[ CUE_LAND ]        = -1;
	m_aCues[ CUE_CRASH ]       = -1;
	m_aCues[ CUE_SKID ]        = -1;
	m_aCues[ CUE_UNK4 ]        = -1;
	m_aCues[ CUE_BUSH ]        = -1;
	m_aTyreTrackPositions[ 0 ] = TVector4::VEC_ZERO;
	m_aTyreTrackPositions[ 1 ] = TVector4::VEC_ZERO;
	m_afTyreTrackOffsets[ 0 ]  = 0.0f;
	m_afTyreTrackOffsets[ 1 ]  = 0.0f;
	m_aiGroundMaterials[ 0 ]   = -1;
	m_aiGroundMaterials[ 1 ]   = -1;
	m_afTyreTrackWidths[ 0 ]   = 0.1f;
	m_afTyreTrackWidths[ 1 ]   = 0.1f;

	CreatePhysics( a_rcPosition, a_rcDirection );

	AKinematicsObstructionManager::GetSingleton()->AddObstruction( TSTATICCAST( AKinematicsObstruction, GetParentUnit().GetObstruction() ) );
}

// $Barnyard: FUNCTION 00559030
void ABikeSteer::CreatePhysics( const TVector4& a_rcPosition, const TVector4& a_rcDirection )
{
	m_pPhysics = AVehicleManager::CreateBike();

	APhysicsVehicleParams oVehicleParams;
	oVehicleParams.fMass                = 100.0f;
	oVehicleParams.vSize                = TVector4( 1.75f, 1.5f, 2.0f, 1.0f );
	oVehicleParams.fGearShiftDelay      = 0.5f;
	oVehicleParams.fMaxSteeringAngle    = 0.61086524f;
	oVehicleParams.fSteeringSpeedFactor = 0.028f;
	oVehicleParams.fLowSpeed            = 3.4722223f;
	oVehicleParams.iNumWheels           = 2;
	oVehicleParams.fRollingResistance   = 0.01f;
	oVehicleParams.fSurfaceDrag         = 0.0f;
	oVehicleParams.fUnk30               = 0.0f;
	oVehicleParams.fSurfaceDrag7        = 0.1f;
	oVehicleParams.fSurfaceDrag0        = 0.005f;

	APhysicsBikeParams oBikeParams;

	// NOTE: retail also constructs a third, unused ARigidBodyShape here
	ARigidBodyShape oUnusedShape;

	oBikeParams.fHandBrakeTurnRate = 5.0f;
	oBikeParams.fMaxSpeed          = 8.333334f;
	oBikeParams.fMaxBoostSpeed     = 13.333334f;
	oBikeParams.fMaxReverseSpeed   = 2.7777779f;
	oBikeParams.fMaxWheelieSpeed   = 5.5555558f;
	oBikeParams.fMaxLeanAngle      = 0.5235988f;
	oBikeParams.fDriveForce        = 750.0f;
	oBikeParams.fBoostDriveForce   = 1000.0f;
	oBikeParams.fBrakeForce        = 800.0f;
	oBikeParams.fJumpImpulse       = 60000.0f;
	oBikeParams.fWheelieMinSpeed   = 1.388889f;
	oBikeParams.fWheelieFullSpeed  = 2.7777779f;
	oBikeParams.fWheelieAngle      = 0.7853982f;
	oBikeParams.fStoppieAngle      = 0.7853982f;

	// The collision box of the bike with its rider
	TVector4 vecSize( 0.57f, 0.90000004f, 1.6999999f, 1.0f );
	TVector4 vecExtents;
	vecExtents.Multiply( vecSize, 0.5f );

	TVector4 vecCentre;
	vecCentre.x = 0.0f;
	vecCentre.y = -0.3f - vecExtents.y - 0.1f;
	vecCentre.z = 0.0f;
	vecCentre.w = 1.0f;

	oBikeParams.oShape.SetType( ARigidBodyShape::TYPE_BOX );

	TMatrix44 mBox                                 = TMatrix44::IDENTITY;
	mBox.AsBasisVector4( BASISVECTOR_TRANSLATION ) = vecCentre;
	oBikeParams.oShape.SetBox( TOBBox( mBox, vecExtents ) );

	oBikeParams.oRiderlessShape.SetType( ARigidBodyShape::TYPE_BOX );
	oBikeParams.oRiderlessShape.SetBox( TOBBox( mBox, vecExtents ) );

	TFLOAT fDamping   = sqrtf( oVehicleParams.fMass * 12000.0f );
	TFLOAT fDownForce = oVehicleParams.fMass * 0.5f;

	APhysicsWheelParams oFrontWheel;
	oFrontWheel.fGrip               = 1.8f;
	oFrontWheel.fRadius             = 0.3f;
	oFrontWheel.fSpringStiffness    = 6000.0f;
	oFrontWheel.fCompressionDamping = fDamping;
	oFrontWheel.fReboundDamping     = fDamping;
	oFrontWheel.fMaxCompression     = 0.15f;
	oFrontWheel.fMaxExtension       = 0.15f;
	oFrontWheel.fUnk1C              = 0.0f;
	oFrontWheel.fUnk20              = -0.3f;
	oFrontWheel.fUnk24              = 0.58f;
	oFrontWheel.fUnk28              = 1.0f;
	oFrontWheel.vPosition           = TVector4( 0.0f, 0.0f, 0.58f, 1.0f );
	oFrontWheel.bSteerable          = TTRUE;
	oFrontWheel.bUnk3D              = TFALSE;
	oFrontWheel.fDownForce          = fDownForce;
	oFrontWheel.eType               = APhysicsWheelParams::TYPE_BIKE;
	oFrontWheel.fBrakeGripFast      = 0.5f;
	oFrontWheel.fBrakeGripSlow      = 1.0f;

	APhysicsWheelParams oRearWheel;
	oRearWheel.fGrip               = 1.8f;
	oRearWheel.fRadius             = 0.3f;
	oRearWheel.fSpringStiffness    = 6000.0f;
	oRearWheel.fCompressionDamping = fDamping;
	oRearWheel.fReboundDamping     = fDamping;
	oRearWheel.fMaxCompression     = 0.15f;
	oRearWheel.fMaxExtension       = 0.15f;
	oRearWheel.fUnk1C              = 0.0f;
	oRearWheel.fUnk20              = -0.3f;
	oRearWheel.fUnk24              = -0.58f;
	oRearWheel.fUnk28              = 1.0f;
	oRearWheel.vPosition           = TVector4( 0.0f, 0.0f, -0.58f, 1.0f );
	oRearWheel.bSteerable          = TFALSE;
	oRearWheel.bUnk3D              = TTRUE;
	oRearWheel.fDownForce          = fDownForce;
	oRearWheel.eType               = APhysicsWheelParams::TYPE_BIKE;
	oRearWheel.fBrakeGripFast      = 0.575f;
	oRearWheel.fBrakeGripSlow      = 0.3f;

	m_pPhysics->Setup( oBikeParams, oVehicleParams, oFrontWheel, oRearWheel );

	TMatrix44 mRotation;
	mRotation.LookAtDirection( a_rcDirection, TVector4::VEC_NEGY );

	TVector4    vecPosition = a_rcPosition;
	TQuaternion qRotation;
	TMatrix44::MatToQuat( qRotation, mRotation );
	m_pPhysics->Reset( vecPosition, qRotation );

	m_vecLastPosition           = m_pPhysics->GetBody().GetPosition();
	m_pPhysics->m_bFlipRecovery = m_bHasCrashAnim == TFALSE;
	m_pPhysics->m_bUnkCD2       = TTRUE;

	AVehicleManager::RemoveVehicle( m_pPhysics );
	AVehicleManager::AddVehicle( m_pPhysics );
}

// $Barnyard: FUNCTION 00558840
void ABikeSteer::UpdateControlLock()
{
	if ( m_bStopped == TTRUE || m_bStoppingToGetOff == TTRUE )
	{
		m_pPhysics->m_oGearBox.bAllowReverse = TFALSE;
		m_pPhysics->SetHandBrake( 0.0f );

		// NOTE: retail only clears the flag when it's already cleared
		if ( !m_pPhysics->m_bJump )
			m_pPhysics->m_bJump = TFALSE;

		if ( m_pPhysics->m_oGearBox.iGear != 0 )
		{
			m_pPhysics->SetThrottle( 0.0f );
			m_pPhysics->SetBrake( 1.0f );
		}
		else
		{
			m_pPhysics->SetThrottle( 1.0f );
			m_pPhysics->SetBrake( 0.0f );
		}
	}
	else
	{
		m_pPhysics->m_oGearBox.bAllowReverse = TTRUE;
	}
}

// $Barnyard: FUNCTION 00558920
void ABikeSteer::UpdateSkidding()
{
	static TFLOAT s_fSkidStartAngle = 0.09599311f;  // 0x00788b58
	static TFLOAT s_fSkidStopAngle  = 0.061086524f; // 0x00788b54

	if ( m_pPhysics->m_bUnkCD2 )
	{
		m_bSkidding = m_pPhysics->IsSliding();
	}
	else if ( m_bSkidding )
	{
		if ( m_pPhysics->m_oGearBox.iGear != 2 || TMath::Abs( m_pPhysics->m_aWheels[ 1 ].m_fSlipAngle ) < s_fSkidStopAngle )
			m_bSkidding = TFALSE;
	}
	else if ( TMath::Abs( m_pPhysics->m_aWheels[ 1 ].m_fSlipAngle ) > s_fSkidStartAngle && m_pPhysics->m_iNumWheelsOnGround == 2 && m_pPhysics->m_oGearBox.iGear == 2 && m_pPhysics->GetBody().GetVelocity().Magnitude() > 2.2222223f )
	{
		if ( m_pPhysics->m_fHandBrake > 0.0f || m_pPhysics->m_oParams.fMaxLeanAngle * 0.9f < TMath::Abs( m_pPhysics->m_fLeanAngle ) )
			m_bSkidding = TTRUE;
	}
}

// $Barnyard: FUNCTION 00558a30
void ABikeSteer::UpdateSpeed( TFLOAT a_fDeltaTime )
{
	ARigidBody& rBody = m_pPhysics->GetBody();
	m_fSpeed          = TVector4::DotProduct3( rBody.GetVelocity(), rBody.GetTransform().AsBasisVector4( BASISVECTOR_FORWARD ) );
	m_bAirborne       = m_pPhysics->m_iNumWheelsOnGround == 0;

	if ( m_pPhysics->m_iNumWheelsOnGround != 0 || m_pPhysics->m_bFrozen || m_pPhysics->IsDriverless() || m_pPhysics->m_iNumContacts != 0 )
	{
		m_fAirTime = 0.0f;

		if ( m_pPhysics->m_iNumWheelsOnGround == 2 )
			m_vecLastGroundPosition = m_pPhysics->GetBody().GetPosition();
	}
	else
	{
		m_fAirTime += a_fDeltaTime;
	}
}

// $Barnyard: FUNCTION 00558ae0
void ABikeSteer::UpdateRespawn()
{
	if ( m_fAirTime > 5.0f )
	{
		// The bike has been in the air for too long, put it back where it last stood on its wheels
		m_pPhysics->GetBody().m_vVelocity = TVector4::VEC_ZERO;
		m_pPhysics->GetBody().SetAngularVelocity( TVector4::VEC_ZERO );

		TMatrix44 mRotation = TMatrix44::IDENTITY;
		TVector4  vecDirection;
		vecDirection         = m_pPhysics->GetBody().GetTransform().AsBasisVector4( BASISVECTOR_FORWARD );
		TVector4 vecPosition = m_vecLastGroundPosition;

		// NOTE: retail aligns copies of the position and the direction to the ground but uses neither of them
		ABikeHelpers::AlignToGround( vecPosition, vecDirection, 2.0f, 40.0f );
		vecDirection.w = 1.0f;

		TQuaternion qRotation;
		TMatrix44::MatToQuat( qRotation, mRotation );
		m_pPhysics->Reset( m_vecLastGroundPosition, qRotation );

		m_vecLastPosition = m_vecLastGroundPosition;
		m_fAirTime        = 0.0f;
	}
}

// $Barnyard: FUNCTION 00558f20
void ABikeSteer::UpdateTyreTrackSizes()
{
	switch ( m_iBikeType )
	{
		case 3:
			m_afTyreTrackWidths[ 0 ] = 0.065f;
			m_afTyreTrackWidths[ 1 ] = 0.125f;
			break;
		case 4:
			m_afTyreTrackWidths[ 0 ] = 0.08f;
			m_afTyreTrackWidths[ 1 ] = 0.08f;
			break;
		default:
			m_afTyreTrackWidths[ 0 ] = 0.1f;
			m_afTyreTrackWidths[ 1 ] = 0.1f;
			break;
	}
}

// $Barnyard: FUNCTION 00559630
void ABikeSteer::UpdateStamina( TFLOAT a_fDeltaTime )
{
	TFLOAT fStaminaUse;

	switch ( m_iBikeType )
	{
		case 1: fStaminaUse = s_fStaminaUseBike1; break;
		case 2: fStaminaUse = s_fStaminaUseBike2; break;
		case 3: fStaminaUse = s_fStaminaUseBike3; break;
		case 4: fStaminaUse = s_fStaminaUseBike4; break;
		default: fStaminaUse = s_fStaminaUseDefault; break;
	}

	APhysicsBike* pPhysics = m_pPhysics;

	if ( pPhysics->m_fThrottle <= 1.0f || m_bExhausted == TTRUE )
	{
		// Recover the stamina while not boosting
		m_fStaminaTime += a_fDeltaTime;
		m_fStamina = TMath::Min( a_fDeltaTime * 0.06666667f + m_fStamina, 1.0f );
	}

	if ( pPhysics->m_fThrottle <= 1.0f )
		m_bExhausted = TFALSE;

	if ( m_bExhausted == TTRUE )
		m_pPhysics->SetThrottle( 1.0f );

	if ( m_pPhysics->m_iNumWheelsOnGround > 0 && !m_pPhysics->IsWheelie() && !m_pPhysics->IsStoppie() )
	{
		if ( m_pPhysics->m_oGearBox.iGear == 2 && m_pPhysics->m_fThrottle > 1.0f )
		{
			TFLOAT fUse = fStaminaUse * a_fDeltaTime;

			if ( fUse >= m_fStamina )
			{
				m_pPhysics->SetThrottle( 1.0f );
				m_fStamina   = 0.0f;
				m_bExhausted = TTRUE;
			}
			else
			{
				m_fStaminaTime = 0.0f;
				m_fStamina -= fUse;
			}
		}
	}
}

// $Barnyard: FUNCTION 005597b0
void ABikeSteer::UpdateAIControl( TFLOAT a_fDeltaTime )
{
	static TFLOAT s_fMaxSteeringAngle = 0.08726646f; // 0x00788b60

	if ( !m_bAIControlled )
		return;

	TFLOAT   fDistanceDown = 30.0f;
	TFLOAT   fDistanceUp   = 30.0f;
	TVector4 vecPosition   = *GetPosition();

	ATerrainInterface* pTerrain = ATerrainInterface::GetSingleton();

	if ( !pTerrain->CollideRay( vecPosition, TVector4::VEC_POSY, fDistanceDown, TCollisionCommon::TOSHICGROUPFLAG_CAMERA, TFALSE ) &&
	     !pTerrain->CollideRay( vecPosition, TVector4::VEC_NEGY, fDistanceUp, TCollisionCommon::TOSHICGROUPFLAG_CAMERA, TFALSE ) &&
	     !m_bInWater )
	{
		// No terrain around the bike (it's far from the camera), move it without the physics
		m_bOffTerrain                    = TTRUE;
		m_pPhysics->GetBody().m_bEnabled = TFALSE;

		if ( !m_bSeeking || m_bStopped )
		{
			SetPosition( vecPosition );
			return;
		}

		TVector4 vecToTarget;
		vecToTarget.Substract( m_vecSeekTarget, vecPosition );
		TFLOAT fDistance = vecToTarget.NormaliseMag();

		TVector4 vecDirection = vecToTarget;
		vecDirection.y        = 0.0f;
		vecDirection.Normalise();
		SetDirection( vecDirection );

		TFLOAT fSpeed = 8.0f;

		if ( m_fSeekSpeed < 1.0f )
			fSpeed = 6.0f;
		else if ( m_fSeekSpeed >= 2.0f )
			fSpeed = 12.0f;

		TFLOAT fStep = fSpeed * a_fDeltaTime;
		fStep        = TMath::Min( fDistanceDown, fStep );

		fDistanceDown = fStep;
		vecToTarget.Multiply( fStep );

		vecPosition.x += vecToTarget.x;
		vecPosition.y += vecToTarget.y;
		vecPosition.z += vecToTarget.z;
		SetPosition( vecPosition );

		// NOTE: retail calculates the distance to the target but never uses it
		(void)fDistance;

		TFLOAT fDeltaZ = vecPosition.z - m_vecSeekTarget.z;
		TFLOAT fDeltaX = vecPosition.x - m_vecSeekTarget.x;

		if ( fDeltaZ * fDeltaZ + fDeltaX * fDeltaX < 10.0f )
			m_bSeeking = TFALSE;

		return;
	}

	if ( m_bOffTerrain )
	{
		// Back on the terrain, put the bike on the ground
		m_bOffTerrain                    = TFALSE;
		m_pPhysics->GetBody().m_bEnabled = TTRUE;

		if ( fDistanceDown < fDistanceUp )
			vecPosition.y = vecPosition.y + fDistanceDown - 1.0f;
		else
			vecPosition.y = vecPosition.y - fDistanceUp - 1.0f;

		SetPosition( vecPosition );
		m_pPhysics->GetBody().m_vVelocity.Multiply( *GetDirection(), 8.0f );
	}

	if ( !m_bSeeking || m_bStopped )
		return;

	m_pPhysics->ResetThrowRider();

	if ( TVector4::DistanceXZ( m_vecSeekTarget, *GetPosition() ) < 2.0f )
	{
		m_bSeeking = TFALSE;
		m_pPhysics->SetThrottle( 0.0f );
		return;
	}

	// Steer towards the target
	TVector4 vecRight;
	vecRight.CrossProduct( *GetDirection(), TVector4::VEC_NEGY );

	TVector4 vecPlane;
	MakePlane( vecPlane, vecRight, *GetPosition() );

	TFLOAT fSide = vecPlane.y * m_vecSeekTarget.y + vecPlane.z * m_vecSeekTarget.z + vecPlane.x * m_vecSeekTarget.x - vecPlane.w;
	TFLOAT fSin  = fSide / TVector4::Distance( m_vecSeekTarget, *GetPosition() );
	TMath::Clip( fSin, -1.0f, 1.0f );
	TFLOAT fAngle = TMath::ASin( fSin );

	TFLOAT fSteering = fAngle;

	if ( TMath::Abs( fAngle ) > s_fMaxSteeringAngle )
	{
		fSteering = 1.0f;

		if ( fAngle <= 0.0f )
			fSteering = -1.0f;
	}

	if ( 0.0f > m_fSeekSpeed && GetSpeed() < 0.0f )
		fSteering *= -1.0f;

	m_pPhysics->SetSteering( fSteering );

	if ( 0.0f > m_fSeekSpeed )
	{
		// Reverse
		m_pPhysics->SetThrottle( 0.0f );
		m_pPhysics->SetBrake( 1.0f );
		m_fSeekSpeed = 1.0f;
		return;
	}

	m_pPhysics->SetBrake( 0.0f );

	if ( m_pPhysics->m_iNumWheelsOnGround != 2 )
	{
		m_pPhysics->SetThrottle( 0.0f );
		m_fSeekSpeed = 1.0f;
		return;
	}

	if ( m_fSeekSpeed > 1.0f && TMath::Abs( fAngle ) > 0.21816616f )
	{
		// Too fast for the turn
		m_pPhysics->SetThrottle( 0.0f );
		m_pPhysics->SetBrake( 0.2f );
		m_fSeekSpeed = 1.0f;
		return;
	}

	m_pPhysics->SetThrottle( m_fSeekSpeed );
	m_fSeekSpeed = 1.0f;
}

// $Barnyard: FUNCTION 0055abf0
void ABikeSteer::UpdateWater( TFLOAT a_fDeltaTime )
{
	if ( m_bInWater == TTRUE )
	{
		m_fInWaterTime += a_fDeltaTime;
		return;
	}

	// Cast a ray from the last position to the current one
	const TVector4& rcPosition = m_pPhysics->GetBody().GetPosition();

	TVector4 vecDirection;
	vecDirection.x = rcPosition.x - m_vecLastPosition.x;
	vecDirection.y = rcPosition.y - m_vecLastPosition.y;
	vecDirection.z = rcPosition.z - m_vecLastPosition.z;
	vecDirection.w = rcPosition.w;

	TFLOAT fDistance = vecDirection.NormaliseMag();

	if ( fDistance < 0.0001f )
		return;

	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideRay( m_vecLastPosition, vecDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE );

	TINT   iClosestHit      = -1;
	TFLOAT fClosestDistance = TMath::MAXFLOAT;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		if ( oHitList.aHits[ i ].fDistance < fClosestDistance )
		{
			iClosestHit      = i;
			fClosestDistance = oHitList.aHits[ i ].fDistance;
		}
	}

	if ( iClosestHit <= -1 )
		return;

	ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosestHit ];
	TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );

	// Rode into deep water
	if ( pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup != TCollisionCommon::TOSHICGROUP_DEEPWATER )
		return;

	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, fClosestDistance );

	TVector4 vecSplash;
	vecSplash.Add( m_vecLastPosition, vecOffset );

	m_bInWater     = TTRUE;
	m_fInWaterTime = 0.0f;

	ASoundManager::GetSingleton()->PlayCueAtPosition(
	    ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_FEMALE ) ? 678 : 637,
	    m_pPhysics->GetBody().GetPosition()
	);

	AParticleEffectManager* pEffectManager = AParticleEffectManager::GetSingleton();
	pEffectManager->PlayEffect( m_aiEffects[ EFFECT_WATERSPLASH2A ], vecSplash, TVector4::VEC_NEGY );
	pEffectManager->PlayEffect( m_aiEffects[ EFFECT_WATERSPLASH2B ], vecSplash, TVector4::VEC_NEGY );
	pEffectManager->PlayEffect( m_aiEffects[ EFFECT_WATERRIPPLE ], vecSplash, TVector4::VEC_NEGY );

	ASplashManager::GetSingleton()->Spawn( vecSplash, TVector4::VEC_NEGY, 1.0f, ASplashManager::SPLASHTYPE_WATER );

	if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) == &GetParentUnit() && !m_pPhysics->IsDriverless() )
	{
		AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
		AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
	}
}

// $Barnyard: FUNCTION 0055b360
TBOOL ABikeSteer::UpdatePlayerControl( TFLOAT a_fDeltaTime )
{
	m_bJump = TFALSE;
	m_pPhysics->ResetThrowRider();

	UpdateControlLock();

	if ( !m_bAIControlled )
		m_pPhysics->m_bUnkCD2 = m_bStoppingToGetOff != TTRUE;

	UpdateStamina( a_fDeltaTime );
	UpdateSkidding();

	if ( m_pPhysics->m_bJumping )
	{
		m_pPhysics->m_bJumping = TFALSE;
		m_bJump                = TTRUE;
	}

	UpdateWater( a_fDeltaTime );
	UpdateSpeed( a_fDeltaTime );
	UpdateRespawn();

	if ( m_iTrickState == TRICKSTATE_LOOP )
	{
		m_pPhysics->m_bForceThrowRider = TTRUE;
	}
	else
	{
		m_pPhysics->m_bForceThrowRider = TFALSE;
		m_aCues[ CUE_THROWN ]          = -1;
	}

	m_vecLastPosition = m_pPhysics->GetBody().GetPosition();
	return TTRUE;
}

// $Barnyard: FUNCTION 0055b450
void ABikeSteer::UpdateSounds( TFLOAT a_fDeltaTime )
{
	ASoundManager* pSoundManager = ASoundManager::GetSingleton();
	AUnit*         pUnit         = &GetParentUnit();

	// The engine
	pSoundManager->SetCuePositionToUnit( m_aCues[ CUE_ENGINE ], pUnit );

	if ( m_pPhysics->m_oGearBox.iGear == 1 || m_pPhysics->m_iNumWheelsOnGround <= 0 )
	{
		if ( m_aCues[ CUE_ENGINE ] != -1 )
		{
			pSoundManager->StopCue( m_aCues[ CUE_ENGINE ] );
			m_aCues[ CUE_ENGINE ] = -1;
		}
	}
	else
	{
		TFLOAT fFrequency = TMath::Abs( GetSpeed() ) / m_pPhysics->m_oParams.fMaxBoostSpeed * 1.3f + 0.2f;

		if ( m_aCues[ CUE_ENGINE ] == -1 || !pSoundManager->IsCueUsed( m_aCues[ CUE_ENGINE ] ) )
			m_aCues[ CUE_ENGINE ] = pSoundManager->PlayCueAtUnit( 247, &GetParentUnit() );

		pSoundManager->SetCueFrequency( m_aCues[ CUE_ENGINE ], fFrequency );

		TINT iSurface = m_pPhysics->m_aWheels[ 0 ].m_iSurfaceType;

		if ( iSurface != AVehicleManager::SURFACE_GRASS )
		{
			if ( iSurface <= AVehicleManager::SURFACE_DIRT || iSurface > AVehicleManager::SURFACE_TIN )
				pSoundManager->SetCueVolume( m_aCues[ CUE_ENGINE ], 1.0f );
			else
				pSoundManager->SetCueVolume( m_aCues[ CUE_ENGINE ], 0.5f );
		}
		else
		{
			pSoundManager->SetCueVolume( m_aCues[ CUE_ENGINE ], 0.0f );
		}
	}

	// The skid
	pSoundManager->SetCuePositionToUnit( m_aCues[ CUE_SKID ], &GetParentUnit() );

	if ( m_bSkidding && m_pPhysics->m_aWheels[ 1 ].m_bOnGround == TTRUE )
	{
		if ( m_aCues[ CUE_SKID ] == -1 || !pSoundManager->IsCueUsed( m_aCues[ CUE_SKID ] ) )
		{
			m_aCues[ CUE_SKID ] = pSoundManager->PlayCueAtUnit( 267, &GetParentUnit() );
		}
		else
		{
			TFLOAT fVolume    = 1.0f;
			TFLOAT fSlipAngle = TMath::Abs( m_pPhysics->m_aWheels[ 1 ].m_fSlipAngle );

			if ( fSlipAngle < 0.08726646f )
				fVolume = fSlipAngle * 11.459156f;

			pSoundManager->SetCueVolume( m_aCues[ CUE_SKID ], fVolume );
			pSoundManager->SetCueFrequency( m_aCues[ CUE_SKID ], 0.8f );
		}
	}
	else if ( m_aCues[ CUE_SKID ] != -1 )
	{
		pSoundManager->StopCue( m_aCues[ CUE_SKID ] );
		m_aCues[ CUE_SKID ] = -1;
	}

	// Landing hard
	APhysicsVehicleBottomOut oBottomOut  = m_pPhysics->m_oBottomOut;
	TFLOAT                   fMaxImpulse = 25.0f;

	if ( oBottomOut.iCount > 0 )
	{
		if ( m_pPhysics->IsStoppie() || m_pPhysics->IsWheelie() )
			fMaxImpulse = 50.0f;

		if ( oBottomOut.vImpulse.Magnitude() > fMaxImpulse )
			PlayLandSound();
	}

	// Crashing into things
	TINT iNumContacts = m_pPhysics->m_iNumContacts;

	if ( iNumContacts > 0 )
	{
		TVector4 vecStaticImpulse = TVector4::VEC_ZERO;
		TVector4 vecBodyImpulse   = TVector4::VEC_ZERO;

		for ( TINT i = 0; i < iNumContacts; i++ )
		{
			APhysicsVehicleContact* pContact = m_pPhysics->GetContact( i );

			TVector4 vecImpulse;
			vecImpulse.Multiply( pContact->vNormal, pContact->fImpulse );

			if ( pContact->pOtherBody )
			{
				vecBodyImpulse.x += vecImpulse.x;
				vecBodyImpulse.y += vecImpulse.y;
				vecBodyImpulse.z += vecImpulse.z;
			}
			else
			{
				vecStaticImpulse.x += vecImpulse.x;
				vecStaticImpulse.y += vecImpulse.y;
				vecStaticImpulse.z += vecImpulse.z;
			}
		}

		if ( vecStaticImpulse.Magnitude() > 400.0f )
			PlayCrashSound();

		if ( vecBodyImpulse.Magnitude() > 10.0f )
			PlayCrashSound();
	}

	// The rider is thrown off
	if ( m_iTrickState == TRICKSTATE_LOOP && m_aCues[ CUE_THROWN ] == -1 )
	{
		if ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_FEMALE )
			m_aCues[ CUE_THROWN ] = pSoundManager->PlayCueAtUnit( 641, &GetParentUnit() );
		else
			m_aCues[ CUE_THROWN ] = pSoundManager->PlayCueAtUnit( 607, &GetParentUnit() );
	}
}

// $Barnyard: FUNCTION 0055b880
void ABikeSteer::UpdateEffects( TFLOAT a_fDeltaTime )
{
	if ( m_pPhysics->IsDriverless() )
		return;

	APhysicsBike* pPhysics = m_pPhysics;

	TVector4 vecEffectDirection = TVector4::VEC_NEGY;
	TVector4 vecLift;
	vecLift.Multiply( vecEffectDirection, 0.2f );

	TVector4 vecMovement;
	vecMovement.Multiply( m_pPhysics->GetBody().GetVelocity(), a_fDeltaTime );

	TFLOAT fSpeedFactor = m_pPhysics->GetBody().GetVelocity().Magnitude() * 0.089999996f;
	TMath::Clip( fSpeedFactor, 0.0f, 1.0f );
	TFLOAT fDustInterval = TMath::LERP( 0.05f, 0.05f, fSpeedFactor );

	for ( TINT i = 0; i < TARRAYSIZE( m_afEffectTimers ); i++ )
		m_afEffectTimers[ i ] = TMath::Max( m_afEffectTimers[ i ] - a_fDeltaTime, 0.0f );

	if ( m_pPhysics->GetBody().GetVelocity().Magnitude() > 0.55555558f )
	{
		// Mud, dust and grass under the wheels
		for ( TINT i = 0; i < 2; i++ )
		{
			APhysicsWheel& rWheel = pPhysics->m_aWheels[ i ];

			if ( rWheel.m_bOnGround != TTRUE )
				continue;

			TVector4 vecTemp;
			vecTemp.Add( rWheel.m_vPosition, vecLift );
			TVector4 vecMudPosition;
			vecMudPosition.Substract( vecTemp, vecMovement );

			vecTemp.Add( rWheel.m_vPosition, vecLift );
			TVector4 vecDustPosition;
			vecDustPosition.Substract( vecTemp, vecMovement );

			TINT iSurface = rWheel.m_iSurfaceType;

			TFLOAT& rfMudTimer   = m_afEffectTimers[ 0 + i ];
			TFLOAT& rfDustTimer  = m_afEffectTimers[ 2 + i ];
			TFLOAT& rfGrassTimer = m_afEffectTimers[ 4 + i ];

			if ( iSurface == AVehicleManager::SURFACE_MUD && rfMudTimer <= 0.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_MUDRIPPLES ], vecMudPosition, vecEffectDirection );
				rfMudTimer = 0.1f;
			}
			else if ( ( iSurface == AVehicleManager::SURFACE_DIRT && rfDustTimer <= 0.0f ) || ( iSurface == AVehicleManager::SURFACE_HAY && rfDustTimer <= 0.0f ) )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_DUSTPUFFSMALL ], vecDustPosition, vecEffectDirection );
				rfDustTimer = fDustInterval;
			}
			else if ( iSurface == AVehicleManager::SURFACE_GRASS && rfGrassTimer <= 0.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_GRASSWALK ], vecDustPosition, vecEffectDirection );
				rfGrassTimer = 0.1f;
			}

			// Rode into the mud
			if ( rWheel.m_iSurfaceType == AVehicleManager::SURFACE_MUD && m_aiGroundMaterials[ i ] != AVehicleManager::SURFACE_MUD )
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_MUDSPLASH ], vecMudPosition, vecEffectDirection );
		}
	}

	for ( TINT i = 0; i < 2; i++ )
		m_aiGroundMaterials[ i ] = pPhysics->m_aWheels[ i ].m_bOnGround ? pPhysics->m_aWheels[ i ].m_iSurfaceType : AVehicleManager::SURFACE_NONE;

	// The tyre tracks
	AVehicleSkidManager* pSkidManager = AVehicleSkidManager::GetSingleton();

	if ( pSkidManager )
	{
		for ( TINT i = 0; i < NUM_TYRE_TRACKS; i++ )
		{
			APhysicsWheel& rWheel      = m_pPhysics->m_aWheels[ i ];
			TVector4&      rLastPos    = m_aTyreTrackPositions[ i ];
			TFLOAT&        rfOffset    = m_afTyreTrackOffsets[ i ];
			TFLOAT         fWidth      = m_afTyreTrackWidths[ i ];
			TBOOL          bLeaveTrack = TFALSE;
			TFLOAT         fRed        = 0.0f;
			TFLOAT         fGreen      = 0.0f;
			TFLOAT         fBlue       = 0.0f;
			TFLOAT         fMaxAlpha   = 1.0f;

			if ( rWheel.m_bOnGround )
			{
				switch ( rWheel.m_iSurfaceType )
				{
					case AVehicleManager::SURFACE_DIRT:
					case AVehicleManager::SURFACE_SAND:
						fRed        = 0.71f;
						fGreen      = 0.64f;
						fBlue       = 0.39f;
						fMaxAlpha   = 0.5f;
						bLeaveTrack = TTRUE;
						break;
					case AVehicleManager::SURFACE_WATER:
					case AVehicleManager::SURFACE_MUD:
						break;
					default:
						// Dark tracks only while power sliding
						if ( i == 1 && m_pPhysics->IsSliding() == TTRUE )
							bLeaveTrack = TTRUE;
						break;
				}
			}

			if ( !bLeaveTrack )
			{
				rLastPos = TVector4::VEC_ZERO;
				rfOffset = 0.0f;
				continue;
			}

			const TVector4& rcContact = rWheel.m_vContactPosition;

			TVector4 vecDelta;
			vecDelta.Substract( rLastPos, rcContact );

			if ( vecDelta.MagnitudeSq() <= 0.090000004f )
				continue;

			if ( !rLastPos.IsEqual( TVector4::VEC_ZERO ) )
			{
				AVehicleSkidManager::TyreTrack* pTrack = pSkidManager->GetFreeTrack();

				if ( pTrack )
				{
					pTrack->vPosition = rcContact;
					pTrack->vNormal   = rWheel.m_vGroundNormal;

					TVector4 vecDirection;
					vecDirection.Substract( rLastPos, rcContact );
					TFLOAT fLength = vecDirection.NormaliseMag();

					TFLOAT fAlpha = rWheel.m_vContactVelocity.Magnitude() * 0.2f;
					TMath::Clip( fAlpha, 0.0f, fMaxAlpha );

					pTrack->vDirection = vecDirection;
					pTrack->fWidth     = fWidth;
					pTrack->fLength    = fLength;
					pTrack->uiColour   = PackColour( fRed, fGreen, fBlue, fAlpha );
					pTrack->fUVOffset  = rfOffset;

					rfOffset += fLength / fWidth;
					rfOffset -= TMath::FloorToInt( rfOffset );
				}
			}

			rLastPos = rcContact;
		}
	}

	// The lights at night
	if ( !AGameTimeManager::GetSingleton()->IsItNight() || m_bInWater || ASSeqState::IsPlaying() )
	{
		if ( m_pTailLight )
		{
			m_pTailLight->GetInstance()->Kill();
			m_pTailLight = TNULL;
		}

		if ( m_pFrontLight )
		{
			m_pFrontLight->GetInstance()->Kill();
			m_pFrontLight = TNULL;
		}
	}
	else
	{
		if ( !m_pTailLight )
		{
			AAnimatableObject* pBikeObject = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject(), TPS8( body_loc ), 0 );
			TINT               iBone       = pBikeObject->GetModelInstance()->GetSkeletonInstance()->GetBoneID( "breaklight_loc" );

			if ( iBone != -1 )
			{
				m_pTailLight                                = AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_BIKE_TAILLIGHT ], TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
				m_pTailLight->GetInstance()->GetTransform() = TMatrix44::IDENTITY;
				m_pTailLight->AttachToBone( pBikeObject->GetModelInstance(), iBone, TTRUE, TFALSE );
			}
		}

		if ( !m_pFrontLight )
		{
			AAnimatableObject* pBikeObject = AAnimatableObjectManager::GetSingleton()->GetAttachedObjectByBoneName( TSTATICCAST( AAnimatedUnit, &GetParentUnit() )->GetAnimatableObject(), TPS8( body_loc ), 0 );
			TINT               iBone       = pBikeObject->GetModelInstance()->GetSkeletonInstance()->GetBoneID( "headlight_loc" );

			if ( iBone != -1 )
			{
				m_pFrontLight                                = AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_BIKE_FRONTLIGHT ], TVector4::VEC_ZERO, TVector4::VEC_NEGZ );
				m_pFrontLight->GetInstance()->GetTransform() = TMatrix44::IDENTITY;
				m_pFrontLight->AttachToBone( pBikeObject->GetModelInstance(), iBone, TTRUE, TFALSE );
			}
		}
	}
}

// $Barnyard: FUNCTION 0055c380
void ABikeSteer::UpdateCollisions( TFLOAT a_fDeltaTime )
{
	for ( TINT i = 0; i < m_pPhysics->m_iNumContacts; i++ )
	{
		APhysicsVehicleContact* pContact = m_pPhysics->GetContact( i );
		ACollisionObject*       pObject  = pContact->pCollisionObject;

		if ( !pObject )
			continue;

		if ( pObject->IsA( &TGetClass( ACollisionObjectUnit ) ) )
		{
			if ( pContact->fImpulse <= 10.0f )
				continue;

			// Hit a character
			AUnit* pUnit = TSTATICCAST( AUnit, pObject->GetOwnerUnit() );

			if ( !pUnit || !pUnit->IsA( &TGetClass( ACharacterUnit ) ) )
				continue;

			if ( AGameLoader::IsMainGameLoaded() )
			{
				const TCHAR* szName = pUnit->GetType()->GetName().GetString();

				if ( szName && pUnit->GetSteer() )
					AVOStream::GetSingleton()->PlaySet( szName, "angry", *pUnit->GetSteer()->GetPosition() );
			}

			if ( pUnit->IsA( &TGetClass( AAnimatedUnit ) ) && AGameLoader::IsMainGameLoaded() && pUnit->CanInteract() )
			{
				AToshiAnimationInterface& rAnimInterface = TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->GetToshiAnimInterface();

				TBOOL bImpact = rAnimInterface.GetAnimationSet()->GetNamedAnimation( TPS8( bikeimpact ) ) != TNULL;
				TBOOL bYuk    = rAnimInterface.GetAnimationSet()->GetNamedAnimation( TPS8( yuk ) ) != TNULL;

				if ( bImpact || bYuk )
				{
					if ( bImpact )
						AInteractionManager::GetSingleton()->AssignInteractionForced( pUnit, TPS8( trigger_onbikehit_impact ) );
					else
						AInteractionManager::GetSingleton()->AssignInteractionForced( pUnit, TPS8( trigger_onbikehit ) );
				}
			}
		}
		else if ( pObject->GetType() & ACollisionObject::TYPE_BUSH )
		{
			// Rode through a bush
			if ( pContact->fImpulse > 15.0f )
			{
				AParticleEffectManager::GetSingleton()->PlayEffect( m_aiEffects[ EFFECT_BUSHHIT ], pContact->vPosition, pContact->vNormal );

				if ( m_aCues[ CUE_BUSH ] == -1 || !ASoundManager::GetSingleton()->IsCueUsed( m_aCues[ CUE_BUSH ] ) )
					m_aCues[ CUE_BUSH ] = ASoundManager::GetSingleton()->PlayCueAtPosition( 272, m_pPhysics->GetBody().GetPosition() );
			}
		}
		else if ( !( pObject->GetType() & ACollisionObject::TYPE_TRUNK ) )
		{
			// NOTE: retail checks the class of the object here but doesn't do anything with the result
			pObject->IsA( &TGetClass( ACollisionObjectModel ) );
		}
	}
}
