#pragma once
#include <Toshi/TDList.h>
#include <Toshi/TEvent.h>
#include <Toshi/TFreeList.h>

TOSHI_NAMESPACE_START

class TGUIWidget;

//-----------------------------------------------------------------------------
// Purpose: an action of a widget: a doodad (button) of the input devices that
// fires the action when the widget gets it (TGUIWidget::OnButtonPress), with a
// label (shown by the input context, TGUIInputContext::FireWidgetAction) and a
// command. The emitter throws the widget.
//
// NOTE: the label is a TLString in JPOG, a wide string pointer here (Nicktoons
// only checks that it's set).
//-----------------------------------------------------------------------------
class TGUIWidgetAction : public TDList<TGUIWidgetAction>::TNode
{
public:
	TDECLARE_FREELIST_ALLOCATOR( TGUIWidgetAction );

	using Emitter = TEmitter<TGUIWidgetAction, TGUIWidget*>;

public:
	TGUIWidgetAction( TINT a_iDoodadID, const TWCHAR* a_wszLabel, TINT a_iCommandID, TBOOL a_bEnabled );
	TGUIWidgetAction( const TGUIWidgetAction& a_rcOther );
	~TGUIWidgetAction();

	TGUIWidgetAction& operator=( const TGUIWidgetAction& a_rcOther );

	// Throws the action emitter for a_pWidget
	void Throw( TGUIWidget* a_pWidget );

	// $JPOG: FUNCTION TGuiInterface.dll+00007a00
	void SetLabel( const TWCHAR* a_wszLabel ) { m_wszLabel = a_wszLabel; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007a50
	const TWCHAR* GetLabel() const { return m_wszLabel; }

	// $JPOG: FUNCTION TGuiInterface.dll+000079d0
	void SetEnabled( TBOOL a_bEnabled ) { m_bEnabled = a_bEnabled; }

	// $JPOG: FUNCTION TGuiInterface.dll+000079c0
	TBOOL IsEnabled() const { return m_bEnabled; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007a70
	TINT GetDoodadID() const { return m_iDoodadID; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007a40
	TINT GetCommandID() const { return m_iCommandID; }

	Emitter& GetEmitter() { return m_oEmitter; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007ad0
	static TFreeList& GetFreeList() { return ms_oFreeList; }

private:
	Emitter       m_oEmitter;   // 0x08
	TINT          m_iDoodadID;  // 0x14
	const TWCHAR* m_wszLabel;   // 0x18
	TINT          m_iCommandID; // 0x1C
	TBOOL         m_bEnabled;   // 0x20
};

TOSHI_NAMESPACE_END
