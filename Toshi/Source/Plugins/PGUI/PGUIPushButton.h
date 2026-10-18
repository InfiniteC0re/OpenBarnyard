#pragma once
#include "PGUIButton.h"

//-----------------------------------------------------------------------------
// Purpose: a button that throws its action event when it is pressed. Its
// style has a background, a foreground (the label) and an effect component.
//-----------------------------------------------------------------------------
class PGUIPushButton : public PGUIButton
{
public:
	TDECLARE_CLASS( PGUIPushButton, PGUIButton );

public:
	PGUIPushButton();
	PGUIPushButton( const PGUIPushButton& a_rcOther );
	~PGUIPushButton();

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
	virtual PGUIPushButton*              Clone() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUIButton
	//-----------------------------------------------------------------------------
	virtual void DoButtonAction() override;

	// $JPOG: FUNCTION PGUICore.dll+00013910
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;
};
