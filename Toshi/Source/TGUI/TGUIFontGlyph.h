#pragma once
#include "TGUITextureSection.h"

class PPropertyValueArray;

TOSHI_NAMESPACE_START

class TGUIFont;

//-----------------------------------------------------------------------------
// Purpose: a character of a TGUIFont: the texture section of its image, its
// baseline and its kerning ("glyph[c] = { name, texture, width, height, x, y,
// baseline, kernleft, kernright };" in the [font] blocks).
//-----------------------------------------------------------------------------
class TGUIFontGlyph : public TGUITextureSection
{
public:
	TGUIFontGlyph();
	TGUIFontGlyph( TUINT16 a_uiCharacter );
	TGUIFontGlyph( TINT16 a_iTexture, const TPString8& a_rcName, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight, TUINT16 a_uiCharacter, TINT16 a_iBaseline, TINT16 a_iKerningLeft, TINT16 a_iKerningRight );
	TGUIFontGlyph( const TGUIFontGlyph& a_rcOther );
	~TGUIFontGlyph();

	virtual TGUIFontGlyph* Clone() const override;

	TBOOL Deserialise( TGUIFont* a_pFont, const PPropertyValueArray& a_rcArray );
	TBOOL Serialise( TGUIFont* a_pFont, PPropertyValueArray& a_rArray );

	// $JPOG: FUNCTION TGuiInterface.dll+0000e620
	TUINT16 GetCharacter() const { return m_uiCharacter; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e5e0
	void SetCharacter( TUINT16 a_uiCharacter ) { m_uiCharacter = a_uiCharacter; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e610
	TINT GetBaseline() const { return m_iBaseline; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e5d0
	void SetBaseline( TINT16 a_iBaseline ) { m_iBaseline = a_iBaseline; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e600
	TINT GetKerningLeft() const { return m_iKerningLeft; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e5c0
	void SetKerningLeft( TINT16 a_iKerning ) { m_iKerningLeft = a_iKerning; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e5f0
	TINT GetKerningRight() const { return m_iKerningRight; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e5b0
	void SetKerningRight( TINT16 a_iKerning ) { m_iKerningRight = a_iKerning; }

private:
	TUINT16 m_uiCharacter;   // 0x28
	TINT16  m_iBaseline;     // 0x2A
	TINT16  m_iKerningLeft;  // 0x2C
	TINT16  m_iKerningRight; // 0x2E
};

TOSHI_NAMESPACE_END
