#include "pch.h"
#include "ACollisionObjectUnit.h"
#include "ACollisionObjectMesh.h"
#include "ACollisionObjectModel.h"
#include "ACollisionModelInstance.h"
#include "Animation/AAnimatableObject.h"
#include "Units/AAnimatedUnit.h"
#include "Animation/AModel.h"
#include "Units/AVisibleUnit.h"
#include "Units/AUnitType.h"
#include "Units/AUnitCollisionComponentData.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0061bd80, CreateObject 0061d380, CreateObjectInPlace 0061d410
TDEFINE_CLASS( ACollisionObjectUnit );

// $Barnyard: FUNCTION 0061cd60
ACollisionObjectUnit::ACollisionObjectUnit()
    : m_pUnit( TNULL )
    , m_eFlags( FLAGS_NONE )
    , m_iBone( -1 )
{
	m_eType = TYPE_UNIT;
}

// $Barnyard: FUNCTION 0061c750
// $Barnyard: FUNCTION 0061cda0
// $Barnyard: FUNCTION 0061c7a0
ACollisionObjectUnit::~ACollisionObjectUnit()
{
	// NOTE: 0061c7a0 is the this-adjusting thunk of the TObject vtable
}

// $Barnyard: FUNCTION 0061cdc0
TBOOL ACollisionObjectUnit::Create( AVisibleUnit* a_pUnit, AModelInstance* a_pModelInstance, TBOOL a_bIgnoreObjectCollision )
{
	m_pUnit = a_pUnit;

	AUnitCollisionComponentData* pCollisionData = TSTATICCAST( AUnitCollisionComponentData, a_pUnit->GetType()->FindStaticData( &TGetClass( AUnitCollisionComponentData ) ) );

	m_pModelInstance = TNULL;
	m_fRadius        = pCollisionData->m_fCollisionRadius;
	m_fHeightScale   = pCollisionData->m_fHeight; // NOTE: retail reads it through an out-of-line getter (0058ac50)
	m_fSphereRaise   = pCollisionData->m_fSphereRaise;

	TVector4 vecPosition;
	a_pUnit->GetSteer()->CopyPosition( vecPosition );
	m_vPosition = vecPosition.AsVector3();

	m_eFlags |= FLAGS_CREATED;

	if ( pCollisionData->m_iCollideFlags != -1 )
		m_eType = TYPE( pCollisionData->m_iCollideFlags );

	if ( a_bIgnoreObjectCollision == TTRUE )
		m_eFlags |= FLAGS_IGNORE_OBJECT_COLLISION;

	// NOTE: inlined in retail
	Activate();

	return TTRUE;
}

// $Barnyard: FUNCTION 0061d4a0
void ACollisionObjectUnit::Update( void* a_pUnk )
{
	if ( m_pUnit && !m_pUnit->IsDying() )
	{
		TVector4 vecPosition;

		if ( m_iBone == -1 )
		{
			m_pUnit->GetSteer()->CopyPosition( vecPosition );
		}
		else
		{
			TMatrix44 matBone;
			TSTATICCAST( AAnimatedUnit, m_pUnit )->GetAnimatableObject()->GetModelInstance()->GetBoneTransform( m_iBone, matBone );

			vecPosition = matBone.GetTranslation();
		}

		UpdateHash( vecPosition.AsVector3() );
	}
}

