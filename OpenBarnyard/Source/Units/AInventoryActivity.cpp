#include "pch.h"
#include "AInventoryActivity.h"
#include "Helpers/ACoinSpawner.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "AUnitManager.h"
#include "AUnitFactory.h"
#include "AUnitType.h"
#include "AAnimatedUnit.h"
#include "AInventoryGroupComponent.h"
#include "ACarryableUnitHelper.h"
#include "Animation/AAnimatableObject.h"
#include "UnitMovement/Steering/ASteer.h"
#include "FX/AAniEffectManager.h"
#include "FX/ATrackBoneAniEffect.h"
#include "FX/AInventoryAniEffect.h"
#include "Helpers/AGateManager.h"
#include "AInventoryItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005b8800, CreateObject 005b98f0, CreateObjectInPlace 005b9970
TDEFINE_CLASS( AInventoryActivity );

TPSTRING8_DECLARE( other );
TPSTRING8_DECLARE( putaway );
TPSTRING8_DECLARE( Hand );
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

// NOTE: inlined into CreateObject, retail leaves m_eItemCategory and m_fMaxRadius uninitialised
AInventoryActivity::AInventoryActivity()
    : m_pItemType( TNULL )
    , m_iCount( 1 )
    , m_uiFlags( FLAGS_NONE )
    , m_eItemCategory( AInteractionSequence::CATEGORY_PRIMARY )
    , m_fMaxRadius( 0.0f )
{
}

// $Barnyard: FUNCTION 005b8940
// $Barnyard: FUNCTION 005b99f0
AInventoryActivity::~AInventoryActivity()
{
}

// $Barnyard: FUNCTION 005b89e0
TBOOL AInventoryActivity::Create( const PBProperties* a_pProperties )
{
	TPString8 strAction;

	TBOOL bHasItemType = a_pProperties->GetOptionalPropertyValue( m_strItemType, TPS8D( "itemtype" ) );
	m_pItemType        = TNULL;

	if ( bHasItemType && m_strItemType != TPS8( other ) )
		m_pItemType = AUnitManager::GetSingleton()->GetUnitFactory()->Find( m_strItemType );

	a_pProperties->GetOptionalPropertyValue( m_iCount, TPS8D( "count" ) );
	a_pProperties->GetOptionalPropertyValue( strAction, TPS8D( "action" ) );

	if ( strAction == TPS8D( "spillmoney" ) )
	{
		m_uiFlags |= FLAGS_SPILLMONEY;
	}
	else if ( strAction == TPS8D( "spillmoneyrand" ) )
	{
		m_uiFlags |= FLAGS_SPILLMONEYRAND;
	}
	else if ( strAction == TPS8D( "putawaycarry" ) )
	{
		m_uiFlags |= FLAGS_PUTAWAYCARRY;

		if ( m_pItemType )
			a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8D( "maxradius" ) );
		else
			m_fMaxRadius = 0.0f;
	}
	else if ( strAction == TPS8D( "add" ) )
	{
		m_uiFlags |= FLAGS_ADD;

		// NOTE: retail reads "itemcategory" into the action string, so without it the category of "add" is looked up
		a_pProperties->GetOptionalPropertyValue( strAction, TPS8D( "itemcategory" ) );
		m_eItemCategory = AInteractionSequence::GetCategoryFromName( strAction.GetString() );
	}
	else if ( strAction == TPS8D( "remove" ) )
	{
		m_uiFlags |= FLAGS_REMOVE;
	}
	else if ( strAction == TPS8D( "delete" ) )
	{
		m_uiFlags |= FLAGS_DELETE;
	}
	else if ( strAction == TPS8D( "deleteall" ) )
	{
		m_uiFlags |= FLAGS_DELETEALL;
	}
	else if ( strAction == TPS8D( "pickupdirect" ) || strAction == TPS8D( "pickup" ) || strAction == TPS8D( "pickupfollow" ) )
	{
		if ( strAction == TPS8D( "pickupdirect" ) )
		{
			m_uiFlags |= FLAGS_PICKUPDIRECT;
		}
		else if ( strAction == TPS8D( "pickup" ) )
		{
			m_uiFlags |= FLAGS_PICKUP;
		}
		else if ( strAction == TPS8D( "pickupfollow" ) )
		{
			m_uiFlags |= FLAGS_PICKUPFOLLOW;
			a_pProperties->GetOptionalPropertyValue( m_strBone, TPS8D( "bone" ) );
		}

		if ( m_pItemType )
			a_pProperties->GetOptionalFloatOrIntValue( m_fMaxRadius, TPS8D( "maxradius" ) );
		else
			m_fMaxRadius = 0.0f;
	}

	TBOOL bResult = AInteractionActivity::Create( a_pProperties );

	// The added items need a single unit
	if ( ( m_uiFlags & FLAGS_ADD ) && m_eCategory == AInteractionSequence::CATEGORY_ALL )
		bResult = TFALSE;

	return bResult;
}

