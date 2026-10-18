#pragma once
#include "PGUIButton.h"

//-----------------------------------------------------------------------------
// Purpose: a button that toggles its "Toggled" state when it is pressed (then
// throws its action event). Its style has a background, a foreground (the
// label, between the icons) and the "iconleft" and "iconright" components.
//-----------------------------------------------------------------------------
class PGUIToggleButton : public PGUIButton
{
public:
	TDECLARE_CLASS( PGUIToggleButton, PGUIButton );

	enum State : TUINT
	{
		State_Toggled = BITFLAG( 6 ),
	};

public:
	PGUIToggleButton();
	PGUIToggleButton( const PGUIToggleButton& a_rcOther );
	~PGUIToggleButton();

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
	virtual PGUIToggleButton*            Clone() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUIButton
	//-----------------------------------------------------------------------------
	virtual void DoButtonAction() override;

	// A test property, it does nothing (always true)
	void           Set_test_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_test_property() const;
	TBOOL          Val_test_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+0001c3a0
	void SetToggled( TBOOL a_bToggled ) { SetState( State_Toggled, a_bToggled ? TUINT( State_Toggled ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+0001c3c0
	TBOOL IsToggled() const { return ( m_uiState & State_Toggled ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+0001c3d0
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;
};
