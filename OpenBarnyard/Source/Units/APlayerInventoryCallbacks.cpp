#include "pch.h"
#include "APlayerInventoryCallbacks.h"
#include "AInventoryGroupComponent.h"
#include "ACarryableUnitHelper.h"
#include "AAnimatedUnit.h"
#include "AUnitManager.h"
#include "AUnitType.h"
#include "Animation/AAnimatableObject.h"
#include "FX/AAniEffectManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "Quests/AQuestManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInventoryState2.h"

#include <Toshi/TString8.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( Equip );
TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( getiteminv );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( precisioncoin );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( ABarnExploreMinigameState );

// $Barnyard: FUNCTION 00622840
APlayerInventoryCallbacks* APlayerInventoryCallbacks::Create()
{
	return new APlayerInventoryCallbacks();
}

// $Barnyard: FUNCTION 00622760
TBOOL APlayerInventoryCallbacks::OnRemove( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount )
{
	ACarryableUnitHelper* pHelper = a_pItem->GetUnit()->GetCarryableHelper();

	// NOTE: the result isn't used
	a_pInventory->GetOwner();

	// The carried item leaves the hand
	if ( a_pInventory->GetName() == TPS8( Hand ) )
	{
		pHelper->DetachFromHolder();
		return TTRUE;
	}

	if ( a_pInventory->GetName() == TPS8( Equip ) )
		return pHelper->UnEquip();

	return TTRUE;
}

