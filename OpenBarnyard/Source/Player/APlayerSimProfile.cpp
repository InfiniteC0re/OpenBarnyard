#include "pch.h"
#include "APlayerSimProfile.h"
#include "MiniGame/AGopherShopInterface.h"
#include "ALoadScreen.h"
#include "GameInterface/AGameStateController.h"
#include "GameInterface/ASSeqState.h"
#include "Helpers/AGameLoader.h"
#include "Units/AUnit.h"
#include "Memory/AMemory.h"
#include "HUD/AHudManager.h"
#include "HUD/ASimPlayerMainHudElement.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "Sound/ASoundManager.h"
#include "SoundBank/ui.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerSet.h"
#include "Quests/AQuestManager.h"
#include "Units/AAnimatedUnit.h"
#include "Units/AUnitNameManager.h"
#include "Animation/AAnimatableObjectManager.h"
#include "GameInterface/AMobilePhoneUIState2.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <Plugins/PPropertyParser/PPropertyValue.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( pip );
TPSTRING8_DECLARE( c_l_clavicle );
TPSTRING8_DECLARE( ASleepCycleState );

TDEFINE_CLASS_NORUNTIME( APlayerSimProfile );

// $Barnyard: FUNCTION 00621c10
APlayerSimProfile::APlayerSimProfile()
    : m_iMiniGamePickups( 0 )
    , m_iUnk14( 0 )
    , m_iDisplayedPickups( 0 )
    , m_iPickups( 0 )
    , m_fPickupsCountTime( 0.0f )
    , m_strName( AMemory::GetAllocator( AMemory::POOL_StringPool ) )
    , m_iPickupsSound( -1 )
    , m_bEnableUpdates( TTRUE )
    , m_iUnk1AC( 0 )
    , m_iNumHoneycombs( 0 )
    , m_bRoyalJelly( TFALSE )
{
	Reset();

	for ( TINT i = 0; i < NUM_ITEMS; i++ )
		m_llFreeItems.InsertTail( &m_aItems[ i ] );
}

// $Barnyard: FUNCTION 00621d20
// $Barnyard: FUNCTION 00621370
APlayerSimProfile::~APlayerSimProfile()
{
	m_llFreeItems.TGenericDList::RemoveAll();
	m_llUsedItems.TGenericDList::RemoveAll();
	m_llPendingItems.TGenericDList::RemoveAll();
}

// $Barnyard: FUNCTION 00621450
void APlayerSimProfile::Reset()
{
	m_fMaxStamina        = 5.0f;
	m_fStamina           = 5.0f;
	m_iDisplayedPickups  = 0;
	m_iPickups           = 0;
	m_fPickupsCountTime  = 0.0f;
	m_fStaminaRaiseSpeed = 0.25f;
	m_fStaminaWasteSpeed = 0.8f;
	m_bUpdateStamina     = TTRUE;
	m_iCostume           = 0;
	m_iSavedAct          = -1;
	m_iSavedTier         = -1;

	for ( TINT i = 0; i < NUM_SQUIRTED_UNITS; i++ )
		m_aSquirtedUnits[ i ].SetPooledString( TNULL );

	m_strInventoryItem.SetPooledString( TNULL );

	// NOTE: overwrites the -1 set above in retail too
	m_iSavedAct      = 0;
	m_iNumHoneycombs = 0;
	m_bRoyalJelly    = TFALSE;
}

