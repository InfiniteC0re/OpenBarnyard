#include "pch.h"
#include "AProjectileWithUnit.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AObjectTraits.h"
#include "Units/ACarryableUnit.h"
#include "AProjectileLauncher.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "UnitMovement/Steering/ASteerTraits.h"
#include "Animation/AAnimatableObject.h"
#include "FX/AAniEffectManager.h"
#include "FX/AWaitAniEffect.h"
#include "FX/AFadeAniEffect.h"
#include "FX/ADieAniEffect.h"
#include "Units/AUnitManager.h"
#include "ARandom.h"
#include "Physics/ACollisionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 0066bdc0, CreateObject 0066c3d0, CreateObjectInPlace 0066c450
TDEFINE_CLASS( AProjectileWithUnit );

// $Barnyard: FUNCTION 0066be10
AProjectileWithUnit::AProjectileWithUnit()
    : m_pPrevSteerClass( TNULL )
    , m_fRadius( 1.0f )
{
}

// $Barnyard: FUNCTION 0066c4d0
// $Barnyard: FUNCTION 0066be40
AProjectileWithUnit::~AProjectileWithUnit()
{
	m_oUnit = TNULL;
}

// $Barnyard: FUNCTION 0066c060
TBOOL AProjectileWithUnit::Update( TFLOAT a_fDeltaTime )
{
	m_fAge += a_fDeltaTime;

	if ( !m_oUnit || !m_oUnit->GetUnit() )
	{
		Kill();
		return TFALSE;
	}

	ASteer* pSteer = m_oUnit->GetUnit()->GetSteer();

	if ( !pSteer->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) || TSTATICCAST( AThrowItemSteer, pSteer )->IsStopped() )
	{
		Kill();
		return TFALSE;
	}

	AThrowItemSteer* pThrowSteer = TSTATICCAST( AThrowItemSteer, pSteer );
	TBOOL            bResult     = TFALSE;

	// Follow the unit
	if ( !m_vPrevPosition.IsEqual( *pThrowSteer->GetPosition() ) )
		m_vPosition = *pThrowSteer->GetPosition();

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

// $Barnyard: FUNCTION 0066c4f0
void AProjectileWithUnit::Kill()
{
	if ( m_pPrevSteerClass )
	{
		AUnit* pUnit = m_oUnit->GetUnit();
		pUnit->ChangeSteer( m_pPrevSteerClass );

		if ( AAniEffectManager::GetSingleton() )
		{
			AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

			if ( !pTraits || !pTraits->m_pWhenThrow || pTraits->m_pWhenThrow->m_bDestroy )
			{
				AAniEffectManager* pAniEffectManager = AAniEffectManager::GetSingleton();

				if ( !pAniEffectManager->ReserveEffects( 3 ) )
				{
					AUnitManager::GetSingleton()->DestroyUnit( m_oUnit->GetUnit() );
				}
				else
				{
					// Wait, fade the unit out and destroy it
					AWaitAniEffect* pWaitEffect = pAniEffectManager->CreateEffect<AWaitAniEffect>();
					AFadeAniEffect* pFadeEffect = pAniEffectManager->CreateEffect<AFadeAniEffect>();
					ADieAniEffect*  pDieEffect  = pAniEffectManager->CreateEffect<ADieAniEffect>();

					pDieEffect->SetUnit( m_oUnit->GetUnit() );
					pFadeEffect->SetUnit( m_oUnit->GetUnit() );
					pFadeEffect->SetSpeed( 0.5f );
					pFadeEffect->SetNextEffect( pDieEffect );

					pWaitEffect->SetTime( ARandom::GetSingleton()->m_oRandom.GetFloatMinMax( 15.0f, 20.0f ) );
					pWaitEffect->SetNextEffect( pFadeEffect );
				}
			}
			else
			{
				// Drop the unit slightly to let it rest on the ground
				TVector4 vPosition;
				pUnit->GetSteer()->CopyPosition( vPosition );
				vPosition.y -= 0.3f;
				pUnit->GetSteer()->SetPosition( vPosition );
			}
		}
	}

	if ( m_oUnit->GetUnit()->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) && m_oUnit->GetUnit() )
		TSTATICCAST( AAnimatedUnit, m_oUnit->GetUnit() )->GetAnimatableObject()->SetAnimControllersDisabled( TFALSE );

	m_oUnit = TNULL;
	AProjectile::Kill();
}