// $Barnyard: FUNCTION 00622860
TBOOL APlayerInventoryCallbacks::OnAdd( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount )
{
	if ( a_pItem->GetUnit() )
	{
		AAnimatedUnit* pItemUnit = TSTATICCAST( AAnimatedUnit, a_pItem->GetUnit() );

		AAniEffectManager::GetSingleton()->RemoveUnitEffects( pItemUnit );
		pItemUnit->GetAnimatableObject()->SetOpacity( 1.0f, TTRUE );
	}

	AUnit*                pItemUnit = a_pItem->GetUnit();
	ACarryableUnitHelper* pHelper   = a_pItem->GetUnit()->GetCarryableHelper();
	AAnimatedUnit*        pOwner    = TSTATICCAST( AAnimatedUnit, a_pInventory->GetOwner() );

	if ( a_pInventory->GetName() == TPS8( Hand ) )
	{
		TBOOL bCarryAll  = TTRUE;
		TINT  iNumInHand = a_pInventory->GetNumItems();

		if ( iNumInHand >= 1 )
		{
			AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, a_pInventory->GetOwner()->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );
			AUnit*                    pHandUnit  = a_pInventory->GetItem( 0 )->GetUnit();
			TINT                      iMaxCarry  = 1;

			if ( pHandUnit && pHandUnit->GetType() == a_pItem->GetUnit()->GetType() )
				iMaxCarry = pHelper->GetMaxCarry();

			if ( iNumInHand < iMaxCarry )
			{
				bCarryAll = TFALSE;
			}
			else
			{
				// The hand is full, the items in it are used
				AInventoryItem* pHandItem = a_pInventory->GetItems().Begin();
				pHandItem->GetUnit()->GetCarryableHelper()->DetachFromHolder();

				pInventory->MoveItemToUseCategory( a_pInventory->GetItems().Begin(), iNumInHand );
			}
		}

		return pHelper->Carry( bCarryAll, a_iCount - 1 );
	}

	if ( a_pInventory->GetName() == TPS8( Equip ) )
		return pHelper->Equip();

	// The item goes to the inventory screen, it's hidden unless the player puts it away
	AAnimatableObject* pAnimObject = pOwner->GetAnimatableObject();

	if ( !pAnimObject->IsAnimationPlanned( TPS8( putaway ) ) && !pOwner->GetAnimatableObject()->IsAnimPlaying( TPS8( putaway ), 0.0f ) )
	{
		if ( pItemUnit->GetClass()->IsA( &TGetClass( AVisibleUnit ) ) )
			TSTATICCAST( AVisibleUnit, pItemUnit )->SetVisible( TFALSE );
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00622a90
TBOOL APlayerInventoryCallbacks::OnCountChanged( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount )
{
	ACarryableUnitHelper* pHelper = a_pItem->GetUnit()->GetCarryableHelper();
	AAnimatedUnit*        pOwner  = TSTATICCAST( AAnimatedUnit, a_pInventory->GetOwner() );

	if ( a_pInventory->GetName() == TPS8( Equip ) )
		return TTRUE;

	if ( pHelper && pHelper->GetMaxCarry() > 1 && pHelper->GetMaxCarry() <= a_pItem->GetCount() )
	{
		// Enough items to make a bunch of them
		TString8 strBunch;
		strBunch.Format( "%s_bunch", a_pItem->GetUnitType()->GetName().GetString() );

		TINT iMaxCarry = pHelper->GetMaxCarry();

		if ( a_pInventory->GetName() == TPS8( Hand ) )
		{
			pHelper->PutAway();
			pOwner->DetachObjectsAtBone( TPS8( cb_rlocator ) );
		}

		TINT iNumLeft = iMaxCarry - a_pInventory->RemoveItems( a_pItem->GetUnitType()->GetName(), iMaxCarry, TTRUE );

		if ( a_pInventory->GetName() == TPS8( Hand ) )
		{
			AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, a_pInventory->GetOwner()->FindComponentExact( &TGetClass( AInventoryGroupComponent ) ) );

			if ( iNumLeft > 0 )
				pInventory->MoveItemToUseCategory( a_pItem, iNumLeft );
		}

		AUnit* pBunch = AUnitManager::GetSingleton()->SpawnUnit( TPString8( strBunch.GetString() ), TVector4( 0.0f, 0.0f, 0.0f, 1.0f ), TVector4( 1.0f, 0.0f, 0.0f, 1.0f ) );
		a_pInventory->AddUnit( pBunch, 1, AInventoryComponentInterface::ms_strDefaultInventory );

		return TTRUE;
	}

	if ( a_pInventory->GetName() == TPS8( Hand ) )
		return TTRUE;

	if ( a_pItem->GetUnit()->GetType()->GetName() == TPS8( precisioncoin ) )
	{
		// The coins only show up on the pick up counter
		ASimPlayerPickUpHudElement* pPickUpHud = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();

		if ( pPickUpHud )
			pPickUpHud->SetNumber( a_pItem->GetCount() );
	}
	else if ( !AQuestManager::GetSingleton()->IsTimerPaused() )
	{
		// Shows the item in the inventory screen
		AGameStateController* pController = AGameStateController::GetSingleton();
		AGameState*           pState      = pController->FindGameStateOnStack( TPS8( ASimState ) );

		if ( !pState )
			pState = pController->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) );

		if ( !pState )
			pState = pController->GetCurrentState();

		AInventoryState2* pInventoryState = TSTATICCAST( AInventoryState2, pState->FindChildState( &TGetClass( AInventoryState2 ) ) );

		if ( !pInventoryState )
		{
			pInventoryState = new AInventoryState2();
			pState->AddChildState( pInventoryState );
		}

		pInventoryState->SelectType( a_pItem->GetUnit()->GetType(), TTRUE );
		pInventoryState->UpdateItemText();
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006227c0
TBOOL APlayerInventoryCallbacks::OnTransfer( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, AInventoryComponentInterface* a_pTarget, TINT a_iCount )
{
	// NOTE: the result isn't used
	a_pItem->GetUnit();

	if ( a_pInventory->GetOwner() == a_pTarget->GetOwner() )
	{
		// The player takes the item in the hand
		if ( a_pInventory->GetName() != TPS8( Hand ) || a_pTarget->GetName() == TPS8( Equip ) )
		{
			if ( a_pTarget->GetName() == TPS8( Hand ) )
				TSTATICCAST( AAnimatedUnit, a_pInventory->GetOwner() )->GetAnimatableObject()->PlayAnimation( TPS8( getiteminv ) );
		}
	}

	return TTRUE;
}
