#pragma once
#include "PGUIPanel.h"
#include "PGUIButtonListenerList.h"

class PGUIMenuTree;
class PGUIMenuTreeItem;

//-----------------------------------------------------------------------------
// Purpose: a panel with one button per visible child of its root item (the
// current item styles of the items); pressing a button selects its item in
// the menu tree.
//-----------------------------------------------------------------------------
class PGUIMenuBar : public PGUIPanel
{
public:
	TDECLARE_CLASS( PGUIMenuBar, PGUIPanel );

public:
	PGUIMenuBar();
	PGUIMenuBar( const PGUIMenuBar& a_rcOther );
	~PGUIMenuBar();

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
	virtual PGUIMenuBar*                 Clone() const override;

	//-----------------------------------------------------------------------------
	// PGUIMenuBar
	//-----------------------------------------------------------------------------

	// +0xB4: The item of a button of the bar
	virtual PGUIMenuTreeItem* FindItemData( Toshi::TGUIWidget* a_pWidget );

	// Shows the children of a_pItem without selection
	void SetRootItem( PGUIMenuTreeItem* a_pItem );

	// Shows the siblings of a_pItem, a_pItem selected
	void SetSelectedItem( PGUIMenuTreeItem* a_pItem );

	// Recreates the buttons of the items of the root item
	void UpdateMenu();

	// $JPOG: FUNCTION PGUICore.dll+0000e030
	void SetMenuTree( PGUIMenuTree* a_pMenuTree ) { m_pMenuTree = a_pMenuTree; }

	// $JPOG: FUNCTION PGUICore.dll+0000e020
	PGUIMenuTree* GetMenuTree() const { return m_pMenuTree; }

	// $JPOG: FUNCTION PGUICore.dll+0000e010
	PGUIMenuTreeItem* GetRootItem() const { return m_pRootItem; }

	// $JPOG: FUNCTION PGUICore.dll+0000e000
	Toshi::TGUIWidget* GetSelectedButton() const { return m_pSelectedButton; }

	// $JPOG: FUNCTION PGUICore.dll+0000e040
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

protected:
	void SetSelectedButton( Toshi::TGUIWidget* a_pButton );
	void UpdateSelection();

	// Keeps a button that isn't shown any more to reuse it
	void DiscardButton( Toshi::TGUIWidget* a_pButton );

	// A recycled button of the class of a_pStyle (with that style), or a new one
	Toshi::TGUIWidget* RecycleButton( Toshi::TGUIStyle* a_pStyle );

	static TBOOL __stdcall OnButtonAction( PGUIMenuBar* a_pMenuBar, PGUIButton* a_pButton, Toshi::TGUIActionEvent* a_pEvent );

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

protected:
	PGUIMenuTree*                       m_pMenuTree;        // 0x8C
	PGUIMenuTreeItem*                   m_pRootItem;        // 0x90
	PGUIMenuTreeItem*                   m_pSelectedItem;    // 0x94
	Toshi::TGUIWidget*                  m_pSelectedButton;  // 0x98
	Toshi::TNodeList<Toshi::TGUIWidget> m_oRecycledButtons; // 0x9C
	PGUIButtonListenerList<PGUIMenuBar> m_oButtonListeners; // 0xAC
};
