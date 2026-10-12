#include "pch.h"
#include "AThrowItemSteer.h"
#include "Units/AVisibleUnit.h"
#include "Units/AUnitType.h"
#include "Units/AObjectTraits.h"
#include "Animation/AModel.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/barnyard.h"

#include <Math/TMathInline.h>
#include <Toshi/T2String8.h>

#include "Terrain/ATerrainInterface.h"
#include "Physics/ACollisionCommon.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005724b0, CreateObject 00572820, CreateObjectInPlace 00572880
TDEFINE_CLASS( AThrowItemSteer, 3, 0 );

static constexpr TFLOAT GRAVITY      = 9.8f;
static constexpr TFLOAT HALF_GRAVITY = 4.9f;

// $Barnyard: FUNCTION 005724c0
AThrowItemSteer::AThrowItemSteer()
    : m_vecPosition( TVector4::VEC_ZERO )
    , m_vecVelocity( TVector4::VEC_ZERO )
    , m_vecDirection( TVector4::VEC_POSX )
    , m_fSpinSpeed( 0.0f )
    , m_fSpinAngle( 0.0f )
    , m_fRestitution( 0.5f )
    , m_fTangentialRestitution( 0.5f )
    , m_bStopped( TTRUE )
    , m_bBounced( TFALSE )
    , m_bUpdateModelTransform( TFALSE )
    , m_fSpeed( 0.0f )
    , m_bFreezeSpeed( TFALSE )
    , m_iNumStillUpdates( 0 )
    , m_eIgnoreGroups( TCollisionCommon::TOSHICGROUPFLAG_NONE )
{
}

// $Barnyard: FUNCTION 005728c0
// $Barnyard: FUNCTION 00572590
AThrowItemSteer::~AThrowItemSteer()
{
}

// $Barnyard: FUNCTION 005729e0
TBOOL AThrowItemSteer::CreateFrom( ASteer* a_pOther, TINT a_iLOD )
{
	ASteer::CreateFrom( a_pOther, a_iLOD );

	a_pOther->CopyPosition( m_vecPosition );
	a_pOther->CopyDirection( m_vecDirection );
	a_pOther->CopyVelocity( m_vecVelocity );

	m_bUpdateModelTransform = TFALSE;
	m_eIgnoreGroups         = TCollisionCommon::TOSHICGROUPFLAG_NONE;

	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_oOwner->GetUnit()->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits )
	{
		m_fRestitution           = pTraits->m_fRestitution;
		m_fTangentialRestitution = pTraits->m_fTangentialRestitution;
	}

	// The model spins around its center
	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, m_oOwner->GetUnit() );

	if ( pUnit )
	{
		AModelInstance* pModelInstance = pUnit->GetModelInstance();

		if ( pModelInstance )
		{
			TINT iBone = pModelInstance->GetSkeletonInstance()->GetBoneID( "center" );

			if ( iBone < 0 )
			{
				m_matCenterInv.Identity();
			}
			else
			{
				pModelInstance->GetSkeletonInstance()->GetBoneTransformCurrent( iBone, m_matCenterInv );
				m_matCenterInv.Invert( m_matCenterInv );
			}
		}
	}

	m_fSpeed       = 0.0f;
	m_bFreezeSpeed = TFALSE;
	m_fSpinSpeed   = 1.0f;

	return TTRUE;
}

// $Barnyard: FUNCTION 005728e0
TBOOL AThrowItemSteer::Create( AUnit* a_pOwner, const TVector4& a_rcPosition, const TVector4& a_rcDirection, TINT a_iLOD )
{
	ASteer::Create( a_pOwner, a_rcPosition, a_rcDirection, a_iLOD );

	m_vecPosition           = a_rcPosition;
	m_vecDirection          = a_rcDirection;
	m_fSpinSpeed            = 1.0f;
	m_bUpdateModelTransform = TFALSE;
	m_eIgnoreGroups         = TCollisionCommon::TOSHICGROUPFLAG_NONE;

	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, m_oOwner->GetUnit()->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits )
	{
		m_fRestitution           = pTraits->m_fRestitution;
		m_fTangentialRestitution = pTraits->m_fTangentialRestitution;
	}

	// The model spins around its center
	AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, m_oOwner->GetUnit() );

	if ( pUnit )
	{
		AModelInstance* pModelInstance = pUnit->GetModelInstance();

		if ( pModelInstance )
		{
			TINT iBone = pModelInstance->GetSkeletonInstance()->GetBoneID( "center" );

			if ( iBone >= 0 )
			{
				pModelInstance->GetSkeletonInstance()->GetBoneTransformCurrent( iBone, m_matCenterInv );
				m_matCenterInv.Invert( m_matCenterInv );
			}
			else
			{
				m_matCenterInv.Identity();
			}
		}
	}

	m_fSpeed       = 0.0f;
	m_bFreezeSpeed = TFALSE;

	return TTRUE;
}