// $Barnyard: FUNCTION 00621d40
TBOOL APlayerSimProfile::Update( TFLOAT a_fDeltaTime )
{
	if ( !m_bEnableUpdates )
		return TFALSE;

	// NOTE: inlined in retail
	AddStamina( a_fDeltaTime * m_fStaminaRaiseSpeed );

	ASimPlayerPickUpHudElement* pPickUpElement = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();

	if ( pPickUpElement )
	{
		if ( m_iDisplayedPickups != m_iPickups )
		{
			// Count the displayed value towards m_iPickups so that it reaches it in m_fPickupsCountTime seconds
			if ( m_iDisplayedPickups < m_iPickups )
			{
				TINT iStep = TMath::CeilToInt( ( a_fDeltaTime / m_fPickupsCountTime ) * TFLOAT( m_iPickups - m_iDisplayedPickups ) );
				m_fPickupsCountTime -= a_fDeltaTime;
				m_iDisplayedPickups += iStep;

				if ( m_iDisplayedPickups > m_iPickups )
					m_iDisplayedPickups = m_iPickups;
			}
			else
			{
				TINT iStep = TMath::CeilToInt( ( a_fDeltaTime / m_fPickupsCountTime ) * TFLOAT( m_iDisplayedPickups - m_iPickups ) );
				m_fPickupsCountTime -= a_fDeltaTime;
				m_iDisplayedPickups -= iStep;

				if ( m_iDisplayedPickups < m_iPickups )
					m_iDisplayedPickups = m_iPickups;
			}

			pPickUpElement->SetNumber( m_iDisplayedPickups );

			// Looping sound while counting
			if ( m_iPickupsSound == -1 )
				m_iPickupsSound = ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN_LOOP );
		}
		else if ( m_iPickupsSound != -1 )
		{
			// Done counting
			ASoundManager::GetSingleton()->StopCue( m_iPickupsSound );
			m_iPickupsSound = -1;
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
		}
	}

	m_fPendingItemsDelay -= a_fDeltaTime;

	if ( m_llPendingItems.Count() > 0 && m_fPendingItemsDelay < 0.0f )
	{
		if ( APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->CanInteract() &&
		     !AGameStateController::GetSingleton()->FindGameStateOnStack( TPS8( ASleepCycleState ) ) &&
		     AGameLoader::IsMainGameLoaded() && !ASSeqState::IsPlaying() && !g_oLoadScreen.IsLoadingScreen() )
		{
			// Move the m_llPendingItems items to the used items, only the first one flashes the HUD
			TBOOL bShowHud = TTRUE;

			for ( auto it = m_llPendingItems.Begin(); it != m_llPendingItems.End(); it = m_llPendingItems.Begin() )
			{
				TPString8 strName = it->m_strName;
				TINT      iText   = it->m_iText;
				TINT      iTextId = it->m_iTextId;

				ReleaseItem( strName );
				AddUsedItem( TREINTERPRETCAST( const TWCHAR*, TUINTPTR( iText ) ), strName, iTextId, bShowHud );
				bShowHud = TFALSE;
			}
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00621140
void APlayerSimProfile::StoreQuestTier()
{
	m_iSavedAct  = AQuestManager::GetSingleton()->GetAct();
	m_iSavedTier = AQuestManager::GetSingleton()->GetTier();
}

// $Barnyard: FUNCTION 00621530
TBOOL APlayerSimProfile::Save( AHackedQuickSave::Buffer* a_pBuffer )
{
	a_pBuffer->WriteInt( m_eGender );
	a_pBuffer->WriteInt( m_iCostume );
	a_pBuffer->WriteWString( m_strName );

	m_iDisplayedPickups = m_iPickups;
	m_fPickupsCountTime = 0.0f;
	a_pBuffer->WriteInt( m_iPickups );
	a_pBuffer->WriteFloat( m_fStamina );
	a_pBuffer->WriteFloat( m_fStaminaRaiseSpeed );
	a_pBuffer->WriteBool( m_bUpdateStamina );
	a_pBuffer->WriteFloat( m_fMaxStamina );

	// Is pip sitting on the player's shoulder
	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 ) );
	TBOOL          bHasPip = AAnimatableObjectManager::GetSingleton()->GetAttachedObject( pPlayer->GetAnimatableObject(), TPS8( c_l_clavicle ), 0 ) != TNULL;
	a_pBuffer->WriteBool( bHasPip );

	for ( TINT i = 0; i < NUM_SQUIRTED_UNITS; i++ )
		a_pBuffer->WriteString( m_aSquirtedUnits[ i ] );

	// Up to 10 used items, 200 is added to the number to say the m_llPendingItems items follow
	TINT iNumUsedItems = 0;

	for ( TINT i = 0; i < m_llUsedItems.Count(); )
	{
		Item* pItem = GetUsedItem( ++i );

		if ( pItem->m_iTextId != -1 && ++iNumUsedItems == 10 )
			break;
	}

	a_pBuffer->WriteInt( iNumUsedItems + 200 );

	TINT iNumWritten = 0;

	for ( TINT i = 0; i < m_llUsedItems.Count(); )
	{
		Item* pItem = GetUsedItem( ++i );

		if ( pItem->m_iTextId != -1 )
		{
			TINT iSender = AMobilePhoneUIState2::FindSender( pItem->m_strName );
			a_pBuffer->WriteInt( pItem->m_iTextId );
			a_pBuffer->WriteInt( iSender );

			if ( ++iNumWritten == iNumUsedItems )
				break;
		}
	}

	TINT iNumPendingItems = 0;

	for ( auto it = m_llPendingItems.Begin(); it != m_llPendingItems.End(); it++ )
	{
		if ( it->m_iTextId != -1 )
			iNumPendingItems++;
	}

	a_pBuffer->WriteInt( iNumPendingItems );

	for ( auto it = m_llPendingItems.Begin(); it != m_llPendingItems.End(); it++ )
	{
		if ( it->m_iTextId != -1 )
		{
			TINT iSender = AMobilePhoneUIState2::FindSender( it->m_strName );
			a_pBuffer->WriteInt( it->m_iTextId );
			a_pBuffer->WriteInt( iSender );
		}
	}

	return TTRUE;
}

