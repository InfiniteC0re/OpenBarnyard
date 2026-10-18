#pragma once
#include "PGUIItemData.h"

#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIEvents.h>

class PGUIListItem;
class PGUISliderBar;

// Thrown by the item change emitter of the list boxes when the selected item changes
struct PGUIItemChangeEvent
{
	PGUIItemData* pItem; // 0x0 the selected item (TNULL for none)
};

//-----------------------------------------------------------------------------
// Purpose: a list of items (PGUIItemData) shown by list item widgets (made
// from the list item style) in the "bucket" component: as many as fit, in a
// row, a column or a grid, from the first visible item. The navigation moves
// the selection (scrolling the list), select or a click "hits" the selected
// item (action event, if the selection mode allows the state of the item);
// "scrollback"/"scrollforward" scroll by one item.
//
// NOTE: the scroll bar (scroll bar style) is created but never added to the
// list box (as in the original), it leaks and GetScrollBar returns the first
// child.
//-----------------------------------------------------------------------------
class PGUIListBox : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUIListBox, Toshi::TGUIWidget );

	using ItemChangeEmitter = Toshi::TEmitter<PGUIListBox, PGUIItemChangeEvent>;
	using ActionEmitter     = Toshi::TEmitter<PGUIListBox, Toshi::TGUIActionEvent>;

	enum State : TUINT
	{
		State_ScrollableBack    = BITFLAG( 5 ),
		State_ScrollableForward = BITFLAG( 6 ),
	};

	enum ArrangeMode : TINT
	{
		ArrangeMode_Horizontal = 0,
		ArrangeMode_Vertical   = 1,
		ArrangeMode_Grid       = 2,
		ArrangeMode_NUMOF      = 3
	};

	// The states of the items that can be hit
	enum SelectionMode : TINT
	{
		SelectionMode_None     = 0,
		SelectionMode_Enabled  = BITFLAG( 0 ),
		SelectionMode_Disabled = BITFLAG( 1 ),
		SelectionMode_Any      = SelectionMode_Enabled | SelectionMode_Disabled,
		SelectionMode_NUMOF    = 4
	};

	enum Component : TINT
	{
		Component_Background,
		Component_Bucket,
		Component_ScrollBack,
		Component_ScrollForward,
		Component_Effect,
	};

