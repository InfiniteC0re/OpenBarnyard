#pragma once
#ifdef BARNYARD_LATEST

#  include "GUI/AGUI2TextBox.h"

//-----------------------------------------------------------------------------
// The Wii AGUI2TextBox: controller button icons in the text, a limit of characters to draw
// and new lines that end a line instead of adding an empty one.
// NOTE: the Wii build has these in AGUI2TextBox itself (0x84 bytes, the fields at 0x6C-0x80;
// it keeps m_bRenderShadow and m_bIncludeNewLine as bits 0 and 1 of the flags byte at 0x80).
// They're in a subclass so that the PC classes that embed text boxes keep their retail sizes.
// Only AGUI2XUIText uses them.
//-----------------------------------------------------------------------------
class AGUI2XUITextBox
    : public AGUI2TextBox
{
public:
	AGUI2XUITextBox();

	Toshi::T2GUIFont::IconRenderCallback SetIconRenderCallback( Toshi::T2GUIFont::IconRenderCallback a_fnIconRender );
	Toshi::T2GUIFont::IconWidthCallback  SetIconWidthCallback( Toshi::T2GUIFont::IconWidthCallback a_fnIconWidth );

	// -1 draws the whole text
	void SetMaxChars( TINT a_iMaxChars )
	{
		m_iMaxChars        = TINT16( a_iMaxChars );
		m_bDimensionsDirty = TTRUE;
	}

	void SetIncludeNewLine( TBOOL a_bIncludeNewLine ) { m_bIncludeNewLine = a_bIncludeNewLine; }

	//-----------------------------------------------------------------------------
	// Toshi::T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() OVERRIDE;
	virtual void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) OVERRIDE;

private:
	void RenderText( void* a_fnCallback );

private:
	TINT16                               m_iMaxChars;
	Toshi::T2GUIFont::IconRenderCallback m_fnIconRender;
	Toshi::T2GUIFont::IconWidthCallback  m_fnIconWidth;
	TBOOL                                m_bIncludeNewLine;
};

#endif // BARNYARD_LATEST
