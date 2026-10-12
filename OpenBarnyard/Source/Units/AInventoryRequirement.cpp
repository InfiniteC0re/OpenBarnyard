#include "pch.h"
#include "AInventoryRequirement.h"
#include "MiniGame/AGopherShopInterface.h"
#include "AInventoryComponentInterface.h"
#include "AInventoryGroupComponent.h"
#include "AInventoryItem.h"
#include "AUnit.h"
#include "AUnitType.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"

#include <Toshi/TStringManager.h>

// NOTE: the physics headers pull in Opcode's PI macro, keep them last
#include "Helpers/AGateManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005ac3f0, CreateObject 005accb0
// NOTE: GetClass 005ac3f0, CreateObject 005accb0, CreateObjectInPlace 005acd10
TDEFINE_CLASS( AInventoryRequirement );

TPSTRING8_DECLARE( money );
TPSTRING8_DECLARE( all );
TPSTRING8_DECLARE( music );
TPSTRING8_DECLARE( special_gate_key );
TPSTRING8_DECLARE( barn_gate_key );
TPSTRING8_DECLARE( southpath_gate_key );
TPSTRING8_DECLARE( woods_gate_key );
TPSTRING8_DECLARE( golf_gate_key );
TPSTRING8_DECLARE( honey_gate_key );
TPSTRING8_DECLARE( junkyard_gate_key );
TPSTRING8_DECLARE( oldmill_gate_key );
TPSTRING8_DECLARE( gulch_gate_key );
TPSTRING8_DECLARE( nightbarn_gate_key );
TPSTRING8_DECLARE( Hand );
TPSTRING8_DECLARE( BackPack );
TPSTRING8_DECLARE( Equip );

// $Barnyard: FUNCTION 005ac7b0
AInventoryRequirement::AInventoryRequirement()
    : m_iItemId( -1 )
    , m_iCount( -1 )
    , m_oCompare( AInteractionComparison::OPERATOR_ALL )
    , m_uiFlags( FLAGS_NONE )
{
	m_uiFlags |= FLAGS_INVENTORY;
}

// $Barnyard: FUNCTION 005ac820
// $Barnyard: FUNCTION 005acd50
AInventoryRequirement::~AInventoryRequirement()
{
}

// $Barnyard: FUNCTION 005ac8c0
TBOOL AInventoryRequirement::Create( const PBProperties* a_pProperties )
{
	TPString8 strValue;

	if ( !a_pProperties->GetOptionalPropertyValue( m_strType, TPS8D( "itemtype" ) ) )
		a_pProperties->GetOptionalPropertyValue( m_iItemId, TPS8D( "itemid" ) );

	if ( m_strType == TPS8D( "none" ) )
		m_uiFlags |= FLAGS_ANY_TYPE;

	if ( a_pProperties->GetOptionalPropertyValue( m_iCount, TPS8D( "count" ) ) )
		m_oCompare.SetOperator( AInteractionComparison::OPERATOR_EQUAL );

	if ( a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "search" ) ) == TTRUE )
	{
		const TCHAR* szSearch = strValue.GetString();

		if ( strValue == TPS8D( "shop" ) )
			m_uiFlags |= FLAGS_SHOP;
		else if ( TStringManager::String8Compare( szSearch, "photos" ) == 0 )
			; // NOTE: nothing is set for the photos
		else if ( TStringManager::String8Compare( szSearch, "hand" ) == 0 )
			m_uiFlags |= FLAGS_INVENTORY | FLAGS_HAND;
		else if ( TStringManager::String8Compare( szSearch, "backpack" ) == 0 )
			m_uiFlags |= FLAGS_INVENTORY | FLAGS_BACKPACK;
		else if ( TStringManager::String8Compare( szSearch, "equip" ) == 0 )
			m_uiFlags |= FLAGS_INVENTORY | FLAGS_EQUIP;
		else if ( TStringManager::String8Compare( szSearch, "playerhand" ) == 0 )
			m_uiFlags |= FLAGS_PLAYER | FLAGS_PLAYER_HAND;
		else if ( TStringManager::String8Compare( szSearch, "playerbackpack" ) == 0 )
			m_uiFlags |= FLAGS_PLAYER | FLAGS_PLAYER_BACKPACK;
		else if ( TStringManager::String8Compare( szSearch, "playerequip" ) == 0 )
			m_uiFlags |= FLAGS_PLAYER | FLAGS_PLAYER_EQUIP;
		else if ( TStringManager::String8Compare( szSearch, "playerinventory" ) == 0 )
			m_uiFlags |= FLAGS_PLAYER;
		else
			m_uiFlags |= FLAGS_INVENTORY;
	}

	if ( a_pProperties->GetOptionalPropertyValue( strValue, TPS8D( "compare" ) ) )
		m_oCompare.SetOperator( strValue );

	return AInteractionRequirement::Create( a_pProperties );
}

AInventoryComponentInterface* AInventoryRequirement::GetCategory( AInventoryComponentInterface* a_pInventory, FLAGS a_uiHand, FLAGS a_uiBackPack, FLAGS a_uiEquip ) const
{
	const TPString8* pCategory;

	if ( m_uiFlags & a_uiHand )
		pCategory = &TPS8( Hand );
	else if ( m_uiFlags & a_uiBackPack )
		pCategory = &TPS8( BackPack );
	else if ( m_uiFlags & a_uiEquip )
		pCategory = &TPS8( Equip );
	else
		return a_pInventory;

	if ( !a_pInventory->GetClass()->IsA( &TGetClass( AInventoryGroupComponent ) ) )
		return a_pInventory;

	return TSTATICCAST( AInventoryGroupComponent, a_pInventory )->GetCategory( *pCategory );
}

