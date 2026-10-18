#pragma once
#include "TGUIDeserialisable.h"

#include <Toshi/TNodeList.h>

TOSHI_NAMESPACE_START

class TGUIWidget;

//-----------------------------------------------------------------------------
// Purpose: description of a layout manager class, registered to the
// TGUIManager by the static initialiser of the class: its name (the "type"
// of the "layout" block of the widgets), a label and its properties.
//-----------------------------------------------------------------------------
class TGUILayoutInfo : public TNodeList<TGUILayoutInfo>::TNode
{
public:
	using PropertySet = TGUIDeserialisable::PropertySet;

public:
	TGUILayoutInfo( const TPString8& a_rcName, const TPString8& a_rcLabel, PropertySet* a_pProperties );
	TGUILayoutInfo( const TGUILayoutInfo& a_rcOther );
	~TGUILayoutInfo();

	TGUILayoutInfo& operator=( const TGUILayoutInfo& a_rcOther );

	// $JPOG: FUNCTION TGuiInterface.dll+000065e0
	const TPString8& GetLayoutName() const { return m_strName; }

	// $JPOG: FUNCTION TGuiInterface.dll+000065d0
	const TPString8& GetLayoutLabel() const { return m_strLabel; }

	// $JPOG: FUNCTION TGuiInterface.dll+000065c0
	const TClass* GetTClass() const { return m_pClass; }

	// $JPOG: FUNCTION TGuiInterface.dll+000065b0
	TINT GetPropertyCount() const { return m_pProperties->Size(); }

	// $JPOG: FUNCTION TGuiInterface.dll+000065a0
	const TGUIPropertySpec* GetPropertyInfo( TINT a_iIndex ) const { return ( *m_pProperties )[ a_iIndex ]; }

private:
	TPString8     m_strName;     // 0x0C
	TPString8     m_strLabel;    // 0x10
	const TClass* m_pClass;      // 0x14 found by name
	PropertySet*  m_pProperties; // 0x18
};

//-----------------------------------------------------------------------------
// Purpose: arranges the children of a widget ("layout" block of the widgets).
// The layout managers are defined by PGUI (PGUIStackLayout, PGUIGridLayout...).
//
// NOTE: follows Nicktoons: no SetInterface/GetInterface (JPOG keeps the
// interface at +0x4).
//-----------------------------------------------------------------------------
class TGUILayoutManager : public TGUIDeserialisable
{
public:
	TDECLARE_CLASS( TGUILayoutManager, TGUIDeserialisable );

	enum Alignment : TINT
	{
		Alignment_Left,
		Alignment_Center,
		Alignment_Right,
		Alignment_Top    = Alignment_Left,
		Alignment_Bottom = Alignment_Right,
	};

public:
	TGUILayoutManager();
	TGUILayoutManager( const TGUILayoutManager& a_rcOther );
	~TGUILayoutManager();

	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	// +0x34
	virtual TGUILayoutManager* Clone() const = 0;

	// +0x38: Packs the children of a_pWidget (TGUIWidget::PackChildren)
	virtual void DoPack( TGUIWidget* a_pWidget ) = 0;

	// +0x3C: Lays out the children of a_pWidget, returns the size it needs (TGUIWidget::LayoutChildren)
	virtual void DoLayout( TGUIWidget* a_pWidget, TINT& a_rWidth, TINT& a_rHeight ) = 0;

public:
	static PropertySet* ms_oPropertySet;
	static PropertySet* ms_oPropertySetOrdered;
};

TOSHI_NAMESPACE_END
