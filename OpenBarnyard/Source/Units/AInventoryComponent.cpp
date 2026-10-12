#include "pch.h"
#include "AInventoryComponent.h"
#include "AInventoryComponentData.h"
#include "AInventoryItemData.h"
#include "ACarryableUnitHelper.h"
#include "ACarryableUnit.h"
#include "Helpers/AGameLoader.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimMiniMap.h"
#include "AAnimatedUnit.h"
#include "AUnitManager.h"
#include "AUnitType.h"
#include "Animation/AModel.h"
#include "Animation/AAnimatableObject.h"
#include "UnitMovement/Steering/ASteer.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00622dd0, CreateObject 00625920, CreateObjectInPlace 006259b0
TDEFINE_CLASS( AInventoryComponent );

// DAT_00778d10
AInventoryComponent::Callbacks AInventoryComponent::ms_oDefaultCallbacks;

// $Barnyard: FUNCTION 00623760
TBOOL AInventoryComponent::Callbacks::OnAdd( AInventoryItem* a_pItem, AInventoryComponent* a_pInventory, TINT a_iCount )
{
	AAnimatedUnit* pUnit = TSTATICCAST( AAnimatedUnit, a_pItem->GetUnit() );

	// Takes the unit out of the world
	pUnit->SetMovement( TFALSE );
	pUnit->SetAIEnabled( TFALSE );
	pUnit->SetHashed( TFALSE );
	pUnit->SetVisible( TFALSE );
	pUnit->SetNoQuery( TTRUE );
	pUnit->GetAnimatableObject()->SetAnimControllersDisabled( TTRUE );

	return TTRUE;
}

// $Barnyard: FUNCTION 00624600
AInventoryComponent::AInventoryComponent()
    : m_pCallbacks( &ms_oDefaultCallbacks )
    , m_iNumItems( 0 )
{
}

// $Barnyard: FUNCTION 00624640
// $Barnyard: FUNCTION 00625450
AInventoryComponent::~AInventoryComponent()
{
	// Every slot of the pool is deleted, the used ones first
	TINT iNumSlots = m_pData->GetMaxItems() + 1;

	while ( !m_llItems.IsEmpty() )
	{
		AInventoryItem* pItem = m_llItems.Begin();
		pItem->Remove();
		iNumSlots--;

		pItem->Reset();
		delete pItem;
	}

	while ( iNumSlots > 0 && !m_llFree.IsEmpty() )
	{
		AInventoryItem* pItem = m_llFree.Begin();
		pItem->Remove();
		iNumSlots--;

		pItem->Reset();
		delete pItem;
	}

	m_llFree.Clear();
	m_llItems.Clear();
}

// $Barnyard: FUNCTION 00623830
void AInventoryComponent::Create( AInventoryComponentData* a_pData, AUnit* a_pOwner )
{
	SetOwner( a_pOwner );
	m_pData = a_pData;

	for ( TINT i = 0; i < a_pData->GetMaxItems(); i++ )
		m_llFree.Push( new AInventoryItem() );

	// One more slot than the maximum
	m_llFree.Push( new AInventoryItem() );

	// Fill the inventory with the starting items
	for ( auto it = a_pData->GetItems().Begin(); it != a_pData->GetItems().End(); it++ )
	{
		AInventoryItem* pItem = TNULL;

		if ( !m_llFree.IsEmpty() )
		{
			pItem = m_llFree.Begin();
			pItem->Remove();
		}

		it->Apply( pItem, GetOwner() );

		m_llItems.Push( pItem );
		m_iNumItems++;
	}
}

// $Barnyard: FUNCTION 00623a70
void AInventoryComponent::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( m_iNumItems );

	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
		it->Save( a_pBuffer );
}

