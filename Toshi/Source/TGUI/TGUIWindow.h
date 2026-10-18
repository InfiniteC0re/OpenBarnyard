#pragma once
#include "TGUIWidget.h"

#include <Toshi/TArray.h>

TOSHI_NAMESPACE_START

//-----------------------------------------------------------------------------
// Purpose: a widget that keeps the focus navigation of its descendants: the
// focus map (the neighbour of a widget in each direction, "focusmap" block),
// the default focus ("default_focus") and the last focused widget, and reads
// the state slaves of its descendants ("stateslaves" block).
//
// NOTE: abstract (the PGUI dialogs derive from it). Nicktoons inlines
// SetFocusNeighbour in DeserialiseFocusMap (the entries aren't replaced).
//-----------------------------------------------------------------------------
class TGUIWindow : public TGUIWidget
{
public:
	TDECLARE_CLASS( TGUIWindow, TGUIWidget );

	// The neighbour of a widget in a direction (sorted by widget then direction)
	struct FocusNeighbour
	{
		TGUIWidget*        pWidget;    // 0x0
		NeighbourDirection eDirection; // 0x4
		TGUIWidget*        pNeighbour; // 0x8
	};

public:
	TGUIWindow();
	TGUIWindow( const TGUIWindow& a_rcOther );
	~TGUIWindow();

	//-----------------------------------------------------------------------------
	// TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual TBOOL        Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL        Serialise( PProperties* a_pProperties ) override;
	virtual PropertySet* GetPropertyInfoSet() const override;
	virtual PropertySet* GetPropertyInfoOrderedSet() const override;

	//-----------------------------------------------------------------------------
	// TGUIWidget
	//-----------------------------------------------------------------------------
	virtual const TGUIWidgetInfo* GetWidgetInfo() const override;
	virtual void                  ResetParameters() override;
	virtual TBOOL                 AcceptsFocus() const override;
	virtual void                  NotifyDescendantRemoval( TGUIWidget* a_pWidget ) override;
	virtual void                  NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus ) override;
	virtual void                  NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus ) override;
	virtual void                  NotifyParentChange( TGUIWidget* a_pParent ) override;

	//-----------------------------------------------------------------------------
	// TGUIWindow
	//-----------------------------------------------------------------------------

	// +0xB4: The widget that gets the focus when the window gets it: the last focused widget, the default focus or a
	// neighbour of it, the window if there's none
	virtual TGUIWidget* GetAutoFocus();

	void        SetFocusNeighbour( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection, TGUIWidget* a_pNeighbour );
	TGUIWidget* GetFocusNeighbour( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection );

	// The first neighbour (following the neighbours) that can get the focus
	TGUIWidget* FindFocusNeighbour( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection );

	TBOOL DeserialiseFocusMap( const PProperties* a_pProperties );
	TBOOL DeserialiseStateSlaves( const PProperties* a_pProperties );
	TBOOL SerialiseFocusMap( PProperties* a_pProperties );

	void           Set_default_focus_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_default_focus_property() const;
	TBOOL          Val_default_focus_property( const PPropertyValue& a_rcValue );

	// $JPOG: FUNCTION TGuiInterface.dll+00024360
	void SetDefaultFocus( TGUIWidget* a_pWidget )
	{
		m_pDefaultFocus = a_pWidget;
		m_pLastFocus    = a_pWidget;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000153c0
	TGUIWidget* GetDefaultFocus() const { return m_pDefaultFocus; }

	// NOTE: added (used by the Barnyard route that makes the missing focus maps, see AGUISystem::BuildTGUIFocusMap)
	TINT GetFocusMapSize() const { return m_aFocusMap.Size(); }

	// $JPOG: FUNCTION TGuiInterface.dll+000153e0
	static const TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

private:
	// Index of the first entry of the focus map with the widget and the direction, -1 if there's none
	TINT FindFocusNeighbourEntry( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection ) const;

	// Index to insert an entry at (after the entries with the same widget and direction)
	TINT GetFocusNeighbourInsertIndex( TGUIWidget* a_pWidget, NeighbourDirection a_eDirection ) const;

	// RecurseTree callbacks
	static TBOOL __stdcall clone_stateslaves_callback( TGUIWidget* a_pWidget, void* a_pUserData );
	static TBOOL __stdcall serialise_stateslaves_callback( TGUIWidget* a_pWidget, void* a_pUserData );

public:
	static PropertySet*    ms_oPropertySet;
	static PropertySet*    ms_oPropertySetOrdered;
	static TGUIWidgetInfo* ms_pWidgetInfo;

	static TPString8 str_focusmap;
	static TPString8 str_neighbour;
	static TPString8 str_default_focus;
	static TPString8 str_stateslaves;

private:
	TGUIWidget*            m_pDefaultFocus; // 0x98
	TGUIWidget*            m_pLastFocus;    // 0x9C
	TArray<FocusNeighbour> m_aFocusMap;     // 0xA0
};

TOSHI_NAMESPACE_END