// $Barnyard: FUNCTION 00572b20
TBOOL AThrowItemSteer::Update( TFLOAT a_fDeltaTime )
{
	if ( m_bStopped )
		return TTRUE;

	TFLOAT fTimeLeft = a_fDeltaTime;

	// Where the unit gets this frame
	TVector4 vecMove;
	vecMove.Multiply( m_vecVelocity, a_fDeltaTime );

	TFLOAT fFallDistance = a_fDeltaTime * a_fDeltaTime * HALF_GRAVITY;
	vecMove.y += fFallDistance;

	// NOTE: the result isn't used
	m_vecVelocity.Magnitude();

	ACollisionModelRayHitList oHitList;

	TFLOAT   fMoveLength    = vecMove.NormaliseMag();
	TFLOAT   fDistance      = fMoveLength;
	TVector4 vecOldPosition = m_vecPosition;

	m_bBounced       = TFALSE;
	TINT iNumBounces = 0;

	if ( !m_bStopped )
	{
		while ( iNumBounces < 4 && fMoveLength > 0.0f && ATerrainInterface::GetSingleton()->CollideRay( m_vecPosition, vecMove, fDistance, &oHitList, m_eIgnoreGroups, TFALSE ) )
		{
			TVector4 vecNormal = oHitList.aHits[ 0 ].GetNormal();
			oHitList.Clear();

			// Bounce off the hit triangle
			TFLOAT   fDot           = TVector4::DotProduct3( m_vecVelocity, vecNormal );
			TVector4 vecNewVelocity = m_vecVelocity;

			TVector4 vecNormalVelocity;
			vecNormalVelocity.Multiply( vecNormal, fDot );
			vecNewVelocity.x -= vecNormalVelocity.x;
			vecNewVelocity.y -= vecNormalVelocity.y;
			vecNewVelocity.z -= vecNormalVelocity.z;
			vecNewVelocity.Multiply( m_fTangentialRestitution );

			TVector4 vecBounce;
			vecBounce.Multiply( vecNormal, fDot * m_fRestitution );
			vecNewVelocity.x -= vecBounce.x;
			vecNewVelocity.y -= vecBounce.y;
			vecNewVelocity.z -= vecBounce.z;

			// Move up to the hit point
			TVector4 vecNewPosition = m_vecPosition;

			if ( fDistance > 0.05f )
			{
				TVector4 vecStep;
				vecStep.Multiply( vecMove, fDistance - 0.05f );

				vecNewPosition.x += vecStep.x;
				vecNewPosition.y += vecStep.y;
				vecNewPosition.z += vecStep.z;
			}

			TFLOAT fMovedRatio = fDistance / fMoveLength;
			m_vecPosition      = vecNewPosition;
			m_vecVelocity      = vecNewVelocity;
			fTimeLeft          = ( 1.0f - fMovedRatio ) * fTimeLeft;
			m_fSpinSpeed       = -( m_fTangentialRestitution * m_fSpinSpeed );

			TFLOAT fSpeed = m_vecVelocity.Magnitude();

			if ( fSpeed < 1.05f )
			{
				PlayLandSound();

				m_bStopped    = TTRUE;
				m_vecVelocity = TVector4::VEC_ZERO;
				return TTRUE;
			}

			ARayData oRay( m_vecPosition, TVector4::VEC_POSY, 1000.0f );

			if ( !ACollisionManager::GetSingleton()->CollideRayWithTerrain( oRay, &fDistance, m_eIgnoreGroups ) )
			{
				// There's no ground below, put the unit back on top of it
				TFLOAT fGroundHeight = ATerrainInterface::GetSingleton()->GetGroundHeightTop( m_vecPosition.x, m_vecPosition.z, TCollisionCommon::TOSHICGROUPFLAG_NONE );
				PlayLandSound();

				m_bStopped      = TTRUE;
				m_vecVelocity   = TVector4::VEC_ZERO;
				m_vecPosition.y = fGroundHeight - 0.05f;
			}

			// Move for the rest of the time
			vecMove.Multiply( m_vecVelocity, fTimeLeft );
			vecMove.y += fTimeLeft * fTimeLeft * HALF_GRAVITY;

			fMoveLength = vecMove.NormaliseMag();
			m_bBounced  = TTRUE;
			fDistance   = fMoveLength;

			if ( fSpeed > 3.05f )
				ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::BARNYARD_LAND, *GetPosition() );

			iNumBounces++;

			if ( m_bStopped )
				break;
		}
	}

	if ( !m_bBounced )
	{
		TVector4 vecStep;
		vecStep.Multiply( m_vecVelocity, a_fDeltaTime );

		m_vecPosition.x += vecStep.x;
		m_vecPosition.y += vecStep.y;
		m_vecPosition.z += vecStep.z;
		m_vecPosition.y += fFallDistance;
	}

	TFLOAT fMovedX = m_vecPosition.x - vecOldPosition.x;
	TFLOAT fMovedY = m_vecPosition.y - vecOldPosition.y;
	TFLOAT fMovedZ = m_vecPosition.z - vecOldPosition.z;

	if ( TMath::Abs( fMovedX ) < TMath::TFLOAT_EPSILON && TMath::Abs( fMovedY ) < TMath::TFLOAT_EPSILON && TMath::Abs( fMovedZ ) < TMath::TFLOAT_EPSILON )
	{
		// Stops once it doesn't move anymore
		if ( ++m_iNumStillUpdates > 2 )
		{
			m_bStopped    = TTRUE;
			m_vecVelocity = TVector4::VEC_ZERO;
			return TTRUE;
		}
	}
	else
	{
		m_iNumStillUpdates = 0;
	}

	if ( !m_bStopped )
		m_vecVelocity.y += a_fDeltaTime * GRAVITY;

	m_fSpinAngle += a_fDeltaTime * m_fSpinSpeed;

	if ( m_bUpdateModelTransform )
	{
		AVisibleUnit* pUnit = TSTATICCAST( AVisibleUnit, m_oOwner->GetUnit() );

		if ( pUnit )
		{
			AModelInstance* pModelInstance = pUnit->GetModelInstance();

			if ( pModelInstance )
			{
				TMatrix44 matTransform;
				GetTransform( matTransform );
				pModelInstance->SetTransform( matTransform );
			}
		}
	}

	m_vecDirection   = m_vecVelocity;
	m_vecDirection.y = 0.0f;
	m_vecDirection.Normalise();

	if ( !m_bFreezeSpeed )
		m_fSpeed = TMath::LERP( m_fSpeed, m_vecVelocity.Magnitude(), 0.9f );

	return TTRUE;
}

