#include "pch.h"
#include "AUnitPlayer.h"
#include "AUnitType.h"
#include "AInteractionComponent.h"
#include "ACharacterMovementComponent.h"
#include "AModelSwapComponent.h"
#include "ASimplePlayerControlComponent.h"
#include "Projectiles/AProjectileLauncher.h"
#include "Projectiles/AMilkSquirtProjectileLauncher.h"
#include "Projectiles/AAimedSquirtProjectileLauncher.h"
#include "Projectiles/APlayerLauncherData.h"
#include "Projectiles/AMilkComponent.h"
#include "Projectiles/ALaunchAmmo.h"
#include "Animation/AAnimatableObject.h"
#include "Render/AShadowManager.h"
#include "UnitMovement/Steering/APlayerSteer.h"
#include "Player/APlayerProfileManager.h"
#include "Helpers/AGameLoader.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( land );
TPSTRING8_DECLARE( up );

// NOTE: GetClass 004145f0, CreateObject 004148f0, CreateObjectInPlace 00414950
TDEFINE_CLASS( AUnitPlayer );

// $Barnyard: FUNCTION 00414600
AUnitPlayer::AUnitPlayer()
    : m_pActiveLauncher( TNULL )
    , m_fUnkAC( 1.0f )
{
	m_aiLauncherSlots[ 0 ] = -1;
	m_aiLauncherSlots[ 1 ] = -1;
	m_apLaunchers[ 0 ]     = TNULL;
	m_apLaunchers[ 1 ]     = TNULL;

	SetHashed( TFALSE );
}

// $Barnyard: FUNCTION 00414990
// $Barnyard: FUNCTION 00414680
AUnitPlayer::~AUnitPlayer()
{
	for ( TINT i = 0; i < NUM_LAUNCHERS; i++ )
	{
		if ( m_apLaunchers[ i ] )
		{
			m_apLaunchers[ i ]->Deactivate();

			if ( m_apLaunchers[ i ] )
				delete m_apLaunchers[ i ];

			m_apLaunchers[ i ] = TNULL;
		}
	}
}

// $Barnyard: FUNCTION 00414d80
TBOOL AUnitPlayer::SetMovement( TBOOL a_bEnabled )
{
	if ( m_eFlags & FLAGS_DYING )
		return AUnit::SetMovement( a_bEnabled );

	ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );

	if ( pControl )
	{
		pControl->SetCanSprint( a_bEnabled );
		pControl->SetCanJump( a_bEnabled );
	}

	return AUnit::SetMovement( a_bEnabled );
}

// $Barnyard: FUNCTION 00414b20
TBOOL AUnitPlayer::Create( AUnitTransform* a_pTransform )
{
	AUnitType* pType = m_pType;

	if ( !AAnimatedUnit::Create( a_pTransform ) )
		return TFALSE;

	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) );
	pMovement->SetWalkingSpeed( ACharacterMovementComponent::APPLY_MODIFIER_ONLY, pMovement->GetWalkSpeed() );

	m_eFlags |= FLAGS_AI_ENABLED;
	m_eFlags |= FLAGS_THINK_ENABLED;

	AInteractionComponent* pInteraction = TSTATICCAST( AInteractionComponent, FindComponent( &TGetClass( AInteractionComponent ) ) );

	if ( pInteraction )
		pInteraction->SetAllowNormalInteractions( TFALSE );

	APlayerLauncherData* pLauncherData = TSTATICCAST( APlayerLauncherData, pType->FindStaticData( &TGetClass( APlayerLauncherData ) ) );

	// No launchers while the barn explore minigame is loaded
	if ( pLauncherData && !AGameLoader::IsBarnExploreMiniGame() )
	{
		if ( pLauncherData->IsLauncherEnabled( APlayerLauncherData::LAUNCHER_MILK_SQUIRT ) )
		{
			m_apLaunchers[ 0 ] = new AMilkSquirtProjectileLauncher();

			AMilkComponent* pMilkData = TSTATICCAST( AMilkComponent, m_pType->FindStaticData( &TGetClass( AMilkComponent ) ) );

			if ( pMilkData )
			{
				m_apLaunchers[ 0 ]->GetAmmo()->SetRechargeRate( pMilkData->GetRechargeRate() );
				m_apLaunchers[ 0 ]->SetFireInterval( pMilkData->GetFireDelay() );
				m_apLaunchers[ 0 ]->GetAmmo()->SetMaxAmmo( pMilkData->GetMaxAmmo() );
				m_apLaunchers[ 0 ]->GetAmmo()->SetAmmo( pMilkData->GetMaxAmmo() );
				m_apLaunchers[ 0 ]->SetLaunchSpeed( pMilkData->GetFireSpeed() );
				m_apLaunchers[ 0 ]->SetVerticalAngle( pMilkData->GetVerticalAngle() );
			}

			m_apLaunchers[ 0 ]->UpdateGender();
			m_apLaunchers[ 0 ]->SetOwner( this );
		}

		if ( pLauncherData->IsLauncherEnabled( APlayerLauncherData::LAUNCHER_AIMED_SQUIRT ) )
		{
			m_apLaunchers[ 1 ] = new AAimedSquirtProjectileLauncher();
			m_apLaunchers[ 1 ]->SetOwner( this );
			m_apLaunchers[ 1 ]->UpdateGender();
		}

		m_aiLauncherSlots[ 0 ] = pLauncherData->GetPrimaryLauncher();
		m_aiLauncherSlots[ 1 ] = pLauncherData->GetSecondaryLauncher();
		SetActiveLauncher( m_aiLauncherSlots[ 0 ], TTRUE );
	}

	AShadowManager::GetSingleton()->AddUnitShadow( this, 1.25f, -1, -0.1f, AShadowManager::FLAGS_UNIFORM_SCALE, AShadowManager::TYPE_DEFAULT );

	return TTRUE;
}

