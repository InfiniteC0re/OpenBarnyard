#include "pch.h"
#include "AMilkPrecisionProjectile.h"
#include "AProjectileLauncher.h"
#include "FX/AParticleEffectManager.h"
#include "Units/AUnitPlayer.h"
#include "Units/APrecisionCoyote.h"
#include "Units/APrecisionJersyCow.h"
#include "Units/APrecisionRacoon.h"
#include "Units/APrecisionScarecrow.h"
#include "Units/APrecisionTargetItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 006665b0, CreateObject 00666660, CreateObjectInPlace 006666c0
TDEFINE_CLASS( AMilkPrecisionProjectile );

// $Barnyard: FUNCTION 006665e0
AMilkPrecisionProjectile::AMilkPrecisionProjectile()
{
	m_oHitUnit = TNULL;
}

// $Barnyard: FUNCTION 00666640
// $Barnyard: FUNCTION 006665c0
AMilkPrecisionProjectile::~AMilkPrecisionProjectile()
{
}

// $Barnyard: FUNCTION 00666700
void AMilkPrecisionProjectile::OnHit()
{
	if ( m_oHitUnit )
	{
		AUnit* pHitUnit = m_oHitUnit->GetUnit();
		AUnit* pOwner   = m_pLauncher->GetOwner()->GetUnit();

		if ( pOwner->IsA( &TGetClass( APrecisionJersyCow ) ) )
		{
			// The milk of a cow
			if ( !pHitUnit->IsA( &TGetClass( APrecisionJersyCow ) ) && pHitUnit->IsA( &TGetClass( AUnitPlayer ) ) )
				TSTATICCAST( APrecisionJersyCow, pOwner )->SetHitPlayer( TTRUE );
		}
		else if ( pHitUnit->IsA( &TGetClass( APrecisionTargetItem ) ) )
		{
			TSTATICCAST( APrecisionTargetItem, pHitUnit )->SetHit( TTRUE, m_iStreamId );
		}
		else if ( pHitUnit->IsA( &TGetClass( APrecisionCoyote ) ) )
		{
			TSTATICCAST( APrecisionCoyote, pHitUnit )->SetMilked( TTRUE );
		}
		else if ( pHitUnit->IsA( &TGetClass( APrecisionRacoon ) ) )
		{
			TSTATICCAST( APrecisionRacoon, pHitUnit )->SetMilked( TTRUE );
		}
		else if ( pHitUnit->IsA( &TGetClass( APrecisionScarecrow ) ) )
		{
			TSTATICCAST( APrecisionScarecrow, pHitUnit )->SetHit( TTRUE );
		}
		else if ( pHitUnit->IsA( &TGetClass( APrecisionJersyCow ) ) )
		{
			TSTATICCAST( APrecisionJersyCow, pHitUnit )->SetMilked( TTRUE );
		}
	}

	TVector4 vDirection = GetDirection();
	vDirection.y *= -1.0f;
	vDirection.Normalise();
	AParticleEffectManager::GetSingleton()->PlayEffect( m_iSplashEffect, m_vPosition, vDirection );

	AMilkProjectile::OnHit();
}

// $Barnyard: FUNCTION 006665d0
TINT AMilkPrecisionProjectile::GetType()
{
	return 7;
}