// $Barnyard: FUNCTION 0061d030
TBOOL ACollisionObjectUnit::CollideSweptSphere( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TVector4& a_rNormal, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	if ( m_eFlags & FLAGS_IGNORE_OBJECT_COLLISION )
		return TFALSE;

	if ( a_pObstacle->IsA( &TGetClass( ACollisionObjectUnit ) ) )
	{
		if ( IsIgnoredUnitObject( TSTATICCAST( ACollisionObjectUnit, a_pObstacle ) ) )
			return TFALSE;

		return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
	}
	else if ( a_pObstacle->GetClass() == &TGetClass( ACollisionObjectMesh ) )
	{
		ACollisionObjectMesh* pMesh = TSTATICCAST( ACollisionObjectMesh, a_pObstacle );

		if ( pMesh->GetSceneObject()->IsSkeletonUpdateEnabled() )
		{
			TFLOAT fRadius      = GetObjectRadius();
			TFLOAT fPenetration = 0.0f;

			return pMesh->GetModelInstance()->CollideSweptSphere( a_rcCenter, fRadius, a_rcDirection, a_rfDistance, a_rNormal, a_fMaxStepHeight, a_rfStepHeight, fPenetration, TCollisionCommon::TOSHICGROUPFLAG_NONE );
		}

		return TFALSE;
	}
	else if ( a_pObstacle->IsA( &TGetClass( ACollisionObjectModel ) ) )
	{
		return a_pObstacle->ReciprocateSweptSphere( this, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
	}

	return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, a_rNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061cea0
TBOOL ACollisionObjectUnit::CollideSweptSphereNoNormal( ACollisionObject* a_pObstacle, TFLOAT& a_rfDistance, const TVector4& a_rcDirection, const TVector4& a_rcCenter, TFLOAT a_fMaxStepHeight, TFLOAT& a_rfStepHeight )
{
	if ( m_eFlags & FLAGS_IGNORE_OBJECT_COLLISION )
		return TFALSE;

	if ( a_pObstacle->IsA( &TGetClass( ACollisionObjectUnit ) ) )
	{
		if ( IsIgnoredUnitObject( TSTATICCAST( ACollisionObjectUnit, a_pObstacle ) ) )
			return TFALSE;
	}

	// NOTE: the normal is written to a temporary vector initialised with the position
	TVector4 vecNormal( m_vPosition.x, m_vPosition.y, m_vPosition.z, 1.0f );

	return CollideSweptSphereImpl( a_pObstacle, a_rfDistance, a_rcDirection, a_rcCenter, vecNormal, a_fMaxStepHeight, a_rfStepHeight );
}

// $Barnyard: FUNCTION 0061d250
TBOOL ACollisionObjectUnit::CollideSphere( ACollisionObject* a_pObstacle, const TVector4& a_rcCenter, TFLOAT& a_rfDistance, TVector4& a_rNormal )
{
	if ( m_eFlags & FLAGS_IGNORE_OBJECT_COLLISION )
		return TFALSE;

	// NOTE: unlike the other tests, this one doesn't look at the ignore list of the other object
	if ( m_vecIgnoredUnits.Size() != 0 && a_pObstacle->IsA( &TGetClass( ACollisionObjectUnit ) ) )
	{
		AUnit* pOtherUnit = TSTATICCAST( AVisibleUnit, a_pObstacle->GetOwnerUnit() );

		if ( m_vecIgnoredUnits.Find( pOtherUnit ) != m_vecIgnoredUnits.End() )
			return TFALSE;
	}

	return CollideSphereImpl( a_pObstacle, a_rcCenter, a_rfDistance, a_rNormal );
}

// $Barnyard: FUNCTION 0061c140
void ACollisionObjectUnit::GetVelocity( TVector4& a_rOutVelocity )
{
	TVector4 vecVelocity;
	m_pUnit->GetSteer()->CopyVelocity( vecVelocity );

	a_rOutVelocity = vecVelocity;
}

// $Barnyard: FUNCTION 0061c790
void* ACollisionObjectUnit::GetOwnerUnit()
{
	return m_pUnit;
}

// $Barnyard: FUNCTION 0061c180
void ACollisionObjectUnit::TranslateBy( const TVector4& a_rDirection, TFLOAT a_fLength )
{
	ASteer* pSteer = m_pUnit->GetSteer();

	TVector4 vecPosition;
	pSteer->CopyPosition( vecPosition );

	TVector4 vecTranslation;
	vecTranslation.Multiply( a_rDirection, a_fLength );

	vecPosition.x += vecTranslation.x;
	vecPosition.y += vecTranslation.y;
	vecPosition.z += vecTranslation.z;

	m_vPosition.x += vecTranslation.x;
	m_vPosition.y += vecTranslation.y;
	m_vPosition.z += vecTranslation.z;

	pSteer->SetPosition3( vecPosition.AsVector3() );
}

// $Barnyard: FUNCTION 0061c230
void ACollisionObjectUnit::SetVelocity( void* a_pVelocity )
{
	const TVector4& rcVelocity = *TSTATICCAST( const TVector4, a_pVelocity );

	TVector4 vecDirection( rcVelocity.x, rcVelocity.y, rcVelocity.z, 1.0f );
	ASteer*  pSteer = m_pUnit->GetSteer();
	TFLOAT   fSpeed = vecDirection.AsVector3().Magnitude();

	if ( fSpeed > 0.0f )
	{
		vecDirection.Divide( fSpeed );
		pSteer->SetDirection( vecDirection );
	}

	pSteer->SetSpeed( fSpeed );
}

TBOOL ACollisionObjectUnit::IsIgnoredUnitObject( ACollisionObjectUnit* a_pOther )
{
	// NOTE: inlined in retail (the ignore lists hold the units returned by GetOwnerUnit)
	if ( m_vecIgnoredUnits.Size() != 0 )
	{
		AUnit* pOtherUnit = TSTATICCAST( AVisibleUnit, a_pOther->GetOwnerUnit() );

		if ( m_vecIgnoredUnits.Find( pOtherUnit ) != m_vecIgnoredUnits.End() )
			return TTRUE;
	}

	if ( a_pOther->m_vecIgnoredUnits.Size() != 0 )
	{
		AUnit* pUnit = TSTATICCAST( AVisibleUnit, GetOwnerUnit() );

		if ( a_pOther->m_vecIgnoredUnits.Find( pUnit ) != a_pOther->m_vecIgnoredUnits.End() )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0061d330
void ACollisionObjectUnit::AddIgnoredUnit( AUnit* a_pUnit )
{
	if ( m_vecIgnoredUnits.Find( a_pUnit ) == m_vecIgnoredUnits.End() )
		m_vecIgnoredUnits.PushBack( a_pUnit );
}
