#include "pch.h"
#include "AGUI2Button.h"
#include "AGUI2TextureSectionManager.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AGUI2Button::AGUI2Button()
{
	m_bScaleOnFocus     = TTRUE;
	m_bFocused          = TFALSE;
	m_fRectangle1Width  = 0.0f;
	m_fRectangle1Height = 0.0f;
	m_bUseRectangle1    = TFALSE;
	m_Unk3              = 0;
	m_uiFocusedColour   = 0xffffff80;
	m_uiEnabledColour   = 0xffffffff;
	m_uiDisabledColour  = 0xffc0c0c0;
	m_fFocusedScale     = 1.25f;
	m_fScale            = 1.0f;
	m_pFont             = TNULL;
	m_fGap              = 0.0f;
	m_wszButtonText     = TNULL;
}

AGUI2Button::~AGUI2Button()
{
	m_oBackgroundRect.Unlink();
}

void AGUI2Button::Create( AGUI2Font* a_pFont, TFLOAT a_fWidth, const TWCHAR* a_wszText, TFLOAT a_fGap )
{
	TVALIDPTR( a_pFont );
	TVALIDPTR( a_wszText );

	m_fGap  = a_fGap;
	m_pFont = a_pFont;

	m_oText.Create( a_pFont, a_fWidth );
	SetButtonColour( m_uiEnabledColour );

	if ( m_bScaleOnFocus )
		m_oTransform.SetScale( m_fScale, m_fScale );

	m_oText.SetText( a_wszText );
	AddChildTail( m_oText );

	m_oText.SetAttachment( AGUI2ATTACHMENT_MIDDLECENTER, AGUI2ATTACHMENT_MIDDLECENTER );
	m_oText.SetTextAlign( AGUI2Font::TextAlign_Center );
}

void AGUI2Button::SetImage( const TCHAR* a_szTexSec, TFLOAT a_fScaleX, TFLOAT a_fScaleY )
{
	TVALIDPTR( a_szTexSec );

	AGUI2TextureSection* pTexSec = AGUI2TextureSectionManager::GetTextureSection( a_szTexSec );
	TVALIDPTR( pTexSec );

	TFLOAT fHeight = m_oText.GetHeight() * a_fScaleY;
	TFLOAT fWidth  = m_oText.GetWidth() * a_fScaleX;
	m_oBackgroundRect.SetDimensions( fWidth, fHeight );
	m_oBackgroundRect.SetTextureSection( pTexSec );
	m_oBackgroundRect.SetAttachment( AGUI2ATTACHMENT_MIDDLECENTER, AGUI2ATTACHMENT_MIDDLECENTER );
	LinkChildBefore( m_oBackgroundRect, m_oText );
}

void AGUI2Button::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	m_oText.GetDimensions( a_rWidth, a_rHeight );
}

void AGUI2Button::SetAlpha( TFLOAT a_fAlpha )
{
	m_oText.SetAlpha( a_fAlpha );
	m_Rectangle1.SetAlpha( a_fAlpha );
	m_oBackgroundRect.SetAlpha( a_fAlpha );

	for ( auto it = m_oText.ChildRBegin(); it != TNULL; it = m_oText.GetPrevChild( it ) )
	{
		it->SetAlpha( a_fAlpha );
	}

	AGUI2MenuItem::SetAlpha( a_fAlpha );
}

void AGUI2Button::SetShadowAlpha( TFLOAT a_fAlpha )
{
	m_oText.SetShadowAlpha( a_fAlpha );

	for ( auto it = m_oText.ChildRBegin(); it != TNULL; it = m_oText.GetPrevChild( it ) )
	{
		it->SetShadowAlpha( a_fAlpha );
	}

	AGUI2MenuItem::SetShadowAlpha( a_fAlpha );
}

void AGUI2Button::OnFocus()
{
	m_bFocused = TTRUE;

	if ( m_bScaleOnFocus )
		m_oTransform.SetScale( m_fFocusedScale, m_fFocusedScale );

	SetButtonColour( m_uiFocusedColour );

	if ( m_bUseRectangle1 )
	{
		TFLOAT fWidth, fHeight;
		GetDimensions( fWidth, fHeight );

		m_Rectangle1.SetDimensions( fWidth + m_fRectangle1Width, fHeight + m_fRectangle1Height );
		LinkChildBefore( m_Rectangle1, m_oText );
	}
}

void AGUI2Button::OnFocusLost()
{
	m_bFocused = TFALSE;

	if ( m_bScaleOnFocus )
		m_oTransform.SetScale( m_fScale, m_fScale );

	SetButtonColour( IsEnabled() ? m_uiEnabledColour : m_uiDisabledColour );

	if ( m_bUseRectangle1 )
		m_Rectangle1.RemoveSelf();
}

void AGUI2Button::SetEnabled( TBOOL a_bEnabled )
{
	if ( IsEnabled() != a_bEnabled )
	{
		AGUI2MenuItem::SetEnabled( a_bEnabled );

		SetButtonColour( IsEnabled() ? m_uiEnabledColour : m_uiDisabledColour );
	}
}

TFLOAT AGUI2Button::GetFlowVisualOffset()
{
	TVALIDPTR( m_pFont );
	return m_pFont->GetLinesMargin() * 0.5f + m_fGap;
}

TFLOAT AGUI2Button::GetFlowOffset()
{
	TVALIDPTR( m_pFont );
	return m_pFont->GetLinesMargin() * 0.5f + m_fGap;
}

void AGUI2Button::SetButtonColour( TUINT a_uiColour )
{
	m_oText.SetColour( a_uiColour );

	for ( auto it = m_oText.ChildRBegin(); it != TNULL; it = m_oText.GetPrevChild( it ) )
	{
		it->SetColour( a_uiColour );
	}
}