// Closest unit of the type within a_fRadius of a_rcPosition, invisible units are skipped
// $Barnyard: FUNCTION 005b8810
static AUnit* FindClosestUnitOfType( const TVector4& a_rcPosition, AUnitType* a_pType, TFLOAT a_fRadius )
{
	AUnit* pClosest       = TNULL;
	TFLOAT fClosestDistSq = TMath::MAXFLOAT;
	TINT   iNumUnits      = 0;

	AUnitHash::QueryResult* pResults = AUnitManager::GetSingleton()->FindUnits( a_rcPosition, a_fRadius, &iNumUnits, TFALSE, 420, TFALSE, TNULL );

	for ( TUINT i = 0; i < TUINT( iNumUnits ); i++ )
	{
		AUnit* pUnit = pResults[ i ].pUnit;

		if ( pUnit->GetType() != a_pType )
			continue;

		TFLOAT fDistSq = TVector4::DistanceSq( *pUnit->GetSteer()->GetPosition(), a_rcPosition );

		if ( !pClosest || fDistSq < fClosestDistSq )
		{
			if ( !pUnit->GetClass()->IsA( &TGetClass( AVisibleUnit ) ) || TSTATICCAST( AVisibleUnit, pUnit )->IsVisible() )
			{
				pClosest       = pUnit;
				fClosestDistSq = fDistSq;
			}
		}
	}

	return pClosest;
}