// $Barnyard: FUNCTION 00414710
void AUnitPlayer::OnDestroy()
{
	for ( TINT i = 0; i < NUM_LAUNCHERS; i++ )
	{
		if ( m_apLaunchers[ i ] )
		{
			m_apLaunchers[ i ]->Deactivate();

			if ( m_apLaunchers[ i ] )
				delete m_apLaunchers[ i ];

			m_apLaunchers[ i ] = TNULL;
		}
	}

	AAnimatedUnit::OnDestroy();
}

// NOTE: identical code folding shares this one with AUnitActor::UpdateAI
// $Barnyard: FUNCTION 00414890
void AUnitPlayer::UpdateAI( TFLOAT a_fDeltaTime )
{
	AVisibleUnit::UpdateAI( a_fDeltaTime );
}

// NOTE: same code as ACharacterUnit::Think (004149e0), folded in retail
TBOOL AUnitPlayer::Think( TFLOAT a_fDeltaTime )
{
	ACharacterMovementComponent* pMovement = TSTATICCAST( ACharacterMovementComponent, FindComponent( &TGetClass( ACharacterMovementComponent ) ) );

	// Timed run desire, falls back to walking once it runs out
	if ( pMovement->GetRunTimer() > 0.0f )
	{
		TFLOAT fTimeLeft = pMovement->GetRunTimer() - a_fDeltaTime;

		if ( fTimeLeft <= 0.0f )
			pMovement->SetDesireWalk();

		pMovement->SetRunTimer( fTimeLeft );
	}

	return AVisibleUnit::Think( a_fDeltaTime );
}

// $Barnyard: FUNCTION 00414a50
void AUnitPlayer::SetVisible( TBOOL a_bVisible )
{
	AModelSwapComponent* pModelSwap = TSTATICCAST( AModelSwapComponent, FindComponent( &TGetClass( AModelSwapComponent ) ) );

	if ( pModelSwap && pModelSwap->GetSwappedModel() )
	{
		pModelSwap->GetSwappedModel()->SetSkeletonUpdating( a_bVisible, TTRUE );
		return;
	}

	AAnimatedUnit::SetVisible( a_bVisible );
}

// $Barnyard: FUNCTION 004148a0
TFLOAT AUnitPlayer::GetUnkAC()
{
	return m_fUnkAC;
}

// $Barnyard: FUNCTION 004148b0
void AUnitPlayer::SetUnkAC( TFLOAT a_fValue )
{
	m_fUnkAC = a_fValue;
}

