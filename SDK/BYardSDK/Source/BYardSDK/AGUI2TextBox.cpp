#include "pch.h"
#include "AGUI2TextBox.h"

AGUI2TextBox::AGUI2TextBox()
{
}

void AGUI2TextBox::Create( AGUI2Font* a_pFont, TFLOAT a_fWidth )
{
	m_pFont				   = a_pFont;
	AGUI2Element::m_fWidth = a_fWidth;
	m_bDimensionsDirty	   = TTRUE;
	m_bUnkFlag2			   = TFALSE;
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
	m_bDimensionsDirty	   = TTRUE;
}

void AGUI2TextBox::SetShadowAlpha( TFLOAT a_fAlpha )
{
	TUINT8 ui8Alpha	 = TUINT8( a_fAlpha * 255.0f );
	m_uiShadowColour = ( m_uiShadowColour & 0x00FFFFFF ) | ui8Alpha << 24;
}
