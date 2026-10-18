#pragma once
#include "TGUIDeserialisable.h"
#include "TGUIMemory.h"
#include "TGUIRectangle.h"
#include "TGUIEvents.h"
#include "TGUIWidgetAction.h"

#include <Toshi/TNodeList.h>
#include <Toshi/TDList.h>
#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>
#include <Input/TInputInterface.h>

TOSHI_NAMESPACE_START

class TGUIStyle;
class TGUIStyleParameterInfo;
class TGUIWidgetInfo;
class TGUIComponentCache;
class TGUILayoutManager;
class TGUITextureSection;
class TGUIDisplayContext;
class TGUIWindow;
class TGUIScreen;

//-----------------------------------------------------------------------------
// Purpose: base of the GUI widgets: a rectangle in its parent painted with a
// style (TGUIStyle) and the parameters of the style, a state (the state
// variables of the class, "Enabled", "Focused"...), children (optionally
// arranged by a layout manager), state slaves (widgets whose state follows
// some bits of this one) and actions (input doodads that fire an emitter).
//
// The widgets are created from the "widget" blocks of the dialogs (see
// TGUIDialogBlock) with the class of their "type" property.
//
// NOTE: follows Nicktoons: no interface (JPOG's SetInterface/GetInterface),
// the state variables are properties (TGUIStateVarSpec) and the widget info is
// returned by a virtual. Nicktoons also adds the "alpha" property and two
// virtuals (ResetParameters and Unknown_0x40).
//-----------------------------------------------------------------------------
class TGUIWidget
    : public TGUIDeserialisable
    , public TNodeList<TGUIWidget>::TNode
    , public TGUIMemory
{
public:
	TDECLARE_CLASS( TGUIWidget, TGUIDeserialisable );

	using InputEmitter = TEmitter<TGUIWidget, TGUIWidgetInputEvent>;
	using PaintEmitter = TEmitter<TGUIWidget, TGUIWidgetPaintEvent>;

	// The bits of the state variables of TGUIWidget
	enum State : TUINT
	{
		State_Enabled  = BITFLAG( 0 ),
		State_Focused  = BITFLAG( 1 ),
		State_Visible  = BITFLAG( 2 ),
		State_Rollover = BITFLAG( 3 ),
		State_Selected = BITFLAG( 4 ),
	};

	enum SizeMode : TINT
	{
		SizeMode_Fixed     = 0,
		SizeMode_Preferred = 1,
		SizeMode_Free      = 2,
		SizeMode_NUMOF     = 3
	};

	enum PlaceMode : TINT
	{
		PlaceMode_Fixed   = 0,
		PlaceMode_Managed = 1,
		PlaceMode_NUMOF   = 2
	};

	// The orientation of the PGUI sliders, list and table boxes
	enum Orientation : TINT
	{
		Orientation_Horizontal = 0,
		Orientation_Vertical   = 1,
		Orientation_NUMOF      = 2
	};

	// The placement of a part of a PGUI widget (the scroll bar of the list boxes)
	enum Placement : TINT
	{
		Placement_Before = 0,
		Placement_After  = 1,
		Placement_NUMOF  = 2
	};

	enum NeighbourDirection : TINT
	{
		NeighbourDirection_North    = 0,
		NeighbourDirection_East     = 1,
		NeighbourDirection_South    = 2,
		NeighbourDirection_West     = 3,
		NeighbourDirection_Previous = 4,
		NeighbourDirection_Next     = 5,
		NeighbourDirection_NUMOF    = 6
	};

	// A widget whose state follows the bits uiMask of this widget
	struct StateSlaveInfo : public TDList<StateSlaveInfo>::TNode
	{
		TGUIWidget* pSlave; // 0x8
		TUINT       uiMask; // 0xC
	};

	using t_RecurseCallback = TBOOL( __stdcall* )( TGUIWidget* a_pWidget, void* a_pUserData );
	using t_WalkCallback    = void( __stdcall* )( TGUIWidget* a_pWidget );

public:
	TGUIWidget();
	TGUIWidget( const TGUIWidget& a_rcOther );
	~TGUIWidget();

	//-----------------------------------------------------------------------------
	// TGUIDeserialisable
	//-----------------------------------------------------------------------------
	virtual TBOOL                   Deserialise( const PProperties* a_pProperties ) override;
	virtual TBOOL                   Serialise( PProperties* a_pProperties ) override;
	virtual TINT                    GetPropertyCount() const override;
	virtual const TGUIPropertySpec* FindPropertyInfo( const TPString8& a_rcName ) const override;
	virtual const TGUIPropertySpec* GetPropertyInfo( TINT a_iIndex ) const override;
	virtual TBOOL                   ValidatePropertyInfo( const TGUIPropertySpec* a_pSpec ) const override;
	virtual PropertySet*            GetPropertyInfoSet() const override;
	virtual PropertySet*            GetPropertyInfoOrderedSet() const override;

	//-----------------------------------------------------------------------------
	// TGUIWidget
	//-----------------------------------------------------------------------------

	// +0x34
	virtual const TGUIWidgetInfo* GetWidgetInfo() const;

	// +0x38: Clears the parameters of the widget and of its children and invalidates the caches
	// NOTE: the name is unknown
	virtual void ResetParameters();

	// +0x3C
	virtual TGUIWidget* Clone() const = 0;

	// +0x40: Sets an unknown flag (+0x90) of the widget and of its descendants
	// NOTE: the name is unknown, nothing reads the flag
	virtual void Unknown_0x40();

	// +0x44: Does the widget arrange its children itself (the focus goes through it, see FindChildManager)
	virtual TBOOL ManagesChildren() const;

	// +0x48
	virtual TBOOL AllowsChildren() const;

	// +0x4C
	virtual TBOOL AcceptsFocus() const;

	// +0x50: The widget that gets the focus instead of a_pWidget (asked to the parents)
	virtual TGUIWidget* AllowsFocus( TGUIWidget* a_pWidget );

	// +0x54
	virtual void NotifyDescendantAddition( TGUIWidget* a_pWidget );

	// +0x58
	virtual void NotifyDescendantRemoval( TGUIWidget* a_pWidget );

	// +0x5C
	virtual void NotifyDescendantRemoved( TGUIWidget* a_pWidget, TGUIWidget* a_pOldParent );

	// +0x60
	virtual void NotifyFocusLost( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus );

	// +0x64
	virtual void NotifyFocusGained( TGUIWidget* a_pNewFocus, TGUIWidget* a_pOldFocus );

	// +0x68
	virtual void NotifySlaveStateChanged( TGUIWidget* a_pMaster, TUINT a_uiChangedMask );

	// +0x6C
	virtual TBOOL OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions );

	// +0x70
	virtual TBOOL OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions );

	// +0x74
	virtual TBOOL OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions );

	// +0x78
	virtual TBOOL OnButtonHit( TInputInterface::InputEvent* a_pEvent, TINT a_iActions );

	// +0x7C
	virtual TBOOL OnAxisMove( TInputInterface::InputEvent* a_pEvent, TINT a_iActions );

	// +0x80
	virtual TBOOL OnPointerMove( TINT a_iX, TINT a_iY );

	// +0x84
	virtual TBOOL OnPointerEnter();

	// +0x88
	virtual TBOOL OnPointerExit();

	// +0x8C: Paints the widget with its style, returns TTRUE to stop painting (the next widgets are painted later)
	virtual TBOOL OnPaint( TGUIDisplayContext* a_pDisplayContext );

	// +0x90
	virtual void Pack();

	// +0x94: Computes the size of the widget for the maximum size a_iMaxWidth, a_iMaxHeight
	virtual void Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight ) = 0;

	// +0x98
	virtual TBOOL DeserialiseState( const PProperties* a_pProperties );

	// +0x9C
	virtual TBOOL DeserialiseParameters( const PProperties* a_pProperties );

	// +0xA0
	virtual TBOOL DeserialiseChildren( const PProperties* a_pProperties );

	// +0xA4
	virtual TBOOL SerialiseState( PProperties* a_pProperties );

	// +0xA8
	virtual TBOOL SerialiseParameters( PProperties* a_pProperties );

	// +0xAC
	virtual TBOOL SerialiseChildren( PProperties* a_pProperties );

	// +0xB0
	virtual void NotifyParentChange( TGUIWidget* a_pParent );

	//-----------------------------------------------------------------------------
	// Style and parameters
	//-----------------------------------------------------------------------------

	// Sets the style (the parameters and the caches are recreated if the widget didn't have one) and applies its fixed properties
	void SetStyle( TGUIStyle* a_pStyle );

	// Sets the style and the name of a new widget (TGUIStyle::CreateWidget)
	// NOTE: JPOG also sets the interface of the widget
	void Create( TGUIStyle* a_pStyle, const TPString8& a_rcName );

	// Recreates the parameters after the parameters of the style changed (TGUIStyle::UpdateDependentWidgets)
	void NotifyStyleChanged();

	void           SetParameter( TINT a_iIndex, const TWCHAR* a_wszString );
	void           SetParameter( TINT a_iIndex, TGUITextureSection* a_pTextureSection );
	void           SetParameter( TINT a_iIndex, const PPropertyValue& a_rcValue );
	PPropertyValue GetParameter( TINT a_iIndex ) const;

	// Resets the parameters a_iFirst to a_iLast - 1 to empty values of their types
	void InitParameters( TINT a_iFirst, TINT a_iLast );

	TBOOL IsReferencingTextureSection( const TGUITextureSection* a_pTextureSection ) const;

	// Invalidates the caches of the components (and of the children if a_bRecurse)
	void InvalidateComponentCache( TBOOL a_bRecurse );

	void SetComponentBBox( TINT a_iComponent, TINT a_iX, TINT a_iY, TINT a_iWidth, TINT a_iHeight );
	void GetComponentBBox( TINT a_iComponent, TINT& a_rX, TINT& a_rY, TINT& a_rWidth, TINT& a_rHeight );
	void GetComponentInteriorBBox( TINT a_iComponent, TINT& a_rX, TINT& a_rY, TINT& a_rWidth, TINT& a_rHeight );

	//-----------------------------------------------------------------------------
	// Children
	//-----------------------------------------------------------------------------

	// Adds a child at the head (a_bAtHead) or at the tail of the children and removes its focus
	void AddWidget( TGUIWidget* a_pWidget, TBOOL a_bAtHead );
	void AddWidgetBefore( TGUIWidget* a_pWidget, TGUIWidget* a_pBefore );
	void RemoveWidget( TGUIWidget* a_pWidget );
	void RemoveAllWidgets();
	void DeleteAllWidgets();

	TGUIWidget* FindIndexedChild( TINT a_iIndex ) const;
	TGUIWidget* FindNamedChild( const TPString8& a_rcName ) const;
	TGUIWidget* FindNamedChild( const TCHAR* a_szName ) const;
	TGUIWidget* FindChildManager();

	TBOOL IsDescendantOf( const TGUIWidget* a_pWidget ) const;
	TBOOL IsAncestorOf( const TGUIWidget* a_pWidget ) const;

	TGUIWindow* GetWindow();
	TGUIScreen* GetScreen();

	// Calls a_fnPre and a_fnPost on the widget and on its descendants (a_fnFilter returns TFALSE to skip a subtree)
	void RecurseTree( t_RecurseCallback a_fnPre, t_RecurseCallback a_fnPost, t_RecurseCallback a_fnFilter, void* a_pUserData );
	void RecurseTree2( t_RecurseCallback a_fnPre, t_RecurseCallback a_fnPost, t_RecurseCallback a_fnFilter, void* a_pUserData );

	void OrderChildMovePosition( TGUIWidget* a_pWidget, TINT a_iOffset );
	void OrderChildSetPosition( TGUIWidget* a_pWidget, TINT a_iPosition );
	void OrderPosition( TINT a_iPosition );
	void OrderBackward();
	void OrderForward();
	void OrderToBack();
	void OrderToFront();

	static TGUIWidget* FindCommonAncestor( TGUIWidget* a_pWidget1, TGUIWidget* a_pWidget2 );

	// Calls a_fnUp on the ancestors of a_pFrom up to the common ancestor and a_fnDown on the ancestors of a_pTo down from it
	static void WalkUpDownTree( TGUIWidget* a_pFrom, TGUIWidget* a_pTo, t_WalkCallback a_fnUp, t_WalkCallback a_fnDown );

	//-----------------------------------------------------------------------------
	// Layout
	//-----------------------------------------------------------------------------

	void SetLayoutManager( TGUILayoutManager* a_pLayoutManager );
	void PackChildren();
	void LayoutChildren( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight );

	void SetPosition( TINT a_iX, TINT a_iY );
	void SetPositionCenter();
	void SetSize( TINT a_iWidth, TINT a_iHeight );
	void SetFixedSize( TINT a_iWidth, TINT a_iHeight );
	void SetFixedWidth( TINT a_iWidth );
	void SetFixedHeight( TINT a_iHeight );
	void SetWidth( TINT a_iWidth );
	void SetHeight( TINT a_iHeight );

	TINT GetInteriorWidth() const;
	TINT GetInteriorHeight() const;
	void GetInteriorSize( TINT& a_rWidth, TINT& a_rHeight ) const;
	void GetMargins( TINT& a_rLeft, TINT& a_rRight, TINT& a_rTop, TINT& a_rBottom );

	TBOOL CanResizeHoriz() const;
	TBOOL CanResizeVert() const;
	TBOOL CanMove() const;

	void TransformDeviceToWidget( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY ) const;
	void TransformWidgetToDevice( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY ) const;

	//-----------------------------------------------------------------------------
	// State, focus, actions
	//-----------------------------------------------------------------------------

	// Sets the bits a_uiMask of the state to a_uiValue, updates the focus and the state slaves
	void SetState( TUINT a_uiMask, TUINT a_uiValue );

	// $JPOG: FUNCTION TGuiInterface.dll+000074d0
	void SetState( TUINT a_uiMask, TBOOL a_bSet ) { SetState( a_uiMask, a_bSet ? a_uiMask : 0u ); }

	// The widget and its parents are enabled and visible
	TBOOL CanFocus();

	void RequestFocus();
	void RemoveFocus();

	void  AddStateSlave( TGUIWidget* a_pSlave, TUINT a_uiMask );
	void  RemoveStateSlave( TGUIWidget* a_pSlave, TUINT a_uiMask );
	TUINT GetStateSlaveMask( TGUIWidget* a_pSlave );

	// Adds an action for a doodad (or changes the label of the existing one)
	TGUIWidgetAction* AddAction( TINT a_iDoodadID, const TWCHAR* a_wszLabel, TINT a_iCommandID );

	//-----------------------------------------------------------------------------
	// Painting and hit tests
	//-----------------------------------------------------------------------------

	// Paints the visible descendants (from a_pFrom if painting was stopped), returns the widget the painting stopped at
	TGUIWidget* Paint( TGUIDisplayContext* a_pDisplayContext, TGUIWidget* a_pFrom );

	// a_iX, a_iY in the parent, returns them in the widget in a_rX, a_rY
	TBOOL       HitTest( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY );
	TGUIWidget* HitTestDeepest( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY, TUINT a_uiStateMask, TUINT a_uiStateValue );
	TGUIWidget* GetChildAtPoint( TINT a_iX, TINT a_iY, TUINT a_uiStateMask, TUINT a_uiStateValue );
	TGUIWidget* GetChildAtPoint( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY, TUINT a_uiStateMask, TUINT a_uiStateValue );
	TGUIWidget* GetDeepestChildAtPoint( TINT a_iX, TINT a_iY, TINT& a_rX, TINT& a_rY, TUINT a_uiStateMask, TUINT a_uiStateValue );

	//-----------------------------------------------------------------------------
	// Properties
	//-----------------------------------------------------------------------------

	void           Set_name_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_name_property() const;
	void           Set_sizemode_horiz_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_sizemode_horiz_property() const;
	void           Set_sizemode_vert_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_sizemode_vert_property() const;
	void           Set_placemode_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_placemode_property() const;
	void           Set_xpos_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_xpos_property() const;
	void           Set_ypos_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_ypos_property() const;
	void           Set_width_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_width_property() const;
	void           Set_height_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_height_property() const;
	void           Set_animsync_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_animsync_property() const;
	void           Set_alpha_property( const PPropertyValue& a_rcValue );
	PPropertyValue Get_alpha_property() const;

	// The properties of the state variables (TGUIStateVarSpec)
	void           Set_state_property( const PPropertyValue& a_rcValue, TUINT a_uiMask );
	PPropertyValue Get_state_property( TUINT a_uiMask ) const;

	// The properties of the parameters of the style (TGUIStyleParameterSpec)
	void           Set_style_parameter( const PPropertyValue& a_rcValue, TGUIStyleParameterInfo* a_pParameter );
	PPropertyValue Get_style_parameter( TGUIStyleParameterInfo* a_pParameter ) const;

	// Declares a state variable of a widget class: adds it to the state variables of a_pWidgetInfo and a property spec to the property sets
	// NOTE: the name is unknown
	static void DeclareStateVar( const TClass* a_pClass, const TCHAR* a_szName, TUINT a_uiMask, const TPString8& a_rcCategory, TINT& a_rSortHint, TGUIWidgetInfo* a_pWidgetInfo, PropertySet* a_pPropertySet, PropertySet* a_pOrderedSet );

	static const TPString8&   GetDirectionName( NeighbourDirection a_eDirection );
	static NeighbourDirection GetDirectionEnum( const TPString8& a_rcName );

	//-----------------------------------------------------------------------------
	// Accessors
	//-----------------------------------------------------------------------------

	// $JPOG: FUNCTION TGuiInterface.dll+00007620
	TGUIStyle* GetStyle() const { return m_pStyle; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007630
	TGUILayoutManager* GetLayoutManager() const { return m_pLayoutManager; }

	// $JPOG: FUNCTION TGuiInterface.dll+000075f0
	TGUIWidget* GetParentWidget() const { return m_pParent; }

	// $JPOG: FUNCTION TGuiInterface.dll+000075d0
	TNodeList<TGUIWidget>& GetChildren() { return m_oChildren; }

	// $JPOG: FUNCTION TGuiInterface.dll+000075e0
	TINT GetChildCount() const { return m_oChildren.Count(); }

	// $JPOG: FUNCTION TGuiInterface.dll+000073e0
	TDList<StateSlaveInfo>& GetStateSlaveList() { return m_oStateSlaves; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007420
	TDList<TGUIWidgetAction>& GetActionList() { return m_oActions; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007640
	const TPString8& GetName() const { return m_strName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007780
	void SetName( const TPString8& a_rcName ) { m_strName = a_rcName; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007680
	void GetBBox( TGUIRectangle& a_rRect ) const { a_rRect = TGUIRectangle( m_iXPos, m_iYPos, m_iWidth, m_iHeight ); }

	// $JPOG: FUNCTION TGuiInterface.dll+000076e0
	TINT GetXPos() const { return m_iXPos; }

	// $JPOG: FUNCTION TGuiInterface.dll+000076d0
	TINT GetYPos() const { return m_iYPos; }

	// $JPOG: FUNCTION TGuiInterface.dll+000076c0
	TINT GetWidth() const { return m_iWidth; }

	// $JPOG: FUNCTION TGuiInterface.dll+000076b0
	TINT GetHeight() const { return m_iHeight; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007710
	void GetPosition( TINT& a_rX, TINT& a_rY ) const
	{
		a_rX = m_iXPos;
		a_rY = m_iYPos;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000076f0
	void GetSize( TINT& a_rWidth, TINT& a_rHeight ) const
	{
		a_rWidth  = m_iWidth;
		a_rHeight = m_iHeight;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+000077e0
	void SetXPos( TINT a_iX ) { SetPosition( a_iX, m_iYPos ); }

	// $JPOG: FUNCTION TGuiInterface.dll+000077c0
	void SetYPos( TINT a_iY ) { SetPosition( m_iXPos, a_iY ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007670
	SizeMode GetSizeModeHoriz() const { return m_eSizeModeHoriz; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007660
	SizeMode GetSizeModeVert() const { return m_eSizeModeVert; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007650
	PlaceMode GetPlaceMode() const { return m_ePlaceMode; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007740
	void SetSizeMode( SizeMode a_eHoriz, SizeMode a_eVert )
	{
		m_eSizeModeHoriz = a_eHoriz;
		m_eSizeModeVert  = a_eVert;
	}

	// $JPOG: FUNCTION TGuiInterface.dll+00007770
	void SetSizeModeHoriz( SizeMode a_eMode ) { m_eSizeModeHoriz = a_eMode; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007760
	void SetSizeModeVert( SizeMode a_eMode ) { m_eSizeModeVert = a_eMode; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007730
	void SetPlaceMode( PlaceMode a_eMode ) { m_ePlaceMode = a_eMode; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007610
	TFLOAT GetAnimSync() const { return m_fAnimSync; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007600
	void SetAnimSync( TFLOAT a_fAnimSync ) { m_fAnimSync = a_fAnimSync; }

	TFLOAT GetAlpha() const { return m_fAlpha; }
	void   SetAlpha( TFLOAT a_fAlpha ) { m_fAlpha = a_fAlpha; }

	// $JPOG: FUNCTION TGuiInterface.dll+000075c0
	TBOOL IsValidated() const { return m_bValidated; }

	// $JPOG: FUNCTION TGuiInterface.dll+000075b0
	void SetValidated( TBOOL a_bValidated ) { m_bValidated = a_bValidated; }

	// $JPOG: FUNCTION TGuiInterface.dll+000073c0
	TBOOL IsLayoutValid() const { return m_bLayoutValid; }

	TUINT GetState() const { return m_uiState; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007570
	TBOOL IsState( TUINT a_uiMask ) const { return ( m_uiState & a_uiMask ) != 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007550
	TBOOL IsState( TUINT a_uiMask, TUINT a_uiValue ) const { return ( m_uiState & a_uiMask ) == a_uiValue; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007530
	TBOOL IsEnabled() const { return ( m_uiState & State_Enabled ) != 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007520
	TBOOL IsFocused() const { return ( m_uiState & State_Focused ) != 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007540
	TBOOL IsVisible() const { return ( m_uiState & State_Visible ) != 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007510
	TBOOL IsRollover() const { return ( m_uiState & State_Rollover ) != 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007500
	TBOOL IsSelected() const { return ( m_uiState & State_Selected ) != 0; }

	// $JPOG: FUNCTION TGuiInterface.dll+00007490
	void SetEnabled( TBOOL a_bEnabled ) { SetState( State_Enabled, a_bEnabled ? State_Enabled : 0 ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007470
	void SetFocused( TBOOL a_bFocused ) { SetState( State_Focused, a_bFocused ? State_Focused : 0 ); }

	// $JPOG: FUNCTION TGuiInterface.dll+000074b0
	void SetVisible( TBOOL a_bVisible ) { SetState( State_Visible, a_bVisible ? State_Visible : 0 ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007450
	void SetRollover( TBOOL a_bRollover ) { SetState( State_Rollover, a_bRollover ? State_Rollover : 0 ); }

	// $JPOG: FUNCTION TGuiInterface.dll+00007430
	void SetSelected( TBOOL a_bSelected ) { SetState( State_Selected, a_bSelected ? State_Selected : 0 ); }

	TUINT8              GetParameterCount() const { return m_uiNumParameters; }
	PPropertyValue*     GetParameters() const { return m_pParameters; }
	TGUIComponentCache* GetComponentCaches() const { return m_pComponentCaches; }

	InputEmitter& GetInputEmitter() { return m_oInputEmitter; }
	PaintEmitter& GetPaintEmitter() { return m_oPaintEmitter; }

	// $JPOG: FUNCTION TGuiInterface.dll+000078b0
	static const TGUIWidgetInfo* GetStaticWidgetInfo() { return ms_pWidgetInfo; }

	static TINT GetNumInstances() { return ms_iNumInstances; }

private:
	// Adds a child at the tail of the children (the focus isn't removed)
	void AddWidgetPrivate( TGUIWidget* a_pWidget );

	// Calls Unknown_0x40 on the children
	void Unknown_0x40Children();

public:
	static PropertySet*    ms_oPropertySet;
	static PropertySet*    ms_oPropertySetOrdered;
	static TGUIWidgetInfo* ms_pWidgetInfo;

	static TPString8 str_widget;
	static TPString8 str_name;
	static TPString8 str_style;
	static TPString8 str_type;
	static TPString8 str_layout;
	static TPString8 str_width;
	static TPString8 str_height;
	static TPString8 str_xpos;
	static TPString8 str_ypos;
	static TPString8 str_animsync;
	static TPString8 str_alpha;

	static TPString8 ms_saDirectionName[ NeighbourDirection_NUMOF ];

	static const TCHAR* const ms_csaSizeModeNameStr[ SizeMode_NUMOF ];
	static const TCHAR* const ms_csaPlaceModeNameStr[ PlaceMode_NUMOF ];
	static const TCHAR* const ms_csaDirectionNameStr[ NeighbourDirection_NUMOF ];

	// The names of the orientations and placements (the PGUI properties)
	// NOTE: JPOG only, Nicktoons doesn't have them
	static const TCHAR* const ms_csaOrientationNameStr[ Orientation_NUMOF ];
	static const TCHAR* const ms_csaPlacementNameStr[ Placement_NUMOF ];

private:
	static TINT ms_iNumInstances;

protected:
	InputEmitter             m_oInputEmitter;    // 0x10
	PaintEmitter             m_oPaintEmitter;    // 0x1C
	TGUIStyle*               m_pStyle;           // 0x28
	TUINT8                   m_uiNumParameters;  // 0x2C
	PPropertyValue*          m_pParameters;      // 0x30 one per parameter of the style
	TGUIComponentCache*      m_pComponentCaches; // 0x34 one per root component of the widget class
	TGUILayoutManager*       m_pLayoutManager;   // 0x38
	TGUIWidget*              m_pParent;          // 0x3C
	TNodeList<TGUIWidget>    m_oChildren;        // 0x40
	TDList<StateSlaveInfo>   m_oStateSlaves;     // 0x50
	TDList<TGUIWidgetAction> m_oActions;         // 0x58
	TPString8                m_strName;          // 0x60
	TINT16                   m_iXPos;            // 0x64
	TINT16                   m_iYPos;            // 0x66
	TINT                     m_iUnknown1;        // 0x68
	TINT16                   m_iWidth;           // 0x6C
	TINT16                   m_iHeight;          // 0x6E
	SizeMode                 m_eSizeModeHoriz;   // 0x70
	SizeMode                 m_eSizeModeVert;    // 0x74
	PlaceMode                m_ePlaceMode;       // 0x78
	TFLOAT                   m_fAnimSync;        // 0x7C
	TFLOAT                   m_fAlpha;           // 0x80
	TBOOL                    m_bValidated;       // 0x84 cleared when the size changes
	TINT                     m_iUnknown2;        // 0x88 never initialised
	TBOOL                    m_bLayoutValid;     // 0x8C
	TBOOL                    m_bUnknown3;        // 0x90 set by Unknown_0x40
	TUINT                    m_uiState;          // 0x94
};

TOSHI_NAMESPACE_END