// $Barnyard: FUNCTION 00414750
void AUnitPlayer::SetActiveLauncher( TINT a_iIndex, TBOOL a_bTransferState )
{
	if ( a_iIndex == -1 )
	{
		if ( m_pActiveLauncher )
			m_pActiveLauncher->Deactivate();

		m_pActiveLauncher = TNULL;
		return;
	}

	if ( a_iIndex >= 0 && a_iIndex < NUM_LAUNCHERS )
	{
		AProjectileLauncher* pLauncher = m_apLaunchers[ a_iIndex ];

		if ( pLauncher && pLauncher != m_pActiveLauncher )
		{
			if ( a_bTransferState && m_pActiveLauncher )
			{
				pLauncher->SetAmmo( m_pActiveLauncher->GetAmmo() );

				if ( m_pActiveLauncher->GetThrowAmmo() )
					pLauncher->SetThrowAmmo( m_pActiveLauncher->GetThrowAmmo() );
			}

			if ( m_pActiveLauncher )
				m_pActiveLauncher->Deactivate();

			m_pActiveLauncher = pLauncher;
			pLauncher->SetOwner( this );
			m_pActiveLauncher->Activate();
		}
	}
}

// $Barnyard: FUNCTION 00414800
void AUnitPlayer::SetActiveLauncher( AProjectileLauncher* a_pLauncher, TBOOL a_bTransferState )
{
	if ( !a_pLauncher )
	{
		if ( m_pActiveLauncher )
			m_pActiveLauncher->Deactivate();

		m_pActiveLauncher = TNULL;
		return;
	}

	if ( a_pLauncher != m_pActiveLauncher )
	{
		if ( a_bTransferState && m_pActiveLauncher )
		{
			a_pLauncher->SetAmmo( m_pActiveLauncher->GetAmmo() );

			if ( m_pActiveLauncher->GetThrowAmmo() )
				a_pLauncher->SetThrowAmmo( m_pActiveLauncher->GetThrowAmmo() );
		}

		if ( m_pActiveLauncher )
			m_pActiveLauncher->Deactivate();

		m_pActiveLauncher = a_pLauncher;
		a_pLauncher->SetOwner( this );
		m_pActiveLauncher->Activate();
	}
}

// $Barnyard: FUNCTION 004149b0
void AUnitPlayer::SelectLauncher( TBOOL a_bSecondary )
{
	TINT iIndex = a_bSecondary ? m_aiLauncherSlots[ 1 ] : m_aiLauncherSlots[ 0 ];

	if ( iIndex >= 0 )
		SetActiveLauncher( iIndex, TTRUE );
}

// $Barnyard: FUNCTION 00414a90
void AUnitPlayer::SetSwappedModel( TBOOL a_bLand )
{
	AModelSwapComponent* pModelSwap = TSTATICCAST( AModelSwapComponent, FindComponent( &TGetClass( AModelSwapComponent ) ) );

	if ( !pModelSwap )
		return;

	APlayerProfileManager* pProfileManager = APlayerProfileManager::GetSingleton();
	pModelSwap->SwapModel( pProfileManager->GetMainGender() == APlayerProfile::GENDER_FEMALE, pProfileManager->GetMainCostume(), a_bLand ? &TPS8( land ) : &TPS8( up ), a_bLand );

	if ( a_bLand && AAnimatedUnit::IsVisible() == a_bLand )
		AAnimatedUnit::SetVisible( TFALSE );

	if ( m_pSteer->IsA( &TGetClass( APlayerSteer ) ) )
		TSTATICCAST( APlayerSteer, m_pSteer )->DisableKinematicsObstructionStop( a_bLand );
}

// $Barnyard: FUNCTION 00414dd0
void AUnitPlayer::SetControlFlags( TBOOL a_bValue )
{
	if ( m_eFlags & FLAGS_DYING )
		return;

	ASimplePlayerControlComponent* pControl = TSTATICCAST( ASimplePlayerControlComponent, FindComponent( &TGetClass( ASimplePlayerControlComponent ) ) );

	if ( pControl )
		pControl->SetLauncherFlags( a_bValue );
}

// $Barnyard: FUNCTION 0043cd80
AProjectileLauncher* AUnitPlayer::GetSecondaryLauncher() const
{
	TINT iIndex = m_aiLauncherSlots[ 1 ];

	if ( iIndex > -1 && iIndex < NUM_LAUNCHERS )
		return m_apLaunchers[ iIndex ];

	return TNULL;
}
