#include "pch.h"
#include "ARacePlacingHudElement.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004570d0, CreateObject 004573a0, CreateObjectInPlace 00457400
TDEFINE_CLASS( ARacePlacingHudElement );

static constexpr TUINT32 TEXT_COLOUR = 0xFFFFC800;

// $Barnyard: FUNCTION 004570e0
ARacePlacingHudElement::ARacePlacingHudElement()
    : AHudElement( MASK_MINIGAME )
    , m_iPlacing( 0 )
{
}

// $Barnyard: FUNCTION 00457150
// $Barnyard: FUNCTION 00457440
ARacePlacingHudElement::~ARacePlacingHudElement()
{
}

// $Barnyard: FUNCTION 00457460
TBOOL ARacePlacingHudElement::OnAttach()
{
	// Wood backing in the top right corner
	m_oBacking.SetTextureSection( *T2GUITextureSectionManager::GetTextureSection( "Score_Wood_Thin" ) );
	AGUI2::GetRootElement()->AddChildTail( m_oBacking );
	m_oBacking.Hide();
	m_oBacking.SetWidth( m_oBacking.GetWidth() * 0.65f );

	// The number
	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );
	m_oNumberText.Create( pFont, 0.0f );
	m_oNumberText.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oNumberText.GetTransform().SetTranslation( 10.0f, 0.0f );
	m_oNumberText.GetTransform().Scale( 2.0f, 2.0f );
	m_oNumberText.SetColour( TEXT_COLOUR );
	m_oBacking.AddChildTail( m_oNumberText );

	// The suffix ("st", "nd"...)
	m_oSuffixText.Create( pFont, 0.0f );
	m_oSuffixText.SetAttachment( T2GUIATTACHMENT_MIDDLELEFT, T2GUIATTACHMENT_MIDDLELEFT );
	m_oSuffixText.SetColour( TEXT_COLOUR );
	m_oBacking.AddChildTail( m_oSuffixText );

	m_oNumberText.SetTextAndFitWidth( L"1" );
	m_oSuffixText.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( 267 ) );

	// NOTE: SetPlacing uses 12 instead of 10 for the gap
	TFLOAT fNumberWidth = m_oNumberText.GetWidth();
	m_oSuffixText.GetTransform().SetTranslation( fNumberWidth + fNumberWidth + 10.0f, 0.0f );

	m_oBacking.Hide();
	m_oBacking.SetAttachment( T2GUIATTACHMENT_TOPRIGHT, T2GUIATTACHMENT_TOPRIGHT );
	m_oBacking.SetTransform( -20.0f, 30.0f, 0.0f );

	return TTRUE;
}

// $Barnyard: FUNCTION 004571d0
void ARacePlacingHudElement::OnDetach()
{
	m_oNumberText.RemoveSelf();
	m_oSuffixText.RemoveSelf();
	m_oBacking.RemoveSelf();
}

// NOTE: folded with ASimPlayerPickUpHudElement::Show (00452790)
void ARacePlacingHudElement::Show()
{
	m_oBacking.T2GUIElement::Show();
	m_bCurrentlyVisible = TTRUE;
}

// NOTE: folded with ASimPlayerPickUpHudElement::Hide (00457210)
void ARacePlacingHudElement::Hide()
{
	m_oBacking.T2GUIElement::Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00457220
void ARacePlacingHudElement::SetPlacing( TINT a_iPlacing )
{
	if ( m_iPlacing == a_iPlacing )
		return;

	m_iPlacing = a_iPlacing;

	const TWCHAR* wszNumber;
	TINT          iSuffix;

	switch ( a_iPlacing )
	{
		case 0:
			wszNumber = L"1";
			iSuffix   = 267;
			break;
		case 1:
			wszNumber = L"2";
			iSuffix   = 268;
			break;
		case 2:
			wszNumber = L"3";
			iSuffix   = 269;
			break;
		case 3:
			wszNumber = L"4";
			iSuffix   = 270;
			break;
		default:
			wszNumber = L"5";
			iSuffix   = 270;
			break;
	}

	m_oNumberText.SetTextAndFitWidth( wszNumber );
	m_oSuffixText.SetTextAndFitWidth( ALocaleManager::Interface()->GetString( iSuffix ) );

	TFLOAT fNumberWidth = m_oNumberText.GetWidth();
	m_oSuffixText.GetTransform().SetTranslation( fNumberWidth + fNumberWidth + 12.0f, 0.0f );
}
