#include "pch.h"
#include "AItemCountHudElement.h"
#include "AHudManager.h"
#include "ASimPlayerPickUpHudElement.h"
#include "GUI/AGUI2.h"
#include "Helpers/AGameLoader.h"
#include "Helpers/ATreasureManager.h"
#include "Helpers/AHintManager.h"
#include "Helpers/ABikeManager.h"
#include "Player/APlayerManager.h"
#include "Player/APlayerProfileManager.h"
#include "Units/AInventoryGroupComponent.h"
#include "Units/AUnit.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>
#include <Toshi/TStringManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TPSTRING8_DECLARE( jp_fan );
TPSTRING8_DECLARE( jp_gramophone );
TPSTRING8_DECLARE( jp_umbrella );
TPSTRING8_DECLARE( jp_wateringcan );
TPSTRING8_DECLARE( jp_wheel );
TPSTRING8_DECLARE( tomatolaunch_battery );
TPSTRING8_DECLARE( tomatolaunch_cog );
TPSTRING8_DECLARE( tomatolaunch_pogospring );
TPSTRING8_DECLARE( tomatolaunch_steerwheel );
TPSTRING8_DECLARE( tomatolaunch_rope );
TPSTRING8_DECLARE( carrot );
TPSTRING8_DECLARE( cabbage );
TPSTRING8_DECLARE( pumpkin );
TPSTRING8_DECLARE( apple );
TPSTRING8_DECLARE( berrys );
TPSTRING8_DECLARE( strawberry );

// NOTE: GetClass 00454870, CreateObject 004556a0, CreateObjectInPlace 00455700
TDEFINE_CLASS( AItemCountHudElement );

// $Barnyard: FUNCTION 00454e50
AItemCountHudElement::AItemCountHudElement()
    : AHudElement( MASK_ITEM_COUNT )
    , m_bVisible( TFALSE )
    , m_iCount( 0 )
    , m_iMaxCount( 0 )
    , m_eMode( MODE_MANUAL )
{
}

// $Barnyard: FUNCTION 00454f10
// $Barnyard: FUNCTION 00455740
AItemCountHudElement::~AItemCountHudElement()
{
}

// $Barnyard: FUNCTION 00454990
TBOOL AItemCountHudElement::OnAttach()
{
	// The wooden backing in the top right corner
	T2GUITextureSection* pBacking = T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" );
	m_oBacking.SetDimensions( pBacking->GetWidth() * 0.6f, pBacking->GetHeight() );
	m_oBacking.SetTextureSection( pBacking );
	m_oBacking.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );

	// The icon on its left
	T2GUITextureSection* pIcon = T2GUITextureSectionManager::GetTextureSection( "veggies" );
	m_oIcon.SetDimensions( pIcon->GetWidth(), pIcon->GetHeight() );
	m_oIcon.SetTextureSection( pIcon );
	m_oIcon.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oIcon.SetTransform( 0.0f, 0.0f );
	m_oBacking.AddChildTail( m_oIcon );

	// The counter on its right, as wide as two "99/99"
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_iCount         = 99;
	m_iMaxCount      = 99;
	TStringManager::String16Format( m_wszCount, 14, L"%d/%d", 99, 99 );

	TFLOAT fTextWidth = pFont->GetTextWidth( m_wszCount, 1.0f );
	m_oText.Create( pFont, fTextWidth + fTextWidth );

	m_iCount = 0;
	TStringManager::String16Format( m_wszCount, 8, L"%d", 0 );
	m_oText.SetTextAndFitWidth( m_wszCount );
	m_oText.SetAttachment( T2GUIATTACHMENT_MIDDLERIGHT, T2GUIATTACHMENT_MIDDLERIGHT );
	m_oText.SetTransform( -20.0f, 0.0f );
	m_oText.SetTextAlign( T2GUIFont::TextAlign_Left );
	m_oBacking.AddChildTail( m_oText );

	AGUI2::GetRootElement()->AddChildTail( m_oBacking );

	m_bVisible = TFALSE;
	m_oBacking.T2GUIElement::Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 00454910
void AItemCountHudElement::OnDetach()
{
	m_oIcon.RemoveSelf();
	m_oText.RemoveSelf();
	m_oBacking.RemoveSelf();
}

