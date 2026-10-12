#include "pch.h"
#include "AVeggieThrowProjectile.h"
#include "AProjectileLauncher.h"
#include "Units/AVeggiePatchThief.h"
#include "Units/ACarryableUnit.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004cda80, CreateObject 004cdae0, CreateObjectInPlace 004cdb50 (the constructor is inlined in both)
TDEFINE_CLASS( AVeggieThrowProjectile );

// $Barnyard: FUNCTION 004cda90
AVeggieThrowProjectile::AVeggieThrowProjectile()
    : m_fCollisionRadius( 1.0f )
{
}

// $Barnyard: FUNCTION 004cdbb0
// $Barnyard: FUNCTION 004cdab0
AVeggieThrowProjectile::~AVeggieThrowProjectile()
{
}

// $Barnyard: FUNCTION 004cdc20
TBOOL AVeggieThrowProjectile::Update( TFLOAT a_fDeltaTime )
{
	m_fAge += a_fDeltaTime;

	if ( !GetUnit().Get() )
	{
		Kill();
		return TFALSE;
	}

	ASteer* pSteer = GetUnit()->GetUnit()->GetSteer();

	if ( !pSteer->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) )
	{
		Kill();
		return TFALSE;
	}

	AThrowItemSteer* pThrowSteer = TSTATICCAST( AThrowItemSteer, pSteer );

	if ( pThrowSteer->IsStopped() )
	{
		// NOTE: the original calls a DOES_NOTHING function (005acd80) with the unit here
		Kill();
		return TFALSE;
	}

	TBOOL bResult = TFALSE;

	// Follow the unit
	if ( !m_vPrevPosition.IsEqual( *pThrowSteer->GetPosition() ) )
		m_vPosition = *pThrowSteer->GetPosition();

	GetUnit()->GetUnit()->SetNoQuery( TTRUE );

	if ( CheckUnitCollision( m_oHitUnit ) )
	{
		OnHitUnit();
		bResult = TTRUE;
	}

	if ( pThrowSteer->HasBounced() )
	{
		OnLanded();
		bResult = TTRUE;
	}

	m_vPrevPosition = m_vPosition;
	return bResult;
}

// $Barnyard: FUNCTION 004cdac0
void AVeggieThrowProjectile::OnLaunched()
{
	AProjectileWithUnit::OnLaunched();
}

// $Barnyard: FUNCTION 004cdad0
TINT AVeggieThrowProjectile::GetType()
{
	return 3;
}

// $Barnyard: FUNCTION 004cde70
TBOOL AVeggieThrowProjectile::CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit )
{
	// Ignore the thrown unit and the one who threw it
	AUnit* apIgnore[ 2 ];
	apIgnore[ 0 ] = GetUnit()->GetUnit();
	apIgnore[ 1 ] = m_pLauncher->GetOwner()->GetUnit();

	TBOOL bHit = TFALSE;

	// NOTE: same queries as AProjectileWithUnit::CheckUnitCollision (inlined by the original)
	if ( m_vPrevPosition.MagnitudeSq() > 0.1f && !m_vPrevPosition.IsEqual( m_vPosition ) )
	{
		ARayData      oRay( m_vPrevPosition, m_vPosition );
		ACollisionHit oHit;

		if ( ACollisionManager::CollideRayUnits( oRay, 10.0f, a_rHitUnit, oHit, 2, apIgnore ) )
			bHit = TTRUE;
		else
			a_rHitUnit = TNULL;
	}

	if ( !bHit )
	{
		if ( !ACollisionManager::CollideSphereUnits( m_vPosition, GetRadius(), a_rHitUnit, 2, apIgnore ) )
			return bHit;

		bHit = TTRUE;
	}

	// Carryable objects and raccoons that are already milked aren't hit
	if ( a_rHitUnit->GetUnit()->GetClass()->IsA( &TGetClass( ACarryableUnit ) ) )
	{
		a_rHitUnit = TNULL;
		return TFALSE;
	}

	if ( a_rHitUnit->GetUnit()->GetClass()->IsA( &TGetClass( AVeggiePatchThief ) ) && TSTATICCAST( AVeggiePatchThief, a_rHitUnit->GetUnit() )->GetState() == AVeggiePatchThief::STATE_MILKED )
	{
		a_rHitUnit = TNULL;
		return TFALSE;
	}

	return bHit;
}

// $Barnyard: FUNCTION 004cdbd0
void AVeggieThrowProjectile::OnHitUnit()
{
	AUnit* pUnit = m_oHitUnit ? m_oHitUnit->GetUnit() : TNULL;

	if ( pUnit->GetClass()->IsA( &TGetClass( AVeggiePatchThief ) ) )
		TSTATICCAST( AVeggiePatchThief, m_oHitUnit ? m_oHitUnit->GetUnit() : TNULL )->SetState( AVeggiePatchThief::STATE_MILKED );
}
