#include "pch.h"
#include "AGiftQuestElement.h"
#include "Units/AUnit.h"
#include "Units/AUnitManager.h"
#include "Units/AUnitNameManager.h"
#include "Units/AUnitType.h"
#include "Units/AUnitGameEventComponent.h"
#include "Units/AInteractionGiftEvent.h"
#include "Units/AInventoryComponent.h"
#include "Player/APlayerManager.h"

#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 005cfe00, CreateObject 005d05e0, CreateObjectInPlace 005d0640
TDEFINE_CLASS( AGiftQuestElement );

// $Barnyard: FUNCTION 005cff40
// NOTE: retail leaves m_iAmount uninitialised
AGiftQuestElement::AGiftQuestElement()
    : m_strDescription( GetGlobalAllocator() )
    , m_iAmount( 0 )
    , m_iReceiverOffset( -1 )
{
}

// $Barnyard: FUNCTION 005cffb0
// $Barnyard: FUNCTION 005d0680
AGiftQuestElement::~AGiftQuestElement()
{
}

// $Barnyard: FUNCTION 005d0070
TBOOL AGiftQuestElement::Create( const PBProperties* a_pProperties, TINT* a_pDataOffset )
{
	for ( auto it = a_pProperties->Begin(); it != a_pProperties->End(); it++ )
	{
		const TCHAR*           szName = it->GetName().GetString();
		const PBPropertyValue* pValue = it->GetValue();

		switch ( szName[ 0 ] )
		{
			case 'a':
				if ( TStringManager::String8CompareNoCase( szName, "amount" ) == 0 )
					m_iAmount = pValue->GetInteger();
				break;
			case 'g':
				if ( TStringManager::String8CompareNoCase( szName, "gifttype" ) == 0 )
					m_strGiftType = pValue->GetTPString8();
				break;
			case 'r':
				if ( TStringManager::String8CompareNoCase( szName, "receivertype" ) == 0 )
					m_strReceiverType = pValue->GetTPString8();
				else if ( TStringManager::String8CompareNoCase( szName, "receivername" ) == 0 )
					m_strReceiverName = pValue->GetTPString8();
				break;
		}
	}

	m_iReceiverOffset = *a_pDataOffset;
	*a_pDataOffset += sizeof( AUnitRef );

	return AQuestElement::Create( a_pProperties, a_pDataOffset );
}

// $Barnyard: FUNCTION 005d02b0
TBOOL AGiftQuestElement::InitData( AInteractionActivityData* a_pData )
{
	AUnitRef* pReceiver = GetReceiver( a_pData );

	// NOTE: the instance data is raw memory, retail clears the reference without releasing anything
	TUtil::MemClear( pReceiver, sizeof( AUnitRef ) );

	AUnit* pUnit;

	if ( !m_strReceiverName.IsEmpty() )
		pUnit = AUnitNameManager::GetSingleton()->RetrieveByName( m_strReceiverName );
	else
		pUnit = AUnitManager::GetSingleton()->FindUnitByTypeName( m_strReceiverType );

	if ( pUnit )
		pReceiver->oProxy = pUnit->GetSelfProxy();

	return AQuestElement::InitData( a_pData );
}

// $Barnyard: FUNCTION 005d06a0
void AGiftQuestElement::DeinitData( AInteractionActivityData* a_pData )
{
	GetReceiver( a_pData )->oProxy = TNULL;
	AQuestElement::DeinitData( a_pData );
}

// $Barnyard: FUNCTION 005cfe10
TBOOL AGiftQuestElement::GetDescription( const TWCHAR** a_ppOutText )
{
	// NOTE: retail passes the TCHAR strings to the wide %s
	m_strDescription.Format( L"Give %s %d %s", m_strReceiverName.GetString(), m_iAmount, m_strGiftType.GetString() );
	*a_ppOutText = m_strDescription.GetString( 0 );

	return TTRUE;
}

// $Barnyard: FUNCTION 005cfe70
void AGiftQuestElement::Save( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	AUnitManager::GetSingleton()->SaveUnitRef( a_pBuffer, GetReceiver( a_pData ) );
	AQuestElement::Save( a_pBuffer, a_pData );
}

// $Barnyard: FUNCTION 005cfea0
TBOOL AGiftQuestElement::Load( AHackedQuickSave::Buffer* a_pBuffer, AInteractionActivityData* a_pData )
{
	AUnitRef* pReceiver = GetReceiver( a_pData );

	if ( pReceiver )
		TUtil::MemClear( pReceiver, sizeof( AUnitRef ) );

	AUnitManager::GetSingleton()->LoadUnitRef( a_pBuffer, pReceiver );
	return AQuestElement::Load( a_pBuffer, a_pData );
}

// $Barnyard: FUNCTION 005d03d0
AQuestElement::PROGRESS AGiftQuestElement::CheckProgress( AInteractionActivityData* a_pData )
{
	AUnit* pPlayer = APlayerManager::GetSingleton()->GetPlayerUnit( 0 );

	if ( !pPlayer )
		return PROGRESS_RUNNING;

	AUnit* pReceiver = GetReceiver( a_pData )->GetUnit();

	if ( !pReceiver )
		return PROGRESS_RUNNING;

	// The player has to witness giving enough items of the type to the receiver
	AUnitGameEventComponent* pGameEvents = TSTATICCAST( AUnitGameEventComponent, pPlayer->FindComponentExact( &TGetClass( AUnitGameEventComponent ) ) );
	TINT                     iNumEvents  = pGameEvents->GetNumWitnessedEvents();

	for ( TINT i = 0; i < iNumEvents; i++ )
	{
		AInteractionEvent* pEvent = pGameEvents->GetWitnessedEvent( i );

		// NOTE: retail compares the names of the classes (with a copy of the TClass)
		if ( !pEvent || pEvent->GetClass() != &TGetClass( AInteractionGiftEvent ) )
			continue;

		AInteractionGiftEvent* pGiftEvent = TSTATICCAST( AInteractionGiftEvent, pEvent );

		if ( pGiftEvent->GetReceiverTypeName() != pReceiver->GetType()->GetName().GetString() )
			continue;

		if ( pGiftEvent->GetGiverTypeName() != pPlayer->GetType()->GetName().GetString() )
			continue;

		if ( pGiftEvent->GetGiftType() == m_strGiftType && pGiftEvent->GetCount() >= m_iAmount )
		{
			// The gift is done, the player has no use for the remaining items of the type
			AInventoryComponentInterface* pInventory = TSTATICCAST( AInventoryComponentInterface, pPlayer->FindComponent( &TGetClass( AInventoryComponent ) ) );

#ifdef BARNYARD_COMMUNITY_PATCH
			// NOTE: the player only has an AInventoryGroupComponent, retail dereferences TNULL here
			if ( !pInventory )
				pInventory = AInventoryComponentInterface::GetInventory( pPlayer );
#endif

			pInventory->RemoveItems( m_strGiftType, pInventory->GetNumItemsOfType( m_strGiftType ), TTRUE );
			return PROGRESS_COMPLETE;
		}
	}

	return PROGRESS_RUNNING;
}
