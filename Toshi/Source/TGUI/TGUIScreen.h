#pragma once
#include "TGUIWidget.h"
#include "TGUIInputManager.h"

#include <Toshi/TArray.h>

TOSHI_NAMESPACE_START

class TGUIWindow;
class TGUIDisplayContext;
class TGUIAudioContext;
class TGUIInputContext;
class TGUITextureSection;

//-----------------------------------------------------------------------------
// Purpose: the root widget of a GUI: paints its widgets with its display
// context, keeps the focused widget (moved with the navigation actions and
// the focus map of the windows), the focus grabs, the pointer (the widget
// under it gets the pointer events) and dispatches the input events to the
// input managers (TGUIInputManager, the screen first). The emitters throw the
// focus changes, the rollovers (the widget under the pointer) and the paint.
//
// NOTE: follows Nicktoons, the TGUIInterface of JPOG is the TGUIManager here
// (the sounds come from its sound factory, the actions from its action map).
//-----------------------------------------------------------------------------
class TGUIScreen
    : public TGUIWidget
    , public TGUIInputManager
    , public TNodeList<TGUIScreen>::TNode
{
public:
	TDECLARE_CLASS( TGUIScreen, TGUIWidget );

	using FocusChangeEmitter = TEmitter<TGUIScreen, TGUIFocusChangeEvent>;
	using RolloverEmitter    = TEmitter<TGUIScreen, TGUIRolloverEvent>;
	using PaintEmitter       = TEmitter<TGUIScreen, TGUIScreenPaintEvent>;

	static constexpr TINT MAX_INPUT_MANAGERS = 8;

	// The input actions of the navigation
	enum Action : TUINT
	{
		Action_North        = BITFLAG( 9 ),
		Action_East         = BITFLAG( 10 ),
		Action_South        = BITFLAG( 11 ),
		Action_West         = BITFLAG( 12 ),
		Action_Navigate     = Action_North | Action_East | Action_South | Action_West,
		Action_PointerMove  = BITFLAG( 13 ),
		Action_PointerClick = BITFLAG( 16 ),
	};

public:
	TGUIScreen();
	TGUIScreen( const TGUIScreen& a_rcOther );
	~TGUIScreen();

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
	virtual TGUIScreen*           Clone() const override;
	virtual TGUIWidget*           AllowsFocus( TGUIWidget* a_pWidget ) override;
	virtual void                  NotifyDescendantAddition( TGUIWidget* a_pWidget ) override;
	virtual void                  NotifyDescendantRemoval( TGUIWidget* a_pWidget ) override;
	virtual void                  NotifyDescendantRemoved( TGUIWidget* a_pWidget, TGUIWidget* a_pOldParent ) override;
	virtual TBOOL                 OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                 OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual TBOOL                 OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions ) override;
	virtual void                  Pack() override;
	virtual void                  Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) override;

	//-----------------------------------------------------------------------------
	// TGUIInputManager
	//-----------------------------------------------------------------------------
	virtual TBOOL DoButtonPress( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoButtonRepeat( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoButtonRelease( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoAxisMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual TBOOL DoPointerMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions ) override;
	virtual void  DoRolloverUpdate() override;

	//-----------------------------------------------------------------------------
	// TGUIScreen
	//-----------------------------------------------------------------------------

	// Gives an input event to the input manager at the top of the stack (the pointer moves update the rollover)
	TBOOL NotifyInputEvent( TInputInterface::InputEvent* a_pEvent );

	// Paints the screen (from a_pFrom if the painting was stopped)
	void Paint( TGUIWidget* a_pFrom );

	// $JPOG: FUNCTION TGuiInterface.dll+0000f7f0
	void ContinuePaint()
	{
		if ( m_pPaintContinuation )
			Paint( m_pPaintContinuation );
	}

	// $JPOG: FUNCTION TGuiInterface.dll+0000f810
	void RestartPaint() { Paint( TNULL ); }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f7e0
	TBOOL IsPaintContinued() const { return m_pPaintContinuation != TNULL; }

	// Focuses a_pWidget (or what it allows to focus, the default focus of a window if !a_bExact)
	void ChangeWidgetFocus( TGUIWidget* a_pWidget, TBOOL a_bExact );

	// Moves the focus to the neighbour of the focused widget in a direction of the focused window
	TBOOL MoveFocus( TGUIWidget::NeighbourDirection a_eDirection );

	// The focus can't leave a_pWidget (and its descendants) until it releases it
	void GrabFocus( TGUIWidget* a_pWidget );
	void ReleaseFocus( TGUIWidget* a_pWidget );

	// The pointer events go to a_pWidget until it releases it
	// $JPOG: FUNCTION TGuiInterface.dll+000152e0
	void GrabPointer( TGUIWidget* a_pWidget ) { m_pPointerGrabWidget = a_pWidget; }
	void ReleasePointer( TGUIWidget* a_pWidget );

	void NotifyDescendantCantFocus( TGUIWidget* a_pWidget );
	void NotifyDescendantCanFocus( TGUIWidget* a_pWidget );

	// $JPOG: FUNCTION TGuiInterface.dll+000152a0
	void NotifyFocusGained() {}

	// $JPOG: FUNCTION TGuiInterface.dll+000152b0
	void NotifyFocusLost() {}

	TBOOL SetPointerPosition( TINT a_iX, TINT a_iY );
	TBOOL UpdatePointerPosition( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions );
	TBOOL RefreshPointerPosition( TGUIWidget* a_pWidget, TINT a_iX, TINT a_iY );
	void  ApplyPointerAccel( TINT& a_rX, TINT& a_rY );

	// Shows the pointer (centered) or hides it
	TBOOL SetPointerVisible( TBOOL a_bVisible );

	void AllowRolloverUpdate( TBOOL a_bAllow );
	void InvalidateWidgetUnderPointer();

	TGUIWidget* GetWidgetUnderPointer();

	void SetInputContext( TGUIInputContext* a_pInputContext );

	// $JPOG: FUNCTION TGuiInterface.dll+0000f8c0
	void PushInputManager( TGUIInputManager* a_pInputManager )
	{
		if ( ++m_iInputManagerTop < MAX_INPUT_MANAGERS )
			m_apInputManagers[ m_iInputManagerTop ] = a_pInputManager;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+0000f890
	void PopInputManager( TGUIInputManager* a_pInputManager )
	{
		if ( --m_iInputManagerTop < 0 )
			m_iInputManagerTop = 0;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+0000f8f0
	TGUIInputManager* GetInputManager() const { return m_apInputManagers[ m_iInputManagerTop ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+000152c0
	void SetPointerAccel( TINT a_iThreshold1, TINT a_iThreshold2, TINT a_iMode )
	{
		// NOTE: the first threshold is set to both (as in the original)
		m_iPointerAccelThreshold1 = a_iThreshold1;
		m_iPointerAccelThreshold2 = a_iThreshold1;
		m_iPointerAccelMode       = a_iMode;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000152f0
	void SetPointerImage( TGUITextureSection* a_pImage, TINT a_iHotSpotX, TINT a_iHotSpotY )
	{
		m_pPointerImage    = a_pImage;
		m_iPointerHotSpotX = a_iHotSpotX;
		m_iPointerHotSpotY = a_iHotSpotY;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+0000f7d0
	void SetPointerSpeed( TFLOAT a_fSpeed ) { m_fPointerSpeed = a_fSpeed; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f7a0
	TINT GetPointerPositionX() const { return m_iPointerX; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f790
	TINT GetPointerPositionY() const { return m_iPointerY; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f7b0
	void GetPointerPosition( TINT& a_rX, TINT& a_rY ) const
	{
		a_rX = m_iPointerX;
		a_rY = m_iPointerY;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+0000f900
	TBOOL IsPointerVisible() const { return m_bPointerVisible; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f820
	TGUIWidget* GetFocusGrabWidget() const { return m_aFocusGrabs[ m_aFocusGrabs.Size() - 1 ]; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f870
	TGUIWidget* GetFocusedWidget() const { return m_pFocusedWidget; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f880
	TGUIWindow* GetFocusedWindow() const { return m_pFocusedWindow; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f920
	TGUIInputContext* GetInputContext() const { return m_pInputContext; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f930
	TGUIAudioContext* GetAudioContext() const { return m_pAudioContext; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f940
	void SetAudioContext( TGUIAudioContext* a_pAudioContext ) { m_pAudioContext = a_pAudioContext; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f950
	TGUIDisplayContext* GetDisplayContext() const { return m_pDisplayContext; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f960
	void SetDisplayContext( TGUIDisplayContext* a_pDisplayContext ) { m_pDisplayContext = a_pDisplayContext; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f780
	FocusChangeEmitter& GetFocusChangeEmitter() { return m_oFocusChangeEmitter; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f770
	RolloverEmitter& GetRolloverEmitter() { return m_oRolloverEmitter; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f760
	PaintEmitter& GetScreenPaintEmitter() { return m_oScreenPaintEmitter; }

	// $JPOG: FUNCTION TGuiInterface.dll+0000f9a0
	static const TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

private:
	// Callbacks of WalkUpDownTree
	static void __stdcall pointer_out_callback( TGUIWidget* a_pWidget );
	static void __stdcall pointer_in_callback( TGUIWidget* a_pWidget );

	// Removes the focus grabs of a_pWidget and of its descendants
	// NOTE: inlined in NotifyDescendantCantFocus and NotifyDescendantRemoval
	void RemoveFocusGrabs( TGUIWidget* a_pWidget );

	// The widget is a_pWidget or one of its descendants
	static TBOOL IsWidgetOrDescendant( TGUIWidget* a_pWidget, TGUIWidget* a_pAncestor ) { return a_pWidget == a_pAncestor || a_pWidget->IsDescendantOf( a_pAncestor ); }

public:
	static PropertySet*    ms_oPropertySet;
	static PropertySet*    ms_oPropertySetOrdered;
	static TGUIWidgetInfo* ms_pWidgetInfo;

private:
	FocusChangeEmitter  m_oFocusChangeEmitter;                   // 0x0A8
	RolloverEmitter     m_oRolloverEmitter;                      // 0x0B4
	PaintEmitter        m_oScreenPaintEmitter;                   // 0x0C0
	TGUIDisplayContext* m_pDisplayContext;                       // 0x0CC
	TGUIAudioContext*   m_pAudioContext;                         // 0x0D0
	TGUIInputContext*   m_pInputContext;                         // 0x0D4
	TINT                m_iInputManagerTop;                      // 0x0D8
	TGUIInputManager*   m_apInputManagers[ MAX_INPUT_MANAGERS ]; // 0x0DC
	TGUIWindow*         m_pFocusedWindow;                        // 0x0FC
	TGUIWidget*         m_pFocusedWidget;                        // 0x100
	TGUIWidget*         m_pPointerGrabWidget;                    // 0x104
	TArray<TGUIWidget*> m_aFocusGrabs;                           // 0x108 the last one is the current grab (TNULL for none)
	TGUIWidget*         m_pWidgetUnderPointer;                   // 0x118
	TBOOL               m_bRolloverUpdated;                      // 0x11C the widget under the pointer is up to date
	TBOOL               m_bAllowRolloverUpdate;                  // 0x120
	TINT                m_iPointerX;                             // 0x124
	TINT                m_iPointerY;                             // 0x128
	TFLOAT              m_fPointerSpeed;                         // 0x12C
	TINT16              m_iNavigateSound;                        // 0x130
	TINT16              m_iNavigateFailSound;                    // 0x132
	TGUITextureSection* m_pPointerImage;                         // 0x134
	TINT                m_iPointerHotSpotX;                      // 0x138
	TINT                m_iPointerHotSpotY;                      // 0x13C
	TBOOL               m_bPointerVisible;                       // 0x140
	TGUIWidget*         m_pPaintContinuation;                    // 0x144 the widget the painting stopped at
	TINT                m_iUnknown;                              // 0x148
	TINT                m_iPointerAccelThreshold1;               // 0x14C
	TINT                m_iPointerAccelThreshold2;               // 0x150 unused
	TINT                m_iPointerAccelMode;                     // 0x154 0: none, 1: x2, 2: x4
};

TOSHI_NAMESPACE_END
