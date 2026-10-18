#pragma once
#include "PGUIPanel.h"

//-----------------------------------------------------------------------------
// Purpose: two children on top of each other, separated by the "splitter"
// component. The space left is shared by the stretch factors of the top and
// bottom children (equally without them).
//-----------------------------------------------------------------------------
class PGUIHorizontalSplit : public PGUIPanel
{
public:
	TDECLARE_CLASS( PGUIHorizontalSplit, PGUIPanel );

public:
	PGUIHorizontalSplit();
	PGUIHorizontalSplit( const PGUIHorizontalSplit& a_rcOther );
	~PGUIHorizontalSplit();

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
	virtual PGUIHorizontalSplit*         Clone() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	Toshi::TGUIWidget* GetTopWidget();
	Toshi::TGUIWidget* GetBottomWidget();

	void           Set_stretchtop_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_stretchtop_property() const;
	TBOOL          Val_stretchtop_property( const PPropertyValue& a_rcValue );
	void           Set_stretchbottom_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_stretchbottom_property() const;
	TBOOL          Val_stretchbottom_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+000074b0
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

private:
	TFLOAT m_fStretchTop;    // 0x8C
	TFLOAT m_fStretchBottom; // 0x90
};
