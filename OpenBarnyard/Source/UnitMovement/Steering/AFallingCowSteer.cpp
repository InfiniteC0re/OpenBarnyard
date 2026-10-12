#include "pch.h"
#include "AFallingCowSteer.h"
#include "ASteerTraits.h"
#include "FX/AParticleEffectManager.h"
#include "FX/ASplashManager.h"
#include "Units/AUnit.h"
#include "Units/AVisibleUnit.h"
#include "Player/APlayerProfileManager.h"
#include "Sound/ASoundManager.h"
#include "Input/AInputHandler.h"
#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionModelInstance.h"
#include "Physics/ACollisionObject.h"
#include "Physics/AVehicleManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0055f990, CreateObject 0055ff40, CreateObjectInPlace 0055ffa0
TDEFINE_CLASS( AFallingCowSteer );

// The cow lies on the ground once it moves less than this in a step (dynamic initialiser 0070a360)
static TFLOAT s_fStopDistanceSq = 0.019290125f; // 0x00788d5c

// $Barnyard: FUNCTION 0055f9a0
AFallingCowSteer::AFallingCowSteer()
{
	m_vDirection     = TVector4::VEC_POSX;
	m_bFinished      = TFALSE;
	m_bLanded        = TFALSE;
	m_bInWater       = TFALSE;
	m_fInWaterTime   = 0.0f;
	m_fTime          = 0.0f;
	m_fNoContactTime = 0.0f;
	m_vOffset        = TVector4::VEC_ZERO;
	m_iLandCue       = -1;

	m_aiEffects[ EFFECT_WATERSPLASH2A ] = AParticleEffectManager::FindEffect( "watersplash2a" );
	m_aiEffects[ EFFECT_WATERSPLASH2B ] = AParticleEffectManager::FindEffect( "watersplash2b" );
	m_aiEffects[ EFFECT_WATERRIPPLE ]   = AParticleEffectManager::FindEffect( "waterripple" );
}

// $Barnyard: FUNCTION 0055ffe0
// $Barnyard: FUNCTION 0055fbc0
AFallingCowSteer::~AFallingCowSteer()
{
	m_oBody.Disable();
}

// $Barnyard: FUNCTION 0055fa80
void AFallingCowSteer::SetupBody( const TVector4& a_rcPosition, const TQuaternion& a_rcOrientation )
{
	m_oBody.SetSphereInertia( 1000.0f, 0.35f );
	m_oBody.m_vForce    = TVector4::VEC_ZERO;
	m_oBody.m_vTorque   = TVector4::VEC_ZERO;
	m_oBody.m_vVelocity = TVector4::VEC_ZERO;
	m_oBody.SetAngularVelocity( TVector4::VEC_ZERO );
	m_oBody.SetPosition( TVector4::VEC_ZERO, TTRUE );
	m_oBody.SetOrientation( TQuaternion::IDENTITY, TTRUE );

	m_oBody.m_fAngularDamping = 0.5f;
	m_oBody.m_fLinearDamping  = 0.4f;
	m_oBody.m_fContactScale   = 0.6f;

	m_oBody.SetPosition( a_rcPosition, TTRUE );
	m_oBody.SetOrientation( a_rcOrientation, TTRUE );

	ARigidBodyMaterial oMaterial( 1.2f, 0.1f, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, ACollisionObject::TYPE_GATE | ACollisionObject::TYPE_MOVABLE | ACollisionObject::TYPE_BUSH | ACollisionObject::TYPE_INSTANCE | ACollisionObject::TYPE_TRUNK | ACollisionObject::TYPE_MESH | ACollisionObject::TYPE_UNIT | ACollisionObject::TYPE_SIMPLE | ACollisionObject::TYPE_TERRAIN, 0x30000, -1 );
	m_oBody.m_oMaterial = oMaterial;
}

// $Barnyard: FUNCTION 0055fc20
void AFallingCowSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	const TVector4& rcPosition = m_oBody.GetPosition();

	a_rOutPosition.x = rcPosition.x - m_vOffset.x;
	a_rOutPosition.y = rcPosition.y - m_vOffset.y;
	a_rOutPosition.z = rcPosition.z - m_vOffset.z;
}

