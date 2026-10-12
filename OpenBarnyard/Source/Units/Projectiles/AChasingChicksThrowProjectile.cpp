#include "pch.h"
#include "AChasingChicksThrowProjectile.h"
#include "AProjectileLauncher.h"
#include "Cameras/ACameraManager.h"
#include "Cameras/AGTAStyleCameraHelper.h"
#include "Units/AUnitType.h"
#include "Units/AUnitCollisionComponentData.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "Units/AChasingChicksChick.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AChasingChicksThrowProjectile );

// $Barnyard: FUNCTION 00512510
// NOTE: also inlined by the TClass create (00512550) and init (005125d0) functions
AChasingChicksThrowProjectile::AChasingChicksThrowProjectile()
    : m_iPlayer( -1 )
    , m_fCollisionRadius( 1.0f )
{
}

// $Barnyard: FUNCTION 00512630
// $Barnyard: FUNCTION 00512530
AChasingChicksThrowProjectile::~AChasingChicksThrowProjectile()
{
}

// $Barnyard: FUNCTION 005126b0
TBOOL AChasingChicksThrowProjectile::Update( TFLOAT a_fDeltaTime )
{
	AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, GetUnit()->GetUnit() );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: a chick that misses the basket and leaves the map falls forever (nothing
	// below to land on), and the end of the game waits for it. Land it after a while:
	// the landing puts it back on the closest area of the network.
	if ( pChick->GetState() == AChasingChicksChick::STATE_THROWN && GetAge() < 15.0f )
#else
	if ( pChick->GetState() == AChasingChicksChick::STATE_THROWN )
#endif
		return AProjectileWithUnit::Update( a_fDeltaTime );

	// Stops following the chick once it's not flying anymore
	Kill();
	return TFALSE;
}

// $Barnyard: FUNCTION 00512650
void AChasingChicksThrowProjectile::Kill()
{
	AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, GetUnit()->GetUnit() );

	AProjectileWithUnit::Kill();

	if ( pChick->GetState() < AChasingChicksChick::STATE_POCKETED || pChick->GetState() > AChasingChicksChick::STATE_RESPAWN )
		pChick->SetState( AChasingChicksChick::STATE_LANDED );
}

// $Barnyard: FUNCTION 00512710
void AChasingChicksThrowProjectile::OnLaunched()
{
	AChasingChicksChick* pChick = TSTATICCAST( AChasingChicksChick, GetUnit()->GetUnit() );
	AChasingChicksGame*  pGame  = AChasingChicksGame::ms_pCurrent;

	// Find the player who threw the chick
	m_iPlayer = -1;

	for ( TINT i = 0; i < pGame->iNumPlayers; i++ )
	{
		if ( pGame->aPlayers[ i ].oUnit.Get() == m_pLauncher->GetOwner().Get() )
		{
			m_iPlayer = i;
			break;
		}
	}

	// NOTE: retail doesn't check if the player was found (aPlayers[ -1 ] otherwise)
	AChasingChicksPlayer* pPlayer = &pGame->aPlayers[ m_iPlayer ];

	AThrowItemSteer* pSteer = TSTATICCAST( AThrowItemSteer, pChick->GetSteer() );
	pSteer->SetSmoothedSpeed( 0.0f );
	pSteer->SetFreezeSpeed( TTRUE );
	pSteer->IgnoreCollisionGroup( 19 );

	if ( m_iPlayer == 0 && pPlayer->pHotSpot->fThrowAngle > 0.0f && pPlayer->pHotSpot->fThrowAngle < TMath::HALF_PI )
	{
		// The camera of the player follows the chick into the pen
		AGTAStyleCameraHelper* pHelper = TSTATICCAST( AGTAStyleCameraHelper, ACameraManager::GetSingleton()->GetSplitscreenCameraHelper( 1 ) );
		pHelper->SetTarget( GetUnit() );
		pHelper->SetHeightOffset( 0.5f );
		pHelper->SetDefaultMaxPitch( 70.0f );
		pHelper->SetMaxPitch( 70.0f );
		pHelper->SetTargetPitch( 0.0f );
		pHelper->SetDefaultPitch( 0.0f );
		pHelper->SetCollisionRadius( 0.43f * 0.01f );
		pHelper->SetSmallCollisionRadius( 0.2f * 0.01f );

		// AChasingChicksMiniGame::PHASE_
		if ( pGame->ePhase != 2 && pGame->ePhase != 3 )
			pGame->ePhase = 1;
	}

	pChick->SetState( AChasingChicksChick::STATE_THROWN );

	pPlayer->pChick = TNULL;

	AUnitCollisionComponentData* pCollisionData = TSTATICCAST( AUnitCollisionComponentData, pChick->GetType()->FindStaticData( &TGetClass( AUnitCollisionComponentData ) ) );

	if ( pCollisionData )
		m_fCollisionRadius = pCollisionData->GetCollisionRadius();

	pChick->m_iValue = pPlayer->pHotSpot->iValue;

	AProjectileWithUnit::OnLaunched();
}

// $Barnyard: FUNCTION 00512540
TINT AChasingChicksThrowProjectile::GetType()
{
	return 4;
}
