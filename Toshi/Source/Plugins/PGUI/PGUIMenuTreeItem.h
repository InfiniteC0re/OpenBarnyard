#pragma once
#include "PGUIItemData.h"
#include "PGUIMenuTreeItemStyle.h"

//-----------------------------------------------------------------------------
// Purpose: an item of a menu tree: its values (the label first) and state, the
// item style of its buttons and menus, its parent and its children (the items
// of the menu it opens).
//
// NOTE: JPOG gives the labels as TLString (its localised strings), they're
// wide strings here.
//-----------------------------------------------------------------------------
class PGUIMenuTreeItem : public PGUIItemData
{
public:
	using t_RecurseCallback = TBOOL( __stdcall* )( PGUIMenuTreeItem* a_pItem, void* a_pUserData );

public:
	PGUIMenuTreeItem( PGUIMenuTreeItemStyle* a_pStyle, TINT a_iNumValues, TUINT a_uiState );

	// Visible, with the label as its first value
	PGUIMenuTreeItem( PGUIMenuTreeItemStyle* a_pStyle, const TWCHAR* a_wszLabel );
	~PGUIMenuTreeItem();

	virtual PGUIMenuTreeItem* Clone() const override;

	// Adds a child with the item style of this item (or returns the child with that label)
	PGUIMenuTreeItem* AddItem( const TWCHAR* a_wszLabel, TUINT a_uiState );
	void              AddItem( PGUIMenuTreeItem* a_pItem );
	void              RemoveItem( PGUIMenuTreeItem* a_pItem );
	void              DeleteItem( PGUIMenuTreeItem* a_pItem );

	PGUIMenuTreeItem* FindChildItem( TINT a_iIndex );
	PGUIMenuTreeItem* FindChildItem( const TWCHAR* a_wszLabel, TBOOL a_bRecurse );

	TBOOL IsParentOf( PGUIMenuTreeItem* a_pItem );

	// The next sibling that can be selected (a_iDirection > 0) or the previous one (< 0), wrapping around, TNULL if
	// there's no other one
	PGUIMenuTreeItem* ChooseNextItem( TINT a_iDirection );

	// The first child that can be selected
	PGUIMenuTreeItem* ChooseChildItem();

	// Calls a_fnPre and a_fnPost on the descendants (a_fnFilter returns TFALSE to skip a subtree)
	void RecurseTree( t_RecurseCallback a_fnPre, t_RecurseCallback a_fnPost, t_RecurseCallback a_fnFilter, void* a_pUserData );

	// $JPOG: FUNCTION PGUICore.dll+00005e60
	PGUIMenuTreeItemStyle* GetStyle() const { return m_pStyle; }

	// $JPOG: FUNCTION PGUICore.dll+00005e80
	void SetStyle( PGUIMenuTreeItemStyle* a_pStyle ) { m_pStyle = a_pStyle; }

	// $JPOG: FUNCTION PGUICore.dll+00005e70
	PGUIMenuTreeItem* GetParent() const { return m_pParent; }

	// $JPOG: FUNCTION PGUICore.dll+00005e90
	void SetParent( PGUIMenuTreeItem* a_pParent ) { m_pParent = a_pParent; }

	// $JPOG: FUNCTION PGUICore.dll+00005ea0
	const Toshi::TNodeList<PGUIItemData>& GetChildren() const { return m_oChildren; }

	TBOOL HasChildren() const { return m_oChildren.Count() > 0; }

	PGUIMenuTreeItem* GetFirstChild() const { return TSTATICCAST( PGUIMenuTreeItem, m_oChildren.Head().Get() ); }
	PGUIMenuTreeItem* GetLastChild() const { return TSTATICCAST( PGUIMenuTreeItem, m_oChildren.Tail().Get() ); }

protected:
	PGUIMenuTreeItem( const PGUIMenuTreeItem& a_rcOther );

protected:
	PGUIMenuTreeItemStyle*         m_pStyle;    // 0x18
	PGUIMenuTreeItem*              m_pParent;   // 0x1C
	Toshi::TNodeList<PGUIItemData> m_oChildren; // 0x20
};
