#pragma once
#include <TGUI/TGUIWidget.h>

//-----------------------------------------------------------------------------
// Purpose: a static widget that shows the "background" and "foreground" (the
// text) components of its style, sized to fit them.
//-----------------------------------------------------------------------------
class PGUILabel : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUILabel, Toshi::TGUIWidget );

public:
	PGUILabel();
	PGUILabel( const PGUILabel& a_rcOther );
	~PGUILabel();

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
	virtual PGUILabel*              Clone() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	// $JPOG: FUNCTION PGUICore.dll+00008690
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;
};