// $Barnyard: FUNCTION 005ac400
void AInventoryRequirement::Evaluate( AInteractionParticipants* a_pParticipants )
{
	TINT iNumUnits = a_pParticipants->iNumUnits;

	if ( m_strType == TPS8( money ) )
	{
		// The player's money
		TBOOL bAllowed = m_oCompare.Compare( APlayerProfileManager::GetSingleton()->GetMainProfile()->GetDisplayedPickups(), m_iCount );

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			if ( a_pParticipants->Get( i )->IsPlayer() == TTRUE )
				SetUnitAllowed( m_eCategory, i, bAllowed );
		}

		return;
	}

	if ( m_uiFlags & FLAGS_SHOP )
	{
		// The purchases from the gopher shop
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			TBOOL bAllowed;

			if ( m_strType == TPS8( all ) )
			{
				// The percentage of the shop's items that were bought
				bAllowed = m_oCompare.Compare( AGopherShopInterface::GetPercentBought(), m_iCount );
			}
			else if ( m_strType == TPS8( music ) )
			{
				bAllowed = TFALSE;
			}
			else
			{
				// The number of items with the id that were bought
				bAllowed = m_oCompare.Compare( AGopherShopInterface::GetNumBought( m_iItemId ), m_iCount );
			}

			if ( a_pParticipants->Get( i )->IsPlayer() == TTRUE )
				SetUnitAllowed( m_eCategory, i, bAllowed );
		}

		return;
	}

	if ( m_uiFlags & FLAGS_UNK1 )
		return;

	if ( m_uiFlags & FLAGS_PLAYER )
	{
		// The player's inventory
		TBOOL                         bAllowed   = TFALSE;
		AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( APlayerManager::GetSingleton()->GetPlayerUnit( 0 ) );

		if ( pInventory )
		{
			pInventory = GetCategory( pInventory, FLAGS_PLAYER_HAND, FLAGS_PLAYER_BACKPACK, FLAGS_PLAYER_EQUIP );

			TINT iNumItems = 0;

			if ( m_uiFlags & FLAGS_ANY_TYPE )
			{
				iNumItems = pInventory->GetNumItems();
			}
			else if ( TPS8( special_gate_key ) == m_strType )
			{
				// Whether the player has the key of the current gate
				// The keys (PTR_g_str_barn_gate_key_007786d0) and the screen names of the keys of the gate types (DAT_007323dc)
				static const TPString8* s_apKeyTypes[] = {
					&TPS8( barn_gate_key ),
					&TPS8( southpath_gate_key ),
					&TPS8( woods_gate_key ),
					&TPS8( golf_gate_key ),
					&TPS8( honey_gate_key ),
					&TPS8( junkyard_gate_key ),
					&TPS8( oldmill_gate_key ),
					&TPS8( gulch_gate_key ),
					&TPS8( nightbarn_gate_key ),
				};

				static const TINT s_aiKeyScreenNames[ AGateManager::GATETYPE_NUMOF ] = { 3335, 3334, 3326, 3332, 3331, 3330, 3329, 3328, 3327 };

				// NOTE: retail doesn't check if AGateManager exists
				AGateManager::Gate* pPadlock = AGateManager::GetSingleton()->GetNearbyPadlock();

				if ( pPadlock )
				{
					for ( TINT i = 0; i < TARRAYSIZE( s_apKeyTypes ); i++ )
					{
						AInventoryItem* pItem = pInventory->FindItemByType( *s_apKeyTypes[ i ] );

						if ( pItem && pItem->GetUnit() && pItem->GetUnit()->GetType() )
						{
							TINT iScreenNameId = pItem->GetUnit()->GetType()->GetScreenNameId();

							// 3307: the key that opens every gate
							if ( iScreenNameId == s_aiKeyScreenNames[ pPadlock->eType ] || iScreenNameId == 3307 )
							{
								iNumItems = 1;
								break;
							}
						}
					}
				}
			}
			else
			{
				iNumItems = pInventory->GetNumItemsOfType( m_strType );
			}

			bAllowed = m_oCompare.Compare( iNumItems, m_iCount );
		}

		// NOTE: every role is set, "category" is ignored
		if ( m_bInvert == TTRUE )
			bAllowed = !bAllowed;

		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_PRIMARY, i, bAllowed );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_SECONDARY, i, bAllowed );
			m_oRoles.SetUnit( AInteractionSequence::CATEGORY_AUXILIARY, i, bAllowed );
		}
	}
	else if ( m_uiFlags & FLAGS_INVENTORY )
	{
		// The unit's inventory
		for ( TINT i = 0; i < iNumUnits; i++ )
		{
			TBOOL                         bAllowed   = TFALSE;
			AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( a_pParticipants->Get( i ) );

			if ( pInventory )
			{
				pInventory = GetCategory( pInventory, FLAGS_HAND, FLAGS_BACKPACK, FLAGS_EQUIP );

				TINT iNumItems = ( m_uiFlags & FLAGS_ANY_TYPE ) ? pInventory->GetNumItems() : pInventory->GetNumItemsOfType( m_strType );
				bAllowed       = m_oCompare.Compare( iNumItems, m_iCount );
			}

			SetUnitAllowed( m_eCategory, i, bAllowed );
		}
	}
}
