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
	void SetScale( TFLOAT a_fScale );
	void SetShadow( TBOOL a_bEnabled, TUINT32 a_uiColour );
	void SetShadowEnabled( TBOOL a_bEnabled );
	void SetShadowColour( const Toshi::TColor& a_rColour );
	void SetShadowColour( TUINT32 a_uiColour );
	void SetShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY );
	void SetTextAlign( Toshi::T2GUIFont::TextAlign a_eTextAlign );

	const TWCHAR* GetText() const { return m_wszText; }
	TFLOAT        GetScale() const { return m_fScale; }
	TFLOAT        GetShadowOffsetX() const { return m_fShadowOffsetX; }
	TFLOAT        GetShadowOffsetY() const { return m_fShadowOffsetY; }

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void   Render() OVERRIDE;
	virtual void   GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) OVERRIDE;
	virtual TFLOAT GetWidth() OVERRIDE;
	virtual TFLOAT GetHeight() OVERRIDE;
	virtual void   SetWidth( TFLOAT a_fWidth ) OVERRIDE;
	virtual void   SetShadowAlpha( TFLOAT a_fAlpha ) OVERRIDE;

private:
	void RenderText( void* a_fnCallback );

private:
	TBOOL                       m_bRenderShadow;
	TFLOAT                      m_fShadowOffsetX;
	TFLOAT                      m_fShadowOffsetY;
	TUINT32                     m_uiShadowColour;
	const TWCHAR*               m_wszText;
	Toshi::T2GUIFont*           m_pFont;
	TFLOAT                      m_fScale;
	Toshi::T2GUIFont::TextAlign m_eTextAlign;
	TBOOL                       m_bDimensionsDirty;
	TBOOL                       m_bUnkFlag2;
};
