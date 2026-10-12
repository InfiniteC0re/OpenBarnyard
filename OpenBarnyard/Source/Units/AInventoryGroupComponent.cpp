#include "pch.h"
#include "AInventoryGroupComponent.h"
#include "AInventoryGroupComponentData.h"
#include "AInventoryComponentData.h"
#include "ACarryableUnitHelper.h"
#include "AObjectTraits.h"
#include "AAnimatedUnit.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "Animation/AAnimatableObject.h"
#include "Player/APlayerManager.h"
#include "FX/AAniEffectManager.h"
#include "FX/AInventoryAniEffect.h"
#include "FX/ASwirlTrackHudAniEffect.h"
#include "FX/AHudTrackBoneAniEffect.h"
#include "FX/AWaitAniEffect.h"
#include "APlayerControlComponent.h"
#include "Animation/AAnimatableObjectType.h"
#include "Player/APlayerProfileManager.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "Cameras/ACameraManager.h"
#include "Render/ARenderer.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/AInventoryState2.h"
#include "Helpers/ABikeManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00622de0, CreateObject 006233a0, CreateObjectInPlace 00623420
TDEFINE_CLASS( AInventoryGroupComponent );

TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( getiteminv );
TPSTRING8_DECLARE( ASimState );
TPSTRING8_DECLARE( ABarnExploreMinigameState );
TPSTRING8_DECLARE( AInventoryState2 );
TPSTRING8_DECLARE( precisioncoin );
TPSTRING8_DECLARE( detector );
TPSTRING8_DECLARE( carrylarge );
TPSTRING8_DECLARE( carrysmall );
TPSTRING8_DECLARE( cb_rlocator );
TPSTRING8_DECLARE( handle );

// NOTE: inlined into CreateObject
AInventoryGroupComponent::AInventoryGroupComponent()
    : m_pCategories( TNULL )
    , m_pSelectedItem( TNULL )
{
}

// $Barnyard: FUNCTION 00622f20
// $Barnyard: FUNCTION 006236b0
AInventoryGroupComponent::~AInventoryGroupComponent()
{
	if ( m_pCategories )
		delete[] m_pCategories;
}

// $Barnyard: FUNCTION 00625390
void AInventoryGroupComponent::Create( AInventoryGroupComponentData* a_pData )
{
	m_pData = a_pData;

	TINT iNumCategories = a_pData->GetNumCategories();
	m_pCategories       = new AInventoryComponent[ iNumCategories ];

	for ( TINT i = 0; i < iNumCategories; i++ )
		m_pCategories[ i ].Create( m_pData->GetCategory( i ), GetOwner() );
}

// NOTE: inlined everywhere
TINT AInventoryGroupComponent::GetNumCategories() const
{
	return m_pData->GetNumCategories();
}

// $Barnyard: FUNCTION 00623200
void AInventoryGroupComponent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
		m_pCategories[ i ].Save( a_pBuffer );
}

// $Barnyard: FUNCTION 006257f0
void AInventoryGroupComponent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	TINT iNumCategories = GetNumCategories();

	if ( m_pCategories )
		Clear();
	else
		m_pCategories = new AInventoryComponent[ iNumCategories ];

	for ( TINT i = 0; i < iNumCategories; i++ )
		m_pCategories[ i ].Load( a_pBuffer );
}

// $Barnyard: FUNCTION 00622f90
void AInventoryGroupComponent::Clear()
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
		m_pCategories[ i ].Clear();
}

// $Barnyard: FUNCTION 006256a0
TBOOL AInventoryGroupComponent::AddItem( AInventoryItem* a_pItem, TPString8 a_strInventory )
{
	if ( ContainsItem( a_pItem ) )
		return TFALSE;

	if ( a_strInventory == ms_strDefaultInventory )
	{
		// The item goes to its store inventory
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, a_pItem->GetUnitType()->FindStaticData( &TGetClass( AObjectTraits ) ) );
		a_strInventory         = pTraits->m_strInventoryForStore;
	}

	return GetCategory( a_strInventory )->AddItem( a_pItem, ms_strDefaultInventory );
}

