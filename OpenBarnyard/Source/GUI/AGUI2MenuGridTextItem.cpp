#include "pch.h"
#include "AGUI2MenuGridTextItem.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

AGUI2MenuGridTextItem::AGUI2MenuGridTextItem()
{
	m_bScaleOnFocus     = TTRUE;
	m_bFocused          = TFALSE;
	m_fRectangle1Width  = 0.0f;
	m_fRectangle1Height = 0.0f;
	m_bUseRectangle1    = TFALSE;
	m_Unk1              = 0;
	m_uiFocusedColour   = 0xffffff80;
	m_uiEnabledColour   = 0xffffffff;
	m_uiDisabledColour  = 0xffc0c0c0;
	m_fFocusedScale     = 1.25f;
	m_fScale            = 1.0f;
	m_pFont             = TNULL;
	m_fGap              = 0.0f;
	m_wszText           = TNULL;
}

AGUI2MenuGridTextItem::~AGUI2MenuGridTextItem()
{
	delete[] m_wszText;
	m_oTextBox.RemoveSelf();
	m_oRectangle2.RemoveSelf();
}

void AGUI2MenuGridTextItem::Create( AGUI2Font* a_pFont, TFLOAT a_fWidth, const TWCHAR* a_wszText, TFLOAT a_fGap )
{
	m_fWidth = a_fWidth;
	m_fGap   = a_fGap;
	m_pFont  = a_pFont;

	m_oTextBox.Create( a_pFont, a_fWidth );
	m_oTextBox.SetColour( m_uiEnabledColour );

	// Delete previously allocated string
	if ( m_wszText )
	{
		delete[] m_wszText;
		m_wszText = TNULL;
	}

	// Copy string
	if ( a_wszText )
		m_wszText = T2String16::CreateCopy( a_wszText );

	m_oTextBox.SetText( m_wszText );
	AddChildTail( m_oTextBox );

	m_oTextBox.SetTextAlign( AGUI2Font::TextAlign_Center );
	AGUI2MenuGridItem::SetDimensions( m_oTextBox.GetWidth(), m_oTextBox.GetHeight() );
}

void AGUI2MenuGridTextItem::SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight )
{
	m_oTextBox.SetWidth( a_fWidth );
	AGUI2MenuGridItem::SetDimensions( a_fWidth, a_fHeight );
}

void AGUI2MenuGridTextItem::SetAlpha( TFLOAT a_fAlpha )
{
	m_oTextBox.SetAlpha( a_fAlpha );
	m_oRectangle1.SetAlpha( a_fAlpha );
	m_oRectangle2.SetAlpha( a_fAlpha );
}

void AGUI2MenuGridTextItem::SetShadowAlpha( TFLOAT a_fAlpha )
{
	m_oTextBox.SetShadowAlpha( a_fAlpha );
}

void AGUI2MenuGridTextItem::OnFocus()
{
	m_bFocused = TTRUE;

	if ( m_bScaleOnFocus )
	{
		m_oTextBox.SetScale( m_fFocusedScale );
		m_oTextBox.SetWidth( m_fWidth * m_fFocusedScale );
	}

	m_oTextBox.SetColour( m_uiFocusedColour );

	if ( m_bUseRectangle1 )
	{
		TFLOAT fWidth, fHeight;
		GetDimensions( fWidth, fHeight );

		m_oRectangle1.SetDimensions( fWidth + m_fRectangle1Width, fHeight + m_fRectangle1Height );
		LinkChildBefore( m_oRectangle1, m_oTextBox );
	}
}

void AGUI2MenuGridTextItem::OnFocusLost()
{
	m_bFocused = TFALSE;

	if ( m_bScaleOnFocus )
	{
		m_oTextBox.SetScale( m_fScale );
		m_oTextBox.SetWidth( m_fWidth * m_fScale );
	}

	m_oTextBox.SetColour( m_uiEnabledColour );

	if ( m_bUseRectangle1 )
		m_oRectangle1.RemoveSelf();
}

void AGUI2MenuGridTextItem::SetEnabled( TBOOL a_bEnabled )
{
	AGUI2MenuGridItem::SetEnabled( a_bEnabled );
	m_oTextBox.SetColour( IsEnabled() ? m_uiEnabledColour : m_uiDisabledColour );
}

TFLOAT AGUI2MenuGridTextItem::GetFlowVisualOffset()
{
	TVALIDPTR( m_pFont );
	return m_pFont->GetLinesMargin() + m_fGap;
}

TFLOAT AGUI2MenuGridTextItem::GetFlowOffset()
{
	TVALIDPTR( m_pFont );
	return m_pFont->GetLinesMargin() + m_fGap;
}
