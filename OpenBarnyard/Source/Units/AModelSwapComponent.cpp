#include "pch.h"
#include "AModelSwapComponent.h"
#include "AModelSwapComponentData.h"
#include "AGeneralUnitComponentManager.h"
#include "AAnimatedUnit.h"
#include "Animation/AAnimatableObject.h"
#include "Animation/AAnimatableObjectManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: CreateObject 0058e0d0 / CreateObjectInPlace 0058e130
// NOTE: GetClass 0058dcd0, CreateObject 0058e0d0, CreateObjectInPlace 0058e130
TDEFINE_CLASS( AModelSwapComponent );

// $Barnyard: FUNCTION 0058dd60
AModelSwapComponent::AModelSwapComponent()
    : m_pSwappedModel( TNULL )
    , m_eFlags( FLAGS_NONE )
{
}

// $Barnyard: FUNCTION 0058e1e0
// $Barnyard: FUNCTION 0058ddb0
AModelSwapComponent::~AModelSwapComponent()
{
	AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

	for ( TINT i = 0; i < m_aModels.Size(); i++ )
	{
		if ( m_aModels[ i ] )
		{
			pAnimObjManager->DestroyAnimatableObject( m_aModels[ i ] );
			m_aModels[ i ] = TNULL;
		}
	}

	m_aModels.Resize( 0 );
}

// NOTE: folded getter of AGeneralUnitComponentManager::ms_pSingleton (005d2300)
AUnitComponentManager* AModelSwapComponent::GetManager()
{
	return AGeneralUnitComponentManager::GetSingleton();
}

// $Barnyard: FUNCTION 0058dce0
void AModelSwapComponent::UpdateAI( TFLOAT a_fDeltaTime )
{
	if ( !( m_eFlags & FLAGS_UNTIL_ANIM_IS_OVER ) )
		return;

	if ( m_pSwappedModel )
	{
		if ( m_pSwappedModel->IsAnimPlaying( m_strAnimation, 0.0f ) )
			return;

		m_pSwappedModel->KillAllAnimations();
		m_pSwappedModel->SetSkeletonUpdating( TFALSE, TTRUE );
		m_pSwappedModel->SetAnimControllersDisabled( TTRUE );
	}

	AAnimatedUnit* pOwner = TSTATICCAST( AAnimatedUnit, GetOwner() );

	if ( pOwner && !pOwner->IsVisible() )
		pOwner->GetAnimatableObject()->SetSkeletonUpdating( TTRUE, TTRUE );

	m_eFlags &= ~FLAGS_UNTIL_ANIM_IS_OVER;
	m_pSwappedModel = TNULL;
}

// $Barnyard: FUNCTION 0058deb0
TBOOL AModelSwapComponent::CreateModels( AUnit* a_pUnit, AModelSwapComponentData* a_pData )
{
	TINT iNumModels = a_pData->GetNumModels();

	if ( iNumModels > 0 )
	{
		if ( m_aModels.Capacity() < iNumModels )
			m_aModels.Resize( iNumModels );

		m_aModels.SetSize( iNumModels );

		AAnimatableObjectManager* pAnimObjManager = AAnimatableObjectManager::GetSingleton();

		for ( TINT i = 0; i < iNumModels; i++ )
		{
			m_aModels[ i ] = TNULL;

			AAnimatableObjectType* pType = pAnimObjManager->FindType( a_pData->GetModel( i ) );

			if ( pType )
			{
				m_aModels[ i ] = pAnimObjManager->CreateAnimatableObject( pType, a_pUnit, 0 );
				m_aModels[ i ]->SetAnimControllersDisabled( TTRUE );
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 0058df70
void AModelSwapComponent::SwapModel( TINT a_iModel, TINT a_iSkin, const TPString8* a_pAnimation, TBOOL a_bApplySkin )
{
	if ( m_pSwappedModel )
	{
		m_pSwappedModel->KillAllAnimations();
		m_pSwappedModel->SetAnimControllersDisabled( TTRUE );
		m_pSwappedModel->SetSkeletonUpdating( TFALSE, TTRUE );
	}

	m_eFlags &= ~FLAGS_UNTIL_ANIM_IS_OVER;
	AAnimatableObject* pModel = m_aModels[ a_iModel ];

	if ( !a_pAnimation )
	{
		m_strAnimation = TPString8();
	}
	else
	{
		m_strAnimation = *a_pAnimation;

		pModel->PlayAnimation( *a_pAnimation );
		pModel->SetAnimControllersDisabled( TFALSE );
		pModel->SetSkeletonUpdating( TTRUE, TTRUE );
	}

	if ( a_bApplySkin )
	{
		pModel->ApplyMaterial( a_iSkin, TTRUE );
		pModel->SetAnimControllersDisabled( TFALSE );
		pModel->SetSkeletonUpdating( TTRUE, TTRUE );
		m_pSwappedModel = pModel;
		return;
	}

	if ( a_pAnimation )
	{
		m_eFlags |= FLAGS_UNTIL_ANIM_IS_OVER;
		m_pSwappedModel = pModel;
		return;
	}

	pModel->KillAllAnimations();
	pModel->SetAnimControllersDisabled( TTRUE );
	pModel->SetSkeletonUpdating( TFALSE, TTRUE );
	m_pSwappedModel = TNULL;
}
