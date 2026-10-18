#pragma once
#include "PGUIPanel.h"

//-----------------------------------------------------------------------------
// Purpose: two children side by side, separated by the "splitter" component.
// The space left is shared by the stretch factors of the left and right
// children (equally without them).
//-----------------------------------------------------------------------------
class PGUIVerticalSplit : public PGUIPanel
{
public:
	TDECLARE_CLASS( PGUIVerticalSplit, PGUIPanel );

public:
	PGUIVerticalSplit();
	PGUIVerticalSplit( const PGUIVerticalSplit& a_rcOther );
	~PGUIVerticalSplit();

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
	virtual PGUIVerticalSplit*         Clone() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	Toshi::TGUIWidget* GetLeftWidget();
	Toshi::TGUIWidget* GetRightWidget();

	void           Set_stretchleft_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_stretchleft_property() const;
	TBOOL          Val_stretchleft_property( const PPropertyValue& a_rcValue );
	void           Set_stretchright_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_stretchright_property() const;
	TBOOL          Val_stretchright_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+0001d520
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

private:
	TFLOAT m_fStretchLeft;  // 0x8C
	TFLOAT m_fStretchRight; // 0x90
};