// $Barnyard: FUNCTION 005725d0
TBOOL AThrowItemSteer::SetPosition( const TVector4& a_rcPosition )
{
	m_vecPosition      = a_rcPosition;
	m_iNumStillUpdates = 0;

	return TTRUE;
}

// $Barnyard: FUNCTION 005725a0
TBOOL AThrowItemSteer::SetPosition3( const TVector3& a_rcPosition )
{
	m_vecPosition.Set( a_rcPosition.x, a_rcPosition.y, a_rcPosition.z, 1.0f );
	m_iNumStillUpdates = 0;

	return TTRUE;
}

// $Barnyard: FUNCTION 00572600
void AThrowItemSteer::SetDirection( const TVector4& a_rcDirection )
{
	m_vecDirection   = a_rcDirection;
	m_vecDirection.y = 0.0f;
	m_vecDirection.Normalise();
}

// $Barnyard: FUNCTION 0055fd50
void AThrowItemSteer::CopyPosition( TVector4& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition;
}

// $Barnyard: FUNCTION 00556a20
void AThrowItemSteer::CopyPosition3( TVector3& a_rOutPosition )
{
	a_rOutPosition = m_vecPosition.AsVector3();
}

// NOTE: same code as ASteerKinematics3D::GetPosition (005772b0), folded in retail
const TVector4* AThrowItemSteer::GetPosition()
{
	return &m_vecPosition;
}