// $Barnyard: FUNCTION 0055fc80
void AFallingCowSteer::CopyPosition( TVector4& a_rOutPosition )
{
	const TVector4& rcPosition = m_oBody.GetPosition();

	a_rOutPosition.x = rcPosition.x - m_vOffset.x;
	a_rOutPosition.y = rcPosition.y - m_vOffset.y;
	a_rOutPosition.z = rcPosition.z - m_vOffset.z;
	a_rOutPosition.w = rcPosition.w;
}

// $Barnyard: FUNCTION 0055fce0
const TVector4* AFallingCowSteer::GetPosition()
{
	static TVector4 s_vecPosition; // 0x00788d60

	const TVector4& rcPosition = m_oBody.GetPosition();

	s_vecPosition.x = rcPosition.x - m_vOffset.x;
	s_vecPosition.y = rcPosition.y - m_vOffset.y;
	s_vecPosition.z = rcPosition.z - m_vOffset.z;
	s_vecPosition.w = rcPosition.w;

	return &s_vecPosition;
}

// $Barnyard: FUNCTION 0055fd70
TFLOAT AFallingCowSteer::GetSpeed()
{
	return m_oBody.GetVelocity().Magnitude();
}

// $Barnyard: FUNCTION 0055fd80
TBOOL AFallingCowSteer::SetPosition( const TVector4& a_rcPosition )
{
	TVector4 vecLift;
	vecLift.Multiply( TVector4::VEC_NEGY, 0.5f );

	TVector4 vecPosition;
	vecPosition.x = vecLift.x + a_rcPosition.x + m_vOffset.x;
	vecPosition.y = vecLift.y + a_rcPosition.y + m_vOffset.y;
	vecPosition.z = vecLift.z + a_rcPosition.z + m_vOffset.z;
	vecPosition.w = a_rcPosition.w;

	m_oBody.SetPosition( vecPosition, TTRUE );
	return TTRUE;
}

// $Barnyard: FUNCTION 0055fe70
void AFallingCowSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vDirection = a_rcDirection;

	TMatrix44 mRotation = TMatrix44::IDENTITY;
	mRotation.LookAtDirection( a_rcDirection, TVector4::VEC_NEGY );

	TQuaternion qRotation;
	TMatrix44::MatToQuat( qRotation, mRotation );
	m_oBody.SetOrientation( qRotation, TTRUE );
}

// $Barnyard: FUNCTION 0055fee0
void AFallingCowSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_bFinished         = TFALSE;
	m_oBody.m_vVelocity = a_rcVelocity;
}

// $Barnyard: FUNCTION 0055ff10
TBOOL AFallingCowSteer::HasLanded() const
{
	return m_bLanded;
}

// $Barnyard: FUNCTION 0055ff20
TBOOL AFallingCowSteer::IsSunk() const
{
	return m_fInWaterTime > 1.0f;
}

// $Barnyard: FUNCTION 00560160
TBOOL AFallingCowSteer::IsFinished() const
{
	return m_bFinished;
}

// $Barnyard: FUNCTION 00560000
TBOOL AFallingCowSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vDirection     = a_rcDirection;
	m_fTime          = 0.0f;
	m_fNoContactTime = 0.0f;
	m_bLanded        = TFALSE;
	m_bFinished      = TFALSE;
	m_bInWater       = TFALSE;
	m_vOffset.Multiply( TVector4::VEC_NEGY, m_pTraits->m_fRadius );
	m_iLandCue = -1;

	SetupBody( a_rcPosition, TQuaternion::IDENTITY );
	return TTRUE;
}

// $Barnyard: FUNCTION 005600b0
TBOOL AFallingCowSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	m_vDirection     = *a_pOther->GetDirection();
	m_fTime          = 0.0f;
	m_fNoContactTime = 0.0f;
	m_bLanded        = TFALSE;
	m_bFinished      = TFALSE;
	m_bInWater       = TFALSE;
	m_vOffset.Multiply( TVector4::VEC_NEGY, m_pTraits->m_fRadius );
	m_iLandCue = -1;

	SetupBody( *a_pOther->GetPosition(), TQuaternion::IDENTITY );
	return TTRUE;
}

