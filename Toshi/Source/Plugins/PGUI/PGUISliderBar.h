#pragma once
#include <TGUI/TGUIWidget.h>
#include <TGUI/TGUIEvents.h>

//-----------------------------------------------------------------------------
// Purpose: a slider: a value between a minimum and a maximum shown by the
// position of a knob on a track, between the "preicon" (decrement) and the
// "posticon" (increment) components. The value is changed by the navigation
// actions along the orientation (repeated while held), by clicking the icons
// or the track and by dragging the knob; the value change event is thrown
// for every change.
//
// The root components: background, track, pretrack (the track before the
// knob), posttrack, preicon, posticon, knob, knoblabel, tracklabel, prelabel,
// postlabel and foreground.
//-----------------------------------------------------------------------------
class PGUISliderBar : public Toshi::TGUIWidget
{
public:
	TDECLARE_CLASS( PGUISliderBar, Toshi::TGUIWidget );

	using ValueChangeEmitter = Toshi::TEmitter<PGUISliderBar, Toshi::TGUIValueChangeEvent>;

	enum State : TUINT
	{
		State_Dragging     = BITFLAG( 5 ),
		State_Incrementing = BITFLAG( 6 ),
		State_Decrementing = BITFLAG( 7 ),
		State_CanIncrement = BITFLAG( 8 ),
		State_CanDecrement = BITFLAG( 9 ),
	};

	enum Component : TINT
	{
		Component_Background,
		Component_Track,
		Component_PreTrack,
		Component_PostTrack,
		Component_PreIcon,
		Component_PostIcon,
		Component_Knob,
		Component_KnobLabel,
		Component_TrackLabel,
		Component_PreLabel,
		Component_PostLabel,
		Component_Foreground,
	};

public:
	PGUISliderBar();
	PGUISliderBar( const PGUISliderBar& a_rcOther );
	~PGUISliderBar();

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
	virtual PGUISliderBar*               Clone() const override;
	virtual TBOOL                        AcceptsFocus() const override;
	virtual void                         NotifyFocusLost( Toshi::TGUIWidget* a_pNewFocus, Toshi::TGUIWidget* a_pOldFocus ) override;
	virtual TBOOL                        OnButtonPress( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRepeat( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnButtonRelease( Toshi::TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                        OnPointerMove( TINT a_iX, TINT a_iY ) override;
	virtual void                         Pack() override;
	virtual void                         Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// PGUISliderBar
	//-----------------------------------------------------------------------------
	void SetStepValue( TFLOAT a_fStep );
	void SetMaximumValue( TFLOAT a_fMaximum );
	void SetMinimumValue( TFLOAT a_fMinimum );
	// Clamped to the range
	void SetCurrentValue( TFLOAT a_fValue );
	void SetRange( TFLOAT a_fMinimum, TFLOAT a_fMaximum );

	// Ignored if a_iOrientation isn't an Orientation
	void SetOrientation( TINT a_iOrientation );

	// $JPOG: FUNCTION PGUICore.dll+00007cf0
	void SetOrientation( Orientation a_eOrientation ) { m_eOrientation = a_eOrientation; }

	// $JPOG: FUNCTION PGUICore.dll+00007bb0
	TFLOAT GetStepValue() const { return m_fStep; }

	// $JPOG: FUNCTION PGUICore.dll+00007bc0
	TFLOAT GetMaximumValue() const { return m_fMaximum; }

	// $JPOG: FUNCTION PGUICore.dll+00007bd0
	TFLOAT GetMinimumValue() const { return m_fMinimum; }

	// $JPOG: FUNCTION PGUICore.dll+00007be0
	TFLOAT GetCurrentValue() const { return m_fValue; }

	// $JPOG: FUNCTION PGUICore.dll+00007bf0
	void SetCanIncrement( TBOOL a_bCanIncrement ) { SetState( State_CanIncrement, a_bCanIncrement ? TUINT( State_CanIncrement ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+00007c10
	TBOOL IsCanIncrement() const { return ( m_uiState & State_CanIncrement ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+00007c20
	void SetCanDecrement( TBOOL a_bCanDecrement ) { SetState( State_CanDecrement, a_bCanDecrement ? TUINT( State_CanDecrement ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+00007c40
	TBOOL IsCanDecrement() const { return ( m_uiState & State_CanDecrement ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+00007c50
	void SetDecrementing( TBOOL a_bDecrementing ) { SetState( State_Decrementing, a_bDecrementing ? TUINT( State_Decrementing ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+00007c70
	TBOOL IsDecrementing() const { return ( m_uiState & State_Decrementing ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+00007c90
	void SetIncrementing( TBOOL a_bIncrementing ) { SetState( State_Incrementing, a_bIncrementing ? TUINT( State_Incrementing ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+00007cb0
	TBOOL IsIncrementing() const { return ( m_uiState & State_Incrementing ) != 0; }

	// $JPOG: FUNCTION PGUICore.dll+00007cc0
	void SetDragging( TBOOL a_bDragging ) { SetState( State_Dragging, a_bDragging ? TUINT( State_Dragging ) : 0u ); }

	// $JPOG: FUNCTION PGUICore.dll+00007ce0
	TBOOL IsDragging() const { return ( m_uiState & State_Dragging ) != 0; }

	ValueChangeEmitter& GetValueChangeEmitter() { return m_oValueChangeEmitter; }

	// $JPOG: FUNCTION PGUICore.dll+00007d10
	static const Toshi::TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

	void           Set_valuemin_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_valuemin_property() const;
	TBOOL          Val_valuemin_property( const PPropertyValue& a_rcValue );
	void           Set_valuemax_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_valuemax_property() const;
	TBOOL          Val_valuemax_property( const PPropertyValue& a_rcValue );
	void           Set_valuecurr_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_valuecurr_property() const;
	TBOOL          Val_valuecurr_property( const PPropertyValue& a_rcValue );
	void           Set_valuestep_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_valuestep_property() const;
	TBOOL          Val_valuestep_property( const PPropertyValue& a_rcValue );
	void           Set_orientation_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_orientation_property() const;
	TBOOL          Val_orientation_property( const PPropertyValue& a_rcValue );

protected:
	// Updates the can increment/decrement states and moves the knob (and the ends of the pre and post tracks) to the value
	void UpdateKnob();

	void ThrowValueChange();

	// Sets the rectangles of the components: the icons at the ends of the track, the knob at the value
	void PackComponents();

	// The preferred size: the track and the icons along the orientation, the background, foreground and knob (three
	// knobs at least along the orientation)
	void LayoutComponents( TINT& a_rWidth, TINT& a_rHeight );


public:
	static PropertySet*           ms_oPropertySet;
	static PropertySet*           ms_oPropertySetOrdered;
	static Toshi::TGUIWidgetInfo* ms_pWidgetInfo;


protected:
	ValueChangeEmitter m_oValueChangeEmitter; // 0x8C
	TINT               m_iDragOffsetX;        // 0x98 the pointer relative to the centre of the knob when the drag started
	TINT               m_iDragOffsetY;        // 0x9C
	TFLOAT             m_fMinimum;            // 0xA0
	TFLOAT             m_fMaximum;            // 0xA4
	TFLOAT             m_fValue;              // 0xA8
	TFLOAT             m_fStep;               // 0xAC
	Orientation        m_eOrientation;        // 0xB0
	TINT               m_iRepeatDoodad;       // 0xB4 the doodad that increments/decrements the value
};
