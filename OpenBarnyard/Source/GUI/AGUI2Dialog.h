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

	// Same as SetDialogSize, but the borders are at least a_fMinBorder big and the middle rectangle only gets the inner size
	void SetDialogSize( TFLOAT a_fWidth, TFLOAT a_fHeight, TFLOAT a_fMinBorder );

	// Unlinks the dialog and every element below it
	void Destroy();

	// Unlinks every element below a_pElement (recursively)
	static void RemoveChildrenRecursive( Toshi::T2GUIElement* a_pElement );

	// Size of the middle rectangle that holds the content of the dialog
	void GetContentDimensions( TFLOAT& a_rWidth, TFLOAT& a_rHeight );

	// Makes a_pElement a child of the rectangle a_eRectangle
	void AddChildToRectangle( Toshi::T2GUIElement* a_pElement, Toshi::T2GUIATTACHMENT a_eRectangle );

	void SetUseScissor( TBOOL a_bUseScissor ) { m_bUseScissor = a_bUseScissor; }

private:
	Toshi::T2GUIElement*        m_pFocusElement;
	Toshi::T2GUITextureSection* m_apTexTiles[ Toshi::T2GUIATTACHMENT_NUMOF ];
	Toshi::T2GUIRectangle       m_aRectangles[ Toshi::T2GUIATTACHMENT_NUMOF ];
	TBOOL                       m_bUseScissor;
};
