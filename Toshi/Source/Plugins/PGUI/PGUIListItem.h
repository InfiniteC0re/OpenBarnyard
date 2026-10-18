#pragma once
#include <TGUI/TGUIWidget.h>

class PGUIItemData;

//-----------------------------------------------------------------------------
// Purpose: the widget showing an item of a list box (the list item style of
// the list box): the "background" and "foreground" components, the item it
// shows and its index in the list box.
//-----------------------------------------------------------------------------
class PGUIListItem : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUIListItem, Toshi::TGUIWidget );

public:
	PGUIListItem();
	PGUIListItem( const PGUIListItem& a_rcOther );
	~PGUIListItem();

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
	virtual PGUIListItem*                Clone() const override;
	virtual TBOOL                        AcceptsFocus() const override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	// $JPOG: FUNCTION PGUICore.dll+0000c850
	void SetData( PGUIItemData* a_pData ) { m_pData = a_pData; }

	PGUIItemData* GetData() const { return m_pData; }

	// $JPOG: FUNCTION PGUICore.dll+0000c830
	void SetIndex( TINT a_iIndex ) { m_iIndex = a_iIndex; }

	// $JPOG: FUNCTION PGUICore.dll+0000c840
	TINT GetIndex() const { return m_iIndex; }

	// $JPOG: FUNCTION PGUICore.dll+0000c870
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

private:
	PGUIItemData* m_pData;  // 0x8C
	TINT          m_iIndex; // 0x90 -1 when it shows no item
};
