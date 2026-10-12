#include "pch.h"
#include "AGiftActivity.h"
#include "AInteractionComponent.h"
#include "AUnit.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "AInventoryGroupComponent.h"
#include "AInteractionGiftEvent.h"
#include "AInteractionEventManager.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ABarnRewardState.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b7850, CreateObject 005b7f90, CreateObjectInPlace 005b8020
TDEFINE_CLASS( AGiftActivity );

// NOTE: inlined into CreateObject, retail leaves m_eReceiver and m_iUnk20 uninitialised
AGiftActivity::AGiftActivity()
    : m_iCount( 1 )
    , m_eReceiver( AInteractionSequence::CATEGORY_PRIMARY )
    , m_iScreenName( -1 )
    , m_uiFlags( FLAGS_NONE )
    , m_iUnk20( 0 )
{
}

// $Barnyard: FUNCTION 005b7920
// $Barnyard: FUNCTION 005b80a0
AGiftActivity::~AGiftActivity()
{
}

// $Barnyard: FUNCTION 005b79c0
TBOOL AGiftActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strReceiver;

	a_pProperties->GetOptionalPropertyValue( m_strItemType, TPS8D( "itemtype" ) );
	a_pProperties->GetOptionalPropertyValue( m_strInventoryCategory, TPS8D( "inventorycatagory" ) );

	const PBPropertyValue* pScreenName = a_pProperties->GetProperty( "screenname" );

	if ( pScreenName )
		m_iScreenName = TINT( pScreenName->GetLocaleString().uiIndex );

	a_pProperties->GetOptionalPropertyValue( m_iCount, TPS8D( "count" ) );
	a_pProperties->GetOptionalPropertyValue( strReceiver, TPS8D( "receiver" ) );

	TBOOL bGenItem = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bGenItem, TPS8D( "genitem" ) );

	if ( bGenItem )
		m_uiFlags |= FLAGS_GENITEM;

	TBOOL bShowReward = TFALSE;
	a_pProperties->GetOptionalPropertyValue( bShowReward, TPS8D( "showreward" ) );

	if ( bShowReward )
		m_uiFlags |= FLAGS_SHOWREWARD;

	m_eReceiver = AInteractionSequence::GetCategoryFromName( strReceiver.GetString() );

	return AInteractionActivity::Create( a_pProperties );
}

// NOTE: folded with the GetDataSize of another activity (005b5d60)
void AGiftActivity::GetDataSize( TINT* a_pSize )
{
	// AGameState*, the item reward state the activity pushed
	*a_pSize = 4;
}

// $Barnyard: FUNCTION 005b7900
AInteractionActivity::RESULT AGiftActivity::OnStart( AInteractionSequence* a_pSequence )
{
	*TSTATICCAST( AGameState*, a_pSequence->GetActivityData() ) = TNULL;

	return RESULT_DONE;
}

TPSTRING8_DECLARE( cookbook );
TPSTRING8_DECLARE( cocktails );
TPSTRING8_DECLARE( scorebook );
TPSTRING8_DECLARE( sunglasses );
TPSTRING8_DECLARE( barn_gate_key );
TPSTRING8_DECLARE( bikekey );
TPSTRING8_DECLARE( honey_gate_key );
TPSTRING8_DECLARE( southpath_gate_key );
TPSTRING8_DECLARE( woods_gate_key );
TPSTRING8_DECLARE( metaldetector );
TPSTRING8_DECLARE( golf_gate_key );
TPSTRING8_DECLARE( oldmill_gate_key );
TPSTRING8_DECLARE( northpath_gate_key );
TPSTRING8_DECLARE( junkyard_gate_key );
TPSTRING8_DECLARE( gulch_gate_key );
TPSTRING8_DECLARE( goldrush_token );

// The items with a reward screen (0x00775260), with the sim loading picture they unlock
struct RewardItem
{
	Toshi::TPString8*      pName;
	TINT                   iPicture;
	ABarnRewardState::TYPE eType;
};

