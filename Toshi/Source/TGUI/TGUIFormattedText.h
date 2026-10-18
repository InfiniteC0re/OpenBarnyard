#pragma once
#include "TGUIColour.h"
#include "TGUIAlignment.h"

#include <Toshi/TNodeList.h>
#include <Toshi/TString16.h>

TOSHI_NAMESPACE_START

class TGUIFont;

//-----------------------------------------------------------------------------
// Purpose: a text broken into lines by TGUIFont::BreakString: one segment per
// line with its size, colour, font, text and horizontal alignment, painted by
// TGUIDisplayContext::DrawFormattedText.
//
// NOTE: the text of the segments is a pooled wide string in the original
// (TPWString), a TString16 here.
//-----------------------------------------------------------------------------
class TGUIFormattedText
    : public TNodeList<class TGUIFormattedTextSegment>
{
public:
	using Segment = TGUIFormattedTextSegment;

public:
	TGUIFormattedText( TBOOL a_bTemporaryString = TFALSE );
	~TGUIFormattedText();

	// $JPOG: FUNCTION TGuiInterface.dll+0000a890
	TNodeList<Segment>& GetSegments() { return *this; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000a870
	void SetTemporaryString( TBOOL a_bTemporaryString ) { m_bTemporaryString = a_bTemporaryString; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000a880
	TBOOL IsTemporaryString() const { return m_bTemporaryString; }

private:
	TBOOL m_bTemporaryString; // 0x10 the text comes from a temporary localised string
};

// A line of a formatted text
class TGUIFormattedTextSegment
    : public TNodeList<TGUIFormattedTextSegment>::TNode
{
public:
	TGUIFormattedTextSegment( TUINT16 a_uiWidth, TUINT16 a_uiHeight, const TGUIColour& a_rcColour, const TGUIFont* a_pFont, const TString16& a_rcText, TGUIAlignment a_eAlignment );
	TGUIFormattedTextSegment( const TGUIFormattedTextSegment& a_rcOther );
	~TGUIFormattedTextSegment();

	TGUIFormattedTextSegment& operator=( const TGUIFormattedTextSegment& a_rcOther );

public:
	TUINT16         uiWidth;    // 0x0C
	TUINT16         uiHeight;   // 0x0E
	TGUIColour      oColour;    // 0x10
	const TGUIFont* pFont;      // 0x14
	TString16       strText;    // 0x18
	TGUIAlignment   eAlignment; // 0x1C
};

TOSHI_NAMESPACE_END
