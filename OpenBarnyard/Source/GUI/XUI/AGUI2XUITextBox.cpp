#include "pch.h"
#ifdef BARNYARD_LATEST
#  include "AGUI2XUITextBox.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// $BarnyardWiiJP: FUNCTION 804a23d0
AGUI2XUITextBox::AGUI2XUITextBox()
{
	m_iMaxChars       = -1;
	m_fnIconRender    = TNULL;
	m_fnIconWidth     = TNULL;
	m_bIncludeNewLine = TFALSE;
}

// $BarnyardWiiJP: FUNCTION 804a2638
void AGUI2XUITextBox::RenderText( void* a_fnCallback )
{
	if ( m_pFont && m_wszText )
	{
		TFLOAT fWidth;
		TFLOAT fHeight;
		GetDimensions( fWidth, fHeight );

		TFLOAT fPosX = -fWidth * 0.5f;
		TFLOAT fPosY = -fHeight * 0.5f;

		// The shadow has no icons
		if ( m_bRenderShadow )
			m_pFont->DrawTextWrapped( m_wszText, m_uiShadowColour, m_eTextAlign, TREINTERPRETCAST( T2GUIFont::CharCallback, a_fnCallback ), TNULL, TNULL, m_bIncludeNewLine, fPosX + m_fShadowOffsetX, fPosY + m_fShadowOffsetY, fWidth, fHeight, m_fScale, m_iMaxChars );

		m_pFont->DrawTextWrapped( m_wszText, T2GUIElement::m_uiColour, m_eTextAlign, TREINTERPRETCAST( T2GUIFont::CharCallback, a_fnCallback ), m_fnIconWidth, m_fnIconRender, m_bIncludeNewLine, fPosX, fPosY, fWidth, fHeight, m_fScale, m_iMaxChars );
	}
}

// $BarnyardWiiJP: FUNCTION 804a2750
void AGUI2XUITextBox::Render()
{
	if ( T2GUIElement::HasFlags( T2GUIElement::FLAGS_RENDER_BEFORE_CHILDREN ) )
		T2GUIElement::Render();

	RenderText( TNULL );

	if ( !T2GUIElement::HasFlags( T2GUIElement::FLAGS_RENDER_BEFORE_CHILDREN ) )
		T2GUIElement::Render();
}

// $BarnyardWiiJP: FUNCTION 804a24f8
void AGUI2XUITextBox::GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight )
{
	if ( !m_bFixedHeight && m_bDimensionsDirty )
	{
		if ( !m_wszText || !m_pFont )
			T2GUIElement::m_fHeight = 0.0f;
		else
			T2GUIElement::m_fHeight = m_pFont->GetTextHeightWrapped( m_wszText, m_fnIconWidth, m_bIncludeNewLine, m_iMaxChars, T2GUIElement::m_fWidth, m_fScale );

		m_bDimensionsDirty = TFALSE;
	}

	a_rWidth  = T2GUIElement::m_fWidth;
	a_rHeight = T2GUIElement::m_fHeight;
}

// $BarnyardWiiJP: FUNCTION 804a27ec
T2GUIFont::IconRenderCallback AGUI2XUITextBox::SetIconRenderCallback( T2GUIFont::IconRenderCallback a_fnIconRender )
{
	T2GUIFont::IconRenderCallback fnPrevious = m_fnIconRender;
	m_fnIconRender                           = a_fnIconRender;
	return fnPrevious;
}

// $BarnyardWiiJP: FUNCTION 804a27fc
T2GUIFont::IconWidthCallback AGUI2XUITextBox::SetIconWidthCallback( T2GUIFont::IconWidthCallback a_fnIconWidth )
{
	T2GUIFont::IconWidthCallback fnPrevious = m_fnIconWidth;
	m_fnIconWidth                           = a_fnIconWidth;
	return fnPrevious;
}

#endif // BARNYARD_LATEST
