#include "pch.h"
#include "AGUI2TextBox.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

AGUI2TextBox::AGUI2TextBox()
{
	m_bRenderShadow  = TFALSE;
	m_wszText        = TNULL;
	m_pFont          = TNULL;
	m_bUnkFlag2      = TFALSE;
	m_fShadowOffsetX = 5.0f;
	m_fShadowOffsetY = 5.0f;
	m_uiShadowColour = 0x80000000;
	m_fScale         = 1.0f;
	m_eTextAlign     = AGUI2Font::TextAlign_Center;
}

 AGUI2TextBox::~AGUI2TextBox()
{
}

void AGUI2TextBox::Create( AGUI2Font* a_pFont, TFLOAT a_fWidth )
{
	m_pFont                = a_pFont;
	AGUI2Element::m_fWidth = a_fWidth;
	m_bDimensionsDirty     = TTRUE;
	m_bUnkFlag2            = TFALSE;
}

void AGUI2TextBox::SetText( const TWCHAR* a_wszText )
{
	m_wszText          = a_wszText;
	m_bDimensionsDirty = TTRUE;
}

void AGUI2TextBox::RenderText( void* a_fnCallback )
{
	if ( m_pFont && m_wszText )
	{
		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions( fWidth, fHeight );

		TFLOAT fPosX = -fWidth * 0.5f;
		TFLOAT fPosY = -fHeight * 0.5f;

		if ( m_bRenderShadow )
		{
			m_pFont->DrawTextWrapped(
			    m_wszText,
			    fPosX + m_fShadowOffsetX,
			    fPosY + m_fShadowOffsetY,
			    fWidth,
			    fHeight,
			    m_uiShadowColour,
			    m_fScale,
			    m_eTextAlign,
			    a_fnCallback );
		}

		m_pFont->DrawTextWrapped(
		    m_wszText,
		    fPosX,
		    fPosY,
		    fWidth,
		    fHeight,
		    AGUI2Element::m_uiColour,
		    m_fScale,
		    m_eTextAlign,
		    a_fnCallback );
	}
}

void AGUI2TextBox::SetScale( TFLOAT a_fScale )
{
	m_fScale           = a_fScale;
	m_bDimensionsDirty = TTRUE;
}

void AGUI2TextBox::SetShadow( TBOOL a_bEnabled, TUINT32 a_uiColour )
{
	m_bRenderShadow  = a_bEnabled;
	m_uiShadowColour = a_uiColour;
}

void AGUI2TextBox::SetShadowEnabled( TBOOL a_bEnabled )
{
	m_bRenderShadow = a_bEnabled;
}

void AGUI2TextBox::SetShadowColour( TUINT32 a_uiColour )
{
	m_uiShadowColour = a_uiColour;
}

void AGUI2TextBox::SetShadowColour( const Toshi::TColor& a_rColour )
{
	m_uiShadowColour = a_rColour.Value32;
}

void AGUI2TextBox::SetShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY )
{
	m_fShadowOffsetX = m_fScale * a_fOffsetX;
	m_fShadowOffsetY = m_fScale * a_fOffsetY;
}

void AGUI2TextBox::SetTextAlign( AGUI2Font::TextAlign a_eTextAlign )
{
	m_eTextAlign = a_eTextAlign;
}

const TWCHAR* AGUI2TextBox::GetText() const
{
	return m_wszText;
}

void AGUI2TextBox::Render()
{
	TASSERT( m_pFont != TNULL );

	if ( AGUI2Element::HasFlags( 8 ) )
	{
		// Render children before text
		AGUI2Element::Render();
	}

	RenderText( TNULL );

	if ( !AGUI2Element::HasFlags( 8 ) )
	{
		// Render children after text
		AGUI2Element::Render();
	}
}

void AGUI2TextBox::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	if ( !m_bUnkFlag2 && m_bDimensionsDirty )
	{
		if ( !m_wszText || !m_pFont )
		{
			AGUI2Element::m_fHeight = 0.0f;
		}
		else
		{
			AGUI2Element::m_fHeight = m_pFont->GetTextHeightWrapped( m_wszText, AGUI2Element::m_fWidth, m_fScale );
		}

		m_bDimensionsDirty = TFALSE;
	}

	a_rWidth  = AGUI2Element::m_fWidth;
	a_rHeight = AGUI2Element::m_fHeight;
}

TFLOAT AGUI2TextBox::GetWidth()
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	GetDimensions( fWidth, fHeight );

	return fWidth;
}

TFLOAT AGUI2TextBox::GetHeight()
{
	TFLOAT fWidth;
	TFLOAT fHeight;
	GetDimensions( fWidth, fHeight );

	return fHeight;
}

void AGUI2TextBox::SetWidth( TFLOAT a_fWidth )
{
	AGUI2Element::m_fWidth = a_fWidth;
	m_bDimensionsDirty     = TTRUE;
}

void AGUI2TextBox::SetShadowAlpha( TFLOAT a_fAlpha )
{
	TUINT8 ui8Alpha  = TUINT8( a_fAlpha * 255.0f );
	m_uiShadowColour = ( m_uiShadowColour & 0x00FFFFFF ) | ui8Alpha << 24;
}