// $Barnyard: FUNCTION 005b8fa0
AInteractionActivity::RESULT AInventoryActivity::Update( AInteractionSequence* a_pSequence )
{
	if ( ( m_uiFlags & FLAGS_SPILLMONEY ) || ( m_uiFlags & FLAGS_SPILLMONEYRAND ) )
	{
		// Throws money of the player around him
		APlayerSimProfile* pProfile = APlayerProfileManager::GetSingleton()->GetMainProfile();
		TINT               iMoney   = pProfile->GetDisplayedPickups();

		if ( iMoney >= 0 )
		{
			TINT iCount = m_iCount;

			// "spillmoneyrand" throws half of the money (5 if there's little, 300 at most)
			if ( !( m_uiFlags & FLAGS_SPILLMONEY ) && ( m_uiFlags & FLAGS_SPILLMONEYRAND ) )
			{
				if ( iMoney < 26 )
				{
					iCount = 5;
				}
				else
				{
					iCount = iMoney / 2;

					if ( iCount > 300 )
						iCount = 300;
				}
			}

			if ( iMoney < iCount )
				iCount = iMoney;

			TVector4 vecPosition = *APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->GetSteer()->GetPosition();
			vecPosition.y -= 1.8f;

			TPString8 aTypes[ 3 ];
			aTypes[ 0 ] = TPS8D( "buck_1" );
			aTypes[ 1 ] = TPS8D( "buck_5" );
			aTypes[ 2 ] = TPS8D( "buck_20" );

			pProfile->AddPickups( -iCount );

			// The number of coins of each type
			TINT aNumCoins[ 3 ] = { iCount, 0, 0 };

			if ( iCount > 19 )
			{
				aNumCoins[ 1 ] = iCount / 5;
				aNumCoins[ 0 ] = iCount % 5;

				if ( aNumCoins[ 1 ] > 19 )
				{
					aNumCoins[ 2 ] = aNumCoins[ 1 ] / 4;
					aNumCoins[ 1 ] = aNumCoins[ 1 ] % 4;
				}
			}

			for ( TINT i = 0; i < 3; i++ )
			{
				for ( TINT k = aNumCoins[ i ]; k > 0; k-- )
					ACoinSpawner::SpawnCoin( aTypes[ i ], vecPosition );
			}
		}
		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_PUTAWAYCARRY )
	{
		AUnit* pUnit = a_pSequence->GetUnit( m_eCategory );

		if ( pUnit->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
		{
			AUnit* pItemUnit = TNULL;

			if ( m_pItemType )
				pItemUnit = FindClosestUnitOfType( *pUnit->GetSteer()->GetPosition(), m_pItemType, m_fMaxRadius );
			else if ( m_strItemType == TPS8( other ) )
				pItemUnit = a_pSequence->GetUnit( m_eCategory == AInteractionSequence::CATEGORY_PRIMARY );

			// Wait for the end of the previous put away
			if ( !TSTATICCAST( AAnimatedUnit, pUnit )->GetAnimatableObject()->IsAnimationFinished( TPS8( putaway ) ) )
				return RESULT_RUNNING;

			AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, AInventoryComponentInterface::GetInventory( pUnit ) );
			AInventoryComponent*      pHand      = pInventory->GetCategory( TPS8( Hand ) );

			if ( pHand )
			{
				TINT iNumItems = pHand->GetNumItems();

				if ( iNumItems > 0 )
				{
					AUnit* pHandUnit = pHand->GetItem( 0 )->GetUnit();
					TINT   iMaxCarry = 1;

					// The hand is full once it carries as many items of the type as it can
					if ( pHandUnit && pItemUnit && pHandUnit->GetType() == pItemUnit->GetType() )
					{
						if ( ACarryableUnitHelper* pHelper = pHandUnit->GetCarryableHelper() )
							iMaxCarry = pHelper->GetMaxCarry();
					}

					if ( iNumItems >= iMaxCarry )
						return AInventoryGroupComponent::PutAwayHandItem() ? RESULT_RUNNING : RESULT_DONE;
				}
			}
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_DELETEALL )
	{
		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( it->GetOwner() );
			TINT                          iNumItems  = pInventory->GetNumItemsOfType( m_strItemType );

			if ( iNumItems > 0 )
				pInventory->RemoveItems( m_strItemType, iNumItems, TTRUE );
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & ( FLAGS_REMOVE | FLAGS_DELETE ) )
	{
		TBOOL bDestroy = ( m_uiFlags & FLAGS_DELETE ) != 0;

		for ( auto it = a_pSequence->GetCategoryBegin( m_eCategory ); it != a_pSequence->GetCategoryEnd( m_eCategory ); it++ )
		{
			AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( it->GetOwner() );

			if ( pInventory )
			{
				if ( m_strItemType == TPS8( special_gate_key ) )
				{
					// The special key also takes the key of the gate the player is at
					// The keys (PTR_g_str_barn_gate_key_007786d0) and the screen names of the keys of the gate types (DAT_007323dc)
					// NOTE: the same tables are used by AInventoryRequirement
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

					AGateManager::Gate* pPadlock = AGateManager::GetSingleton()->GetNearbyPadlock();

					if ( pPadlock || ( pPadlock = AGateManager::GetSingleton()->FindNearbyPadlock( 16.0f ) ) )
					{
						TINT iKeyScreenName = s_aiKeyScreenNames[ pPadlock->eType ];

						for ( TINT i = 0; i < TARRAYSIZE( s_apKeyTypes ); i++ )
						{
							AInventoryItem* pItem = pInventory->FindItemByType( *s_apKeyTypes[ i ] );

							if ( pItem )
							{
								AUnit* pKey = pItem->GetUnit();

								if ( pKey && pKey->GetType()->GetScreenNameId() == iKeyScreenName )
								{
									pInventory->RemoveItems( pKey->GetType()->GetName(), m_iCount, bDestroy );
									break;
								}
							}
						}
					}
				}

				pInventory->RemoveItems( m_strItemType, m_iCount, bDestroy );
			}
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_ADD )
	{
		// The units of m_eItemCategory are added to the inventory of the unit
		AUnit*                        pUnit      = a_pSequence->GetUnit( m_eCategory );
		AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pUnit );

		if ( pInventory )
		{
			for ( auto it = a_pSequence->GetCategoryBegin( m_eItemCategory ); it != a_pSequence->GetCategoryEnd( m_eItemCategory ); it++ )
			{
				AUnit* pItemUnit = it->GetOwner();

				if ( pItemUnit != pUnit )
					pInventory->AddUnit( pItemUnit, 1, AInventoryComponentInterface::ms_strDefaultInventory );
			}
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_PICKUPFOLLOW )
	{
		AUnit* pUnit = a_pSequence->GetUnit( m_eCategory );

		if ( AInventoryComponentInterface::GetInventory( pUnit ) )
		{
			AUnit* pItemUnit;

			if ( m_pItemType )
				pItemUnit = FindClosestUnitOfType( *pUnit->GetSteer()->GetPosition(), m_pItemType, m_fMaxRadius );
			else if ( m_strItemType == TPS8( other ) )
				pItemUnit = a_pSequence->GetUnit( m_eCategory == AInteractionSequence::CATEGORY_PRIMARY );
			else
				return RESULT_DONE;

			if ( pItemUnit )
			{
				// The item flies to the bone of the unit
				ATrackBoneAniEffect* pTrackEffect     = AAniEffectManager::GetSingleton()->CreateEffect<ATrackBoneAniEffect>();
				AInventoryAniEffect* pInventoryEffect = AAniEffectManager::GetSingleton()->CreateEffect<AInventoryAniEffect>();

				pItemUnit->SetHashed( TFALSE );
				pItemUnit->SetNoQuery( TTRUE );

				pInventoryEffect->SetUnit( pItemUnit );
				pInventoryEffect->SetCollector( pUnit );
				pTrackEffect->Setup( pItemUnit, pUnit, m_strBone, 2.5f, 1.0f );
				pTrackEffect->SetNextEffect( pInventoryEffect );
			}
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_PICKUPDIRECT )
	{
		AUnit*                        pUnit      = a_pSequence->GetUnit( m_eCategory );
		AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pUnit );

		if ( pInventory )
		{
			if ( m_pItemType )
			{
				AUnit* pItemUnit = FindClosestUnitOfType( *pUnit->GetSteer()->GetPosition(), m_pItemType, m_fMaxRadius );

				if ( pItemUnit )
					pInventory->AddUnit( pItemUnit, 1, AInventoryComponentInterface::ms_strDefaultInventory );
			}
			else if ( m_strItemType == TPS8( other ) )
			{
				pInventory->AddUnit( a_pSequence->GetUnit( m_eCategory == AInteractionSequence::CATEGORY_PRIMARY ), 1, AInventoryComponentInterface::ms_strDefaultInventory );
			}
			else
			{
				pInventory->AddItems( m_strItemType, m_iCount, AInventoryComponentInterface::ms_strDefaultInventory );
			}
		}

		return RESULT_DONE;
	}

	if ( m_uiFlags & FLAGS_PICKUP )
	{
		AUnit*                        pItemUnit  = TNULL;
		AUnit*                        pUnit      = a_pSequence->GetUnit( m_eCategory );
		AInventoryComponentInterface* pInventory = AInventoryComponentInterface::GetInventory( pUnit );

		if ( m_strItemType == TPS8( other ) )
		{
			pItemUnit = a_pSequence->GetUnit( m_eCategory == AInteractionSequence::CATEGORY_PRIMARY );
		}
		else if ( m_pItemType )
		{
			pItemUnit = FindClosestUnitOfType( *pUnit->GetSteer()->GetPosition(), m_pItemType, m_fMaxRadius );

			if ( !pItemUnit )
				return RESULT_FINISH_SEQUENCE;
		}

		if ( pUnit->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
		{
			if ( !pItemUnit )
			{
				if ( pInventory )
					pInventory->AddItems( m_strItemType, m_iCount, AInventoryComponentInterface::ms_strDefaultInventory );

				return RESULT_DONE;
			}

			if ( !pItemUnit->GetClass()->IsA( &TGetClass( AAnimatedUnit ) ) )
			{
				if ( pInventory )
					pInventory->AddUnit( pItemUnit, 1, AInventoryComponentInterface::ms_strDefaultInventory );

				return RESULT_DONE;
			}

			// An animated item goes to the hand
			pInventory->AddUnit( pItemUnit, 1, TPS8( Hand ) );
			return RESULT_DONE;
		}

		if ( !pInventory )
			return RESULT_DONE;

		if ( !pItemUnit )
		{
			pInventory->AddItems( m_strItemType, m_iCount, AInventoryComponentInterface::ms_strDefaultInventory );
			return RESULT_DONE;
		}

		pInventory->AddUnit( pItemUnit, 1, AInventoryComponentInterface::ms_strDefaultInventory );
	}

	return RESULT_DONE;
}
