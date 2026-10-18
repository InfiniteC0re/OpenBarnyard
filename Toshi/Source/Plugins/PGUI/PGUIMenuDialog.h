#pragma once
#include "PGUIDialog.h"
#include "PGUIButtonListenerList.h"
#include "PGUIMenuTreeItemStyle.h"

class PGUIMenuTree;
class PGUIMenuTreeItem;

//-----------------------------------------------------------------------------
// Purpose: a menu of a menu tree (made from a dialog template): one button
// per visible child of its root item (in its "menubucket" child, with the
// item styles of the items), the "menuheading" (the root item), and
// "itemselected"/"itemback" (the selected item, or back when there's none).
// The navigation scrolls, descends and ascends in the tree, the select
// action selects the item (or goes back).
//-----------------------------------------------------------------------------
class PGUIMenuDialog : public PGUIDialog
{
public:
	TDECLARE_CLASS( PGUIMenuDialog, PGUIDialog );

public:
	PGUIMenuDialog();
	PGUIMenuDialog( const PGUIMenuDialog& a_rcOther );
	~PGUIMenuDialog();

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
	virtual PGUIMenuDialog*              Clone() const override;
	virtual TBOOL                        AcceptsFocus() const override;
	virtual void                         NotifyFocusLost( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual void                         NotifyFocusGained( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRepeat( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRelease( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUIMenuDialog
	//-----------------------------------------------------------------------------

	// +0xB8: Places the menu for its display mode (the next menu at the middle of the screen, the current one at
	// a_rX, a_rY), within the screen margins of the tree; returns in a_rX, a_rY where the next menu goes
	virtual void UpdatePosition( TINT& a_rX, TINT& a_rY );

	// +0xBC: The item of a button of the menu
	virtual PGUIMenuTreeItem* FindItemData( Toshi::TGUIWidget* a_pWidget );

	// +0xC0: Shows the selected button (or the back item without selection)
	virtual void SetSelectedButton( Toshi::TGUIWidget* a_pButton );

	// +0xC4: Gives the selected button its selected style
	virtual void UpdateSelection();

	// +0xC8: Recreates the buttons of the items of the root item
	virtual void UpdateMenu();

	// Shows the children of a_pItem without selection
	void SetRootItem( PGUIMenuTreeItem* a_pItem );

	// Shows the siblings of a_pItem, a_pItem selected
	void SetSelectedItem( PGUIMenuTreeItem* a_pItem );

	// The a_iIndex-th visible item
	PGUIMenuTreeItem* GetItemAtIndex( TINT a_iIndex );

	// $JPOG: FUNCTION PGUICore.dll+00005d90
	void SetMenuTree( PGUIMenuTree* a_pMenuTree ) { m_pMenuTree = a_pMenuTree; }

	// $JPOG: FUNCTION PGUICore.dll+00005d80
	void SetDisplayMode( PGUIMenuTreeItemStyle::DisplayLevel a_eDisplayMode ) { m_eDisplayMode = a_eDisplayMode; }

	// $JPOG: FUNCTION PGUICore.dll+00005d60
	PGUIMenuTreeItem* GetRootItem() const { return m_pRootItem; }

	// $JPOG: FUNCTION PGUICore.dll+00005d70
	Toshi::TGUIWidget* GetSelectedButton() const { return m_pSelectedButton; }

	// $JPOG: FUNCTION PGUICore.dll+00005db0
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

protected:
	// Keeps a button that isn't shown any more to reuse it
	void DiscardButton( Toshi::TGUIWidget* a_pButton );

	// A recycled button of the class of a_pStyle (with that style), or a new one
	Toshi::TGUIWidget* RecycleButton( Toshi::TGUIStyle* a_pStyle );

	static TBOOL __stdcall OnButtonAction( PGUIMenuDialog* a_pMenu, PGUIButton* a_pButton, Toshi::TGUIActionEvent* a_pEvent );

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

	static Toshi::TPString8 str_menubucket;
	static Toshi::TPString8 str_menuheading;
	static Toshi::TPString8 str_itemselected;
	static Toshi::TPString8 str_itemback;

protected:
	PGUIMenuTree*                        m_pMenuTree;        // 0xB0
	PGUIMenuTreeItem*                    m_pRootItem;        // 0xB4
	PGUIMenuTreeItem*                    m_pSelectedItem;    // 0xB8
	Toshi::TGUIWidget*                   m_pSelectedButton;  // 0xBC
	Toshi::TGUIWidget*                   m_pMenuBucket;      // 0xC0
	Toshi::TGUIWidget*                   m_pMenuHeading;     // 0xC4
	Toshi::TGUIWidget*                   m_pItemSelected;    // 0xC8
	Toshi::TGUIWidget*                   m_pItemBack;        // 0xCC
	PGUIMenuTreeItemStyle::DisplayLevel  m_eDisplayMode;     // 0xD0
	Toshi::TNodeList<Toshi::TGUIWidget>  m_oRecycledButtons; // 0xD4
	PGUIButtonListenerList<PGUIMenuDialog> m_oButtonListeners; // 0xE4
};
