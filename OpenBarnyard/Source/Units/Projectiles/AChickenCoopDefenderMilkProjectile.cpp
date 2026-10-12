#include "pch.h"
#include "AChickenCoopDefenderMilkProjectile.h"
#include "AProjectileLauncher.h"
#include "Units/AChickenCoopDefenderThief.h"
#include "Units/AUnitManager.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00515810, CreateObject 005158d0, CreateObjectInPlace 00515930 (the constructor is inlined in both)
TDEFINE_CLASS( AChickenCoopDefenderMilkProjectile );

// $Barnyard: FUNCTION 00515850
AChickenCoopDefenderMilkProjectile::AChickenCoopDefenderMilkProjectile()
{
	m_oHitUnit = TNULL;
}

// $Barnyard: FUNCTION 005158b0
// $Barnyard: FUNCTION 00515820
AChickenCoopDefenderMilkProjectile::~AChickenCoopDefenderMilkProjectile()
{
}

// $Barnyard: FUNCTION 00515b20
void AChickenCoopDefenderMilkProjectile::OnHit()
{
	// Stuns the thief that was hit...
	if ( m_oHitUnit && m_oHitUnit->GetUnit()->IsA( &TGetClass( AChickenCoopDefenderThief ) ) )
		TSTATICCAST( AChickenCoopDefenderThief, m_oHitUnit->GetUnit() )->OnMilked();

	// ... and the ones around
	// NOTE: the thief that was hit is stunned twice (it's around too), the second time does nothing
	TINT                    iNumFound = 0;
	AUnitHash::QueryResult* pResults  = AUnitManager::GetSingleton()->FindUnits( m_vPosition, 2.0f, &iNumFound, TFALSE, 420, TFALSE, TNULL );

	for ( TUINT i = 0; i < TUINT( iNumFound ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit && m_pLauncher->GetOwner()->GetUnit() != pUnit && pUnit->IsA( &TGetClass( AChickenCoopDefenderThief ) ) )
			TSTATICCAST( AChickenCoopDefenderThief, pUnit )->OnMilked();
	}

	AMilkProjectile::OnHit();
}

// $Barnyard: FUNCTION 00515840
TINT AChickenCoopDefenderMilkProjectile::GetType()
{
	return 8;
}

// $Barnyard: FUNCTION 00515970
TBOOL AChickenCoopDefenderMilkProjectile::CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit )
{
	if ( m_vPrevPosition.MagnitudeSq() <= 0.1f )
		return TFALSE;

	if ( m_vPrevPosition.IsEqual( m_vPosition ) )
		return TFALSE;

	// NOTE: unlike AProjectile::CheckUnitCollision, only the ray from the previous position is tested
	ARayData oRay( m_vPrevPosition, m_vPosition );

	if ( !ACollisionManager::CollideRayUnits( m_pLauncher->GetOwner()->GetUnit(), oRay, 10.0f, a_rHitUnit ) )
	{
		a_rHitUnit = TNULL;
		return TFALSE;
	}

	// Only the thieves that aren't stunned yet can be hit
	if ( !a_rHitUnit->GetUnit()->IsA( &TGetClass( AChickenCoopDefenderThief ) ) ||
	     TSTATICCAST( AChickenCoopDefenderThief, a_rHitUnit->GetUnit() )->GetState() == AChickenCoopDefenderThief::STATE_MILKED )
	{
		a_rHitUnit = TNULL;
		return TFALSE;
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00515830
TFLOAT AChickenCoopDefenderMilkProjectile::GetRadius()
{
	return 0.75f;
}
