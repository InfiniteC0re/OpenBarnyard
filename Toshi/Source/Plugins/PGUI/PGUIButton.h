#pragma once
#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIEvents.h>

//-----------------------------------------------------------------------------
// Purpose: base of the buttons. Pressed by the "select" action (or a click
// when the pointer is over it): it is pressed while the action is held (the
// pointer and the focus are grabbed) and does its action (DoButtonAction) on
// release; plays the "PGUIButton" "down"/"up" sounds.
//-----------------------------------------------------------------------------
class PGUIButton : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUIButton, Toshi::TGUIWidget );

	using ActionEmitter = Toshi::TEmitter<PGUIButton, Toshi::TGUIActionEvent>;

	enum State : TUINT
	{
		State_Pressed = BITFLAG( 5 ),
	};

public:
	PGUIButton();
	PGUIButton( const PGUIButton& a_rcOther );
	~PGUIButton();

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
	virtual TBOOL                        AcceptsFocus() const override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRelease( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;

	// DOES_NOTHING
	virtual void Pack() override {}

	//-----------------------------------------------------------------------------
	// PGUIButton
	//-----------------------------------------------------------------------------

	// +0xB4: Throws the action event (and toggles the toggle buttons)
	virtual void DoButtonAction() = 0;

	// $JPOG: FUNCTION PGUICore.dll+00004400
	void SetPressed( TBOOL a_bPressed ) { SetState( State_Pressed, a_bPressed ? TUINT( State_Pressed ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+00004420
	TBOOL IsPressed() const { return ( m_uiState & State_Pressed ) != 0; }

	ActionEmitter& GetActionEmitter() { return m_oActionEmitter; }

	// $JPOG: FUNCTION PGUICore.dll+00004430
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

	// The input actions that press the buttons: the select action (0x1) and the pointer click (0x4000)
	static constexpr TUINT PRESS_ACTIONS = 0x4001;

public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;

protected:
	ActionEmitter m_oActionEmitter; // 0x8C
};
