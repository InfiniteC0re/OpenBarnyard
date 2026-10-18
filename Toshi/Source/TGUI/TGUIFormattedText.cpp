#include "ToshiPCH.h"
#include "TGUIFormattedText.h"
#include "TGUIFont.h"

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// $JPOG: FUNCTION TGuiInterface.dll+0000a8a0
// $JPOG: FUNCTION TGuiInterface.dll+0000a8c0
TGUIFormattedText::TGUIFormattedText( TBOOL a_bTemporaryString )
    : m_bTemporaryString( a_bTemporaryString )
{
}

// $NicktoonsPS2: FUNCTION 00381400
// $JPOG: FUNCTION TGuiInterface.dll+0000a820
TGUIFormattedText::~TGUIFormattedText()
{
	DeleteAll();
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a7b0
TGUIFormattedTextSegment::TGUIFormattedTextSegment( TUINT16 a_uiWidth, TUINT16 a_uiHeight, const TGUIColour& a_rcColour, const TGUIFont* a_pFont, const TString16& a_rcText, TGUIAlignment a_eAlignment )
    : uiWidth( a_uiWidth )
    , uiHeight( a_uiHeight )
    , oColour( a_rcColour )
    , pFont( a_pFont )
    , strText( a_rcText )
    , eAlignment( a_eAlignment )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a740
TGUIFormattedTextSegment::TGUIFormattedTextSegment( const TGUIFormattedTextSegment& a_rcOther )
    : TNodeList<TGUIFormattedTextSegment>::TNode( a_rcOther )
    , uiWidth( a_rcOther.uiWidth )
    , uiHeight( a_rcOther.uiHeight )
    , oColour( a_rcOther.oColour )
    , pFont( a_rcOther.pFont )
    , strText( a_rcOther.strText )
    , eAlignment( a_rcOther.eAlignment )
{
}

// $JPOG: FUNCTION TGuiInterface.dll+0000a710
TGUIFormattedTextSegment::~TGUIFormattedTextSegment()
{
}

// NOTE: the links of the node are copied too (as in the original)
// $JPOG: FUNCTION TGuiInterface.dll+0000a680
TGUIFormattedTextSegment& TGUIFormattedTextSegment::operator=( const TGUIFormattedTextSegment& a_rcOther )
{
	m_pList    = a_rcOther.m_pList;
	m_pNext    = a_rcOther.m_pNext;
	m_pPrev    = a_rcOther.m_pPrev;
	uiWidth    = a_rcOther.uiWidth;
	uiHeight   = a_rcOther.uiHeight;
	oColour    = a_rcOther.oColour;
	pFont      = a_rcOther.pFont;
	strText    = a_rcOther.strText;
	eAlignment = a_rcOther.eAlignment;

	return *this;
}

TOSHI_NAMESPACE_END
