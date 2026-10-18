#pragma once
#include "PGUIMenuTreeItem.h"

#include <TGUI/TGUIDeserialisable.h>
#include <TGUI/TGUIInputManager.h>
#include <TGUI/TGUIEvents.h>
#include <Toshi/TArray.h>
#include <Toshi/TEvent.h>

namespace Toshi
{
class TGUIScreen;
}

class PGUIMenuDialog;

//-----------------------------------------------------------------------------
// Purpose: a tree of menu items shown as menus (PGUIMenuDialog, from the
// dialog templates of the item styles) on a screen: the current menu, the
// previous ones and the next one. When invoked it is the input manager of
// the screen: the navigation moves the selection, descends into the
// submenus and ascends back, the select action selects a leaf item (select
// event, then the close event), the cancel action goes back.
//
// NOTE: JPOG keeps the GUI interface (SetInterface/GetInterface), the
// TGUIManager singleton is used here.
//-----------------------------------------------------------------------------
class PGUIMenuTree
    : public Toshi::TGUIDeserialisable
    , public Toshi::TGUIInputManager
{
public:
	TDECLARE_CLASS( PGUIMenuTree, Toshi::TGUIDeserialisable );

	// Thrown by the close emitter when the menus close: a leaf item was selected (TFALSE) or the menus were left
	// NOTE: the name is unknown
	struct CloseEvent
	{
		TBOOL bCancelled; // 0x0
	};

	// Thrown by the selection emitter when the selected item changes
	// NOTE: the name is unknown
	struct SelectionEvent
	{
		PGUIMenuTreeItem* pItem; // 0x0
	};

	using SelectEmitter    = Toshi::TEmitter<PGUIMenuTree, Toshi::TGUIActionEvent>;
	using CloseEmitter     = Toshi::TEmitter<PGUIMenuTree, CloseEvent>;
	using SelectionEmitter = Toshi::TEmitter<PGUIMenuTree, SelectionEvent>;

public:
	PGUIMenuTree();
	PGUIMenuTree( const PGUIMenuTree& a_rcOther );
	~PGUIMenuTree();

	//-----------------------------------------------------------------------------
	// Toshi::TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	// +0x34
	virtual PGUIMenuTree* Clone() const;

	//-----------------------------------------------------------------------------
	// Toshi::TGUIInputManager
	//-----------------------------------------------------------------------------
	virtual TBOOL DoButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoButtonRepeat( Toshi::TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoButtonRelease( Toshi::TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoAxisMove( Toshi::TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoPointerMove( Toshi::TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual void  DoRolloverUpdate() override;

	//-----------------------------------------------------------------------------
	// PGUIMenuTree
	//-----------------------------------------------------------------------------

	// Moves the menus to another screen
	void SetScreen( Toshi::TGUIScreen* a_pScreen );

	// The item style group of the items of the menus (styles found by name) and of their menus (templates found by name)
	PGUIMenuTreeItemStyle* CreateItemStyleGroup( const TCHAR* a_szCurrentItemStyle, const TCHAR* a_szCurrentItemSelectedStyle, const TCHAR* a_szNextItemStyle );
	PGUIMenuTreeItemStyle* CreateMenuStyleGroup( const TCHAR* a_szCurrentItemStyle, const TCHAR* a_szCurrentItemSelectedStyle, const TCHAR* a_szNextItemStyle, const TCHAR* a_szPreviousMenuTemplate, const TCHAR* a_szCurrentMenuTemplate, const TCHAR* a_szNextMenuTemplate );

	// Deletes the items (a new empty root item)
	void Reset();

	// Calls a_fnPre and a_fnPost on the root item and its descendants
	void RecurseTree( PGUIMenuTreeItem::t_RecurseCallback a_fnPre, PGUIMenuTreeItem::t_RecurseCallback a_fnPost, PGUIMenuTreeItem::t_RecurseCallback a_fnFilter, void* a_pUserData );

	// Opens the menu of a child of the root item (or of a_pItem) at a_iX, a_iY and becomes the input manager of the screen
	TBOOL InvokeMenu( TINT a_iIndex, TINT a_iX, TINT a_iY );
	TBOOL InvokeMenu( PGUIMenuTreeItem* a_pItem, TINT a_iX, TINT a_iY );

	// Closes the menus
	void CancelMenu();

	// Recreates the menus of the selected item and of its parents (a_pOldSelection: the previous selected item)
	void UpdateMenu( PGUIMenuTreeItem* a_pOldSelection );

	void ScrollMenu( TINT a_iDirection );
	void DescendMenu();
	void AscendMenu();
	void BackMenu();
	void SelectMenu();
	void DeselectMenu();

	void SetSelectedItem( PGUIMenuTreeItem* a_pItem );

	// The current menu (the one before the next menu)
	PGUIMenuDialog* GetCurrentMenu();

	// Sets the bits a_uiMask of the items to the bits of their children (recursively), returns the state of a_pItem
	TUINT PropagateState( TUINT a_uiMask, PGUIMenuTreeItem* a_pItem );

	// Sets the bits a_uiMask to a_uiValue from a_pFrom up to a_pTo (excluded) and updates the menus
	void PropagateState( TUINT a_uiMask, TUINT a_uiValue, PGUIMenuTreeItem* a_pFrom, PGUIMenuTreeItem* a_pTo );

	// Deletes the menus and the recycled menus
	void FlushOldDialogs();

	void SetScreenMargins( TINT a_iLeft, TINT a_iRight, TINT a_iTop, TINT a_iBottom );
	void SetScreenMarginLeft( TINT a_iMargin );
	void SetScreenMarginRight( TINT a_iMargin );
	void SetScreenMarginTop( TINT a_iMargin );
	void SetScreenMarginBottom( TINT a_iMargin );

	// $JPOG: FUNCTION PGUICore.dll+00005e20
	TINT GetScreenMarginLeft() const { return m_iScreenMarginLeft; }

	// $JPOG: FUNCTION PGUICore.dll+00005e10
	TINT GetScreenMarginRight() const { return m_iScreenMarginRight; }

	// $JPOG: FUNCTION PGUICore.dll+00005e00
	TINT GetScreenMarginTop() const { return m_iScreenMarginTop; }

	// $JPOG: FUNCTION PGUICore.dll+00005df0
	TINT GetScreenMarginBottom() const { return m_iScreenMarginBottom; }

	// $JPOG: FUNCTION PGUICore.dll+00005e30
	Toshi::TGUIScreen* GetScreen() const { return m_pScreen; }

	// $JPOG: FUNCTION PGUICore.dll+00005dd0
	PGUIMenuTreeItem* GetRootItem() const { return m_pRootItem; }

	// $JPOG: FUNCTION PGUICore.dll+00005dc0
	PGUIMenuTreeItem* GetSelectedItem() const { return m_pSelectedItem; }

	// $JPOG: FUNCTION PGUICore.dll+00005de0
	TBOOL IsInvoked() const { return m_bInvoked; }

	SelectEmitter&    GetSelectEmitter() { return m_oSelectEmitter; }
	CloseEmitter&     GetCloseEmitter() { return m_oCloseEmitter; }
	SelectionEmitter& GetSelectionEmitter() { return m_oSelectionEmitter; }

private:
	void RemoveMenus();
	void AddMenus();

	// Keeps a menu that isn't shown any more to reuse it
	void DiscardDialog( PGUIMenuDialog* a_pDialog );

	// A recycled menu made from a_pTemplate (same name), or a clone of it
	PGUIMenuDialog* RecycleDialog( PGUIMenuDialog* a_pTemplate );

public:
	static PropertySet* ms_oPropertySet;
	static PropertySet* ms_oPropertySetOrdered;

private:
	Toshi::TGUIScreen*                     m_pScreen;             // 0x0C
	PGUIMenuTreeItem*                      m_pRootItem;           // 0x10
	PGUIMenuTreeItem*                      m_pInvokedItem;        // 0x14 the item whose menu was invoked
	PGUIMenuTreeItem*                      m_pCurrentItem;        // 0x18 the parent of the selected item (its menu is the current one)
	PGUIMenuTreeItem*                      m_pSelectedItem;       // 0x1C
	Toshi::TArray<PGUIMenuDialog*>         m_aMenus;              // 0x20 from the next menu to the first previous one
	Toshi::TNodeList<PGUIMenuTreeItemStyle> m_oItemStyles;        // 0x30
	Toshi::TNodeList<Toshi::TGUIWidget>    m_oRecycledDialogs;    // 0x40
	TINT                                   m_iScreenMarginLeft;   // 0x54
	TINT                                   m_iScreenMarginRight;  // 0x58
	TINT                                   m_iScreenMarginTop;    // 0x5C
	TINT                                   m_iScreenMarginBottom; // 0x60
	TBOOL                                  m_bUpdating;           // 0x64 in SetSelectedItem (the rollover isn't updated)
	TBOOL                                  m_bInvoked;            // 0x65
	TINT                                   m_iInvokeX;            // 0x68
	TINT                                   m_iInvokeY;            // 0x6C
	SelectEmitter                          m_oSelectEmitter;      // 0x70
	CloseEmitter                           m_oCloseEmitter;       // 0x7C
	SelectionEmitter                       m_oSelectionEmitter;   // 0x88
};