// $Barnyard: FUNCTION 0066c220
void AProjectileWithUnit::SetVelocity( const Toshi::TVector4& a_rcVelocity )
{
	if ( m_oUnit && m_oUnit->GetUnit()->GetSteer()->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) )
		m_oUnit->GetUnit()->GetSteer()->SetVelocity( a_rcVelocity );

	AProjectile::SetVelocity( a_rcVelocity );
}

// $Barnyard: FUNCTION 0066c170
void AProjectileWithUnit::OnLaunched()
{
	const TVector4* pPosition = m_oUnit->GetUnit()->GetSteer()->GetPosition();
	m_vPrevPosition           = *pPosition;
	m_vPosition               = *pPosition;

	AUnit* pUnit = m_oUnit->GetUnit();

	if ( pUnit->GetClass()->IsA( &TGetClass( ACarryableUnit ) ) )
	{
		// The launcher's owner threw the unit
		pUnit->GetCarryableHelper()->SetHolder( m_pLauncher->GetOwnerUnit() );
		pUnit->GetCarryableHelper()->Throw();
	}
}

// $Barnyard: FUNCTION 0066bde0
void AProjectileWithUnit::Reset()
{
	m_oUnit = TNULL;
	AProjectile::Reset();
}

// $Barnyard: FUNCTION 0066bee0
TBOOL AProjectileWithUnit::CheckUnitCollision( Toshi::TRef<AUnitProxy>& a_rHitUnit )
{
	// The thrown unit and the thrower can't be hit
	AUnit* apIgnore[ 2 ];
	apIgnore[ 0 ] = m_oUnit.Get()->GetUnit();
	apIgnore[ 1 ] = m_pLauncher->GetOwner().Get()->GetUnit();

	TBOOL bHit = TFALSE;

	// NOTE: retail tests the length of the previous position
	if ( m_vPrevPosition.MagnitudeSq() > 0.1f && !m_vPrevPosition.IsEqual( m_vPosition ) )
	{
		ARayData      oRay( m_vPrevPosition, m_vPosition );
		ACollisionHit oHit;

		if ( ACollisionManager::CollideRayUnits( oRay, 10.0f, a_rHitUnit, oHit, 2, apIgnore ) )
			bHit = TTRUE;
		else
			a_rHitUnit = TNULL;

		if ( bHit )
			return bHit;
	}

	if ( ACollisionManager::CollideSphereUnits( m_vPosition, GetRadius(), a_rHitUnit, 2, apIgnore ) )
		return TTRUE;

	return bHit;
}

// $Barnyard: FUNCTION 0066c270
void AProjectileWithUnit::SetUnit( Toshi::TRef<AUnitProxy> a_oUnit )
{
	if ( a_oUnit.Get() == m_oUnit.Get() )
		return;

	m_oUnit = a_oUnit;

	AUnit* pUnit = a_oUnit->GetUnit();
	pUnit->SetMovement( TTRUE );
	m_fRadius = pUnit->GetSteer()->GetTraits()->m_fRadius;

	if ( !pUnit->GetSteer()->GetClass()->IsA( &TGetClass( AThrowItemSteer ) ) )
	{
		m_pPrevSteerClass = const_cast<TClass*>( pUnit->GetSteer()->GetClass() );
		pUnit->ChangeSteer( &TGetClass( AThrowItemSteer ) );
	}
	else
	{
		m_pPrevSteerClass = TNULL;
	}

	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits && pTraits->m_pWhenThrow && !pTraits->m_pWhenThrow->m_bCanSpin )
		return;

	if ( m_oUnit->GetUnit()->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
	{
		TSTATICCAST( AAnimatedUnit, m_oUnit->GetUnit() )->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

		TSTATICCAST( AThrowItemSteer, pUnit->GetSteer() )->SetUpdateModelTransform( TTRUE );
	}
}

// $Barnyard: FUNCTION 0066c3a0
Toshi::TRef<AUnitProxy> AProjectileWithUnit::GetUnit() const
{
	return m_oUnit;
}