// Text of a saved item, a_iTextId 1748 is the text with the player's name in it
// NOTE: inlined into Load (00622020)
static const TWCHAR* GetSavedItemText( TINT a_iTextId, const TString16& a_rcPlayerName )
{
	if ( a_iTextId == 1748 )
	{
		// The text of Joey's offer
		ALocale::FormatTags( AGopherShopInterface::GetJoeyOfferText(), 128, ALocaleManager::Interface()->GetString( a_iTextId ), PPropertyValue( const_cast<TWCHAR*>( a_rcPlayerName.GetString( 0 ) ), TFALSE ) );
		return AGopherShopInterface::GetJoeyOfferText();
	}

	return ALocaleManager::Interface()->GetString( a_iTextId );
}

// $Barnyard: FUNCTION 00622020
TBOOL APlayerSimProfile::Load( AHackedQuickSave::Buffer* a_pBuffer )
{
	TINT iGender = -1;
	a_pBuffer->ReadInt( iGender );
	a_pBuffer->ReadInt( m_iCostume );
	a_pBuffer->ReadWString( m_strName );

	// NOTE: the original calls RETURNS_TRUE (00413920) with the buffer here

	a_pBuffer->ReadInt( m_iDisplayedPickups );
	a_pBuffer->ReadFloat( m_fStamina );
	a_pBuffer->ReadFloat( m_fStaminaRaiseSpeed );
	a_pBuffer->ReadBool( m_bUpdateStamina );
	a_pBuffer->ReadFloat( m_fMaxStamina );
	m_iPickups          = m_iDisplayedPickups;
	m_fPickupsCountTime = 0.0f;
	m_eGender           = iGender;

	TBOOL bHasPip;
	a_pBuffer->ReadBool( bHasPip );

	for ( TINT i = 0; i < NUM_SQUIRTED_UNITS; i++ )
		a_pBuffer->ReadString( m_aSquirtedUnits[ i ] );

	AAnimatedUnit* pPlayer = TSTATICCAST( AAnimatedUnit, APlayerManager::GetSingleton()->GetPlayerSet( APlayerManager::PLAYERSET_SIM )->GetHumanPlayerUnit( 0 ) );

	if ( pPlayer )
	{
		pPlayer->ApplyMaterial( m_iCostume, TTRUE );

		if ( bHasPip )
		{
			AUnit* pPip = AUnitNameManager::GetSingleton()->RetrieveByName( TPS8( pip ) );

			if ( pPip )
				pPlayer->AttachUnit( pPip, TPS8( c_l_clavicle ) );
		}
	}

	TINT iPickups = m_iDisplayedPickups;
	TMath::Clip( iPickups, 0, MAX_PICKUPS );

	if ( AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>() )
	{
		// Let the counter count up to it
		m_iPickups          = iPickups;
		m_fPickupsCountTime = 1.0f;
	}
	else
	{
		m_iDisplayedPickups = iPickups;
		m_iPickups          = iPickups;
		m_fPickupsCountTime = 0.0f;
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
	}

	TINT iNumUsedItems = 0;
	a_pBuffer->ReadInt( iNumUsedItems );

	TBOOL bHasPendingItems = TFALSE;

	if ( iNumUsedItems >= 200 )
	{
		iNumUsedItems -= 200;
		bHasPendingItems = TTRUE;
	}

	for ( TINT i = 0; i < iNumUsedItems; i++ )
	{
		TINT iTextId, iSender;
		a_pBuffer->ReadInt( iTextId );
		a_pBuffer->ReadInt( iSender );

		const TWCHAR* wszText = GetSavedItemText( iTextId, m_strName );
		AddUsedItem( wszText, *AMobilePhoneUIState2::GetSenderName( iSender ), iTextId, TFALSE );
	}

	if ( bHasPendingItems )
	{
		TINT iNumPendingItems = 0;
		a_pBuffer->ReadInt( iNumPendingItems );

		for ( TINT i = 0; i < iNumPendingItems; i++ )
		{
			TINT iTextId, iSender;
			a_pBuffer->ReadInt( iTextId );
			a_pBuffer->ReadInt( iSender );

			const TWCHAR* wszText = GetSavedItemText( iTextId, m_strName );
			AddPendingItem( wszText, *AMobilePhoneUIState2::GetSenderName( iSender ), iTextId );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00620e30
void APlayerSimProfile::AddStamina( TFLOAT a_fStamina )
{
	if ( m_bUpdateStamina == TTRUE )
	{
		TFLOAT fNewStamina = a_fStamina + m_fStamina;

		if ( fNewStamina > m_fMaxStamina )
		{
			m_fStamina = m_fMaxStamina;
			return;
		}

		if ( fNewStamina < 0.0f )
			fNewStamina = 0.0f;

		m_fStamina = fNewStamina;
	}
}

// $Barnyard: FUNCTION 00620df0
void APlayerSimProfile::SetStamina( TFLOAT a_fStamina )
{
	if ( a_fStamina > m_fMaxStamina )
	{
		m_fStamina = m_fMaxStamina;
		return;
	}

	if ( a_fStamina < 0.0f )
	{
		m_fStamina = 0.0f;
		return;
	}

	m_fStamina = a_fStamina;
}

// $Barnyard: FUNCTION 00620f10
void APlayerSimProfile::SetMaxStamina( TFLOAT a_fMaxStamina )
{
	m_fMaxStamina = a_fMaxStamina;

	if ( a_fMaxStamina < 1.0f )
		m_fMaxStamina = 1.0f;
	else if ( a_fMaxStamina > 5.0f )
		m_fMaxStamina = 5.0f;

	if ( m_fMaxStamina < m_fStamina )
		m_fStamina = m_fMaxStamina;
}

// $Barnyard: FUNCTION 00620f60
void APlayerSimProfile::AddMaxStamina( TFLOAT a_fMaxStamina )
{
	SetMaxStamina( a_fMaxStamina + m_fMaxStamina );
}

// $Barnyard: FUNCTION 00621170
void APlayerSimProfile::AddPickups( TINT a_iPickups )
{
	a_iPickups += m_iDisplayedPickups;
	TMath::Clip( a_iPickups, 0, MAX_PICKUPS );

	if ( AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>() )
	{
		// Let the counter count to the new value
		m_iPickups          = a_iPickups;
		m_fPickupsCountTime = 1.0f;
	}
	else
	{
		m_iDisplayedPickups = a_iPickups;
		m_iPickups          = a_iPickups;
		m_fPickupsCountTime = 0.0f;
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
	}
}

// $Barnyard: FUNCTION 006211e0
void APlayerSimProfile::AddPickupsToTarget( TINT a_iPickups )
{
	if ( m_iDisplayedPickups == m_iPickups )
		a_iPickups += m_iDisplayedPickups;
	else
		a_iPickups += m_iPickups;

	TMath::Clip( a_iPickups, 0, MAX_PICKUPS );

	if ( AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>() )
	{
		// Let the counter count to the new value
		m_iPickups          = a_iPickups;
		m_fPickupsCountTime = 1.0f;
	}
	else
	{
		m_fPickupsCountTime = 0.0f;
		m_iPickups          = a_iPickups;
		m_iDisplayedPickups = a_iPickups;
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
	}
}

// $Barnyard: FUNCTION 00621100
void APlayerSimProfile::AddMiniGamePickups( TINT a_iPickups )
{
	m_iMiniGamePickups += a_iPickups;

	ASimPlayerPickUpHudElement* pPickUpElement = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();

	if ( pPickUpElement )
	{
		pPickUpElement->SetForceHide( TFALSE );
		pPickUpElement->SetNumber( m_iMiniGamePickups );
	}
}

// $Barnyard: FUNCTION 00620f80
void APlayerSimProfile::SetPickups( TINT a_iPickups, TBOOL a_bCountUp, TBOOL a_bUpdateHud )
{
	TMath::Clip( a_iPickups, 0, MAX_PICKUPS );

	ASimPlayerPickUpHudElement* pPickUpElement = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();

	if ( pPickUpElement && a_bUpdateHud == TTRUE )
	{
		if ( a_bCountUp )
		{
			m_iPickups          = a_iPickups;
			m_fPickupsCountTime = 1.0f;
		}
		else
		{
			m_iDisplayedPickups = a_iPickups;
			m_iPickups          = a_iPickups;
			m_fPickupsCountTime = 0.0f;
			pPickUpElement->SetNumber( a_iPickups );
			ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
		}
	}
	else
	{
		m_iDisplayedPickups = a_iPickups;
		m_iPickups          = a_iPickups;
		m_fPickupsCountTime = 0.0f;
		ASoundManager::GetSingleton()->PlayCue( soundbank::UI_COIN );
	}
}

// $Barnyard: FUNCTION 00621020
TBOOL APlayerSimProfile::OnDateChanged( const AGameTimeManager::DateChangeEvent* a_pEvent )
{
	if ( a_pEvent->m_eUpdateType == AGameTimeManager::DATECHANGETYPE_DAY )
		SetMaxStamina( m_fMaxStamina - 1.0f );

	return TTRUE;
}

// $Barnyard: FUNCTION 00621300
APlayerSimProfile::Item* APlayerSimProfile::GetUsedItem( TINT a_iIndex )
{
	for ( auto it = m_llUsedItems.Begin(); it != m_llUsedItems.End(); it++ )
	{
		if ( a_iIndex == 1 )
			return it;

		a_iIndex -= 1;
	}

	return TNULL;
}

// $Barnyard: FUNCTION 00621a80
void APlayerSimProfile::GetUsedItemInfo( TINT a_iIndex, TINT& a_rOutText, Toshi::TPString8& a_rOutName )
{
	for ( auto it = m_llUsedItems.Begin(); it != m_llUsedItems.End(); it++ )
	{
		if ( a_iIndex == 1 )
		{
			a_rOutText = it->m_iText;
			a_rOutName = it->m_strName;
			return;
		}

		a_iIndex -= 1;
	}
}

// $Barnyard: FUNCTION 00621290
void APlayerSimProfile::ReleaseAllItems()
{
	while ( !m_llUsedItems.IsEmpty() )
	{
		Item* pItem = m_llUsedItems.Tail();
		pItem->Remove();
		m_llFreeItems.InsertTail( pItem );
	}

	while ( !m_llPendingItems.IsEmpty() )
	{
		Item* pItem = m_llPendingItems.Tail();
		pItem->Remove();
		m_llFreeItems.InsertTail( pItem );
	}
}

// $Barnyard: FUNCTION 006218e0
void APlayerSimProfile::ReleaseItem( Toshi::TPString8 a_strName )
{
	ReleaseItemFromList( m_llUsedItems, a_strName );
	ReleaseItemFromList( m_llPendingItems, a_strName );
}

// $Barnyard: FUNCTION 00621760
void APlayerSimProfile::AddUsedItem( const TWCHAR* a_wszText, Toshi::TPString8 a_strName, TINT a_iTextId, TBOOL a_bShowHud )
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original takes the tail of the free list even when it's empty, which corrupts the lists
	if ( m_llFreeItems.IsEmpty() )
	{
		TWARN( "APlayerSimProfile: no free items left\n" );
		return;
	}
#endif // BARNYARD_COMMUNITY_PATCH

	Item* pItem = m_llFreeItems.Tail();
	pItem->Remove();
	pItem->m_iText   = TINT( TREINTERPRETCAST( TUINTPTR, a_wszText ) );
	pItem->m_strName = a_strName;
	pItem->m_iTextId = a_iTextId;
	m_llUsedItems.InsertTail( pItem );

	if ( a_bShowHud )
	{
		ASimPlayerMainHudElement* pMainHud = AHudManager::GetSingleton()->FindElement<ASimPlayerMainHudElement>();

		if ( pMainHud )
			pMainHud->ShowMessageIcon();
	}
}

// $Barnyard: FUNCTION 00621980
void APlayerSimProfile::AddPendingItem( const TWCHAR* a_wszText, Toshi::TPString8 a_strName, TINT a_iTextId )
{
	if ( a_wszText == TNULL || a_strName.IsEmpty() )
		return;

	ReleaseItemFromList( m_llPendingItems, a_strName );

	if ( m_llPendingItems.IsEmpty() )
		m_fPendingItemsDelay = 10.0f;

#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: the original takes the tail of the free list even when it's empty, which corrupts the lists
	if ( m_llFreeItems.IsEmpty() )
	{
		TWARN( "APlayerSimProfile: no free items left\n" );
		return;
	}
#endif // BARNYARD_COMMUNITY_PATCH

	Item* pItem = m_llFreeItems.Tail();
	pItem->Remove();
	pItem->m_iText   = TINT( TREINTERPRETCAST( TUINTPTR, a_wszText ) );
	pItem->m_strName = a_strName;
	pItem->m_iTextId = a_iTextId;
	m_llPendingItems.InsertTail( pItem );
}

// $Barnyard: FUNCTION 00621830
void APlayerSimProfile::ReleaseItemFromList( Toshi::TDList<Item>& a_rList, Toshi::TPString8 a_strName )
{
	for ( auto it = a_rList.Begin(); it != a_rList.End(); it++ )
	{
		if ( it->m_strName == a_strName )
		{
			it->Remove();
			m_llFreeItems.InsertTail( it );
			return;
		}
	}
}

// $Barnyard: FUNCTION 00621af0
void APlayerSimProfile::ClearSquirtedUnits()
{
	for ( TINT i = 0; i < NUM_SQUIRTED_UNITS; i++ )
		m_aSquirtedUnits[ i ].SetPooledString( TNULL );
}

// $Barnyard: FUNCTION 00621b70
TBOOL APlayerSimProfile::HasSquirtedUnit( Toshi::TPString8 a_strName )
{
	for ( TINT i = 0; i < NUM_SQUIRTED_UNITS; i++ )
	{
		if ( a_strName == m_aSquirtedUnits[ i ] )
			return TTRUE;
	}

	return TFALSE;
}

// $Barnyard: FUNCTION 00621050
TINT APlayerSimProfile::GetNumSquirtedUnits()
{
	TINT iCount = 0;

	for ( TINT i = 0; i < NUM_SQUIRTED_UNITS; i++ )
	{
		if ( !m_aSquirtedUnits[ i ].IsEmpty() )
			iCount += 1;
	}

	return iCount;
}

// $Barnyard: FUNCTION 006223a0
void APlayerSimProfile::AddSquirtedUnit( Toshi::TPString8 a_strName )
{
	TINT iNumNames = GetNumSquirtedUnits();

	// NOTE: only 19 of the 20 slots are ever used
	if ( GetNumSquirtedUnits() >= NUM_SQUIRTED_UNITS - 1 )
		return;

	if ( HasSquirtedUnit( a_strName ) )
		return;

	// The chicks don't count
	if ( a_strName == TPS8D( "chick_01" ) || a_strName == TPS8D( "chick_02" ) || a_strName == TPS8D( "chick_03" ) ||
	     a_strName == TPS8D( "chick_04" ) || a_strName == TPS8D( "chick_05" ) || a_strName == TPS8D( "chick_06" ) ||
	     a_strName == TPS8D( "chick_07" ) || a_strName == TPS8D( "" ) )
		return;

	// NOTE: assumes the names are packed at the start of the array
	m_aSquirtedUnits[ iNumNames ] = a_strName;
}

// $Barnyard: FUNCTION 00620ed0
const TCHAR* APlayerSimProfile::GetHideTextureName( TINT a_iCostume, GENDER a_eGender ) const
{
	static const TCHAR* s_aBoyHides[] = {
		"Hide_American",
		"Hide_Angus",
		"Hide_Beefmaster",
		"Hide_Spaniard",
		"Hide_Longhorn",
		"Hide_Brahman",
		"Hide_Holsien"
	}; // 0x00778ccc

	static const TCHAR* s_aGirlHides[] = {
		"Hide_Girl_American",
		"Hide_Girl_Angus",
		"Hide_Girl_Beefmaster",
		"Hide_Girl_Spaniard",
		"Hide_Girl_Longhorn",
		"Hide_Girl_Brahman",
		"Hide_Girl_Holsien"
	}; // 0x00778ce8

	if ( a_eGender == GENDER_NONE ) a_eGender = m_eGender;
	if ( a_iCostume == -1 ) a_iCostume = m_iCostume;

	return ( a_eGender != GENDER_MALE ) ? s_aGirlHides[ a_iCostume ] : s_aBoyHides[ a_iCostume ];
}
