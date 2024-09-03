#pragma once
#include "AGUI2Element.h"
#include "AGUI2Rectangle.h"
#include "AGUI2TextureSection.h"

class AGUI2Dialog :
    public AGUI2Element
{
public:
	// constructors/destructor
	AGUI2Dialog();
	~AGUI2Dialog();

	//-----------------------------------------------------------------------------
	// AGUI2Element
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
	AGUI2Element*        m_pFocusElement;
	AGUI2TextureSection* m_apTexTiles[ AGUI2ATTACHMENT_NUMOF ];
	AGUI2Rectangle       m_aRectangles[ AGUI2ATTACHMENT_NUMOF ];
	TBOOL                m_bUseScissor;
};
