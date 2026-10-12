#pragma once
#include <Toshi/TColor.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIFont.h>

class AGUI2TextBox : public Toshi::T2GUIElement
{
public:
	// constructors/destructor
	AGUI2TextBox();
	~AGUI2TextBox();

	void Create( Toshi::T2GUIFont* a_pFont, TFLOAT a_fWidth );

	void SetText( const TWCHAR* a_wszText );

	// Sets the text and makes the box as wide as the text (inlined in the original)
	void SetTextAndFitWidth( const TWCHAR* a_wszText )
	{
		m_wszText          = a_wszText;
		m_bDimensionsDirty = TTRUE;
		m_bFixedHeight     = TFALSE;
		m_fWidth           = m_pFont->GetTextWidth( a_wszText ) * m_fScale;
	}
	void SetScale( TFLOAT a_fScale );
	// Sets the width and, when a_bFixedHeight is set, a height that isn't recalculated from the text
	void SetSize( TFLOAT a_fWidth, TFLOAT a_fHeight, TBOOL a_bFixedHeight );
	void SetShadow( TBOOL a_bEnabled, TUINT32 a_uiColour );
	void SetShadowEnabled( TBOOL a_bEnabled );
	void SetShadowColour( const Toshi::TColor& a_rColour );
	void SetShadowColour( TUINT32 a_uiColour );
	void SetShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY );
	void SetTextAlign( Toshi::T2GUIFont::TextAlign a_eTextAlign );

	// Sets the shadow offset as it is (SetShadowOffset multiplies it by the scale)
	void SetShadowOffsetUnscaled( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY )
	{
		m_fShadowOffsetX = a_fOffsetX;
		m_fShadowOffsetY = a_fOffsetY;
	}

	const TWCHAR* GetText() const { return m_wszText; }
	TFLOAT        GetScale() const { return m_fScale; }

	// Plain setters (inlined in the original)
	void SetFont( Toshi::T2GUIFont* a_pFont ) { m_pFont = a_pFont; }
	void SetDimensionsDirty() { m_bDimensionsDirty = TTRUE; }
	void SetTextNoResize( const TWCHAR* a_wszText )
	{
		m_wszText          = a_wszText;
		m_bDimensionsDirty = TTRUE;
	}

	// Line height of the font, 0 without a font
	TFLOAT GetLineHeight();
	TFLOAT GetShadowOffsetX() const { return m_fShadowOffsetX; }
	TFLOAT GetShadowOffsetY() const { return m_fShadowOffsetY; }

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void   Render() OVERRIDE;
	virtual void   GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) OVERRIDE;
	virtual TFLOAT GetWidth() OVERRIDE;
	virtual TFLOAT GetHeight() OVERRIDE;
	virtual void   SetWidth( TFLOAT a_fWidth ) OVERRIDE;
	virtual void   SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) OVERRIDE {} // +0x28, DOES_NOTHING
	virtual void   SetShadowAlpha( TFLOAT a_fAlpha ) OVERRIDE;

protected:
	void RenderText( void* a_fnCallback );

protected:
	TBOOL                       m_bRenderShadow;
	TFLOAT                      m_fShadowOffsetX;
	TFLOAT                      m_fShadowOffsetY;
	TUINT32                     m_uiShadowColour;
	const TWCHAR*               m_wszText;
	Toshi::T2GUIFont*           m_pFont;
	TFLOAT                      m_fScale;
	Toshi::T2GUIFont::TextAlign m_eTextAlign;
	TBOOL                       m_bDimensionsDirty;
	TBOOL                       m_bFixedHeight;
};
