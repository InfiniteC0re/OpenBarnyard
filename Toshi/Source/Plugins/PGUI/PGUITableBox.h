#pragma once
#include "PGUIPanel.h"
#include "PGUIRowData.h"

#include <TGUI/TGUIEvents.h>
#include <TGUI/TGUIRectangle.h>
#include <Toshi/TArray.h>

//-----------------------------------------------------------------------------
// Purpose: a table of rows (PGUIRowData) under heading widgets: the first
// "columns" children are the headings (they give the widths of the columns),
// the next ones the "stamp" widgets (one per column) that paint every cell
// of the visible rows (Initialise takes them out of the children). The
// navigation moves the selection (scrolling the table), select or a click
// throws the action event.
//-----------------------------------------------------------------------------
class PGUITableBox : public PGUIPanel
{
public:
	TDECLARE_CLASS( PGUITableBox, PGUIPanel );

	// Thrown by the item change emitter when the selected row changes
	// NOTE: the name is unknown
	struct ItemChangeEvent
	{
		PGUIRowData* pRow; // 0x0
	};

	// Thrown by the action emitter when a row is selected (select action) or a cell is clicked
	// NOTE: the name is unknown
	struct ActionEvent
	{
		PGUIRowData* pRow;    // 0x0
		TINT         iColumn; // 0x4 -1 for the select action
		TINT         iSource; // 0x8 0: a click, 2: the select action
	};

	using ItemChangeEmitter = Toshi::TEmitter<PGUITableBox, ItemChangeEvent>;
	using ActionEmitter     = Toshi::TEmitter<PGUITableBox, ActionEvent>;

	enum State : TUINT
	{
		State_ScrollableBack    = BITFLAG( 6 ),
		State_ScrollableForward = BITFLAG( 7 ),
	};

	// The states of the rows that can be selected (the state of the first column)
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
	};

public:
	PGUITableBox();
	PGUITableBox( const PGUITableBox& a_rcOther );
	~PGUITableBox();

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
	virtual PGUITableBox*                Clone() const override;
	virtual TBOOL                        AcceptsFocus() const override;
	virtual void                         NotifyFocusLost( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual void                         NotifyFocusGained( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRepeat( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnPointerMove( TINT a_iX, TINT a_iY ) override;
	virtual TBOOL                        OnPaint( Toshi::TGUIDisplayContext* a_pDisplayContext ) override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUITableBox
	//-----------------------------------------------------------------------------

	// +0xB4: Gives the stamp widgets back to the children (with empty parameters)
	virtual void Flush();

	// Takes the stamp widgets out of the children
	void Initialise();

	// The table box owns the rows it is given
	void AddItem( PGUIRowData* a_pRow );
	void RemoveItem( PGUIRowData* a_pRow );
	void RemoveAllItems();
	void SelectItem( PGUIRowData* a_pRow );
	void EnsureVisible( PGUIRowData* a_pRow );

	TBOOL IsItemVisible( PGUIRowData* a_pRow );

	// The row and the column at a_iX, a_iY
	TBOOL FindItemAtPoint( TINT a_iX, TINT a_iY, PGUIRowData*& a_rpRow, TINT& a_riColumn );

	void           Set_columns_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_columns_property() const;
	TBOOL          Val_columns_property( const PPropertyValue& a_rcValue );
	void           Set_selectionmode_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_selectionmode_property() const;
	TBOOL          Val_selectionmode_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION PGUICore.dll+0001b330
	Toshi::TNodeList<PGUIRowData>* GetItemData() { return &m_oRows; }

	// $JPOG: FUNCTION PGUICore.dll+0001b340
	PGUIRowData* GetSelectedItem() const { return m_pSelectedRow; }

	// $JPOG: FUNCTION PGUICore.dll+0001b410
	TINT GetColumnCount() const { return m_iNumColumns; }

	// $JPOG: FUNCTION PGUICore.dll+0001b3f0
	SelectionMode GetSelectionMode() const { return m_eSelectionMode; }

	// $JPOG: FUNCTION PGUICore.dll+0001b420
	void SetSelectionMode( SelectionMode a_eSelectionMode ) { m_eSelectionMode = a_eSelectionMode; }

	// $JPOG: FUNCTION PGUICore.dll+0001b400
	TBOOL GetHoverSelectMode() const { return m_bHoverSelectMode; }

	// $JPOG: FUNCTION PGUICore.dll+0001b430
	void SetHoverSelectMode( TBOOL a_bHoverSelectMode ) { m_bHoverSelectMode = a_bHoverSelectMode; }

	// $JPOG: FUNCTION PGUICore.dll+0001b440
	void SetScrollableBackward( TBOOL a_bScrollable ) { SetState( State_ScrollableBack, a_bScrollable ? TUINT( State_ScrollableBack ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+0001b460
	TBOOL IsScrollableBackward() const { return ( m_uiState & State_ScrollableBack ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+0001b470
	void SetScrollableForward( TBOOL a_bScrollable ) { SetState( State_ScrollableForward, a_bScrollable ? TUINT( State_ScrollableForward ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+0001b490
	TBOOL IsScrollableForward() const { return ( m_uiState & State_ScrollableForward ) != 0; }

	ItemChangeEmitter& GetItemChangeEmitter() { return m_oItemChangeEmitter; }
	ActionEmitter&     GetActionEmitter() { return m_oActionEmitter; }

	// $JPOG: FUNCTION PGUICore.dll+0001b4c0
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

protected:
	void SelectItemAtPoint( TINT a_iX, TINT a_iY );

	// Lays out the visible rows from the first one (their rectangles), updates the scrollable states
	void RefreshItemList();

	// Sets the values, the states and the widths of the cells of a row to the stamp widgets, returns the height of the row
	TINT UpdateWidgets( PGUIRowData* a_pRow );

	// The widths of the headings
	void UpdateWidgetWidths();

	TBOOL DoButtonEvent( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions );
	TBOOL Navigate( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iDelta );

	void AddItemImpl( PGUIRowData* a_pRow );
	void RemoveItemImpl( PGUIRowData* a_pRow );
	void SelectItemImpl( PGUIRowData* a_pRow );
	void EnsureVisibleImpl( PGUIRowData* a_pRow );
	void RemoveAllItemsImpl();

	void InsertStampWidgets();
	void RemoveStampWidgets();

	void SetFirstItem( PGUIRowData* a_pRow );

	PGUIRowData* GetRowsEnd() const { return m_oRows.End().Get(); }

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

	static const TCHAR* const ms_csaSelectionModeNameStr[ SelectionMode_NUMOF ];

protected:
	ItemChangeEmitter             m_oItemChangeEmitter; // 0x8C
	ActionEmitter                 m_oActionEmitter;     // 0x98
	TINT                          m_iNumColumns;        // 0xA4
	Toshi::TNodeList<PGUIRowData> m_oRows;              // 0xA8
	Toshi::TGUIWidget**           m_ppStampWidgets;     // 0xB8 one per column
	TINT*                         m_piColumnWidths;     // 0xBC
	PGUIRowData*                  m_pFirstVisibleRow;   // 0xC0
	PGUIRowData*                  m_pLastVisibleRow;    // 0xC4
	PGUIRowData*                  m_pSelectedRow;       // 0xC8
	TINT                          m_iNumVisibleRows;    // 0xCC
	Orientation                   m_eOrientation;       // 0xD0
	TBOOL                         m_bHoverSelectMode;   // 0xD4
	SelectionMode                 m_eSelectionMode;     // 0xD8
	Toshi::TArray<Toshi::TGUIRectangle> m_aRowRects;    // 0xDC the rectangles of the visible rows
};
