#include "pch.h"
#include "AVeggieDefenderMilkProjectile.h"
#include "AProjectileLauncher.h"
#include "Units/AVeggiePatchThief.h"
#include "Units/ACarryableUnit.h"
#include "Units/AUnitManager.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004c49d0, CreateObject 004c4a30, CreateObjectInPlace 004c4ab0 (the constructor is inlined in both)
TDEFINE_CLASS( AVeggieDefenderMilkProjectile );

// $Barnyard: FUNCTION 004c49e0
AVeggieDefenderMilkProjectile::AVeggieDefenderMilkProjectile()
{
	m_uiCollisionMask = 0x80002;
}

// $Barnyard: FUNCTION 004c4b10
// $Barnyard: FUNCTION 004c4a00
AVeggieDefenderMilkProjectile::~AVeggieDefenderMilkProjectile()
{
}

// $Barnyard: FUNCTION 004c4dc0
void AVeggieDefenderMilkProjectile::OnHit()
{
	// Milk the raccoons around the splash
	TINT                    iNumFound = 0;
	AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( m_vPosition, 1.0f, &iNumFound, TFALSE, 420, TFALSE, TNULL );

	for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit && m_pLauncher->GetOwner()->GetUnit() != pUnit && pUnit->GetClass()->IsA( &TGetClass( AVeggiePatchThief ) ) )
			TSTATICCAST( AVeggiePatchThief, pUnit )->SetState( AVeggiePatchThief::STATE_MILKED );
	}

	AMilkProjectile::OnHit();
}

// $Barnyard: FUNCTION 004c4a20
TINT AVeggieDefenderMilkProjectile::GetType()
{
	return 1;
}

// $Barnyard: FUNCTION 004c4b60
TBOOL AVeggieDefenderMilkProjectile::CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit )
{
	TBOOL bHit = TFALSE;

	// NOTE: same queries as AProjectile::CheckUnitCollision (inlined by the original)
	if ( m_vPrevPosition.MagnitudeSq() > 0.1f && !m_vPrevPosition.IsEqual( m_vPosition ) )
	{
		ARayData oRay( m_vPrevPosition, m_vPosition );

		if ( ACollisionManager::CollideRayUnits( m_pLauncher->GetOwner()->GetUnit(), oRay, 10.0f, a_rHitUnit ) )
			bHit = TTRUE;
		else
			a_rHitUnit = TNULL;
	}

	if ( !bHit )
	{
		AUnit* pOwner = m_pLauncher->GetOwner()->GetUnit();

		if ( !ACollisionManager::CollideSphereUnits( m_vPosition, GetRadius(), a_rHitUnit, 1, &pOwner ) )
			return bHit;

		bHit = TTRUE;
	}

	// The milk passes through carryable objects and raccoons that are already milked
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