// $Barnyard: FUNCTION 006255b0
TBOOL AInventoryGroupComponent::AddItems( const TPString8& a_rcType, TINT a_iCount, TPString8 a_strInventory )
{
	if ( a_strInventory == ms_strDefaultInventory )
	{
		// The items go to their store inventory
		AUnitType*     pType   = AUnitManager::GetSingleton()->GetUnitFactory()->Find( a_rcType );
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, pType->FindStaticData( &TGetClass( AObjectTraits ) ) );
		a_strInventory         = pTraits->m_strInventoryForStore;
	}

	return GetCategory( a_strInventory )->AddItems( a_rcType, a_iCount, ms_strDefaultInventory );
}

// $Barnyard: FUNCTION 006254a0
TBOOL AInventoryGroupComponent::AddUnit( AUnit* a_pUnit, TINT a_iCount, TPString8 a_strInventory )
{
	if ( ContainsUnit( a_pUnit ) )
		return TFALSE;

	if ( a_strInventory == ms_strDefaultInventory )
	{
		// The unit goes to its store inventory
		AObjectTraits* pTraits = TSTATICCAST( AObjectTraits, a_pUnit->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );
		a_strInventory         = pTraits->m_strInventoryForStore;
	}

	return GetCategory( a_strInventory )->AddUnit( a_pUnit, a_iCount, ms_strDefaultInventory );
}

// $Barnyard: FUNCTION 00624100
AUnit* AInventoryGroupComponent::DropItem( AInventoryItem* a_pItem )
{
	AInventoryComponent* pCategory = FindCategoryWithItem( a_pItem );

	if ( !pCategory )
		return TNULL;

	return pCategory->DropItem( a_pItem );
}

// $Barnyard: FUNCTION 006240d0
AUnit* AInventoryGroupComponent::DropItemOfType( const TPString8& a_rcType )
{
	AInventoryComponent* pCategory = FindCategoryWithType( a_rcType );

	if ( !pCategory )
		return TNULL;

	return pCategory->DropItemOfType( a_rcType );
}

// $Barnyard: FUNCTION 00623730
AUnit* AInventoryGroupComponent::DropItemOfType( const TPString8& a_rcType, const TPString8& a_rcCategory )
{
	AInventoryComponent* pCategory = GetCategory( a_rcCategory );

	if ( !pCategory )
		return TNULL;

	return pCategory->DropItemOfType( a_rcType );
}

// $Barnyard: FUNCTION 00623090
TINT AInventoryGroupComponent::RemoveItems( const TPString8& a_rcType, TINT a_iCount, TBOOL a_bDestroy )
{
	TINT iNumLeft       = a_iCount;
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories && iNumLeft > 0; i++ )
		iNumLeft -= m_pCategories[ i ].RemoveItems( a_rcType, iNumLeft, a_bDestroy );

	return a_iCount - iNumLeft;
}

// $Barnyard: FUNCTION 00622fc0
AInventoryItem* AInventoryGroupComponent::RemoveItem( AInventoryItem* a_pItem )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		if ( m_pCategories[ i ].FindItemByType( a_pItem->GetUnitType()->GetName() ) )
		{
			AInventoryItem* pRemoved = m_pCategories[ i ].RemoveItem( a_pItem );

			if ( pRemoved )
				return pRemoved;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00623030
AInventoryItem* AInventoryGroupComponent::RemoveUnit( AUnit* a_pUnit )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		AInventoryItem* pItem = m_pCategories[ i ].FindItemByUnit( a_pUnit );

		if ( pItem )
		{
			AInventoryItem* pRemoved = m_pCategories[ i ].RemoveItem( pItem );

			if ( pRemoved )
				return pRemoved;
		}
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006230f0
AInventoryItem* AInventoryGroupComponent::FindItemByUnit( AUnit* a_pUnit )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		AInventoryItem* pItem = m_pCategories[ i ].FindItemByUnit( a_pUnit );

		if ( pItem )
			return pItem;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00623140
AInventoryItem* AInventoryGroupComponent::FindItemByType( const TPString8& a_rcType )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		AInventoryItem* pItem = m_pCategories[ i ].FindItemByType( a_rcType );

		if ( pItem )
			return pItem;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00623190
TINT AInventoryGroupComponent::GetNumItems()
{
	TINT iNumItems      = 0;
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
		iNumItems += m_pCategories[ i ].GetNumItems();

	return iNumItems;
}

// $Barnyard: FUNCTION 006231c0
TINT AInventoryGroupComponent::GetNumItemsOfType( const TPString8& a_rcType )
{
	TINT iNumItems      = 0;
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
		iNumItems += m_pCategories[ i ].GetNumItemsOfType( a_rcType );

	return iNumItems;
}

// $Barnyard: FUNCTION 00623230
TINT AInventoryGroupComponent::TransferItems( const TPString8& a_rcType, AInventoryComponentInterface* a_pTarget, TINT a_iCount )
{
	TINT iNumLeft       = a_iCount;
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories && iNumLeft > 0; i++ )
	{
		// The items aren't moved between the categories
		if ( &m_pCategories[ i ] != a_pTarget )
			iNumLeft -= m_pCategories[ i ].TransferItems( a_rcType, a_pTarget, iNumLeft );
	}

	return a_iCount - iNumLeft;
}

