#pragma once
#include "SDK_T2GUIElement.h"
#include "SDK_T2GUIFont.h"

#include <Toshi/TColor.h>

class AGUI2TextBox : public SDK_T2GUIElement
{
public:
	AGUI2TextBox();

	virtual void   Render() override;
	virtual void   GetDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight ) override;
	virtual TFLOAT GetWidth() override;
	virtual TFLOAT GetHeight() override;
	virtual void   SetWidth( TFLOAT a_fWidth ) override;
	virtual void   SetShadowAlpha( TFLOAT a_fAlpha ) override;

	void Create( SDK_T2GUIFont* a_pFont, TFLOAT a_fWidth );

	void SetText( const wchar_t* a_wszText )
	{
		m_wszText          = a_wszText;
		m_bDimensionsDirty = TTRUE;
	}

	void SetScale( TFLOAT a_fScale )
	{
		m_fScale           = a_fScale;
		m_bDimensionsDirty = TTRUE;
	}

	void SetShadow( TBOOL a_bEnabled, TUINT32 a_uiColour )
	{
		m_bRenderShadow  = a_bEnabled;
		m_uiShadowColour = a_uiColour;
	}

	void SetShadowEnabled( TBOOL a_bEnabled )
	{
		m_bRenderShadow = a_bEnabled;
	}

	void SetShadowColour( const Toshi::TColor& a_rColour )
	{
		m_uiShadowColour = a_rColour.Value32;
	}

	void SetShadowColour( TUINT32 a_uiColour )
	{
		m_uiShadowColour = a_uiColour;
	}

	void SetTextAlign( SDK_T2GUIFont::TextAlign a_eTextAlign )
	{
		m_eTextAlign = a_eTextAlign;
	}

	SDK_T2GUIFont* GetFont()
	{
		return m_pFont;
	}

	static AGUI2TextBox* CreateFromEngine()
	{
		return CALL_THIS( 0x006c3620, AGUI2TextBox*, AGUI2TextBox*, new AGUI2TextBox );
	}

private:
	void RenderText( void* a_fnCallback );

private:
	TBOOL                m_bRenderShadow;
	TFLOAT               m_fShadowOffsetX;
	TFLOAT               m_fShadowOffsetY;
	TUINT32              m_uiShadowColour;
	const wchar_t*       m_wszText;
	SDK_T2GUIFont*           m_pFont;
	TFLOAT               m_fScale;
	SDK_T2GUIFont::TextAlign m_eTextAlign;
	TBOOL                m_bDimensionsDirty;
	TBOOL                m_bUnkFlag2;
};
