#pragma once
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <GUI/T2GUITextureSection.h>

class AGUI2Dialog : public Toshi::T2GUIElement
{
public:
	// constructors/destructor
	AGUI2Dialog();
	~AGUI2Dialog();

	//-----------------------------------------------------------------------------
	// T2GUIElement
	//-----------------------------------------------------------------------------
	virtual void Render() OVERRIDE;
	virtual void SetDimensions( TFLOAT a_fWidth, TFLOAT a_fHeight ) OVERRIDE;
	virtual void SetColour( TUINT32 a_uiColour ) OVERRIDE;
	virtual void SetAlpha( TFLOAT a_fAlpha ) OVERRIDE;
	virtual void SetFocus( TBOOL a_bFocused ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void Create( const TCHAR* a_szTopLeft = "Border_Top_Left", const TCHAR* a_szTopCenter = "Border_Horizontal", const TCHAR* a_szTopRight = "Border_Top_Right", const TCHAR* a_szMiddleLeft = "Border_Vertical", const TCHAR* a_szMiddleCenter = "Border_Middle", const TCHAR* a_szMiddleRight = "Border_Vertical", const TCHAR* a_szBottomLeft = "Border_Bottom_Left", const TCHAR* a_szBottomCenter = "Border_Horizontal", const TCHAR* a_szBottomRight = "Border_Bottom_Right" );
	void SetDialogSize( TFLOAT a_fWidth, TFLOAT a_fHeight );

private:
	Toshi::T2GUIElement*        m_pFocusElement;
	Toshi::T2GUITextureSection* m_apTexTiles[ Toshi::T2GUIATTACHMENT_NUMOF ];
	Toshi::T2GUIRectangle       m_aRectangles[ Toshi::T2GUIATTACHMENT_NUMOF ];
	TBOOL                       m_bUseScissor;
};