// $Barnyard: FUNCTION 00560170
void AFallingCowSteer::UpdateWater( const TVector4& a_rcPosition, TFLOAT a_fDeltaTime )
{
	if ( m_bInWater == TTRUE )
	{
		m_fInWaterTime += a_fDeltaTime;
		return;
	}

	const TVector4& rcBodyPosition = m_oBody.GetPosition();

	TVector4 vecDirection;
	vecDirection.x = rcBodyPosition.x - a_rcPosition.x;
	vecDirection.y = rcBodyPosition.y - a_rcPosition.y;
	vecDirection.z = rcBodyPosition.z - a_rcPosition.z;
	vecDirection.w = rcBodyPosition.w;

	TFLOAT fDistance = vecDirection.NormaliseMag();

	if ( fDistance < 0.0001f )
		return;

	TVector4                  vecOrigin = a_rcPosition;
	ACollisionModelRayHitList oHitList;
	ATerrainInterface::GetSingleton()->CollideRay( vecOrigin, vecDirection, fDistance, &oHitList, TCollisionCommon::TOSHICGROUPFLAG_NONE, TFALSE );

	TFLOAT fClosestDistance = TMath::MAXFLOAT;

	if ( oHitList.iNumHits <= 0 )
		return;

	// The closest deep water
	TINT iClosestHit = -1;

	for ( TINT i = 0; i < oHitList.iNumHits; i++ )
	{
		ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ i ];
		TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );
		TINT                            iGroup    = pCollData->GetGroupForTriangleImp( rHit.uiFaceId );

		if ( rHit.fDistance < fClosestDistance && pCollData->GetCollGroup( iGroup ).eCollGroup == TCollisionCommon::TOSHICGROUP_DEEPWATER )
		{
			fClosestDistance = rHit.fDistance;
			iClosestHit      = i;
		}
	}

	if ( iClosestHit <= -1 )
		return;

	ACollisionModelRayHitList::Hit& rHit      = oHitList.aHits[ iClosestHit ];
	TModelCollisionData*            pCollData = rHit.pInstance->GetCollisionData( rHit.iModel );

	if ( pCollData->GetCollGroup( pCollData->GetGroupForTriangleImp( rHit.uiFaceId ) ).eCollGroup != TCollisionCommon::TOSHICGROUP_DEEPWATER )
		return;

	// Fell into the water
	TVector4 vecOffset;
	vecOffset.Multiply( vecDirection, fClosestDistance );

	TVector4 vecSplash;
	vecSplash.Add( vecOrigin, vecOffset );

	m_bInWater     = TTRUE;
	m_fInWaterTime = 0.0f;

	ASoundManager::GetSingleton()->PlayCueAtPosition( ( APlayerProfileManager::GetSingleton()->GetMainGender() == APlayerProfile::GENDER_FEMALE ) ? 678 : 637, vecSplash );

	AParticleEffectManager* pEffectManager = AParticleEffectManager::GetSingleton();
	pEffectManager->PlayEffect( m_aiEffects[ EFFECT_WATERSPLASH2A ], vecSplash, TVector4::VEC_NEGY );
	pEffectManager->PlayEffect( m_aiEffects[ EFFECT_WATERSPLASH2B ], vecSplash, TVector4::VEC_NEGY );
	pEffectManager->PlayEffect( m_aiEffects[ EFFECT_WATERRIPPLE ], vecSplash, TVector4::VEC_NEGY );

	ASplashManager::GetSingleton()->Spawn( vecSplash, TVector4::VEC_NEGY, 1.0f, ASplashManager::SPLASHTYPE_WATER );

	AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
	AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );
}

