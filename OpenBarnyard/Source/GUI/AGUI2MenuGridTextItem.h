#pragma once
#include "AGUI2MenuGrid.h"
#include "AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

class AGUI2MenuGridTextItem : public AGUI2MenuGridItem
{
public:
	AGUI2MenuGridTextItem();
	~AGUI2MenuGridTextItem();

	void Create( Toshi::T2GUIFont* a_pFont, TFLOAT a_fWidth, const TWCHAR* a_wszText, TFLOAT a_fGap );

	void SetId( TINT a_iID ) { m_iID = a_iID; }
	TINT GetId() const { return m_iID; }

	void SetScaleOnFocus( TBOOL a_bScaleOnFocus ) { m_bScaleOnFocus = a_bScaleOnFocus; }
	void SetFocusedScale( TFLOAT a_fScale ) { m_fFocusedScale = a_fScale; }
	void SetFocusedColour( TUINT a_uiColour ) { m_uiFocusedColour = a_uiColour; }
	void SetEnabledColour( TUINT a_uiColour ) { m_uiEnabledColour = a_uiColour; }
	void SetDisabledColour( TUINT a_uiColour ) { m_uiDisabledColour = a_uiColour; }

	void SetTextShadow( TBOOL a_bEnabled, TUINT32 a_uiColour, TFLOAT a_fAlpha )
	{
		m_oTextBox.SetShadow( a_bEnabled, a_uiColour );
		m_oTextBox.SetShadowAlpha( a_fAlpha );
	}

	void SetTextShadowOffset( TFLOAT a_fOffset ) { m_oTextBox.SetShadowOffset( a_fOffset, a_fOffset ); }
	void SetTextShadowOffset( TFLOAT a_fOffsetX, TFLOAT a_fOffsetY ) { m_oTextBox.SetShadowOffset( a_fOffsetX, a_fOffsetY ); }

	AGUI2TextBox& GetTextBox() { return m_oTextBox; }

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) OVERRIDE;
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;
	virtual void SetShadowAlpha( TFLOAT a_fAlpha ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AGUI2MenuGridItem
	//-----------------------------------------------------------------------------
	virtual void   OnFocus() OVERRIDE;
	virtual void   OnFocusLost() OVERRIDE;
	virtual void   SetEnabled( TBOOL a_bEnabled );
	virtual TFLOAT GetFlowVisualOffset() OVERRIDE;
	virtual TFLOAT GetFlowOffset() OVERRIDE;

private:
	TINT                  m_iID;
	TBOOL                 m_bScaleOnFocus;
	TBOOL                 m_bFocused;
	TFLOAT                m_fRectangle1Width;
	TFLOAT                m_fRectangle1Height;
	TBOOL                 m_bUseRectangle1;
	TINT                  m_Unk1;
	Toshi::T2GUIRectangle m_oRectangle1;
	TUINT                 m_uiFocusedColour;
	TUINT                 m_uiEnabledColour;
	TUINT                 m_uiDisabledColour;
	TFLOAT                m_fFocusedScale;
	TFLOAT                m_fScale;
	Toshi::T2GUIFont*     m_pFont;
	AGUI2TextBox          m_oTextBox;
	TFLOAT                m_fGap;
	TWCHAR*               m_wszText;
	TFLOAT                m_fWidth;
	Toshi::T2GUIRectangle m_oRectangle2;
};
