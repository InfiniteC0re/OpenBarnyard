#pragma once
#include "TGUIElement.h"
#include "TGUIComponentInfo.h"
#include "TGUIComponentCache.h"

#include <Toshi/TNodeList.h>
#include <Toshi/TPString8.h>

class PProperties;
class PPropertyValue;

TOSHI_NAMESPACE_START

class TGUIStyle;
class TGUIFunction;
class TGUIDisplayContext;

//-----------------------------------------------------------------------------
// Purpose: a layer of the look of a widget state (a "component" block of a
// style state): a fixed set of elements (TGUIElement) laid out in the
// rectangle of the widget (a frame, a stretched bar, an icon, a text...),
// with margins and optional functions that animate its alpha and its size.
// The classes are registered with a TGUIComponentInfo and created by name
// ("type = TGUIFrameComponent;").
//
// NOTE: follows Nicktoons, which adds "optimise_size" (the preferred size of
// the frame fits a whole number of repeated texture sections).
//-----------------------------------------------------------------------------
class TGUIComponent
    : public TObject
    , public TNodeList<TGUIComponent>::TNode
{
public:
	TDECLARE_CLASS( TGUIComponent, TObject );

	enum Function
	{
		Function_Alpha,
		Function_Size,
		Function_NUMOF
	};

public:
	TGUIComponent();
	TGUIComponent( const TGUIComponent& a_rcOther );
	~TGUIComponent();

	// +0x14
	virtual TGUIComponent* Clone() const = 0;

	// +0x18: Reads the margins, the elements and the functions; the parameters of the elements are declared in a_pStyle
	virtual TBOOL Deserialise( const PProperties* a_pProperties, TGUIStyle* a_pStyle );

	// +0x1C
	virtual TBOOL Serialise( PProperties* a_pProperties, TGUIStyle* a_pStyle );

	// +0x20: Size the component needs (a_pParameters: the style parameters of the widget, a_iMaxWidth/a_iMaxHeight: the
	// size available, -1 if unknown; a_rWidth/a_rHeight can be read as the current size)
	virtual TBOOL GetPreferredSize( PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) const = 0;

	// +0x24
	virtual const TGUIComponentInfo* GetComponentInfo() const = 0;

	// +0x28: The elements, in the order of the element names of the component info
	virtual TGUIElement* GetElements() = 0;

	// +0x2C
	virtual void Paint( TGUIDisplayContext* a_pDisplayContext, PPropertyValue* a_pParameters, const TGUIRectangle& a_rcRect, TGUIComponentCache* a_pCache ) const = 0;

	// +0x30: Called when the elements are changed
	// $NicktoonsPS2: FUNCTION 0037b4a8
	// $JPOG: FUNCTION TGuiInterface.dll+00006cd0
	virtual void NotifyElementsChanged() {}

	// +0x34: Destroys the data the component keeps in a TGUIComponentCache
	// $NicktoonsPS2: FUNCTION 0037b4b0
	// $JPOG: FUNCTION TGuiInterface.dll+00006cc0
	virtual void DestroyCache( void* a_pData ) const {}

	// Index of the element named a_rcName or -1
	TINT FindElementName( const TPString8& a_rcName ) const;

	TGUIElement& GetElement( TINT a_iIndex );
	TGUIElement& GetElement( const TPString8& a_rcName );

	TBOOL IsReferencingTextureSection( const TGUITextureSection* a_pTextureSection ) const;
	TBOOL IsReferencingFont( const TGUIFont* a_pFont ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00006ce0
	void GetMargins( TINT& a_rLeft, TINT& a_rRight, TINT& a_rTop, TINT& a_rBottom )
	{
		a_rLeft   = m_iMarginLeft;
		a_rRight  = m_iMarginRight;
		a_rTop    = m_iMarginTop;
		a_rBottom = m_iMarginBottom;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00006d50
	void SetMargins( TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom )
	{
		m_iMarginLeft   = TINT8( a_iLeft );
		m_iMarginRight  = TINT8( a_iRight );
		m_iMarginTop    = TINT8( a_iTop );
		m_iMarginBottom = TINT8( a_iBottom );
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00006d20
	TINT GetMarginLeft() const { return m_iMarginLeft; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006d10
	TINT GetMarginRight() const { return m_iMarginRight; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006d40
	TINT GetMarginTop() const { return m_iMarginTop; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006d30
	TINT GetMarginBottom() const { return m_iMarginBottom; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006d80
	void SetMarginLeft( TINT a_iMargin ) { m_iMarginLeft = TINT8( a_iMargin ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00006d70
	void SetMarginRight( TINT a_iMargin ) { m_iMarginRight = TINT8( a_iMargin ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00006da0
	void SetMarginTop( TINT a_iMargin ) { m_iMarginTop = TINT8( a_iMargin ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00006d90
	void SetMarginBottom( TINT a_iMargin ) { m_iMarginBottom = TINT8( a_iMargin ); }

	TBOOL         IsOptimiseSize() const { return m_bOptimiseSize; }
	TGUIFunction* GetFunction( Function a_eFunction ) const { return m_apFunctions[ a_eFunction ]; }

protected:
	// Offset of the rectangle of the elements at a_fTime (the size function grows it, centred)
	void ComputeSizeFunction( TFLOAT a_fTime, TGUIRectangle& a_rOffset ) const;

	// Alpha of the elements at a_fTime (0-255)
	TINT ComputeAlphaFunction( TFLOAT a_fTime ) const;

	void CopyElements( TGUIElement* a_pDestination, const TGUIElement* a_pSource, TINT a_iCount );

public:
	static TPString8 str_states;
	static TPString8 str_state;
	static TPString8 str_element;
	static TPString8 str_param;
	static TPString8 str_type;
	static TPString8 str_marginleft;
	static TPString8 str_marginright;
	static TPString8 str_margintop;
	static TPString8 str_marginbottom;
	static TPString8 str_function;
	static TPString8 str_optimise_size;

	// "alpha", "size"
	static TPString8          ms_saFunctionName[ Function_NUMOF ];
	static const TCHAR* const ms_csaFunctionNameStr[ Function_NUMOF ];

protected:
	TINT8         m_iMarginLeft;                   // 0x10
	TINT8         m_iMarginRight;                  // 0x11
	TINT8         m_iMarginTop;                    // 0x12
	TINT8         m_iMarginBottom;                 // 0x13
	TBOOL         m_bOptimiseSize;                 // 0x14 NOTE: not in JPOG
	TGUIFunction* m_apFunctions[ Function_NUMOF ]; // 0x18
};

TOSHI_NAMESPACE_END