// $Barnyard: FUNCTION 00560450
TBOOL AFallingCowSteer::Update( TFLOAT a_fDeltaTime )
{
	m_fTime += a_fDeltaTime;

	if ( m_bFinished )
		return TTRUE;

	UpdateWater( m_oBody.m_oPrevTransform.GetTranslation(), 0.02f );

	// The physics runs at 50 steps per second
	TINT iNumSteps = TMath::Max( TMath::FloorToInt( a_fDeltaTime * 50.0f ), 1 );

	for ( TINT i = 0; i < iNumSteps; i++ )
	{
		TVector4 vecStartPosition = *GetPosition();

		// The gravity
		TFLOAT   fMass = m_oBody.m_fMass;
		TVector4 vecGravity;
		vecGravity.Multiply( TVector4::VEC_POSY, 19.6f );
		TVector4 vecForce;
		vecForce.Multiply( vecGravity, fMass );

		m_oBody.m_vForce.x += vecForce.x;
		m_oBody.m_vForce.y += vecForce.y;
		m_oBody.m_vForce.z += vecForce.z;

		m_oBody.Integrate( 0.02f );

		TVector4 vecSphere( TVector4::VEC_ZERO.x, TVector4::VEC_ZERO.y, TVector4::VEC_ZERO.z, m_pTraits->m_fRadius );

		for ( TINT k = 0; k < m_oBody.m_iNumContacts; k++ )
			m_oBody.m_apContacts[ k ] = TNULL;

		m_oBody.m_iNumContacts = 0;

		ACollisionObject* pCollisionObject = TSTATICCAST( AVisibleUnit, &GetParentUnit() )->GetCollisionObject();
		pCollisionObject->SetType( pCollisionObject->GetType() | ACollisionObject::TYPE_IGNORED );

		AVehicleManager::ContactSummary oSummary;
		TBOOL                           bHit = AVehicleManager::CollideSphereAndResolve( &m_oBody, vecSphere, 0.02f, &oSummary );

		pCollisionObject->SetType( pCollisionObject->GetType() & ~ACollisionObject::TYPE_IGNORED );

		ASoundManager* pSoundManager = ASoundManager::GetSingleton();
		ASoundId       eLandSound    = ( APlayerProfileManager::GetSingleton()->GetMainGender() != APlayerProfile::GENDER_FEMALE ) ? 604 : 642;

		if ( bHit )
		{
			if ( m_iLandCue == -1 )
				m_iLandCue = pSoundManager->PlayCueAtUnit( eLandSound, &GetParentUnit() );

			if ( !m_bLanded && m_oBody.m_iNumContacts > 0 )
			{
				TVector4 vecNormal = m_oBody.m_apContacts[ 0 ]->m_vNormal;

				// Landed on something flat enough
				if ( TVector4::DotProduct3( TVector4::VEC_NEGY, vecNormal ) > 0.5f )
				{
					m_bLanded = TTRUE;

					AInputHandler::RumbleParams oRumble = { { 0.8f, 0.0f, 0.0f, 0.4f, 0.0f, 1.0f, 1.0f }, 1 };
					AInputHandler::GetSingleton()->SetRumble( AInputHandler::GetSingleton()->GetMainController(), &oRumble );

					if ( m_iLandCue == -1 || !pSoundManager->IsCueUsed( m_iLandCue ) )
						m_iLandCue = pSoundManager->PlayCueAtUnit( eLandSound, &GetParentUnit() );
				}
			}

			m_fNoContactTime = 0.0f;
		}
		else
		{
			m_fNoContactTime += 0.02f;
		}

		UpdateWater( vecStartPosition, 0.02f );

		if ( m_fNoContactTime > 4.0f )
		{
			// Fell for too long, treat it as sunk
			m_bInWater       = TTRUE;
			m_fInWaterTime   = 1.01f;
			m_fNoContactTime = 0.0f;
		}

		const TVector4* pPosition = GetPosition();

		TVector4 vecMovement( vecStartPosition.x - pPosition->x, vecStartPosition.y - pPosition->y, vecStartPosition.z - pPosition->z, vecStartPosition.w );

		if ( vecMovement.MagnitudeSq() < s_fStopDistanceSq && m_bLanded && !m_bInWater )
		{
			m_bFinished = TTRUE;
			break;
		}
	}

	return TTRUE;
}
