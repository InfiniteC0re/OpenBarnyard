#pragma once
#include "AGUI2Menu.h"
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUIFont.h>

class AGUI2Button : public AGUI2MenuItem
{
public:
	// constructors/destructor
	AGUI2Button();
	~AGUI2Button();

	void Create( Toshi::T2GUIFont* a_pFont, TFLOAT a_fWidth, const TWCHAR* a_wszText, TFLOAT a_fGap );
	void SetImage( const TCHAR* a_szTexSec, TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	// Resizes the image to the size of the text multiplied by the scale
	void ResizeImage( TFLOAT a_fScaleX, TFLOAT a_fScaleY );

	void SetId( TINT a_iID ) { m_iButtonID = a_iID; }
	TINT GetId() const { return m_iButtonID; }

	void SetScaleOnFocus( TBOOL a_bScaleOnFocus ) { m_bScaleOnFocus = a_bScaleOnFocus; }

	// Shows the highlight rectangle behind the focused button (00640230, one-liner)
	void SetUseHighlight( TBOOL a_bUseHighlight ) { m_bUseRectangle1 = a_bUseHighlight; }

	// Sets the texture section of the highlight rectangle (00640240)
	void SetHighlightImage( const TCHAR* a_szTexSec );

	// $Barnyard: FUNCTION 00640180
	void SetScale( TFLOAT a_fScale ) { m_fScale = a_fScale; }

	// $Barnyard: FUNCTION 00640190
	void SetFocusedScale( TFLOAT a_fScale ) { m_fFocusedScale = a_fScale; }

	// $Barnyard: FUNCTION 006401a0
	void SetFocusedColour( TUINT a_uiColour ) { m_uiFocusedColour = a_uiColour; }

	// $Barnyard: FUNCTION 006401b0
	void SetUnkColour( TUINT a_uiColour ) { m_Unk4 = a_uiColour; }

	// $Barnyard: FUNCTION 006401c0
	void SetEnabledColour( TUINT a_uiColour ) { m_uiEnabledColour = a_uiColour; }

	// $Barnyard: FUNCTION 006401d0
	void SetDisabledColour( TUINT a_uiColour ) { m_uiDisabledColour = a_uiColour; }

	// $Barnyard: FUNCTION 006402b0
	TBOOL IsFocused() const { return m_bFocused; }

	void SetTextShadow( TBOOL a_bEnabled, TUINT32 a_uiColour, TFLOAT a_fAlpha )
	{
		m_oText.SetShadow( a_bEnabled, a_uiColour );
		m_oText.SetShadowAlpha( a_fAlpha );
	}

	void SetTextShadowOffset( TFLOAT a_fOffset ) { m_oText.SetShadowOffset( a_fOffset, a_fOffset ); }
	void SetTextShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY ) { m_oText.SetShadowOffset( a_fOffsetX, a_fOffsetY ); }

	Toshi::T2GUIRectangle& GetBackground() { return m_oBackgroundRect; }

	// $Barnyard: FUNCTION 00640220
	AGUI2TextBox& GetTextBox() { return m_oText; }

	// $Barnyard: FUNCTION 006401e0
	const TWCHAR* GetText() const { return m_wszButtonText; }

	//-----------------------------------------------------------------------------
	// T2GUIElement
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
	TINT                        m_iButtonID;
	TBOOL                       m_bScaleOnFocus;
	TBOOL                       m_bFocused;
	TFLOAT                      m_fRectangle1Width;
	TFLOAT                      m_fRectangle1Height;
	TBOOL                       m_bUseRectangle1;
	Toshi::T2GUITextureSection* m_pRectangle1Section;
	Toshi::T2GUIRectangle       m_Rectangle1;
	TUINT                       m_uiFocusedColour;
	TUINT                       m_Unk4;
	TUINT                       m_uiEnabledColour;
	TUINT                       m_uiDisabledColour;
	TFLOAT                      m_fFocusedScale;
	TFLOAT                      m_fScale;
	Toshi::T2GUIFont*           m_pFont;
	AGUI2TextBox                m_oText;
	TFLOAT                      m_fGap;
	const TWCHAR*               m_wszButtonText;
	Toshi::T2GUIRectangle       m_oBackgroundRect;
};
