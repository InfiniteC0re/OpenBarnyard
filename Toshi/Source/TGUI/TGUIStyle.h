#pragma once
#include "TGUIDeserialisable.h"
#include "TGUIStyleRootComponent.h"
#include "TGUIStyleParameterInfo.h"
#include "TGUIPairNode.h"
#include "TGUIComponentCache.h"

#include <Toshi/TNodeList.h>
#include <Plugins/PPropertyParser/PPropertyValue.h>

TOSHI_NAMESPACE_START

class TGUIWidgetInfo;
class TGUIWidget;
class TGUIComponent;
class TGUIDisplayContext;
class TGUITextureSection;
class TGUIFont;

//-----------------------------------------------------------------------------
// Purpose: the look of a widget class ("[style]" blocks): for each root
// component of the class (TGUIWidgetInfo), states made of components
// (TGUIComponent) that are painted when the state of the widget matches their
// rule. The style also declares parameters (values the widgets give to the
// elements, like their text) and widget properties it fixes ("widgetprop").
//
// NOTE: follows Nicktoons: no interface (the TGUIManager singleton is used),
// the margins are 16-bit.
//-----------------------------------------------------------------------------
class TGUIStyle
    : public TGUIDeserialisable
    , public TNodeList<TGUIStyle>::TNode
{
public:
	TDECLARE_CLASS( TGUIStyle, TGUIDeserialisable );

	static constexpr TINT MAX_PARAMETERS = 16;

	using FixedWidgetProperty = TGUIPairNode<TPString8, PPropertyValue>;

public:
	TGUIStyle();
	TGUIStyle( const TGUIStyle& a_rcOther );
	~TGUIStyle();

	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;
	virtual TGUIStyle*   Clone() const;

	// Sets the widget class and the name, creates the root components of the class
	void Create( const TGUIWidgetInfo* a_pWidgetInfo, const TPString8& a_rcName );

	// Creates a component of the class a_rcComponentClass in the state of a root component with the rule a_oRule
	TGUIComponent* CreateComponent( TINT a_iRootComponent, const TPString8& a_rcComponentClass, TGUIStateRule a_oRule );
	TGUIComponent* CreateComponent( const TPString8& a_rcRootComponent, const TPString8& a_rcComponentClass, TGUIStateRule a_oRule );

	// The state of a root component that applies to the widget state a_uiState
	TGUIStyleState* GetState( TINT a_iRootComponent, TUINT a_uiState ) const;
	TGUIStyleState* GetState( const TPString8& a_rcRootComponent, TUINT a_uiState ) const;

	// The largest preferred size of the components of a root component (with its margins)
	TBOOL GetComponentPreferredSize( TINT a_iRootComponent, PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight );
	TBOOL GetComponentPreferredSize( TINT a_iRootComponent, PPropertyValue* a_pParameters, TINT& a_rWidth, TINT& a_rHeight, TINT16 a_iMaxWidth, TINT16 a_iMaxHeight, TBOOL& a_rbAllKnown );

	// Paints the states matching a_uiState of the root components (a_pCaches: one cache per root component, the
	// bounding box is the rectangle of the root component)
	void Paint( TGUIDisplayContext* a_pDisplayContext, TGUIWidget* a_pWidget, TUINT a_uiState, PPropertyValue* a_pParameters, TGUIComponentCache* a_pCaches );

	// Checks if a_iX, a_iY is in a root component that has a state matching a_uiState
	TBOOL HitTest( TGUIWidget* a_pWidget, TINT a_iX, TINT a_iY, TUINT a_uiState, PPropertyValue* a_pParameters, TGUIComponentCache* a_pCaches );

	// Declares a parameter (or changes its type), returns its index or -1
	TINT                          DeclareParameter( const TPString8& a_rcName, const TClass* a_pType );
	TINT                          FindParameter( const TPString8& a_rcName ) const;
	TBOOL                         SetParameterCount( TINT a_iCount );
	const TGUIStyleParameterInfo* GetParameterInfo( TINT a_iIndex ) const;
	const TGUIStyleParameterInfo* GetParameterInfo( const TPString8& a_rcName ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00006620
	TINT GetParameterCount() const { return m_uiNumParameters; }

	TBOOL IsCompatibleWith( TGUIStyle* a_pStyle );
	TBOOL IsReferencingTextureSection( const TGUITextureSection* a_pTextureSection ) const;
	TBOOL IsReferencingFont( const TGUIFont* a_pFont ) const;

	// The widget properties the style sets ("widgetprop" block)
	void           SetFixedWidgetProperty( const TPString8& a_rcName, const PPropertyValue& a_rcValue );
	PPropertyValue GetFixedWidgetProperty( const TPString8& a_rcName ) const;
	void           ReleaseFixedWidgetProperty( const TPString8& a_rcName );
	TBOOL          IsWidgetPropertyFixed( const TPString8& a_rcName ) const;

	// Creates a widget of the class of the style with the style
	TGUIWidget* CreateWidget();

	// Sets the fixed widget properties of the style to a_pWidget
	// NOTE: takes a TGUIWidget (part C of the TGUI port)
	void ApplyFixedWidgetProperties( TGUIDeserialisable* a_pWidget );

	// $JPOG: FUNCTION TGuiInterface.dll+000199b0
	TGUIStyleRootComponent& GetRootComponent( TINT a_iIndex ) { return m_pRootComponents[ a_iIndex ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006730
	const TGUIWidgetInfo* GetWidgetInfo() const { return m_pWidgetInfo; }

	TBOOL IsWidgetStyle( const TPString8& a_rcWidgetName ) const;

	// $JPOG: FUNCTION TGuiInterface.dll+00006740
	const TPString8& GetName() const { return m_strName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006750
	void SetName( const TPString8& a_rcName ) { m_strName = a_rcName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006630
	void GetMargins( TINT& a_rLeft, TINT& a_rRight, TINT& a_rTop, TINT& a_rBottom )
	{
		a_rLeft   = m_iMarginLeft;
		a_rRight  = m_iMarginRight;
		a_rTop    = m_iMarginTop;
		a_rBottom = m_iMarginBottom;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000066a0
	void SetMargins( TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom )
	{
		m_iMarginLeft   = TINT16( a_iLeft );
		m_iMarginRight  = TINT16( a_iRight );
		m_iMarginTop    = TINT16( a_iTop );
		m_iMarginBottom = TINT16( a_iBottom );
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00006670
	TINT GetMarginLeft() const { return m_iMarginLeft; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006660
	TINT GetMarginRight() const { return m_iMarginRight; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006690
	TINT GetMarginTop() const { return m_iMarginTop; }

	// $JPOG: FUNCTION TGuiInterface.dll+00006680
	TINT GetMarginBottom() const { return m_iMarginBottom; }

	// $JPOG: FUNCTION TGuiInterface.dll+000066e0
	void SetMarginLeft( TINT a_iMargin ) { m_iMarginLeft = TINT16( a_iMargin ); }

	// $JPOG: FUNCTION TGuiInterface.dll+000066d0
	void SetMarginRight( TINT a_iMargin ) { m_iMarginRight = TINT16( a_iMargin ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00006700
	void SetMarginTop( TINT a_iMargin ) { m_iMarginTop = TINT16( a_iMargin ); }

	// $JPOG: FUNCTION TGuiInterface.dll+000066f0
	void SetMarginBottom( TINT a_iMargin ) { m_iMarginBottom = TINT16( a_iMargin ); }

	// The functions of the property specs
	void           Set_name_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_name_property() const;
	void           Set_marginleft_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_marginleft_property() const;
	void           Set_marginright_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_marginright_property() const;
	void           Set_margintop_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_margintop_property() const;
	void           Set_marginbottom_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_marginbottom_property() const;

protected:
	FixedWidgetProperty* FindFixedWidgetPropertyPair( const TPString8& a_rcName ) const;

	// Updates the widgets that use the style after a change of its parameters or of its fixed widget properties
	void UpdateDependentWidgets();

private:
	TBOOL DeserialiseRootComponents( const PProperties* a_pProperties );
	TBOOL DeserialiseStates( const PProperties* a_pProperties, TGUIStyleRootComponent* a_pRootComponent );
	TBOOL DeserialiseComponents( const PProperties* a_pProperties, TNodeList<TGUIComponent>& a_rComponents );
	TBOOL DeserialiseWidgetProps( const PProperties* a_pProperties );
	TBOOL SerialiseRootComponents( PProperties* a_pProperties );
	TBOOL SerialiseStates( PProperties* a_pProperties, TGUIStyleRootComponent* a_pRootComponent );
	TBOOL SerialiseComponents( PProperties* a_pProperties, TNodeList<TGUIComponent>& a_rComponents );
	TBOOL SerialiseWidgetProps( PProperties* a_pProperties );

public:
	static PropertySet* ms_oPropertySet;
	static PropertySet* ms_oPropertySetOrdered;

	static TPString8 str_type;
	static TPString8 str_widget;
	static TPString8 str_name;
	static TPString8 str_widgetprop;
	static TPString8 str_component;
	static TPString8 str_state;
	static TPString8 str_element;
	static TPString8 str_param;
	static TPString8 str_marginleft;
	static TPString8 str_marginright;
	static TPString8 str_margintop;
	static TPString8 str_marginbottom;

	// Category of the specs of the style properties and of the parameters
	// NOTE: pooled strings of the GUI in Nicktoons ("TGUIStyle" is created by InitialiseClass, "Style Params" is global)
	static TPString8 ms_strCategory;
	static TPString8 ms_strStyleParams;

private:
	TGUIStyleRootComponent*        m_pRootComponents;        // 0x10 one per root component of the widget class
	const TGUIWidgetInfo*          m_pWidgetInfo;            // 0x14
	TPString8                      m_strName;                // 0x18
	TUINT8                         m_uiNumParameters;        // 0x1C
	TGUIStyleParameterInfo*        m_pParameters;            // 0x20
	TNodeList<FixedWidgetProperty> m_oFixedWidgetProperties; // 0x24
	TINT16                         m_iMarginTop;             // 0x38
	TINT16                         m_iMarginBottom;          // 0x3A
	TINT16                         m_iMarginLeft;            // 0x3C
	TINT16                         m_iMarginRight;           // 0x3E
};

TOSHI_NAMESPACE_END
