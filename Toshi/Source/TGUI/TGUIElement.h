#pragma once
#include "TGUIColour.h"
#include "TGUIRectangle.h"
#include "TGUIAlignment.h"

#include <Toshi/TPString8.h>

class PProperties;
class PPropertyValue;

TOSHI_NAMESPACE_START

class TGUITextureSection;
class TGUIFont;
class TGUIDisplayContext;
class TGUIFormattedText;

//-----------------------------------------------------------------------------
// Purpose: a part of a component (TGUIComponent): a texture section, a label
// (a text in a font) or a solid area, with a colour, an alignment and how it
// fills the rectangle the component gives it. The texture section or the text
// can come from a parameter of the style instead ("param = NAME;", the index
// of the parameter + 1, 0 for none).
//
// NOTE: follows Nicktoons, the label is a wide string pointer (a TLString in
// JPOG).
//-----------------------------------------------------------------------------
class TGUIElement
{
public:
	enum ElementType : TINT
	{
		ElementType_Undefined = 0,
		ElementType_Texture   = 1,
		ElementType_Label     = 2,
		ElementType_Solid     = 3,
		ElementType_NUMOF     = 4
	};

	enum Fill : TINT
	{
		Fill_None    = 0,
		Fill_Repeat  = 1,
		Fill_Stretch = 2,
		Fill_NUMOF   = 3
	};

public:
	TGUIElement();
	TGUIElement( const TGUIElement& a_rcOther );
	~TGUIElement();

	TGUIElement& operator=( const TGUIElement& a_rcOther );

	// Reads the element block ("colour", "alignx", "aligny", "fillx", "filly" and a "texture", "label" or "solid" block)
	TBOOL Deserialise( const PProperties* a_pProperties );
	TBOOL DeserialiseTexture( const PProperties* a_pProperties );
	TBOOL DeserialiseLabel( const PProperties* a_pProperties );
	TBOOL DeserialiseSolid( const PProperties* a_pProperties );

	TBOOL Serialise( PProperties* a_pProperties );

