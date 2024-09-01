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
	void SetBorders( const TCHAR* a_szTopLeft, const TCHAR* a_szTopCenter, const TCHAR* a_szTopRight, const TCHAR* a_szMiddleLeft, const TCHAR* a_szMiddleCenter, const TCHAR* a_szMiddleRight, const TCHAR* a_szBottomLeft, const TCHAR* a_szBottomCenter, const TCHAR* a_szBottomRight );
	void SetDimensionsImpl( TFLOAT a_fWidth, TFLOAT a_fHeight );

private:
	AGUI2Element*        m_pFocusElement;
	AGUI2TextureSection* m_apTexTiles[ AGUI2ATTACHMENT_NUMOF ];
	AGUI2Rectangle       m_aRectangles[ AGUI2ATTACHMENT_NUMOF ];
	TBOOL                m_bUseScissor;
};
