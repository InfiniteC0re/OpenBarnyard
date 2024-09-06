#pragma once
#include "AGUI2Element.h"
#include "AGUI2Font.h"

#include <Toshi/TColor.h>

class AGUI2TextBox : public AGUI2Element
{
public:
	// constructors/destructor
	AGUI2TextBox();
	~AGUI2TextBox();

	void Create( AGUI2Font* a_pFont, TFLOAT a_fWidth );

	void SetText( const TWCHAR* a_wszText );
	void SetScale( TFLOAT a_fScale );
	void SetShadow( TBOOL a_bEnabled, TUINT32 a_uiColour );
	void SetShadowEnabled( TBOOL a_bEnabled );
	void SetShadowColour( const Toshi::TColor& a_rColour );
	void SetShadowColour( TUINT32 a_uiColour );
	void SetShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY );
	void SetTextAlign( AGUI2Font::TextAlign a_eTextAlign );

	const TWCHAR* GetText() const { return m_wszText; }
	TFLOAT        GetShadowOffsetX() const { return m_fShadowOffsetX; }
	TFLOAT        GetShadowOffsetY() const { return m_fShadowOffsetY; }

	//-----------------------------------------------------------------------------
	// AGUI2Element
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
	TBOOL                m_bRenderShadow;
	TFLOAT               m_fShadowOffsetX;
	TFLOAT               m_fShadowOffsetY;
	TUINT32              m_uiShadowColour;
	const TWCHAR*        m_wszText;
	AGUI2Font*           m_pFont;
	TFLOAT               m_fScale;
	AGUI2Font::TextAlign m_eTextAlign;
	TBOOL                m_bDimensionsDirty;
	TBOOL                m_bUnkFlag2;
};