// $Barnyard: FUNCTION 00623ad0
void AInventoryComponent::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	Clear();
	a_pBuffer->ReadInt( m_iNumItems );

	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		AInventoryItem* pItem = TNULL;

		if ( !m_llFree.IsEmpty() )
		{
			pItem = m_llFree.Begin();
			pItem->Remove();
		}

		m_llItems.Push( pItem );
		pItem->Load( a_pBuffer );
	}
}

// $Barnyard: FUNCTION 006239e0
void AInventoryComponent::Clear()
{
	while ( !m_llItems.IsEmpty() )
	{
		AInventoryItem* pItem = m_llItems.Begin();
		pItem->Remove();

		pItem->Reset();
		m_llFree.Push( pItem );
	}
}

// $Barnyard: FUNCTION 00624c50
TBOOL AInventoryComponent::AddItem( AInventoryItem* a_pItem, TPString8 a_strInventory )
{
	if ( m_pCallbacks && !m_pCallbacks->OnAdd( a_pItem, this, a_pItem->GetCount() ) )
		return TFALSE;

	if ( a_pItem->GetUnit() )
	{
		AInventoryItem* pSlot = FindItemByType( a_pItem->GetTypeName() );

		if ( !pSlot )
		{
			// The inventory has no item of this type yet
			pSlot = PopFreeItem();
			pSlot->Set( a_pItem->GetUnit(), GetOwner(), a_pItem->GetCount() );

			m_llItems.Push( pSlot );
			m_iNumItems++;

			if ( pSlot->GetCount() > 0 )
				NotifyCountChanged( pSlot, pSlot->GetCount() );
			else
				RemoveItem( pSlot );
		}
		else if ( a_pItem != pSlot )
		{
			// Merge the items, the slot keeps the unit of the added item
			AUnit* pOldUnit = pSlot->GetUnit();
			pSlot->ReplaceUnit( a_pItem->GetUnit(), GetOwner() );
			pSlot->AddCount( a_pItem->GetCount() );
			AUnitManager::GetSingleton()->DestroyUnitNextFrame( pOldUnit );
			a_pItem->Reset();

			if ( pSlot->GetCount() > 0 )
				NotifyCountChanged( pSlot, pSlot->GetCount() );
			else
				RemoveItem( pSlot );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00624a60
TBOOL AInventoryComponent::AddItems( const TPString8& a_rcType, TINT a_iCount, TPString8 a_strInventory )
{
	AInventoryItem* pSlot = FindItemByType( a_rcType );

	if ( pSlot )
	{
		pSlot->AddCount( a_iCount );

		if ( pSlot->GetCount() > 0 )
			NotifyCountChanged( pSlot, pSlot->GetCount() );
		else
			RemoveItem( pSlot );

		return TTRUE;
	}

	if ( m_llFree.IsEmpty() )
		return TFALSE;

	AInventoryItem* pItem = PopFreeItem();
	pItem->Set( a_rcType, GetOwner(), a_iCount );

	if ( m_pCallbacks && !m_pCallbacks->OnAdd( pItem, this, a_iCount ) )
	{
		m_llFree.Push( pItem );
		return TFALSE;
	}

	pSlot = FindItemByType( a_rcType );

	if ( pSlot )
	{
		// NOTE: the slot is put back without releasing the spawned unit
		pSlot->AddCount( pItem->GetCount() );
		m_llFree.Push( pItem );

		if ( pSlot->GetCount() > 0 )
			NotifyCountChanged( pSlot, pSlot->GetCount() );
		else
			RemoveItem( pSlot );

		return TTRUE;
	}

	m_llItems.Push( pItem );
	m_iNumItems++;

	if ( pItem->GetCount() > 0 )
		NotifyCountChanged( pItem, pItem->GetCount() );
	else
		RemoveItem( pItem );

	return TTRUE;
}

// $Barnyard: FUNCTION 006247d0
TBOOL AInventoryComponent::AddUnit( AUnit* a_pUnit, TINT a_iCount, TPString8 a_strInventory )
{
	if ( m_llFree.IsEmpty() )
		return TFALSE;

	AInventoryItem* pItem = PopFreeItem();
	pItem->Set( a_pUnit, GetOwner(), a_iCount );

	if ( AGameLoader::IsMainGameLoaded() && a_pUnit->GetClass()->IsA( &TGetClass( ACarryableUnit ) ) )
	{
		// The carried unit loses its icon on the mini map
		AHudManager::GetSingleton()->FindElement<ASimMiniMap>()->RemoveUnitIcon( a_pUnit->GetSelfProxy() );
	}

	if ( m_pCallbacks && !m_pCallbacks->OnAdd( pItem, this, a_iCount ) )
	{
		m_llFree.Push( pItem );
		return TFALSE;
	}

	AInventoryItem* pSlot = FindItemByType( a_pUnit->GetType()->GetName() );

	if ( pSlot )
	{
		// Merge the items, the slot keeps the added unit
		AUnit* pOldUnit = pSlot->GetUnit();
		pSlot->ReplaceUnit( pItem->GetUnit(), GetOwner() );
		pSlot->AddCount( pItem->GetCount() );
		AUnitManager::GetSingleton()->DestroyUnitNextFrame( pOldUnit );

		// NOTE: the slot is put back without releasing the unit
		m_llFree.Push( pItem );

		if ( pSlot->GetCount() > 0 )
			NotifyCountChanged( pSlot, pSlot->GetCount() );
		else
			RemoveItem( pSlot );
	}
	else
	{
		m_llItems.Push( pItem );
		m_iNumItems++;

		if ( pItem->GetCount() > 0 )
			NotifyCountChanged( pItem, pItem->GetCount() );
		else
		{
#ifdef BARNYARD_COMMUNITY_PATCH
			RemoveItem( pItem );
#else
			// NOTE: retail removes TNULL and crashes
			RemoveItem( TNULL );
#endif
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 006234c0
AUnit* AInventoryComponent::DropItem( AInventoryItem* a_pItem )
{
	if ( !a_pItem )
		return TNULL;

	AUnit* pUnit = a_pItem->GetUnit();

	if ( !pUnit )
		return pUnit;

	if ( a_pItem->GetCount() == 1 )
	{
		// The last item is the unit itself
		if ( RemoveItem( a_pItem ) )
			a_pItem->Reset();

		return pUnit;
	}

	if ( m_pCallbacks && !m_pCallbacks->OnRemove( a_pItem, this, 1 ) )
		return pUnit;

	// Spawn another unit of the type where the kept one is
	TVector4  vecPosition;
	TVector4  vecDirection;
	TMatrix44 matTransform;

	AVisibleUnit* pVisibleUnit = TSTATICCAST( AVisibleUnit, pUnit );

	if ( ASteer* pSteer = pUnit->GetSteer() )
	{
		vecPosition  = *pSteer->GetPosition();
		vecDirection = *pSteer->GetDirection();

		pVisibleUnit->GetModelInstance()->GetLocalTransform( matTransform );
		vecPosition = matTransform.GetTranslation();

		// NOTE: inlined AModelInstance::GetUpVector
		TMatrix44 matModel;
		pVisibleUnit->GetModelInstance()->GetLocalTransform( matModel );
		vecDirection = matModel.AsBasisVector4( BASISVECTOR_UP );
	}
	else
	{
		vecPosition  = TVector4( 0.0f, 0.0f, 0.0f, 1.0f );
		vecDirection = TVector4( 0.0f, -1.0f, 0.0f, 1.0f );

		pVisibleUnit->GetModelInstance()->GetLocalTransform( matTransform );
	}

	AUnit* pNewUnit = AUnitManager::GetSingleton()->SpawnUnit( pUnit->GetType()->GetName(), vecPosition, vecDirection );

	TSTATICCAST( AVisibleUnit, pNewUnit )->GetModelInstance()->SetTransform( matTransform );

	if ( ACarryableUnitHelper* pHelper = pNewUnit->GetCarryableHelper() )
		pHelper->SetHolder( GetOwner() );

	a_pItem->RemoveCount( 1 );
	return pNewUnit;
}

// $Barnyard: FUNCTION 00622e60
AUnit* AInventoryComponent::DropItemOfType( const TPString8& a_rcType )
{
	return DropItem( FindItemByType( a_rcType ) );
}

// $Barnyard: FUNCTION 00624e10
TINT AInventoryComponent::RemoveItems( const TPString8& a_rcType, TINT a_iCount, TBOOL a_bDestroy )
{
	TINT iNumLeft = a_iCount;
	RemoveEmptyItems();

	for ( AInventoryItem* pItem = m_llItems.Begin(); pItem != m_llItems.End() && iNumLeft > 0; )
	{
		AInventoryItem* pNext = pItem->Next();

		if ( pItem->GetUnit() )
		{
			TINT iNumToRemove = TMath::Min( iNumLeft, pItem->GetCount() );

			if ( pItem->GetUnitType()->GetName() == a_rcType && pItem->IsLinked() )
			{
				if ( !m_pCallbacks || m_pCallbacks->OnRemove( pItem, this, iNumToRemove ) )
				{
					pItem->RemoveCount( iNumToRemove );

					if ( pItem->GetCount() == 0 )
					{
						pItem->Remove();
						m_llFree.Push( pItem );
						m_iNumItems--;

						if ( a_bDestroy )
						{
							if ( pItem->GetUnit() )
								AUnitManager::GetSingleton()->DestroyUnit( pItem->GetUnit() );

							pItem->Reset();
						}
					}

					iNumLeft -= iNumToRemove;
				}
			}
		}

		pItem = pNext;
	}

	return a_iCount - iNumLeft;
}

// $Barnyard: FUNCTION 00623b70
AInventoryItem* AInventoryComponent::RemoveItem( AInventoryItem* a_pItem )
{
	if ( !a_pItem->IsLinked() )
		return TNULL;

	if ( m_pCallbacks && !m_pCallbacks->OnRemove( a_pItem, this, a_pItem->GetCount() ) )
		return TNULL;

	a_pItem->Remove();
	m_llFree.Push( a_pItem );
	m_iNumItems--;

	return a_pItem;
}

// $Barnyard: FUNCTION 00623be0
void AInventoryComponent::RemoveEmptyItems()
{
	for ( AInventoryItem* pItem = m_llItems.Begin(); pItem != m_llItems.End(); )
	{
		AInventoryItem* pNext = pItem->Next();

		if ( !pItem->GetUnit() )
		{
			pItem->Remove();
			m_llFree.Push( pItem );
			m_iNumItems--;
		}

		pItem = pNext;
	}
}

// $Barnyard: FUNCTION 006237c0
AInventoryItem* AInventoryComponent::PopFreeItem()
{
	if ( m_llFree.IsEmpty() )
		return TNULL;

	AInventoryItem* pItem = m_llFree.End()->Prev();
	pItem->Remove();

	return pItem;
}

// $Barnyard: FUNCTION 00623ce0
AInventoryItem* AInventoryComponent::FindItemByUnit( AUnit* a_pUnit )
{
	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
	{
		if ( it->GetUnit() == a_pUnit )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00623c70
AInventoryItem* AInventoryComponent::FindItemByType( const TPString8& a_rcType )
{
	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
	{
		if ( it->GetUnitType()->GetName() == a_rcType )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00623d50
AInventoryItem* AInventoryComponent::GetItem( TINT a_iIndex )
{
	TINT i = 0;

	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++, i++ )
	{
		if ( i == a_iIndex )
			return it;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00623dc0
TBOOL AInventoryComponent::ContainsItem( AInventoryItem* a_pItem )
{
	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
	{
		if ( it == a_pItem )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00623e20
TBOOL AInventoryComponent::ContainsUnit( AUnit* a_pUnit )
{
	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
	{
		if ( it->GetUnit() == a_pUnit )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00623e90
TBOOL AInventoryComponent::ContainsType( const TPString8& a_rcType )
{
	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
	{
		if ( it->GetUnitType()->GetName() == a_rcType )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00623f00
TINT AInventoryComponent::GetNumItems()
{
	TINT iNumItems = 0;

	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
		iNumItems += it->GetCount();

	return iNumItems;
}

// $Barnyard: FUNCTION 00623f60
TINT AInventoryComponent::GetNumSlots()
{
	TINT iNumSlots = 0;

	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
		iNumSlots++;

	return iNumSlots;
}

// $Barnyard: FUNCTION 00623fb0
TINT AInventoryComponent::GetNumItemsOfType( const TPString8& a_rcType )
{
	TINT iNumItems = 0;

	for ( auto it = m_llItems.Begin(); it != m_llItems.End(); it++ )
	{
		if ( it->GetUnitType() && it->GetUnitType()->GetName() == a_rcType )
			iNumItems += it->GetCount();
	}

	return iNumItems;
}

// $Barnyard: FUNCTION 00622e80
TINT AInventoryComponent::TransferItems( const TPString8& a_rcType, AInventoryComponentInterface* a_pTarget, TINT a_iCount )
{
	AInventoryItem* pItem = FindItemByType( a_rcType );

	if ( !pItem )
		return 0;

	return TransferItem( pItem, a_pTarget, a_iCount );
}

// $Barnyard: FUNCTION 00624fa0
TINT AInventoryComponent::TransferItem( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount )
{
	if ( a_pTarget == this )
		return 0;

	if ( m_pCallbacks && !m_pCallbacks->OnTransfer( a_pItem, this, a_pTarget, a_iCount ) )
		return 0;

	if ( a_pItem->GetCount() <= a_iCount )
	{
		// The whole slot moves to the target
		if ( !RemoveItem( a_pItem ) )
			return 0;

		TPString8 strType   = a_pItem->GetUnitType()->GetName();
		TINT      iNumItems = a_pItem->GetCount();

		if ( a_pTarget->AddItem( a_pItem, ms_strDefaultInventory ) )
		{
			NotifyTransferred( a_pTarget->FindItemByType( strType ), a_pTarget, a_iCount );
			return iNumItems;
		}

		// The target didn't take it, put it back
		AddItem( a_pItem, ms_strDefaultInventory );
		return 0;
	}

	TPString8 strType = a_pItem->GetUnitType()->GetName();

	if ( !m_pCallbacks || m_pCallbacks->OnRemove( a_pItem, this, a_iCount ) )
	{
		a_pItem->RemoveCount( a_iCount );

		if ( a_pTarget->AddItems( strType, a_iCount, ms_strDefaultInventory ) )
		{
			NotifyTransferred( a_pTarget->FindItemByType( strType ), a_pTarget, a_iCount );
			return a_iCount;
		}

		a_pItem->AddCount( a_iCount );
	}

	return 0;
}

// $Barnyard: FUNCTION 00624630
const TPString8& AInventoryComponent::GetName()
{
	return m_pData->GetCategoryName();
}

// $Barnyard: FUNCTION 00622e00
TBOOL AInventoryComponent::NotifyCountChanged( AInventoryItem* a_pItem, TINT a_iCount )
{
	if ( !m_pCallbacks )
		return TTRUE;

	return m_pCallbacks->OnCountChanged( a_pItem, this, a_iCount );
}

// $Barnyard: FUNCTION 00622e30
TBOOL AInventoryComponent::NotifyTransferred( AInventoryItem* a_pItem, AInventoryComponentInterface* a_pTarget, TINT a_iCount )
{
	if ( !m_pCallbacks )
		return TTRUE;

	return m_pCallbacks->OnTransferred( a_pItem, this, a_pTarget, a_iCount );
}