static RewardItem s_aRewardItems[] = {
	{ &TPS8( cookbook ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( cocktails ), 11, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( scorebook ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( sunglasses ), 12, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( barn_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( bikekey ), 5, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( honey_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( southpath_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( woods_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( metaldetector ), 8, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( golf_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( oldmill_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( northpath_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( junkyard_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( gulch_gate_key ), -1, ABarnRewardState::TYPE_ITEM },
	{ &TPS8( goldrush_token ), 6, ABarnRewardState::TYPE_TOKEN },
};

// Pushes the reward screen of m_strItemType
// NOTE: inlined twice into Update (005b7c20)
AInteractionActivity::RESULT AGiftActivity::PushRewardState( AGameState** a_ppRewardState )
{
	// NOTE: retail never compares the first item (cookbook), any item that isn't in the table gets its reward screen
	// (with no picture), so the "not found" check (-1) is dead code
	TINT i = TARRAYSIZE( s_aRewardItems ) - 1;

	while ( i > 0 && *s_aRewardItems[ i ].pName != m_strItemType )
		i--;

	if ( i <= -1 )
		return RESULT_DONE;

	*a_ppRewardState = new ABarnRewardState( s_aRewardItems[ i ].eType, m_strItemType, s_aRewardItems[ i ].iPicture );
	AGameStateController::GetSingleton()->PushState( *a_ppRewardState );

	return RESULT_RUNNING;
}

// $Barnyard: FUNCTION 005b7c20
AInteractionActivity::RESULT AGiftActivity::Update( AInteractionSequence* a_pSequence )
{
	AGameState** ppRewardState = TSTATICCAST( AGameState*, a_pSequence->GetActivityData() );

	// Wait for the reward state to be closed
	if ( *ppRewardState )
	{
		if ( AGameStateController::GetSingleton()->GetCurrentState() == *ppRewardState )
			return RESULT_RUNNING;

		*ppRewardState = TNULL;
		return RESULT_DONE;
	}

	if ( ( m_uiFlags & FLAGS_SHOWREWARD ) && !( m_uiFlags & FLAGS_GENITEM ) && m_eReceiver == m_eCategory )
	{
		// The category receives the items itself, only the reward is shown
		return PushRewardState( ppRewardState );
	}

	if ( !( m_uiFlags & FLAGS_GENITEM ) && HasEnoughItems( a_pSequence ) != TTRUE )
		return RESULT_DONE;

	AUnit*                        pGiver    = TNULL;
	AUnit*                        pReceiver = a_pSequence->GetUnit( m_eReceiver );
	AInventoryComponentInterface* pTarget   = AInventoryComponentInterface::GetInventory( pReceiver );

	// The items go to the category of the receiver, if specified
	if ( pTarget && !m_strInventoryCategory.IsEmpty() )
		pTarget = TSTATICCAST( AInventoryGroupComponent, pTarget )->GetCategory( m_strInventoryCategory );

	AInventoryItem* pReceivedItem = TNULL;

	if ( !( m_uiFlags & FLAGS_GENITEM ) )
	{
		// The units of the category give their items
		TINT iNumGiven = 0;

		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AUnit*                        pOwner     = it->GetOwner();
			AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pOwner );

			if ( pInventory )
			{
				TINT iPrevNumGiven = iNumGiven;
				iNumGiven += pInventory->TransferItems( m_strItemType, pTarget, m_iCount - iNumGiven );

				if ( iPrevNumGiven != iNumGiven && !pGiver )
				{
					pGiver = pOwner;

					if ( iNumGiven >= m_iCount )
						break;
				}
			}
		}
	}
	else
	{
		// The items are created
		if ( pTarget )
		{
			pTarget->AddItems( m_strItemType, m_iCount, AInventoryComponentInterface::ms_strDefaultInventory );
			pReceivedItem = pTarget->FindItemByType( m_strItemType );
		}

		pGiver = a_pSequence->GetUnit( m_eReceiver != AInteractionSequence::CATEGORY_SECONDARY );
	}

	if ( pGiver )
	{
		// Lets the units know about the gift
		AInteractionGiftEvent* pEvent = TSTATICCAST( AInteractionGiftEvent, AInteractionEventManager::GetSingleton()->NewActiveEvent( &TGetClass( AInteractionGiftEvent ) ) );
		pEvent->SetGiver( pGiver );
		pEvent->SetReceiver( pReceiver );
		pEvent->SetGiftType( m_strItemType );
		pEvent->SetCount( m_iCount );
	}

	if ( m_iScreenName > 0 )
		AUnitManager::GetSingleton()->FindUnitByTypeName( m_strItemType )->GetType()->SetScreenNameId( m_iScreenName );

	if ( !( m_uiFlags & FLAGS_SHOWREWARD ) || !pReceivedItem )
		return RESULT_DONE;

	return PushRewardState( ppRewardState );
}

// $Barnyard: FUNCTION 005b7860
TBOOL AGiftActivity::HasEnoughItems( AInteractionSequence* a_pSequence )
{
	AUnitType* pItemType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( m_strItemType );
	TINT       iNumItems = 0;

	for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
	{
		AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( it->GetOwner() );

		if ( pInventory )
			iNumItems += pInventory->GetNumItemsOfType( pItemType->GetName() );
	}

	return m_iCount <= iNumItems;
}
