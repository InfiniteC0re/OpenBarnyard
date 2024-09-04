#pragma once
#include "AGUI2Menu.h"
#include "AGUI2Rectangle.h"
#include "AGUI2Font.h"
#include "AGUI2TextBox.h"

class AGUI2Button :
    public AGUI2MenuItem
{
public:
	// constructors/destructor
	AGUI2Button();
	~AGUI2Button();

	void Create( AGUI2Font* a_pFont, TFLOAT a_fWidth, const TWCHAR* a_wszText, TFLOAT a_fGap );
	void SetImage( const TCHAR* a_szTexSec, TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	void SetButtonId( TINT a_iID ) { m_iButtonID = a_iID; }
	TINT GetButtonId() const { return m_iButtonID; }

	void SetScaleOnFocus( TBOOL a_bScaleOnFocus ) { m_bScaleOnFocus = a_bScaleOnFocus; }
	void SetFocusedScale( TFLOAT a_fScale ) { m_fFocusedScale = a_fScale; }
	void SetFocusedColour( TUINT a_uiColour ) { m_uiFocusedColour = a_uiColour; }
	void SetEnabledColour( TUINT a_uiColour ) { m_uiEnabledColour = a_uiColour; }
	void SetDisabledColour( TUINT a_uiColour ) { m_uiDisabledColour = a_uiColour; }

	void SetTextShadow( TBOOL a_bEnabled, TUINT32 a_uiColour, TFLOAT a_fAlpha )
	{
		m_oText.SetShadow( a_bEnabled, a_uiColour );
		m_oText.SetShadowAlpha( a_fAlpha );
	}

	void SetTextShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY ) { m_oText.SetShadowOffset( a_fOffsetX, a_fOffsetY ); }

	AGUI2Rectangle& GetBackground() { return m_oBackgroundRect; }
	AGUI2TextBox& GetTextBox() { return m_oText; }

	//-----------------------------------------------------------------------------
	// AGUI2Element
	//-----------------------------------------------------------------------------
	virtual void GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) OVERRIDE;
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;
	virtual void SetShadowAlpha( TFLOAT a_fAlpha ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AGUI2MenuItem
	//-----------------------------------------------------------------------------
	virtual void   OnFocus() OVERRIDE;
	virtual void   OnFocusLost() OVERRIDE;
	virtual void   SetEnabled( TBOOL a_bEnabled ) OVERRIDE;
	virtual TFLOAT GetFlowVisualOffset() OVERRIDE;
	virtual TFLOAT GetFlowOffset() OVERRIDE;

private:
	void SetButtonColour( TUINT a_uiColour );

private:
	TINT           m_iButtonID;
	TBOOL          m_bScaleOnFocus;
	TBOOL          m_bFocused;
	TFLOAT         m_fRectangle1Width;
	TFLOAT         m_fRectangle1Height;
	TBOOL          m_bUseRectangle1;
	TINT           m_Unk3;
	AGUI2Rectangle m_Rectangle1;
	TUINT          m_uiFocusedColour;
	TUINT          m_Unk4;
	TUINT          m_uiEnabledColour;
	TUINT          m_uiDisabledColour;
	TFLOAT         m_fFocusedScale;
	TFLOAT         m_fScale;
	AGUI2Font*     m_pFont;
	AGUI2TextBox   m_oText;
	TFLOAT         m_fGap;
	const TWCHAR*  m_wszButtonText;
	AGUI2Rectangle m_oBackgroundRect;
};
