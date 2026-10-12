#include "pch.h"
#include "AProjectileLauncher.h"
#include "UnitMovement/Steering/AThrowItemSteer.h"
#include "ALaunchAmmo.h"
#include "AProjectile.h"
#include "AProjectileWithUnit.h"
#include "ASimpleProjectileManager.h"
#include "Player/APlayerProfileManager.h"
#include "Units/AVisibleUnit.h"
#include "Units/AUnit.h"
#include "Units/AUnitType.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AObjectTraits.h"
#include "Units/AInventoryGroupComponent.h"
#include "UnitMovement/Steering/ASteer.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/ANamedAnimationSet.h"

#include <Math/TMatrix44.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( squirt );

// NOTE: GetClass 0066afd0, CreateObject 0066b650, CreateObjectInPlace 0066b6b0
TDEFINE_CLASS( AProjectileLauncher );

// $Barnyard: FUNCTION 0066b2c0
AProjectileLauncher::AProjectileLauncher()
    : m_fOwnerVelocityScale( 0.5f )
    , m_fFireInterval( 0.0f )
    , m_fFireTimer( 0.0f )
    , m_fUnk18( 1.0f )
    , m_fHorizontalSpeedScale( 1.0f )
    , m_fThrowSpeedScale( 1.0f )
    , m_iGender( 2 )
    , m_pAmmo( TNULL )
    , m_pThrowAmmo( TNULL )
    , m_bInfiniteAmmo( TFALSE )
    , m_oEmitter( this )
{
	m_fFireInterval  = 1.0f;
	m_fVerticalAngle = 0.3490658f; // 20 degrees
}

// $Barnyard: FUNCTION 0066b790
// $Barnyard: FUNCTION 0066b350
AProjectileLauncher::~AProjectileLauncher()
{
	m_oOwner = TNULL;
}

// $Barnyard: FUNCTION 0066afe0
void AProjectileLauncher::Activate()
{
	m_fFireTimer = GetFireInterval();
}

// NOTE: identical to Activate, both slots point at 0066afe0 in retail
void AProjectileLauncher::Deactivate()
{
	m_fFireTimer = GetFireInterval();
}

// $Barnyard: FUNCTION 0066aff0
TBOOL AProjectileLauncher::ProcessInput( AInputCommand a_eCommand, const Toshi::TInputInterface::InputEvent* a_pEvent, TBOOL a_bUnk1, TBOOL a_bUnk2 )
{
	if ( a_eCommand == AInputCommand_Shoot )
	{
		if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			if ( a_bUnk1 )
			{
				Fire();
				return TTRUE;
			}
		}
		else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
		{
			StopFiring();
		}
	}
	else if ( a_eCommand == AInputCommand_ShootAlt )
	{
		if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
		{
			if ( a_bUnk2 )
				return Throw();
		}
		else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
		{
			StopThrowing();
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0066b1e0
TBOOL AProjectileLauncher::Update( TFLOAT a_fDeltaTime )
{
	m_fFireTimer += a_fDeltaTime;
	m_pAmmo->Update( a_fDeltaTime );

	return TFALSE;
}

// $Barnyard: FUNCTION 0066b430
Toshi::TVector4 AProjectileLauncher::GetLaunchVelocity()
{
	TVector4 vVelocity   = GetAimVelocity();
	TVector4 vHorizontal = GetAimVelocity();
	vHorizontal.y        = 0.0f;

	TVector4 vOffset;
	vOffset.Multiply( vHorizontal, m_fHorizontalSpeedScale - 1.0f );
	vVelocity.Add( vOffset );

	vOffset.Multiply( GetOwnerUnit()->GetSteer()->GetVelocity(), m_fOwnerVelocityScale );
	vVelocity.Add( vOffset );

	return vVelocity;
}

// $Barnyard: FUNCTION 0066b550
Toshi::TVector4 AProjectileLauncher::GetLaunchPosition()
{
	return *GetOwnerUnit()->GetSteer()->GetPosition();
}

// $Barnyard: FUNCTION 0066b590
Toshi::TVector4 AProjectileLauncher::GetLaunchDirection()
{
	return *GetOwnerUnit()->GetSteer()->GetDirection();
}

// $Barnyard: FUNCTION 0066b060
Toshi::TVector4 AProjectileLauncher::GetAimVelocity()
{
	TVector4 vVelocity;
	vVelocity.Multiply( GetLaunchDirection(), m_fLaunchSpeed );

	return vVelocity;
}

// $Barnyard: FUNCTION 0066b890
Toshi::TVector4 AProjectileLauncher::GetThrowPosition()
{
	if ( GetOwnerUnit()->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
	{
		AModelInstanceRef pModelInstance = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() )->GetAnimatableObject()->GetModelInstance();

		TMatrix44 matBone;
		pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) ), matBone );

		return matBone.GetTranslation();
	}

	return GetLaunchPosition();
}