// $Barnyard: FUNCTION 00454c70
void AItemCountHudElement::Show()
{
	m_oBacking.SetVisible( m_bVisible == TTRUE );

	// Place the counter below ASimPlayerPickUpHudElement
	ASimPlayerPickUpHudElement* pPickUpElement = AHudManager::GetSingleton()->FindElement<ASimPlayerPickUpHudElement>();
	m_oBacking.SetWidth( pPickUpElement->m_oBacking.GetWidth() );
	m_oBacking.SetTranslation( 0.0f, pPickUpElement->m_oBacking.GetHeight() * 1.1f );

	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00454ce0
void AItemCountHudElement::Hide()
{
	if ( m_bCurrentlyVisible )
	{
		// Remember whether the counter was shown
		m_bVisible = m_oBacking.IsVisible();
		m_oBacking.T2GUIElement::Hide();
	}

	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00455040
void AItemCountHudElement::Update( TFLOAT a_fDeltaTime )
{
	AInventoryGroupComponent* pInventory = TSTATICCAST( AInventoryGroupComponent, APlayerManager::GetSingleton()->GetPlayerUnit( 0 )->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

	if ( !pInventory )
	{
		// The player rides a bike, count the items of the rider
		AUnit* pRider = ABikeManager::GetSingleton()->GetRider();

		if ( !pRider )
			return;

		pInventory = TSTATICCAST( AInventoryGroupComponent, pRider->FindComponent( &TGetClass( AInventoryGroupComponent ) ) );

		if ( !pInventory )
			return;
	}

	if ( !m_oBacking.IsVisible() )
		return;

	if ( !m_strItem.IsEmpty() && m_eMode == MODE_INVENTORY )
	{
		// The item or a bunch of them
		TString8 strBunch;
		m_iCount = pInventory->GetNumItemsOfType( m_strItem );
		strBunch.Format( "%s_bunch", m_strItem.GetString() );

		TINT iNumBunches = pInventory->GetNumItemsOfType( TPString8( strBunch.GetString() ) );

		if ( iNumBunches <= 0 )
			SetCountAndMax( m_iCount, m_iMaxCount );
		else
			SetCountAndMax( m_iMaxCount, m_iMaxCount );

		return;
	}

	switch ( m_eMode )
	{
		case MODE_NUMJUNK:
			m_iCount = 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( jp_fan ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( jp_gramophone ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( jp_umbrella ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( jp_wateringcan ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( jp_wheel ) ) > 0;
			break;
		case MODE_NUMTLPARTS:
			m_iCount = 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( tomatolaunch_battery ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( tomatolaunch_cog ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( tomatolaunch_pogospring ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( tomatolaunch_steerwheel ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( tomatolaunch_rope ) ) > 0;
			break;
		case MODE_NUMLEAKS:
			if ( AGameLoader::IsBarnExploreMiniGame() )
				return;

			m_iCount = m_iMaxCount - ATreasureManager::GetSingleton()->CountType( ATreasureManager::TYPE_LEAK );
			break;
		case MODE_NUMROCKS:
			// NOTE: the rocks are counted as AHintManager recipes (AddRecipe)
			m_iCount = AHintManager::GetNumRecipes();
			break;
		case MODE_SQUIRT:
			m_iCount = APlayerProfileManager::GetSingleton()->GetMainProfile()->GetNumSquirtedUnits();
			break;
		case MODE_NUMFREDDYVEG:
			m_iCount = 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( carrot ) ) > 0;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( cabbage ) ) > 0;
			m_iMaxCount = 3;
			m_iCount += pInventory->GetNumItemsOfType( TPS8( pumpkin ) ) > 0;
			break;
		case MODE_NUMFREDDYFRUIT:
			m_iCount = 0;
			m_iCount += TMath::Min( pInventory->GetNumItemsOfType( TPS8( apple ) ), 3 );
			m_iCount += TMath::Min( pInventory->GetNumItemsOfType( TPS8( berrys ) ), 3 );
			m_iCount += TMath::Min( pInventory->GetNumItemsOfType( TPS8( strawberry ) ), 3 );
			m_iMaxCount = 9;
			break;
		default:
			return;
	}

	SetCountAndMax( m_iCount, m_iMaxCount );
}

// $Barnyard: FUNCTION 00454880
void AItemCountHudElement::SetCountAndMax( TINT a_iCount, TINT a_iMaxCount )
{
	if ( a_iCount >= a_iMaxCount )
		a_iCount = a_iMaxCount;

	m_iCount = a_iCount;

	// NOTE: the original uses the unbounded T2String16::Format (swprintf)
	TStringManager::String16Format( m_wszCount, TARRAYSIZE( m_wszCount ), L"%d/%d", a_iCount, a_iMaxCount );
	m_oText.SetTextAndFitWidth( m_wszCount );
}

// $Barnyard: FUNCTION 00454b50
void AItemCountHudElement::SetCount( TINT a_iCount, TBOOL a_bWithMax )
{
	if ( a_bWithMax == TTRUE )
	{
		SetCountAndMax( a_iCount, m_iMaxCount );
	}
	else
	{
		m_iCount    = a_iCount;
		m_iMaxCount = -1;
		TStringManager::String16Format( m_wszCount, 8, L"%d", a_iCount );
		m_oText.SetTextAndFitWidth( m_wszCount );
	}
}

// $Barnyard: FUNCTION 00454be0
void AItemCountHudElement::AddCount( TINT a_iCount )
{
	m_iCount += a_iCount;

	if ( m_iMaxCount <= 0 )
	{
		m_iMaxCount = -1;
		TStringManager::String16Format( m_wszCount, 8, L"%d", m_iCount );
		m_oText.SetTextAndFitWidth( m_wszCount );
	}
	else
	{
		SetCountAndMax( m_iCount, m_iMaxCount );
	}
}

// $Barnyard: FUNCTION 00454d20
void AItemCountHudElement::SetIcon( const TCHAR* a_szIcon )
{
	TStringManager::String8Copy( m_szIcon, a_szIcon );

	T2GUITextureSection* pIcon = TNULL;

	if ( a_szIcon )
		pIcon = T2GUITextureSectionManager::GetTextureSection( a_szIcon );

	if ( !pIcon )
		pIcon = T2GUITextureSectionManager::GetTextureSection( "Place_Holder" );

	if ( pIcon )
	{
		m_oIcon.SetTextureSection( pIcon );
		m_oIcon.SetWidth( pIcon->GetWidth() );
		m_oIcon.SetHeight( pIcon->GetHeight() );
	}
}

// $Barnyard: FUNCTION 00455530
void AItemCountHudElement::SetIcon( TPString8 a_strIcon )
{
	SetIcon( a_strIcon.GetString() );
}

// $Barnyard: FUNCTION 00454fc0
void AItemCountHudElement::SetItemName( TPString8 a_strItem )
{
	m_strItem = a_strItem;
}

// $Barnyard: FUNCTION 00454960
void AItemCountHudElement::SetVisibleForce( TBOOL a_bVisible )
{
	m_bVisible = a_bVisible;
	m_oBacking.SetVisible( a_bVisible == TTRUE );
}

// $Barnyard: FUNCTION 00454db0
void AItemCountHudElement::SaveState( State* a_pState )
{
	a_pState->bSaved = TTRUE;

	if ( !m_strItem.IsEmpty() )
		TStringManager::String8Copy( a_pState->szItem, m_strItem.GetString() );
	else
		TStringManager::String8Copy( a_pState->szItem, "none" );

	TStringManager::String8Copy( a_pState->szIcon, m_szIcon );
	a_pState->iCount    = m_iCount;
	a_pState->iMaxCount = m_iMaxCount;
	a_pState->bVisible  = m_bVisible;
	a_pState->eMode     = m_eMode;
}

// $Barnyard: FUNCTION 00455590
void AItemCountHudElement::LoadState( State* a_pState )
{
	m_iCount    = a_pState->iCount;
	m_iMaxCount = a_pState->iMaxCount;
	m_bVisible  = a_pState->bVisible;
	m_eMode     = a_pState->eMode;
	m_strItem   = TPString8( a_pState->szItem );

	if ( m_bVisible )
	{
		TStringManager::String8Copy( m_szIcon, a_pState->szIcon );
		SetIcon( m_szIcon );
		SetCountAndMax( m_iCount, m_iMaxCount );
	}

	Show();
}
