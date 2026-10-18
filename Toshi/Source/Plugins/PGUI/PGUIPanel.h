#pragma once
#include <TGUI/TGUIWidget.h>

//-----------------------------------------------------------------------------
// Purpose: a widget that groups its children (arranged by its layout manager)
// over the "background" component of its style, sized to fit both.
//
// NOTE: the PGUI widgets return their widget info from GetWidgetInfo (the
// JPOG constructors store it in the widget) and create it in InitialiseClass
// (JPOG creates it in the static initialiser of the class). The field offsets
// in the comments of the PGUI classes are the JPOG ones (its TGUIWidget is
// 0x8C bytes).
//-----------------------------------------------------------------------------
class PGUIPanel : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUIPanel, Toshi::TGUIWidget );

	enum State : TUINT
	{
		State_Editing = BITFLAG( 5 ),
	};

public:
	PGUIPanel();
	PGUIPanel( const PGUIPanel& a_rcOther );
	~PGUIPanel();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	//-----------------------------------------------------------------------------
	// Toshi::TGUIWidget
	//-----------------------------------------------------------------------------
	virtual const Toshi::TGUIWidgetInfo* GetWidgetInfo() const override;
	virtual PGUIPanel*                   Clone() const override;
	virtual TBOOL                        AllowsChildren() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	// $JPOG: FUNCTION PGUICore.dll+000074c0
	void SetEditing( TBOOL a_bEditing ) { SetState( State_Editing, a_bEditing ? TUINT( State_Editing ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+000074e0
	TBOOL IsEditing() const { return ( m_uiState & State_Editing ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+00007500
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;
};