public:
	PGUIListBox();
	PGUIListBox( const PGUIListBox& a_rcOther );
	~PGUIListBox();

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
	virtual PGUIListBox*                 Clone() const override;
	virtual TBOOL                        ManagesChildren() const override;
	virtual TBOOL                        AcceptsFocus() const override;
	virtual Toshi::TGUIWidget*           AllowsFocus( Toshi::TGUIWidget* a_pWidget ) override;
	virtual void                         NotifyFocusLost( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual void                         NotifyFocusGained( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRepeat( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnPointerMove( TINT a_iX, TINT a_iY ) override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;
	virtual TBOOL                        DeserialiseChildren( const PProperties* a_pProperties ) override;
	virtual TBOOL                        SerialiseChildren( PProperties* a_pProperties ) override;

	//-----------------------------------------------------------------------------
	// PGUIListBox
	//-----------------------------------------------------------------------------

	// The list box owns the items it is given
	void AddItem( PGUIItemData* a_pItem );
	void RemoveItem( PGUIItemData* a_pItem );
	void RemoveAllItems();
	void SelectItem( PGUIItemData* a_pItem );
	void EnsureVisible( PGUIItemData* a_pItem );

	TBOOL IsItemVisible( PGUIItemData* a_pItem );

	// The size for a_iNumItems items in the orientation (the blank item gives the size of the items, or the largest
	// of the first items)
	void SizeToFit( TINT a_iNumItems );

	// The size for a_iNumRows x a_iNumColumns items
	void SizeToFit( TINT a_iNumRows, TINT a_iNumColumns );

	void SetScrollBarStyle( const Toshi::TPString8& a_rcName );
	void SetScrollBarStyle( Toshi::TGUIStyle* a_pStyle );
	void SetListItemStyle( const Toshi::TPString8& a_rcName );
	void SetListItemStyle( Toshi::TGUIStyle* a_pStyle );

	// Ignored if a_iOrientation isn't an ArrangeMode
	void SetOrientation( TINT a_iOrientation );

	// Ignored if a_iPosition isn't a Placement
	void SetScrollBarPosition( TINT a_iPosition );

	PGUISliderBar* GetScrollBar();
	PGUIListItem*  GetFirstItemWidget();

	void           Set_orientation_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_orientation_property() const;
	TBOOL          Val_orientation_property( const PPropertyValue& a_rcValue );
	void           Set_scrollbarpos_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_scrollbarpos_property() const;
	TBOOL          Val_scrollbarpos_property( const PPropertyValue& a_rcValue );
	void           Set_selectionmode_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_selectionmode_property() const;
	TBOOL          Val_selectionmode_property( const PPropertyValue& a_rcValue );
	void           Set_scrollbarstyle_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_scrollbarstyle_property() const;
	TBOOL          Val_scrollbarstyle_property( const PPropertyValue& a_rcValue );
	void           Set_listitemstyle_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_listitemstyle_property() const;
	TBOOL          Val_listitemstyle_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+0000c650
	Toshi::TNodeList<PGUIItemData>* GetItemData() { return &m_oItems; }

	// $JPOG: FUNCTION PGUICore.dll+0000c660
	PGUIItemData* GetSelectedItem() const { return m_pSelectedItem; }

	// $JPOG: FUNCTION PGUICore.dll+0000c700
	SelectionMode GetSelectionMode() const { return m_eSelectionMode; }

	// $JPOG: FUNCTION PGUICore.dll+0000c760
	void SetSelectionMode( SelectionMode a_eSelectionMode ) { m_eSelectionMode = a_eSelectionMode; }

	// $JPOG: FUNCTION PGUICore.dll+0000c710
	TBOOL GetHoverSelectMode() const { return m_bHoverSelectMode; }

	// $JPOG: FUNCTION PGUICore.dll+0000c770
	void SetHoverSelectMode( TBOOL a_bHoverSelectMode ) { m_bHoverSelectMode = a_bHoverSelectMode; }

	// $JPOG: FUNCTION PGUICore.dll+0000c720
	Toshi::TGUIStyle* GetScrollBarStyle() const { return m_pScrollBarStyle; }

	// $JPOG: FUNCTION PGUICore.dll+0000c730
	Toshi::TGUIStyle* GetListItemStyle() const { return m_pListItemStyle; }

	// $JPOG: FUNCTION PGUICore.dll+0000c740
	PGUIItemData* GetBlankItem() const { return m_pBlankItem; }

	// $JPOG: FUNCTION PGUICore.dll+0000c750
	void SetBlankItem( PGUIItemData* a_pItem ) { m_pBlankItem = a_pItem; }

	// $JPOG: FUNCTION PGUICore.dll+0000c780
	void SetScrollBarPosition( Placement a_ePosition ) { m_eScrollBarPosition = a_ePosition; }

	// $JPOG: FUNCTION PGUICore.dll+0000c790
	void SetOrientation( ArrangeMode a_eArrangeMode ) { m_eArrangeMode = a_eArrangeMode; }

	// $JPOG: FUNCTION PGUICore.dll+0000c7a0
	void SetScrollableBackward( TBOOL a_bScrollable ) { SetState( State_ScrollableBack, a_bScrollable ? TUINT( State_ScrollableBack ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+0000c7c0
	TBOOL IsScrollableBackward() const { return ( m_uiState & State_ScrollableBack ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+0000c7d0
	void SetScrollableForward( TBOOL a_bScrollable ) { SetState( State_ScrollableForward, a_bScrollable ? TUINT( State_ScrollableForward ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+0000c7f0
	TBOOL IsScrollableForward() const { return ( m_uiState & State_ScrollableForward ) != 0; }

	ItemChangeEmitter& GetItemChangeEmitter() { return m_oItemChangeEmitter; }
	ActionEmitter&     GetActionEmitter() { return m_oActionEmitter; }

	// $JPOG: FUNCTION PGUICore.dll+0000c820
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

protected:
	// Selects the item of the list item widget at a_iX, a_iY
	void SelectItemAtPoint( TINT a_iX, TINT a_iY );

	// Creates the list item widgets that fit in the bucket and places them
	void GenerateItemList();

	// Shows the items from the first visible one in the list item widgets, updates the scrollable states
	void RefreshItemList();

	void DeleteItemList();

	// Throws the action event if the selection mode allows the state of the selected item
	void HitSelectedItem();

	// The navigation along the orientation (the grid moves by rows and columns), the mouse wheel
	TBOOL DoButtonEvent( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions );

	// Moves the selection by a_iDelta items (scrolling the list at its ends)
	TBOOL Navigate( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iDelta );

	Toshi::TGUIWidget* MakeNewItem();
	PGUIListItem*      FindItemWidget( PGUIItemData* a_pItem );
	PGUIItemData*      FindWidgetItem( Toshi::TGUIWidget* a_pWidget );

	void AddItemImpl( PGUIItemData* a_pItem );
	void RemoveItemImpl( PGUIItemData* a_pItem );
	void SelectItemImpl( PGUIItemData* a_pItem );
	void EnsureVisibleImpl( PGUIItemData* a_pItem );
	void RemoveAllItemsImpl();

	void SetFirstItem( PGUIItemData* a_pItem );

	PGUIItemData* GetItemsEnd() const { return m_oItems.End().Get(); }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

	static const TCHAR* const ms_csaArrangeModeNameStr[ ArrangeMode_NUMOF ];
	static const TCHAR* const ms_csaSelectionModeNameStr[ SelectionMode_NUMOF ];

protected:
	ItemChangeEmitter              m_oItemChangeEmitter;   // 0x8C
	ActionEmitter                  m_oActionEmitter;       // 0x98
	ArrangeMode                    m_eArrangeMode;         // 0xA4
	Placement                      m_eScrollBarPosition;   // 0xA8
	Toshi::TGUIStyle*              m_pScrollBarStyle;      // 0xAC
	Toshi::TGUIStyle*              m_pListItemStyle;       // 0xB0
	PGUIItemData*                  m_pBlankItem;           // 0xB4 shown when there's no item
	PGUIItemData*                  m_pFirstVisibleItem;    // 0xB8
	PGUIItemData*                  m_pSelectedItem;        // 0xBC
	PGUIListItem*                  m_pSelectedItemWidget;  // 0xC0
	Toshi::TNodeList<PGUIItemData> m_oItems;               // 0xC4
	TINT                           m_iNumItemWidgets;      // 0xD4
	TINT                           m_iNumVisibleItems;     // 0xD8
	TINT                           m_iNumRows;             // 0xDC
	TINT                           m_iNumColumns;          // 0xE0
	TINT                           m_iFirstItemWidget;     // 0xE4 the index of the first list item widget in the children
	TBOOL                          m_bHasScrollBar;        // 0xE8
	TBOOL                          m_bHoverSelectMode;     // 0xE9 the item under the pointer gets selected
	SelectionMode                  m_eSelectionMode;       // 0xEC
	TINT                           m_iItemWidth;           // 0xF0
	TINT                           m_iItemHeight;          // 0xF4
};