// $Barnyard: FUNCTION 006241d0
TINT AInventoryGroupComponent::TransferItem( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount )
{
	return FindCategoryWithItem( a_pItem )->TransferItem( a_pItem, a_pTarget, a_iCount );
}

// $Barnyard: FUNCTION 006236d0
AInventoryComponent* AInventoryGroupComponent::GetCategory( const TPString8& a_rcName )
{
	if ( a_rcName.IsEmpty() )
		return &m_pCategories[ 0 ];

	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		if ( m_pData->GetCategory( i )->GetCategoryName() == a_rcName )
			return &m_pCategories[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00624030
AInventoryComponent* AInventoryGroupComponent::FindCategoryWithItem( AInventoryItem* a_pItem )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		if ( m_pCategories[ i ].ContainsItem( a_pItem ) )
			return &m_pCategories[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00624080
AInventoryComponent* AInventoryGroupComponent::FindCategoryWithType( const TPString8& a_rcType )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		if ( m_pCategories[ i ].ContainsType( a_rcType ) )
			return &m_pCategories[ i ];
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00624130
TBOOL AInventoryGroupComponent::ContainsItem( AInventoryItem* a_pItem )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		if ( m_pCategories[ i ].ContainsItem( a_pItem ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00624180
TBOOL AInventoryGroupComponent::ContainsUnit( AUnit* a_pUnit )
{
	TINT iNumCategories = GetNumCategories();

	for ( TINT i = 0; i < iNumCategories; i++ )
	{
		if ( m_pCategories[ i ].ContainsUnit( a_pUnit ) )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00624200
TBOOL AInventoryGroupComponent::SwapItemCategory( AInventoryItem* a_pItem, TINT a_iCount )
{
	AInventoryComponent* pCategory = FindCategoryWithItem( a_pItem );
	AObjectTraits*       pTraits   = TSTATICCAST( AObjectTraits, a_pItem->GetUnit()->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( !pTraits )
		return TFALSE;

	if ( !pCategory || pCategory->GetName() == pTraits->m_strInventoryForUse )
	{
		// The item is in use, one goes back to the store inventory
		if ( !pTraits->m_strInventoryForStore.IsEmpty() )
			return TransferItem( a_pItem, GetCategory( pTraits->m_strInventoryForStore ), 1 ) == 1;
	}
	else if ( !pTraits->m_strInventoryForUse.IsEmpty() )
	{
		return TransferItem( a_pItem, GetCategory( pTraits->m_strInventoryForUse ), a_iCount ) == a_iCount;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 006242d0
TBOOL AInventoryGroupComponent::MoveItemToUseCategory( AInventoryItem* a_pItem, TINT a_iCount )
{
	AInventoryComponent* pCategory = FindCategoryWithItem( a_pItem );
	AObjectTraits*       pTraits   = TSTATICCAST( AObjectTraits, a_pItem->GetUnit()->GetType()->FindStaticData( &TGetClass( AObjectTraits ) ) );

	if ( !pTraits )
		return TFALSE;

	// Nothing to do if the item has no use inventory or is already in it
	if ( pTraits->m_strInventoryForUse.IsEmpty() || ( pCategory && pCategory->GetName() == pTraits->m_strInventoryForUse ) )
		return TFALSE;

	return a_iCount == TransferItem( a_pItem, GetCategory( pTraits->m_strInventoryForUse ), a_iCount );
}

// $Barnyard: FUNCTION 006257b0
AUnit* AInventoryGroupComponent::DropSelectedItem()
{
	if ( m_pSelectedItem )
	{
		if ( ContainsItem( m_pSelectedItem ) )
		{
			AUnit* pUnit    = DropItem( m_pSelectedItem );
			m_pSelectedItem = TNULL;

			return pUnit;
		}

		m_pSelectedItem = TNULL;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 006264a0
TBOOL AInventoryGroupComponent::PutAwayHandItem()
{
	AAnimatedUnit*     pPlayer     = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	AAnimatableObject* pAnimObject = pPlayer->GetAnimatableObject();

	if ( pAnimObject->IsAnimationPlanned( TPS8( putaway ) ) || pAnimObject->IsAnimPlaying( TPS8( putaway ), 0.0f ) )
		return TFALSE;

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	AInventoryComponent*      pHand      = pInventory->GetCategory( TPS8D( "Hand" ) );

	if ( pHand->GetNumItems() <= 0 )
		return TFALSE;

	ACarryableUnitHelper* pHelper = pHand->GetItems().Begin()->GetUnit()->GetCarryableHelper();

	if ( pHelper )
	{
		// Items that have nowhere else to go stay in the hand
		if ( !pHelper->GetObjectTraits()->m_strInventoryForStore.GetPooledString() )
			return TFALSE;

		if ( pHelper->GetObjectTraits()->m_strInventoryForStore == pHelper->GetObjectTraits()->m_strInventoryForUse )
			return TFALSE;
	}

	pPlayer->GetAnimatableObject()->PlayAnimation( TPS8( putaway ) );
	return TTRUE;
}

// $Barnyard: FUNCTION 006265f0
TBOOL AInventoryGroupComponent::TakeOutItem( AInventoryItem* a_pItem )
{
	if ( !a_pItem )
		return TFALSE;

	AAnimatedUnit*     pPlayer     = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	AAnimatableObject* pAnimObject = pPlayer->GetAnimatableObject();

	if ( pAnimObject->IsAnimationPlanned( TPS8( getiteminv ) ) || pAnimObject->IsAnimPlaying( TPS8( getiteminv ), 0.0f ) )
		return TFALSE;

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	pInventory->SetSelectedItem( a_pItem );

	ShowTakenOutUnit( pInventory->DropSelectedItem() );
	return TTRUE;
}

// $Barnyard: FUNCTION 00626670
TBOOL AInventoryGroupComponent::TakeOutItem( AInventoryItem* a_pItem, AGameState* a_pState )
{
	if ( !a_pItem )
		return TFALSE;

	AAnimatedUnit*     pPlayer     = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	AAnimatableObject* pAnimObject = pPlayer->GetAnimatableObject();

	if ( pAnimObject->IsAnimationPlanned( TPS8( getiteminv ) ) || pAnimObject->IsAnimPlaying( TPS8( getiteminv ), 0.0f ) )
		return TFALSE;

	// Find (or create) the inventory screen to show the item in
	AGameStateController* pController     = AGameStateController::GetSingleton();
	AInventoryState2*     pInventoryState = TSTATICCAST( AInventoryState2, pController->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );

	if ( !pInventoryState )
	{
		AGameState* pState = a_pState;

		if ( !pState )
			pState = pController->FindGameStateOnStack( TPS8( ASimState ) );

		if ( !pState )
			pState = pController->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) );

		if ( pState )
		{
			pInventoryState = TSTATICCAST( AInventoryState2, pState->FindChildState( &TGetClass( AInventoryState2 ) ) );

			if ( !pInventoryState )
			{
				pInventoryState = new AInventoryState2();
				pState->AddChildState( pInventoryState );
			}
		}
		else
		{
			pController->AddChildStateByName( TPS8( AInventoryState2 ) );
			pInventoryState = TSTATICCAST( AInventoryState2, pController->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );
		}
	}

	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, pPlayer->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );
	pInventory->SetSelectedItem( a_pItem );

	pInventoryState->SelectType( a_pItem->GetUnit()->GetType(), TTRUE );

	ShowTakenOutUnit( pInventory->DropSelectedItem() );
	return TTRUE;
}

// $Barnyard: FUNCTION 00625c10
TBOOL AInventoryGroupComponent::CollectUnit( AUnit* a_pUnit, AGameState* a_pParentState )
{
	AAniEffectManager* pAniEffectManager = AAniEffectManager::GetSingleton();

	if ( !pAniEffectManager )
		return TFALSE;

	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );

	pUnit->SetHashed( TFALSE );
	pUnit->SetNoQuery( TTRUE );
	pAniEffectManager->RemoveUnitEffects( pUnit );
	pUnit->GetAnimatableObject()->SetOpacity( 1.0f, TTRUE );
	pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	if ( !pAniEffectManager->ReserveEffects( 2 ) )
		return TFALSE;

	AInventoryAniEffect* pInventoryEffect = pAniEffectManager->CreateEffect<AInventoryAniEffect>();
	AUnit*               pCollector       = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	// The cow riding the bike collects the unit
	if ( ABikeManager::GetSingleton()->IsPlayerOnBike() )
		pCollector = ABikeManager::GetSingleton()->GetRider();

	pInventoryEffect->SetUnit( pUnit );
	pInventoryEffect->SetCollector( pCollector );

	// Where the unit flies to on the screen
	// NOTE: retail leaves it uninitialised until it's set below
	TVector2 vScreenPos = TVector2::VEC_ZERO;

	const TPString8& rcTypeName = pUnit->GetType()->GetName();

	if ( rcTypeName == TPS8D( "buck_1" ) || rcTypeName == TPS8D( "buck_5" ) || rcTypeName == TPS8D( "buck_20" ) || rcTypeName == TPS8( precisioncoin ) )
	{
		// Pick ups fly to the counter
		ASimPlayerPickUpHudElement* pPickUpElement = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();
		pPickUpElement->SetNumber( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups() );
		pPickUpElement->GetIconScreenPosition( vScreenPos );
	}
	else
	{
		// Everything else flies to its category of the inventory screen
		AGameStateController* pController     = AGameStateController::GetSingleton();
		AInventoryState2*     pInventoryState = TSTATICCAST( AInventoryState2, pController->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );

		if ( !pInventoryState )
		{
			AGameState* pParentState = a_pParentState;

			if ( !pParentState )
				pParentState = pController->FindGameStateOnStack( TPS8( ASimState ) );

			if ( !pParentState )
				pParentState = pController->FindGameStateOnStack( TPS8( ABarnExploreMinigameState ) );

			if ( pParentState )
			{
				pInventoryState = TSTATICCAST( AInventoryState2, pParentState->FindChildState( &TGetClass( AInventoryState2 ) ) );

				if ( !pInventoryState )
				{
					pInventoryState = new AInventoryState2();
					pParentState->AddChildState( pInventoryState );
				}
			}
			else
			{
				pController->AddChildStateByName( TPS8( AInventoryState2 ) );
				pInventoryState = TSTATICCAST( AInventoryState2, pController->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) );
			}
		}

		pInventoryState->SelectType( pUnit->GetType(), TTRUE );
		pInventoryState->GetItemScreenPosition( vScreenPos );
	}

	ASwirlTrackHudAniEffect* pSwirlEffect = pAniEffectManager->CreateEffect<ASwirlTrackHudAniEffect>();

	TQuaternion qRotation;
	qRotation.SetFromEulerRollPitchYaw( TMath::PI / 2, 0.0f, 0.0f );
	pSwirlEffect->Setup( pUnit, vScreenPos, qRotation, 5.0f, 12.0f, 0.0f, 0.25f );

	AWaitAniEffect* pWaitEffect = pAniEffectManager->CreateEffect<AWaitAniEffect>();
	pWaitEffect->SetNextEffect( pSwirlEffect );
	pWaitEffect->SetTime( 0.1f );

	// NOTE: this makes the swirl effect active right away, so the wait effect does nothing
	pSwirlEffect->SetNextEffect( pInventoryEffect );
	pSwirlEffect->SetSpinSpeed( 5.0f );

	return TTRUE;
}

// $Barnyard: FUNCTION 00625f80
void AInventoryGroupComponent::ShowTakenOutUnit( AUnit* a_pUnit )
{
	AAniEffectManager* pAniEffectManager = AAniEffectManager::GetSingleton();

	if ( !pAniEffectManager )
		return;

	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pUnit );

	pAniEffectManager->RemoveUnitEffects( pUnit );
	pUnit->GetAnimatableObject()->SetOpacity( 1.0f, TTRUE );
	pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	if ( !pAniEffectManager->ReserveEffects( 4 ) )
		return;

	pUnit->SetHashed( TFALSE );
	pUnit->SetNoQuery( TTRUE );
	ACameraManager::GetSingleton()->Activate( TFALSE );

	// The unit goes to the "InventoryForUse" (the hand) at the end
	AInventoryAniEffect* pInventoryEffect = pAniEffectManager->CreateEffect<AInventoryAniEffect>();
	pInventoryEffect->SetTakeOut( TTRUE );

	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );
	pInventoryEffect->SetUnit( pUnit );
	pInventoryEffect->SetCollector( pPlayer );

	// The AInventoryAniEffect enables the controls again
	TSTATICCAST( AControlComponent, pPlayer->FindComponent( &TGetClass( APlayerControlComponent ) ) )->SetEnabled( TFALSE );

	AObjectTraits*        pTraits = TNULL;
	ACarryableUnitHelper* pHelper = pUnit->GetCarryableHelper();

	if ( pHelper )
	{
		pTraits = pHelper->GetObjectTraits();
		pHelper->SetDelay( 3.0f );

		// Use the animation set of the player matching the item
		AAnimatableObject*    pPlayerObject = pPlayer->GetAnimatableObject();
		ANamedAnimationSetRef pAnimationSet = pPlayerObject->GetObjectType()->FindAnimationSetOrDefault( pTraits->IsDetector() ? TPS8( detector ) : ( pTraits->IsHeavy() ? TPS8( carrylarge ) : TPS8( carrysmall ) ) );

		AToshiAnimationInterface& rAnimInterface = pPlayerObject->GetToshiAnimInterface();

		if ( rAnimInterface.GetAnimationSet().Get() != pAnimationSet.Get() )
			rAnimInterface.SetAnimationSet( pAnimationSet );
	}

	// Where the item is on the inventory screen
	TVector2 vScreenPos;
	TSTATICCAST( AInventoryState2, AGameStateController::GetSingleton()->GetCurrentState()->FindChildState( &TGetClass( AInventoryState2 ) ) )->GetItemScreenPosition( vScreenPos );

	ASwirlTrackHudAniEffect* pSwirlEffect = pAniEffectManager->CreateEffect<ASwirlTrackHudAniEffect>();

	// Place the unit in front of the camera at the item
	TVector4 vPosition;
	TVector4 vDirection;
	ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), vScreenPos, vPosition, vDirection );

	TVector4 vOffset;
	vOffset.Multiply( vDirection, 6.0f );
	vPosition.x += vOffset.x;
	vPosition.y += vOffset.y;
	vPosition.z += vOffset.z;
	pUnit->GetAnimatableObject()->GetModelInstance()->SetTranslation( vPosition.AsVector3() );

	// NOTE: retail computes the position 75 pixels lower too, without using it
	vScreenPos.y += 75.0f;
	ARenderer::GetSingleton()->GetScreenRay( ACameraManager::GetSingleton()->GetCurrentCamera(), vScreenPos, vPosition, vDirection );
	vOffset.Multiply( vDirection, 6.0f );
	vPosition.x += vOffset.x;
	vPosition.y += vOffset.y;
	vPosition.z += vOffset.z;

	TQuaternion qRotation;
	qRotation.SetFromEulerRollPitchYaw( TMath::PI / 2, 0.0f, 0.0f );
	pSwirlEffect->Setup( pUnit, vScreenPos, qRotation, 4.5f, 10.0f, 0.0f, 0.25f );

	// Then it flies into the hand of the player
	AHudTrackBoneAniEffect* pTrackEffect = pAniEffectManager->CreateEffect<AHudTrackBoneAniEffect>();
	pTrackEffect->Setup( pUnit, pPlayer, TPS8( cb_rlocator ), 20.0f, 0.0f );
	pTrackEffect->SetNextEffect( pInventoryEffect );
	pSwirlEffect->SetNextEffect( pTrackEffect );

	AWaitAniEffect* pWaitEffect = pAniEffectManager->CreateEffect<AWaitAniEffect>();
	pWaitEffect->SetNextEffect( pSwirlEffect );

	if ( pTraits )
		pWaitEffect->SetTime( ( !pTraits->IsDetector() && pTraits->IsHeavy() ) ? 0.2f : 0.05f );

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: retail reads m_fHoldScale of TNULL if the unit isn't carryable
	if ( pTraits )
#endif
		pTrackEffect->SetTargetScale( pTraits->m_fHoldScale );

	TINT iHandleBone = pUnit->GetAnimatableObject()->GetModelInstance()->GetBoneID( TPS8( handle ) );

	if ( iHandleBone != -1 )
		pTrackEffect->SetUnitBone( iHandleBone );
}
