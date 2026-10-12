#include "pch.h"
#include "AMenuNavHudElement.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"
#include "Locale/ALocaleManager.h"

#include <GUI/T2GUIFontManager.h>
#include <GUI/T2GUITextureSectionManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00455a40, CreateObject 00455ec0, CreateObjectInPlace 00455f20
TDEFINE_CLASS( AMenuNavHudElement );

// NOTE: inlined in retail, the text box fits the text unless its width is fixed
static void SetNavText( AGUI2TextBox& a_rText, const TWCHAR* a_wszText, TBOOL a_bFixedWidth )
{
	if ( a_bFixedWidth )
		a_rText.SetText( a_wszText );
	else
		a_rText.SetTextAndFitWidth( a_wszText );
}

// $Barnyard: FUNCTION 00455a50
AMenuNavHudElement::AMenuNavHudElement()
    : AHudElement( MASK_MENU_NAV )
{
}

// $Barnyard: FUNCTION 00455ac0
// $Barnyard: FUNCTION 00455f60
AMenuNavHudElement::~AMenuNavHudElement()
{
}

// $Barnyard: FUNCTION 00455f80
TBOOL AMenuNavHudElement::OnAttach()
{
	m_abFixedWidth[ 0 ] = TFALSE;
	m_abFixedWidth[ 1 ] = TFALSE;
	m_abFixedWidth[ 2 ] = TFALSE;

	m_apwszTexts[ 0 ] = TSTATICCAST( TWCHAR, TMalloc( MAX_LENGTH * sizeof( TWCHAR ) ) );
	m_apwszTexts[ 1 ] = TSTATICCAST( TWCHAR, TMalloc( MAX_LENGTH * sizeof( TWCHAR ) ) );
	m_apwszTexts[ 2 ] = TSTATICCAST( TWCHAR, TMalloc( MAX_LENGTH * sizeof( TWCHAR ) ) );

	ALocale::FormatInputTags( m_apwszTexts[ 0 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( 1047 ) );
	ALocale::FormatInputTags( m_apwszTexts[ 1 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( 1048 ) );

	T2GUIFont* pFont = T2GUIFontManager::FindFont( "Rekord26" );

	// The bar at the bottom of the screen
	T2GUITextureSection* pBacking = T2GUITextureSectionManager::GetTextureSection( "Wood_Thin" );
	m_oBacking.SetDimensions( AGUI2::GetRootElement()->GetWidth(), pBacking->GetHeight() );
	m_oBacking.SetTextureSection( pBacking );
	m_oBacking.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
	AGUI2::GetRootElement()->AddChildTail( m_oBacking );

	m_fSpacing = m_oBacking.GetWidth() * 0.25f;

	// The text on the left...
	m_aTexts[ 0 ].Create( pFont, 0.0f );
	SetNavText( m_aTexts[ 0 ], m_apwszTexts[ 0 ], m_abFixedWidth[ 0 ] );
	m_aTexts[ 0 ].SetTransform( -m_fSpacing, 0.0f );
	m_aTexts[ 0 ].SetScale( 0.75f );

	// ...on the right...
	m_aTexts[ 1 ].Create( pFont, 0.0f );
	SetNavText( m_aTexts[ 1 ], m_apwszTexts[ 1 ], m_abFixedWidth[ 1 ] );
	m_aTexts[ 1 ].SetTransform( m_fSpacing, 0.0f );
	m_aTexts[ 1 ].SetScale( 0.75f );

	// ...and in the middle
	m_aTexts[ 2 ].Create( pFont, 0.0f );
	SetNavText( m_aTexts[ 2 ], m_apwszTexts[ 2 ], m_abFixedWidth[ 2 ] );
	m_aTexts[ 2 ].SetScale( 0.75f );
	m_aTexts[ 2 ].T2GUIElement::Hide();

	m_oBacking.AddChildTail( m_aTexts[ 0 ] );
	m_oBacking.AddChildTail( m_aTexts[ 1 ] );
	m_oBacking.AddChildTail( m_aTexts[ 2 ] );
	m_oBacking.T2GUIElement::Hide();

	return TTRUE;
}

// $Barnyard: FUNCTION 00455b30
void AMenuNavHudElement::OnDetach()
{
	TFree( m_apwszTexts[ 0 ] );
	TFree( m_apwszTexts[ 1 ] );
	TFree( m_apwszTexts[ 2 ] );
}

// $Barnyard: FUNCTION 004561e0
void AMenuNavHudElement::Show()
{
	AGUI2::GetRootElement()->AddChildTail( m_oBacking );
	m_oBacking.T2GUIElement::Show();
	m_bCurrentlyVisible = TTRUE;
}

// $Barnyard: FUNCTION 00455b60
void AMenuNavHudElement::Hide()
{
	m_oBacking.T2GUIElement::Hide();
	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00455b70
void AMenuNavHudElement::SetTexts( TINT a_iLeft, TINT a_iRight, TINT a_iCentre )
{
	if ( a_iLeft != -1 )
	{
		ALocale::FormatInputTags( m_apwszTexts[ 0 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( a_iLeft ) );
		SetNavText( m_aTexts[ 0 ], m_apwszTexts[ 0 ], m_abFixedWidth[ 0 ] );
	}

	if ( a_iRight != -1 )
	{
		ALocale::FormatInputTags( m_apwszTexts[ 1 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( a_iRight ) );
		SetNavText( m_aTexts[ 1 ], m_apwszTexts[ 1 ], m_abFixedWidth[ 1 ] );
	}

	if ( a_iCentre != -1 )
	{
		ALocale::FormatInputTags( m_apwszTexts[ 2 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( a_iCentre ) );
		SetNavText( m_aTexts[ 2 ], m_apwszTexts[ 2 ], m_abFixedWidth[ 2 ] );
	}

	m_aTexts[ 0 ].SetVisible( a_iLeft != -1 );
	m_aTexts[ 1 ].SetVisible( a_iRight != -1 );
	m_aTexts[ 2 ].SetVisible( a_iCentre != -1 );
}

// $Barnyard: FUNCTION 00455d90
void AMenuNavHudElement::SetDefaultTexts()
{
	ALocale::FormatInputTags( m_apwszTexts[ 0 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( 1047 ) );
	SetNavText( m_aTexts[ 0 ], m_apwszTexts[ 0 ], m_abFixedWidth[ 0 ] );

	ALocale::FormatInputTags( m_apwszTexts[ 1 ], MAX_LENGTH, ALocaleManager::Interface()->GetString( 1048 ) );
	SetNavText( m_aTexts[ 1 ], m_apwszTexts[ 1 ], m_abFixedWidth[ 1 ] );

	m_aTexts[ 0 ].T2GUIElement::Show();
	m_aTexts[ 1 ].T2GUIElement::Show();
	m_aTexts[ 2 ].T2GUIElement::Hide();
}

// $Barnyard: FUNCTION 00455d20
void AMenuNavHudElement::SetSpacing( TFLOAT a_fSpacing )
{
	m_aTexts[ 0 ].SetTransform( -( a_fSpacing * m_fSpacing ), 0.0f );
	m_aTexts[ 1 ].SetTransform( a_fSpacing * m_fSpacing, 0.0f );
}

// $Barnyard: FUNCTION 00455d70
void AMenuNavHudElement::SetY( TFLOAT a_fY )
{
	m_oBacking.SetTransform( m_oBacking.GetTransform().GetTranslation().x, a_fY );
}

// $Barnyard: FUNCTION 00455e70
void AMenuNavHudElement::SetTextScale( TFLOAT a_fScale )
{
	m_aTexts[ 0 ].SetScale( a_fScale );
	m_aTexts[ 1 ].SetScale( a_fScale );
	m_aTexts[ 2 ].SetScale( a_fScale );
}

// $Barnyard: FUNCTION 00455eb0
TFLOAT AMenuNavHudElement::GetTextScale()
{
	return m_aTexts[ 0 ].GetScale();
}
