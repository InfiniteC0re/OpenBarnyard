#include "pch.h"
#include "AChickenCoopDefenderThrowProjectile.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "MiniGame/AChickenCoopDefender.h"
#include "Units/AChickenCoopDefenderHen.h"
#include "Units/AUnitType.h"
#include "Units/AUnitCollisionComponentData.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AChickenCoopDefenderThrowProjectile );

// $Barnyard: FUNCTION 0051efc0
// NOTE: also inlined by the TClass create (0051f000) and init (0051f070) functions
AChickenCoopDefenderThrowProjectile::AChickenCoopDefenderThrowProjectile()
    : m_fCollisionRadius( 1.0f )
{
}

// $Barnyard: FUNCTION 0051f0d0
// $Barnyard: FUNCTION 0051efe0
AChickenCoopDefenderThrowProjectile::~AChickenCoopDefenderThrowProjectile()
{
}

// $Barnyard: FUNCTION 0051f150
TBOOL AChickenCoopDefenderThrowProjectile::Update( TFLOAT a_fDeltaTime )
{
	AChickenCoopDefenderHen* pHen = TSTATICCAST( AChickenCoopDefenderHen, GetUnit()->GetUnit() );

	if ( pHen->GetState() == AChickenCoopDefenderHen::STATE_THROWN )
		return AProjectileWithUnit::Update( a_fDeltaTime );

	// Stops following the hen once it's not flying anymore
	Kill();
	return TFALSE;
}

// $Barnyard: FUNCTION 0051f0f0
void AChickenCoopDefenderThrowProjectile::Kill()
{
	AChickenCoopDefenderHen* pHen = TSTATICCAST( AChickenCoopDefenderHen, GetUnit()->GetUnit() );

	AProjectileWithUnit::Kill();

	// Missed the coop
	if ( pHen->GetState() != AChickenCoopDefenderHen::STATE_HOME && pHen->GetState() != AChickenCoopDefenderHen::STATE_POCKETED )
		pHen->SetState( AChickenCoopDefenderHen::STATE_LANDED );
}

// $Barnyard: FUNCTION 0051f1b0
void AChickenCoopDefenderThrowProjectile::OnLaunched()
{
	AChickenCoopDefenderHen*  pHen  = TSTATICCAST( AChickenCoopDefenderHen, GetUnit()->GetUnit() );
	AChickenCoopDefenderGame* pGame = pHen->m_pGame;

	AThrowItemSteer* pSteer = TSTATICCAST( AThrowItemSteer, pHen->GetSteer() );
	pSteer->SetSmoothedSpeed( 0.0f );
	pSteer->SetFreezeSpeed( TTRUE );

	AChickenCoopDefenderHotSpot* pHotSpot = pGame->pHotSpot;
	pHen->SetValue( pHotSpot->iValue );

	if ( pHotSpot->fThrowAngle > 0.0f && pHotSpot->fThrowAngle < TMath::HALF_PI )
	{
		// The camera of the player follows the hen into the coop
		AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
		pHelper->SetTarget( GetUnit() );
		pHelper->SetDefaultMaxPitch( 70.0f );
		pHelper->SetMaxPitch( 70.0f );
		pHelper->SetHeightOffset( 0.5f );
		pHelper->SetTargetPitch( 0.0f );
		pHelper->SetDefaultPitch( 0.0f );
		pHelper->SetCollisionRadius( 0.43f * 0.01f );
		pHelper->SetSmallCollisionRadius( 0.2f * 0.01f );

		pSteer->IgnoreCollisionGroup( 19 );
		pGame->ePhase = AChickenCoopDefender::PHASE_FOLLOW;
	}

	pHen->SetState( AChickenCoopDefenderHen::STATE_THROWN );

	AUnitCollisionComponentData* pCollisionData = TSTATICCAST( AUnitCollisionComponentData, pHen->GetType()->FindStaticData( &TGetClass( AUnitCollisionComponentData ) ) );

	if ( pCollisionData )
		m_fCollisionRadius = pCollisionData->GetCollisionRadius();

	AProjectileWithUnit::OnLaunched();
}

// $Barnyard: FUNCTION 0051eff0
TINT AChickenCoopDefenderThrowProjectile::GetType()
{
	return 5;
}