// $Barnyard: FUNCTION 0066b0b0
Toshi::TVector4 AProjectileLauncher::GetThrowDirection( AUnit* a_pObject )
{
	return GetLaunchDirection();
}

// $Barnyard: FUNCTION 0066b610
TFLOAT AProjectileLauncher::GetThrowSpeed( AUnit* a_pObject )
{
	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, a_pObject->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits && pTraits->m_pWhenThrow )
		return pTraits->m_pWhenThrow->m_fBaseThrowSpeed * m_fThrowSpeedScale;

	return 15.0f * m_fThrowSpeedScale;
}

// $Barnyard: FUNCTION 0066b0d0
Toshi::TVector4 AProjectileLauncher::GetThrowVelocity( AUnit* a_pObject )
{
	TFLOAT fSpeed = GetThrowSpeed( a_pObject );

	TVector4 vVelocity;
	vVelocity.Multiply( GetThrowDirection( a_pObject ), fSpeed );

	return vVelocity;
}

// $Barnyard: FUNCTION 0066b5d0
void AProjectileLauncher::Fire()
{
	if ( CanFire() )
	{
		// NOTE: retail inlines LaunchProjectile (0066b400) here
		LaunchProjectile();
		OnFired();
	}
}

// $Barnyard: FUNCTION 0066bd70
TBOOL AProjectileLauncher::Throw()
{
	AUnit* pHeldUnit = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() )->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

	if ( pHeldUnit )
	{
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pHeldUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

		if ( pTraits && pTraits->m_pWhenThrow )
		{
			ThrowHeldUnit();
			return TTRUE;
		}
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0066b170
TBOOL AProjectileLauncher::CanFire()
{
	return GetFireInterval() <= m_fFireTimer;
}

// $Barnyard: FUNCTION 0066b190
TBOOL AProjectileLauncher::CanThrow()
{
	if ( m_pThrowAmmo && GetFireInterval() <= m_fFireTimer )
	{
		// NOTE: retail rounds with FISTP (round half to even)
		return TMath::Round( m_pThrowAmmo->GetAmmo() ) != 0;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 0066ba10
void AProjectileLauncher::OnFired()
{
	if ( GetOwnerUnit()->GetSteer()->GetSpeed() < 1.0f )
	{
		AAnimatableObject* pAnimatableObject = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() )->GetAnimatableObject();

		// NOTE: inlined AAnimatableObject method 0057c850 (does the object's animation set have this animation?)
		if ( pAnimatableObject->GetToshiAnimInterface().GetAnimationSet()->GetNamedAnimation( TPS8( squirt ) ) )
		{
			if ( GetOwnerUnit()->GetSteer()->GetClass()->IsA( &TGetClass( APlayerSteer ) ) )
			{
				if ( TSTATICCAST( APlayerSteer, GetOwnerUnit()->GetSteer() )->GetState() != APlayerSteer::STATE_WALKING )
					return;

				pAnimatableObject->KillAllAnimations();
			}

			pAnimatableObject->PlayAnimation( TPS8( squirt ) );
		}
	}
}

// $Barnyard: FUNCTION 0066b6f0
static TRef<AUnitProxy> GetUnitProxy( AUnit* a_pUnit )
{
	TRef<AUnitProxy> oProxy;
	oProxy = a_pUnit->GetSelfProxy();

	return oProxy;
}

// $Barnyard: FUNCTION 0066bba0
void AProjectileLauncher::OnUnitHit( AUnit* a_pUnit )
{
	TRef<AUnitProxy> oUnit = GetUnitProxy( a_pUnit );
	m_oEmitter.Throw( &oUnit );
}

// $Barnyard: FUNCTION 0066b980
void AProjectileLauncher::SetOwner( AUnit* a_pUnit )
{
	m_oOwner = a_pUnit->GetSelfProxy();
}

// $Barnyard: FUNCTION 0066b2b0
void AProjectileLauncher::UpdateGender()
{
	m_iGender = APlayerProfileManager::GetSingleton()->GetMainGender();
}

// $Barnyard: FUNCTION 0066b140
void AProjectileLauncher::SetAmmo( ALaunchAmmo* a_pAmmo )
{
	m_pAmmo = a_pAmmo;
}

// $Barnyard: FUNCTION 0066b400
AProjectile* AProjectileLauncher::LaunchProjectile()
{
	return Launch( ASimpleProjectileManager::GetSingleton()->GetProjectile( m_pAmmo->GetProjectileType() ) );
}

// $Barnyard: FUNCTION 0066b200
AProjectile* AProjectileLauncher::Launch( AProjectile* a_pProjectile )
{
	a_pProjectile->Reset();

	// NOTE: retail rounds with FISTP (round half to even)
	if ( TMath::Round( m_pAmmo->GetAmmo() ) > 0 )
	{
		m_fFireTimer = 0.0f;

		if ( !m_bInfiniteAmmo )
			m_pAmmo->UseAmmo();
	}

	TVector4 vVelocity = GetLaunchVelocity();
	TVector4 vPosition = GetLaunchPosition();

	a_pProjectile->SetVelocity( vVelocity );
	a_pProjectile->SetLifetime( 3.0f );
	a_pProjectile->SetLauncher( this );
	a_pProjectile->SetPosition( vPosition );

	m_pAmmo->OnLaunch( a_pProjectile );
	OnLaunched( a_pProjectile );

	return a_pProjectile;
}

// $Barnyard: FUNCTION 0066b7b0
AProjectile* AProjectileLauncher::ThrowUnit( AProjectile* a_pProjectile, AUnit* a_pObject )
{
	a_pProjectile->Reset();
	a_pProjectile->SetLifetime( 10.0f );
	a_pProjectile->SetLauncher( this );

	TVector4 vPosition = GetThrowPosition();
	a_pProjectile->SetPosition( vPosition );

	TSTATICCAST( AVisibleUnit, a_pObject )->SetVisible( TTRUE );
	TSTATICCAST( AProjectileWithUnit, a_pProjectile )->SetUnit( a_pObject->GetSelfProxy() );
	a_pObject->GetSteer()->SetPosition( vPosition );

	TVector4 vVelocity = GetThrowVelocity( a_pObject );

	AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, a_pObject->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( pTraits )
	{
		TSTATICCAST( AThrowItemSteer, a_pObject->GetSteer() )->SetRestitution( pTraits->m_fRestitution );

		if ( pTraits->m_pWhenThrow )
			vVelocity.y -= pTraits->m_pWhenThrow->m_fUpDirection;
	}

	a_pObject->GetSteer()->SetVelocity( vVelocity );

	OnThrown( a_pProjectile );
	return a_pProjectile;
}

// $Barnyard: FUNCTION 0066bc10
void AProjectileLauncher::ThrowHeldUnit()
{
	AAnimatedUnit* pOwner = TSTATICCAST( AAnimatedUnit, GetOwnerUnit() );
	ASteer*        pSteer = pOwner->GetSteer();

	AUnit* pHeldUnit = pOwner->GetAttachedUnit( TPS8( cb_rlocator ), -1 );

	if ( !pHeldUnit )
		return;

	AProjectile* pProjectile = ASimpleProjectileManager::GetSingleton()->GetProjectile( m_pThrowAmmo ? m_pThrowAmmo->GetProjectileType() : 2 );
	pProjectile->Reset();

	// NOTE: the result isn't used
	pSteer->GetDirection();

	AInventoryGroupComponent* pInventory     = TSTATICCAST( AInventoryGroupComponent, pOwner->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );
	AModelInstanceRef         pModelInstance = pOwner->GetAnimatableObject()->GetModelInstance();

	// NOTE: the bone's transform isn't used
	TMatrix44 matBone;
	pModelInstance->GetBoneTransform( pModelInstance->GetBoneIndex( TPS8( cb_rlocator ) ), matBone );

	// The unit leaves the hand
	if ( pInventory )
		pInventory->DropItemOfType( pHeldUnit->GetType()->GetName(), TPS8( Hand ) );

	if ( pOwner->GetAttachedUnit( TPS8( cb_rlocator ), -1 ) == pHeldUnit )
		pOwner->DetachUnit( pHeldUnit, TTRUE, pSteer->GetKinematics() != TNULL );

	ThrowUnit( pProjectile, pHeldUnit );
}
