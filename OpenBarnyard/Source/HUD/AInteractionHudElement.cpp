#include "pch.h"
#include "AInteractionHudElement.h"
#include "GUI/AGUI2.h"
#include "Locale/ALocale.h"

#include <GUI/T2GUI.h>
#include <GUI/T2GUIFontManager.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 004543f0, CreateObject 00454680, CreateObjectInPlace 004546e0
TDEFINE_CLASS( AInteractionHudElement );

// $Barnyard: FUNCTION 00454400
AInteractionHudElement::AInteractionHudElement()
    : AHudElement( MASK_INTERACTION )
    , m_pParent( TNULL )
{
	m_abShowLine[ 0 ] = TFALSE;
	m_abShowLine[ 1 ] = TFALSE;
}

// $Barnyard: FUNCTION 00454470
// $Barnyard: FUNCTION 00454720
AInteractionHudElement::~AInteractionHudElement()
{
}

// $Barnyard: FUNCTION 00454740
TBOOL AInteractionHudElement::OnAttach()
{
	if ( !m_pParent )
		m_pParent = AGUI2::GetRootElement();

	m_pFont = T2GUIFontManager::FindFont( "Rekord18" );

	TFLOAT fScreenWidth;
	TFLOAT fScreenHeight;
	T2GUI::GetContext()->GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );

	for ( TINT i = 0; i < NUM_LINES; i++ )
	{
		AGUI2TextBox& rLine = m_aLines[ i ];
		rLine.Create( m_pFont, 0.0f );
		rLine.SetAttachment( T2GUIATTACHMENT_BOTTOMCENTER, T2GUIATTACHMENT_BOTTOMCENTER );
		rLine.T2GUIElement::Show();
		rLine.SetColour( 0xFFFFFFFF );
		rLine.SetShadowEnabled( TTRUE );
		rLine.SetShadowColour( 0xA0000000 );
		rLine.SetShadowAlpha( 0.6f );
		rLine.SetShadowOffset( 3.5f, 3.5f );
		rLine.SetSize( 0.7f * fScreenWidth, 0.0f, TFALSE );

		TFLOAT fLineHeight = m_pFont->GetLineHeight();

		if ( i == 0 )
		{
			rLine.SetTransform( 0.0f, -fLineHeight );
			m_pParent->AddChildTail( rLine );
		}
		else
		{
			// The second line is above the first one
			rLine.SetTransform( 0.0f, fLineHeight * -1.5f );
			m_aLines[ 0 ].AddChildTail( rLine );
			rLine.T2GUIElement::Hide();
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 004544d0
void AInteractionHudElement::OnDetach()
{
	for ( TINT i = 0; i < NUM_LINES; i++ )
		m_aLines[ i ].RemoveSelf();

	T2GUIFontManager::ReleaseFont( m_pFont );
}

// $Barnyard: FUNCTION 00454510
void AInteractionHudElement::Show()
{
	m_bCurrentlyVisible = TTRUE;
	m_aLines[ 0 ].SetVisible( m_abShowLine[ 0 ] == TTRUE );
	m_aLines[ 1 ].SetVisible( m_abShowLine[ 1 ] == TTRUE );
}

// $Barnyard: FUNCTION 00454550
void AInteractionHudElement::Hide()
{
	for ( TINT i = 0; i < NUM_LINES; i++ )
		m_aLines[ i ].T2GUIElement::Hide();

	m_bCurrentlyVisible = TFALSE;
}

// $Barnyard: FUNCTION 00454570
void AInteractionHudElement::SetText( const TWCHAR* a_wszText, TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iLine, TUINT32 a_uiColour, TFLOAT a_fWidth )
{
	AGUI2TextBox& rLine = m_aLines[ a_iLine ];

	if ( a_wszText )
	{
		m_abShowLine[ a_iLine ] = TTRUE;
		rLine.T2GUIElement::Show();

		ALocale::FormatInputTags( m_aTexts[ a_iLine ], MAX_LENGTH, a_wszText );
		rLine.SetText( m_aTexts[ a_iLine ] );

		if ( -1.0f != a_fWidth && a_fWidth >= 0.0f )
			rLine.SetWidth( a_fWidth );

		rLine.SetColour( a_uiColour );
	}
	else
	{
		rLine.T2GUIElement::Hide();
		m_abShowLine[ a_iLine ] = TFALSE;
	}
}

// $Barnyard: FUNCTION 00454620
void AInteractionHudElement::UpdateWidth()
{
	TFLOAT fScreenWidth;
	TFLOAT fScreenHeight;
	T2GUI::GetContext()->GetRootElement()->GetDimensions( fScreenWidth, fScreenHeight );

	for ( TINT i = 0; i < NUM_LINES; i++ )
		m_aLines[ i ].SetSize( 0.7f * fScreenWidth, 0.0f, TFALSE );
}