	// Size of the texture section or of the text (a_pParameters: the parameters of the widget)
	void GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight ) const;

	// Breaks the text into lines of a_iMaxWidth
	void FormatText( PPropertyValue* a_pParameters, TGUIFormattedText* a_pText, TINT& a_rWidth, TINT& a_rHeight, TINT a_iMaxWidth, TINT a_iMaxHeight ) const;

	void Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle* a_pRect ) const;

	// Paints with the rectangle grown by a_pOffset and the colour modulated by a_rcColour (the functions of the component)
	void Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle* a_pRect, const TGUIRectangle* a_pOffset, const TGUIColour& a_rcColour ) const;

	TGUIElement& Reset();
	TBOOL        IsDefined() const;

	// Changes the type (the texture section, font or label is cleared)
	void ChangeType( ElementType a_eType );

	// Type of the values of the parameter of the element
	const TClass* GetParameterType() const;

	// a_iIndex: index of the parameter in the style
	TGUIElement& SetParameter( TINT a_iIndex );
	TGUIElement& SetLabel( const TWCHAR* a_wszLabel );

	// $JPOG: FUNCTION TGuiInterface.dll+000285a0
	TGUIElement& SetTextureSection( const TGUITextureSection* a_pTextureSection )
	{
		ChangeType( ElementType_Texture );
		m_pTextureSection = a_pTextureSection;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028590
	TGUIElement& SetTextureSection()
	{
		ChangeType( ElementType_Texture );
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028570
	TGUIElement& SetFont( const TGUIFont* a_pFont )
	{
		ChangeType( ElementType_Label );
		m_pFont = a_pFont;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028550
	TGUIElement& SetColour( const TGUIColour& a_rcColour )
	{
		m_oColour = a_rcColour;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028530
	TGUIElement& SetAlignX( TGUIAlignment a_eAlign )
	{
		m_eAlignX = a_eAlign;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028520
	TGUIElement& SetAlignY( TGUIAlignment a_eAlign )
	{
		m_eAlignY = a_eAlign;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028510
	TGUIElement& SetFillX( Fill a_eFill )
	{
		m_eFillX = a_eFill;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00028500
	TGUIElement& SetFillY( Fill a_eFill )
	{
		m_eFillY = a_eFill;
		return *this;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+0000ea00
	const TGUITextureSection* GetTextureSection() const { return m_pTextureSection; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e9f0
	const TWCHAR* GetLabel() const { return m_wszLabel; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e9e0
	const TGUIFont* GetFont() const { return m_pFont; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e9d0
	const TGUIColour& GetColour() const { return m_oColour; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e9c0
	TGUIAlignment GetAlignX() const { return m_eAlignX; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e9b0
	TGUIAlignment GetAlignY() const { return m_eAlignY; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e9a0
	Fill GetFillX() const { return m_eFillX; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e990
	Fill GetFillY() const { return m_eFillY; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e980
	ElementType GetElementType() const { return m_eType; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e970
	TINT GetParameterID() const { return m_iParameter; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e960
	static TINT GetElementTypeCount() { return ElementType_NUMOF; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000e950
	static const TPString8& GetElementTypeName( ElementType a_eType ) { return ms_saElementTypeName[ a_eType ]; }

	// Creates and destroys the names of the properties and values
	// NOTE: Nicktoons creates them with its static initialiser (0037cc68), they're created by TGUIComponent::InitialiseClass here
	static void InitialiseStatic();
	static void DeinitialiseStatic();

protected:
	void Assign( const TGUIElement& a_rcOther );
	void PaintTextureSection( TGUIDisplayContext* a_pDisplayContext, const TGUITextureSection* a_pTextureSection, const TGUIRectangle* a_pRect, const TGUIColour& a_rcColour ) const;

	// The texture section or the text of the parameter if the widget sets it, of the element otherwise
	const TGUITextureSection* GetTextureSection( PPropertyValue* a_pParameters ) const;
	const TWCHAR*             GetLabel( PPropertyValue* a_pParameters ) const;

public:
	static TPString8 str_type;
	static TPString8 str_texture;
	static TPString8 str_label;
	static TPString8 str_solid;
	static TPString8 str_name;
	static TPString8 str_font;
	static TPString8 str_colour;
	static TPString8 str_alignx;
	static TPString8 str_aligny;
	static TPString8 str_fillx;
	static TPString8 str_filly;

	static TPString8 ms_saAlignXName[ TGUIAlignment_NUMOF ];
	static TPString8 ms_saAlignYName[ TGUIAlignment_NUMOF ];
	static TPString8 ms_saFillName[ Fill_NUMOF ];
	static TPString8 ms_saElementTypeName[ ElementType_NUMOF ];

	static const TCHAR* const ms_csaAlignXNameStr[ TGUIAlignment_NUMOF ];
	static const TCHAR* const ms_csaAlignYNameStr[ TGUIAlignment_NUMOF ];
	static const TCHAR* const ms_csaFillNameStr[ Fill_NUMOF ];
	static const TCHAR* const ms_csaElementTypeNameStr[ ElementType_NUMOF ];

private:
	ElementType   m_eType;      // 0x00
	TINT          m_iParameter; // 0x04 index of the parameter in the style + 1, 0 for none
	TGUIAlignment m_eAlignX;    // 0x08
	Fill          m_eFillX;     // 0x0C
	TGUIAlignment m_eAlignY;    // 0x10
	Fill          m_eFillY;     // 0x14
	TGUIColour    m_oColour;    // 0x18

	union
	{
		const TGUITextureSection* m_pTextureSection; // 0x1C ElementType_Texture
		const TGUIFont*           m_pFont;           // 0x1C ElementType_Label
	};

	const TWCHAR* m_wszLabel; // 0x20 ElementType_Label
};

TOSHI_NAMESPACE_END