// $Barnyard: FUNCTION 00556a60
void AThrowItemSteer::CopyDirection( TVector4& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection;
}

// $Barnyard: FUNCTION 00556a40
void AThrowItemSteer::CopyDirection3( TVector3& a_rOutDirection )
{
	a_rOutDirection = m_vecDirection.AsVector3();
}

// $Barnyard: FUNCTION 00572640
const TVector4* AThrowItemSteer::GetDirection()
{
	return &m_vecDirection;
}

// $Barnyard: FUNCTION 0056d010
void AThrowItemSteer::CopyVelocity( TVector4& a_rOutVelocity )
{
	a_rOutVelocity = m_vecVelocity;
}

// $Barnyard: FUNCTION 00560ac0
TVector4 AThrowItemSteer::GetVelocity()
{
	return m_vecVelocity;
}

// $Barnyard: FUNCTION 005726d0
TFLOAT AThrowItemSteer::GetSpeed()
{
	return m_vecVelocity.Magnitude();
}

// $Barnyard: FUNCTION 00572580
TFLOAT AThrowItemSteer::GetMeasuredSpeed()
{
	return m_fSpeed;
}

// $Barnyard: FUNCTION 00572690
void AThrowItemSteer::SetSpeed( TFLOAT a_fSpeed )
{
	m_vecVelocity.Normalise();
	m_vecVelocity.Multiply( a_fSpeed );

	if ( !m_vecVelocity.IsEqual( TVector4::VEC_ZERO ) )
		m_bStopped = TFALSE;
}

// $Barnyard: FUNCTION 00572650
void AThrowItemSteer::SetVelocity( const TVector4& a_rcVelocity )
{
	m_vecVelocity = a_rcVelocity;

	if ( !m_vecVelocity.IsEqual( TVector4::VEC_ZERO ) )
	{
		m_fSpinSpeed = 10.0f;
		m_bStopped   = TFALSE;
	}
}

// $Barnyard: FUNCTION 005726e0
void AThrowItemSteer::GetTransform( TMatrix44& a_rOutTransform )
{
	a_rOutTransform.Identity();
	TMatrix44::TransformVector( a_rOutTransform.GetTranslation(), a_rOutTransform, m_vecPosition );

	if ( !m_bStopped )
	{
		// Face the direction of the flight and spin
		TVector4 vecDirection = m_vecVelocity;
		vecDirection.Normalise();

		TFLOAT fYaw;

		if ( vecDirection.z < 0.0f || vecDirection.z > 0.0f )
			fYaw = TMath::ATan( vecDirection.x / vecDirection.z );
		else
			fYaw = 0.0f;

		if ( vecDirection.z < 0.0f )
		{
			if ( vecDirection.x > 0.0f )
				fYaw = fYaw + TMath::PI;
			else
				fYaw = fYaw - TMath::PI;
		}

		a_rOutTransform.RotateY( fYaw );
		a_rOutTransform.RotateX( m_fSpinAngle + TMath::HALF_PI );
	}

	TMatrix44 matTransform = a_rOutTransform;
	a_rOutTransform.Multiply( matTransform, m_matCenterInv );
}

// $Barnyard: FUNCTION 00572800
TBOOL AThrowItemSteer::IgnoreCollisionGroup( TINT a_iGroup )
{
	m_eIgnoreGroups |= 1 << a_iGroup;
	return TTRUE;
}

// $Barnyard: FUNCTION 00572ac0
void AThrowItemSteer::PlayLandSound()
{
	if ( T2String8::Compare( "egg", m_oOwner->GetUnit()->GetType()->GetName().GetString(), -1 ) == 0 )
		ASoundManager::GetSingleton()->PlayCueAtPosition( soundbank::BARNYARD_EGG, *GetPosition() );
}
