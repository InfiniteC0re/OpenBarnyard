#include "ToshiPCH.h"
#include "TGUIScreen.h"
#include "TGUIWindow.h"
#include "TGUIManager.h"
#include "TGUIWidgetInfo.h"
#include "TGUIDisplayContext.h"
#include "TGUIAudioContext.h"
#include "TGUISoundFactory.h"
#include "TGUIInputContext.h"
#include "TGUIActionMap.h"
#include "TGUITextureSection.h"

#include <Plugins/PPropertyParser/PPropertyName.h>
#include <Input/TInputInterface.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include "Core/TMemoryDebugOn.h"

TOSHI_NAMESPACE_START

// NOTE: CreateObject returns TNULL in Nicktoons (00367188)
TDEFINE_CLASS_INIT_NORUNTIME( TGUIScreen );

TGUIDeserialisable::PropertySet* TGUIScreen::ms_oPropertySet        = TNULL;
TGUIDeserialisable::PropertySet* TGUIScreen::ms_oPropertySetOrdered = TNULL;
TGUIWidgetInfo*                  TGUIScreen::ms_pWidgetInfo         = TNULL;

// The widgets the focus enters (ChangeWidgetFocus)
// NOTE: a static stack of 32 widgets in the original
static TGUIWidget* s_apFocusPath[ 32 ];
static TINT        s_iFocusPathSize = 0;

// NOTE: Nicktoons creates the widget info in its static initialiser (00369628)
// $NicktoonsPS2: FUNCTION 003678c0
// $JPOG: FUNCTION TGuiInterface.dll+00013110
void TGUIScreen::InitialiseClass()
{
	ms_pWidgetInfo = new TGUIWidgetInfo();

	CreatePropertySets( ms_oPropertySet, ms_oPropertySetOrdered, *TGUIWidget::ms_oPropertySet );

	ms_pWidgetInfo->SetWidgetName( TPString8( "TGUIScreen" ) );
	ms_pWidgetInfo->SetWidgetLabel( TPString8( "Screen" ) );
	ms_pWidgetInfo->SetProperties( ms_oPropertySet, ms_oPropertySetOrdered );
	ms_pWidgetInfo->AddStateVars( TGUIWidget::ms_pWidgetInfo->GetStateVarSet() );
	ms_pWidgetInfo->SetConcrete( TFALSE );

	// NOTE: the original also shrinks the capacity of the component name array of the widget info to its size here

	TGUIManager::AddWidgetClass( ms_pWidgetInfo );
}

// $NicktoonsPS2: FUNCTION 00367be0
// $JPOG: FUNCTION TGuiInterface.dll+00013450
void TGUIScreen::DeinitialiseClass()
{
	ms_pWidgetInfo->Clear();
	DestroyPropertySets( ms_oPropertySet, ms_oPropertySetOrdered );

	delete ms_pWidgetInfo;
	ms_pWidgetInfo = TNULL;
}

// NOTE: the sound factory of the manager must be set
// $NicktoonsPS2: FUNCTION 00367198
// $JPOG: FUNCTION TGuiInterface.dll+00013510
TGUIScreen::TGUIScreen()
    : m_oFocusChangeEmitter( this )
    , m_oRolloverEmitter( this )
    , m_oScreenPaintEmitter( this )
    , m_pDisplayContext( TNULL )
    , m_pAudioContext( TNULL )
    , m_pInputContext( TNULL )
    , m_iInputManagerTop( 0 )
    , m_pFocusedWindow( TNULL )
    , m_pFocusedWidget( TNULL )
    , m_pPointerGrabWidget( TNULL )
    , m_aFocusGrabs( 4, 4 )
    , m_pWidgetUnderPointer( this )
    , m_bRolloverUpdated( TTRUE )
    , m_bAllowRolloverUpdate( TTRUE )
    , m_iPointerX( 0 )
    , m_iPointerY( 0 )
    , m_fPointerSpeed( 1.0f )
    , m_pPointerImage( TNULL )
    , m_bPointerVisible( TFALSE )
    , m_pPaintContinuation( TNULL )
    , m_iUnknown( 0 )
    , m_iPointerAccelThreshold1( 6 )
    , m_iPointerAccelThreshold2( 10 )
    , m_iPointerAccelMode( 1 )
{
	// No focus grab
	m_aFocusGrabs.Push( TNULL );

	m_apInputManagers[ m_iInputManagerTop ] = this;

	SetState( State_Enabled, State_Enabled );
	SetState( State_Visible, State_Visible );

	TGUISoundFactory* pSoundFactory = TGUIManager::GetSingleton()->GetSoundFactory();
	m_iNavigateSound                = pSoundFactory->GetSoundID( PPropertyName( TPString8( "TGUIScreen" ), TPString8( "navigate" ) ) );
	m_iNavigateFailSound            = pSoundFactory->GetSoundID( PPropertyName( TPString8( "TGUIScreen" ), TPString8( "navigate_fail" ) ) );
}

// The contexts, the pointer speed, image and visibility are copied
// NOTE: the sounds, the hot spot and the pointer acceleration aren't copied (as in the original), they're cleared here
// $NicktoonsPS2: FUNCTION 003675c8
// $JPOG: FUNCTION TGuiInterface.dll+00014d10
TGUIScreen::TGUIScreen( const TGUIScreen& a_rcOther )
    : TGUIWidget( a_rcOther )
    , m_oFocusChangeEmitter( this )
    , m_oRolloverEmitter( this )
    , m_oScreenPaintEmitter( this )
    , m_pDisplayContext( a_rcOther.m_pDisplayContext )
    , m_pAudioContext( a_rcOther.m_pAudioContext )
    , m_pInputContext( a_rcOther.m_pInputContext )
    , m_iInputManagerTop( 0 )
    , m_pFocusedWindow( TNULL )
    , m_pFocusedWidget( TNULL )
    , m_pPointerGrabWidget( TNULL )
    , m_aFocusGrabs( 4, 4 )
    , m_pWidgetUnderPointer( this )
    , m_bRolloverUpdated( TFALSE )
    , m_bAllowRolloverUpdate( TTRUE )
    , m_iPointerX( 0 )
    , m_iPointerY( 0 )
    , m_fPointerSpeed( a_rcOther.m_fPointerSpeed )
    , m_iNavigateSound( 0 )
    , m_iNavigateFailSound( 0 )
    , m_pPointerImage( a_rcOther.m_pPointerImage )
    , m_iPointerHotSpotX( 0 )
    , m_iPointerHotSpotY( 0 )
    , m_bPointerVisible( a_rcOther.m_bPointerVisible )
    , m_pPaintContinuation( TNULL )
    , m_iUnknown( 0 )
    , m_iPointerAccelThreshold1( 0 )
    , m_iPointerAccelThreshold2( 0 )
    , m_iPointerAccelMode( 0 )
{
	m_aFocusGrabs.Push( TNULL );
	m_apInputManagers[ m_iInputManagerTop ] = this;
}

// NOTE: JPOG reports an error if the screen still has an interface
// $NicktoonsPS2: FUNCTION 00367820
// $JPOG: FUNCTION TGuiInterface.dll+000137b0
TGUIScreen::~TGUIScreen()
{
	RemoveAllWidgets();
}

// NOTE: the screens can't be read from a definition file
// $NicktoonsPS2: FUNCTION 003696f0
// $JPOG: FUNCTION TGuiInterface.dll+0000f980
TBOOL TGUIScreen::Deserialise( const PProperties* a_pProperties )
{
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003696e8
// $JPOG: FUNCTION TGuiInterface.dll+0000f990
TBOOL TGUIScreen::Serialise( PProperties* a_pProperties )
{
	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003694e8
// $JPOG: FUNCTION TGuiInterface.dll+00015380
TGUIDeserialisable::PropertySet* TGUIScreen::GetPropertyInfoSet() const
{
	return ms_oPropertySet;
}

// $NicktoonsPS2: FUNCTION 003694f0
// $JPOG: FUNCTION TGuiInterface.dll+00015370
TGUIDeserialisable::PropertySet* TGUIScreen::GetPropertyInfoOrderedSet() const
{
	return ms_oPropertySetOrdered;
}

// $NicktoonsPS2: FUNCTION 003696d8
const TGUIWidgetInfo* TGUIScreen::GetWidgetInfo() const
{
	return ms_pWidgetInfo;
}

// $NicktoonsPS2: FUNCTION 00367d28
// $JPOG: FUNCTION TGuiInterface.dll+00015340
TGUIScreen* TGUIScreen::Clone() const
{
	return new TGUIScreen( *this );
}

// Every widget can get the focus
// $NicktoonsPS2: FUNCTION 003696f8
// $JPOG: FUNCTION TGuiInterface.dll+0000f970
TGUIWidget* TGUIScreen::AllowsFocus( TGUIWidget* a_pWidget )
{
	return a_pWidget;
}

// NOTE: the result of CanFocus isn't used (as in the original), Nicktoons calls Unknown_0x40 on the new widget
// $NicktoonsPS2: FUNCTION 00368ff8
// $JPOG: FUNCTION TGuiInterface.dll+00015230
void TGUIScreen::NotifyDescendantAddition( TGUIWidget* a_pWidget )
{
	if ( !m_pPointerGrabWidget )
		a_pWidget->CanFocus();

	a_pWidget->Unknown_0x40();
	TGUIWidget::NotifyDescendantAddition( a_pWidget );
}

// NOTE: inlined in NotifyDescendantCantFocus and NotifyDescendantRemoval
void TGUIScreen::RemoveFocusGrabs( TGUIWidget* a_pWidget )
{
	for ( TINT i = m_aFocusGrabs.Size() - 1; i >= 0; i-- )
	{
		TGUIWidget* pGrab = m_aFocusGrabs[ i ];

		if ( pGrab && IsWidgetOrDescendant( pGrab, a_pWidget ) )
		{
			for ( TINT j = i; j < m_aFocusGrabs.Size() - 1; j++ )
				m_aFocusGrabs[ j ] = m_aFocusGrabs[ j + 1 ];

			m_aFocusGrabs.SetSize( m_aFocusGrabs.Size() - 1 );
		}
	}
}

// The focus, the pointer grab and the rollover leave the removed widget
// NOTE: the rollover event gets the removed widget as the new one (as in the original)
// $NicktoonsPS2: FUNCTION 00369050
// $JPOG: FUNCTION TGuiInterface.dll+00014910
void TGUIScreen::NotifyDescendantRemoval( TGUIWidget* a_pWidget )
{
	RemoveFocusGrabs( a_pWidget );

	if ( m_pFocusedWidget && IsWidgetOrDescendant( m_pFocusedWidget, a_pWidget ) )
		ChangeWidgetFocus( a_pWidget->GetParentWidget(), TFALSE );

	if ( m_pPointerGrabWidget && IsWidgetOrDescendant( m_pPointerGrabWidget, a_pWidget ) )
		ReleasePointer( m_pPointerGrabWidget );

	if ( m_pWidgetUnderPointer && IsWidgetOrDescendant( m_pWidgetUnderPointer, a_pWidget ) )
	{
		WalkUpDownTree( m_pWidgetUnderPointer, a_pWidget->GetParentWidget(), pointer_out_callback, pointer_in_callback );

		TGUIRolloverEvent oEvent;
		oEvent.pOldWidget     = m_pWidgetUnderPointer;
		oEvent.pNewWidget     = a_pWidget;
		m_pWidgetUnderPointer = a_pWidget->GetParentWidget();
		m_oRolloverEmitter.Throw( &oEvent );
	}

	TGUIWidget::NotifyDescendantRemoval( a_pWidget );
}

// $NicktoonsPS2: FUNCTION 00369218
// $JPOG: FUNCTION TGuiInterface.dll+00014a50
void TGUIScreen::NotifyDescendantRemoved( TGUIWidget* a_pWidget, TGUIWidget* a_pOldParent )
{
	if ( m_pWidgetUnderPointer && IsWidgetOrDescendant( m_pWidgetUnderPointer, a_pWidget ) )
	{
		TINT        iX, iY;
		TGUIWidget* pWidget = HitTestDeepest( m_iPointerX, m_iPointerY, iX, iY, State_Enabled | State_Visible, State_Enabled | State_Visible );

		if ( !pWidget )
			pWidget = this;

		TGUIRolloverEvent oEvent;
		oEvent.pOldWidget     = m_pWidgetUnderPointer;
		oEvent.pNewWidget     = pWidget;
		m_pWidgetUnderPointer = pWidget;
		m_oRolloverEmitter.Throw( &oEvent );

		WalkUpDownTree( a_pOldParent, m_pWidgetUnderPointer, pointer_out_callback, pointer_in_callback );
	}

	TGUIWidget::NotifyDescendantRemoved( a_pWidget, a_pOldParent );
}

// The navigation actions move the focus (the doodad starts repeating)
// $NicktoonsPS2: FUNCTION 003692e8
// $JPOG: FUNCTION TGuiInterface.dll+00014180
TBOOL TGUIScreen::OnButtonPress( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonPress( a_pEvent, a_iActions ) == TTRUE )
		return TTRUE;

	if ( !( a_iActions & Action_Navigate ) )
		return TFALSE;

	if ( m_pFocusedWidget == this || !m_pFocusedWidget )
		return TFALSE;

	a_pEvent->StartRepeat( 0.5f, 0.25f );

	if ( a_iActions & Action_North )
		return MoveFocus( NeighbourDirection_North );
	else if ( a_iActions & Action_East )
		return MoveFocus( NeighbourDirection_East );
	else if ( a_iActions & Action_South )
		return MoveFocus( NeighbourDirection_South );
	else if ( a_iActions & Action_West )
		return MoveFocus( NeighbourDirection_West );

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003693e8
// $JPOG: FUNCTION TGuiInterface.dll+00014230
TBOOL TGUIScreen::OnButtonRepeat( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( TGUIWidget::OnButtonRepeat( a_pEvent, a_iActions ) == TTRUE )
		return TTRUE;

	if ( !( a_iActions & Action_Navigate ) )
		return TFALSE;

	if ( m_pFocusedWidget == this || !m_pFocusedWidget )
		return TFALSE;

	if ( a_iActions & Action_North )
		return MoveFocus( NeighbourDirection_North );
	else if ( a_iActions & Action_East )
		return MoveFocus( NeighbourDirection_East );
	else if ( a_iActions & Action_South )
		return MoveFocus( NeighbourDirection_South );
	else if ( a_iActions & Action_West )
		return MoveFocus( NeighbourDirection_West );

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003694b8
// $JPOG: FUNCTION TGuiInterface.dll+00015270
TBOOL TGUIScreen::OnButtonRelease( TInputInterface::InputEvent* a_pEvent, TINT a_iActions )
{
	if ( a_iActions & Action_Navigate )
		return TFALSE;

	return TGUIWidget::OnButtonRelease( a_pEvent, a_iActions );
}

// $NicktoonsPS2: FUNCTION 00367d60
// $JPOG: FUNCTION TGuiInterface.dll+00015330
void TGUIScreen::Pack()
{
	PackChildren();
}

// $NicktoonsPS2: FUNCTION 00367d80
// $JPOG: FUNCTION TGuiInterface.dll+00015320
void TGUIScreen::Layout( TINT16 a_iMaxWidth, TINT16 a_iMaxHeight )
{
	m_bLayoutValid = TTRUE;
}

// A click focuses the widget under the pointer, the press goes to the focused widget then its parents
// NOTE: JPOG uses another action bit for the click and doesn't return early
// $NicktoonsPS2: FUNCTION 003685c0
// $NicktoonsPS2: FUNCTION 00369568
// $JPOG: FUNCTION TGuiInterface.dll+00014b30
TBOOL TGUIScreen::DoButtonPress( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	if ( a_uiActions & Action_PointerClick )
	{
		if ( !m_bRolloverUpdated )
			GetInputManager()->DoRolloverUpdate();

		ChangeWidgetFocus( m_pWidgetUnderPointer, TTRUE );

		if ( m_pWidgetUnderPointer != this && m_pFocusedWidget == this )
			return TTRUE;
	}

	for ( TGUIWidget* pWidget = m_pFocusedWidget; pWidget; pWidget = pWidget->GetParentWidget() )
	{
		if ( pWidget->OnButtonPress( a_pEvent, a_uiActions ) == TTRUE )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003686a0
// $NicktoonsPS2: FUNCTION 00369588
// $JPOG: FUNCTION TGuiInterface.dll+00014bc0
TBOOL TGUIScreen::DoButtonRepeat( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	for ( TGUIWidget* pWidget = m_pFocusedWidget; pWidget; pWidget = pWidget->GetParentWidget() )
	{
		if ( pWidget->OnButtonRepeat( a_pEvent, a_uiActions ) == TTRUE )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00368720
// $NicktoonsPS2: FUNCTION 003695a8
// $JPOG: FUNCTION TGuiInterface.dll+00014c10
TBOOL TGUIScreen::DoButtonRelease( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	for ( TGUIWidget* pWidget = m_pFocusedWidget; pWidget; pWidget = pWidget->GetParentWidget() )
	{
		if ( pWidget->OnButtonRelease( a_pEvent, a_uiActions ) == TTRUE )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 003687a0
// $NicktoonsPS2: FUNCTION 003695c8
// $JPOG: FUNCTION TGuiInterface.dll+00014c60
TBOOL TGUIScreen::DoAxisMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	for ( TGUIWidget* pWidget = m_pFocusedWidget; pWidget; pWidget = pWidget->GetParentWidget() )
	{
		if ( pWidget->OnAxisMove( a_pEvent, a_uiActions ) == TTRUE )
			return TTRUE;
	}

	return TFALSE;
}

// $NicktoonsPS2: FUNCTION 00368820
// $NicktoonsPS2: FUNCTION 003695e8
// $JPOG: FUNCTION TGuiInterface.dll+00015200
TBOOL TGUIScreen::DoPointerMove( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	return UpdatePointerPosition( a_pEvent, a_uiActions );
}

// $NicktoonsPS2: FUNCTION 00368840
// $NicktoonsPS2: FUNCTION 00369608
// $JPOG: FUNCTION TGuiInterface.dll+00014cb0
void TGUIScreen::DoRolloverUpdate()
{
	TINT        iX      = 0;
	TINT        iY      = 0;
	TGUIWidget* pWidget = HitTestDeepest( m_iPointerX, m_iPointerY, iX, iY, State_Enabled | State_Visible, State_Enabled | State_Visible );

	RefreshPointerPosition( pWidget, iX, iY );
}

// NOTE: the actions come from the action map of the manager (of the interface in JPOG)
// $NicktoonsPS2: FUNCTION 00368440
// $JPOG: FUNCTION TGuiInterface.dll+00014080
TBOOL TGUIScreen::NotifyInputEvent( TInputInterface::InputEvent* a_pEvent )
{
	TUINT uiActions = TGUIManager::GetSingleton()->GetActionMap().TranslateDoodadToAction( a_pEvent->GetDoodad() );

	if ( uiActions & Action_PointerMove )
	{
		TBOOL bResult = GetInputManager()->DoPointerMove( a_pEvent, uiActions );

		if ( m_bAllowRolloverUpdate )
			GetInputManager()->DoRolloverUpdate();

		return bResult;
	}

	switch ( a_pEvent->GetEventType() )
	{
		case TInputInterface::EVENT_TYPE_GONE_DOWN:
			return GetInputManager()->DoButtonPress( a_pEvent, uiActions );
		case TInputInterface::EVENT_TYPE_GONE_UP:
			return GetInputManager()->DoButtonRelease( a_pEvent, uiActions );
		case TInputInterface::EVENT_TYPE_REPEAT:
			return GetInputManager()->DoButtonRepeat( a_pEvent, uiActions );
		case TInputInterface::EVENT_TYPE_VIRTUAL:
		case TInputInterface::EVENT_TYPE_MOVED:
			return GetInputManager()->DoAxisMove( a_pEvent, uiActions );
		default:
			return TFALSE;
	}
}

// Paints the widgets in the screen (the paint emitter is thrown before and after) and the pointer
// $NicktoonsPS2: FUNCTION 00367de0
// $JPOG: FUNCTION TGuiInterface.dll+00014fa0
void TGUIScreen::Paint( TGUIWidget* a_pFrom )
{
	if ( !( m_uiState & State_Visible ) )
		return;

	if ( !a_pFrom )
	{
		m_pDisplayContext->PushClip( 0, 0, m_iWidth, m_iHeight );

		TGUIScreenPaintEvent oEvent;
		oEvent.ePhase = TGUIScreenPaintEvent::Phase_Begin;
		m_oScreenPaintEmitter.Throw( &oEvent );
	}

	m_pPaintContinuation = TGUIWidget::Paint( m_pDisplayContext, a_pFrom );

	if ( !m_pPaintContinuation )
	{
		TGUIScreenPaintEvent oEvent;
		oEvent.ePhase = TGUIScreenPaintEvent::Phase_End;
		m_oScreenPaintEmitter.Throw( &oEvent );

		if ( m_pPointerImage && m_bPointerVisible )
		{
			TGUIColour oColour( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );
			m_pDisplayContext->Imp_DrawTextureSection( m_pPointerImage, oColour, m_iPointerX - m_iPointerHotSpotX, m_iPointerY - m_iPointerHotSpotY, m_pPointerImage->GetWidth(), m_pPointerImage->GetHeight(), TGUIAlignment_Left, TGUIAlignment_Top );
		}

		m_pDisplayContext->PopClip();
	}
}

// The widget actually focused is what the parent of a_pWidget allows (or its window if it doesn't accept the focus), the
// widget can't leave the focus grab. The widgets losing the focus get it removed up to the common ancestor, the widgets
// getting it get it from the common ancestor down.
// NOTE: Nicktoons drops the widget if it doesn't accept the focus and is a window, JPOG takes its window
// $NicktoonsPS2: FUNCTION 00367f00
// $JPOG: FUNCTION TGuiInterface.dll+00013a60
void TGUIScreen::ChangeWidgetFocus( TGUIWidget* a_pWidget, TBOOL a_bExact )
{
	TGUIWidget* pWidget = a_pWidget;
	TGUIWidget* pGrab   = m_aFocusGrabs[ m_aFocusGrabs.Size() - 1 ];

	if ( pGrab )
	{
		if ( !pWidget || ( !pWidget->IsDescendantOf( pGrab ) && pWidget != pGrab ) )
			pWidget = pGrab;
	}

	if ( pWidget && pWidget != this )
	{
		pWidget = pWidget->GetParentWidget()->AllowsFocus( pWidget );

		if ( !pWidget->AcceptsFocus() && pWidget == TSTATICCAST( TGUIWidget, pWidget->GetWindow() ) )
			pWidget = TNULL;
	}

	if ( !pWidget )
	{
		// The last child that can be focused
		pWidget = this;

		for ( auto it = m_oChildren.Tail(); it != m_oChildren.End(); it-- )
		{
			if ( it.Get() != m_pFocusedWindow && it->AcceptsFocus() && it->CanFocus() )
			{
				pWidget = it;
				break;
			}
		}
	}

	if ( !pWidget->CanFocus() || !pWidget->AcceptsFocus() )
		pWidget = this;

	if ( !a_bExact && pWidget->IsA( &TGetClass( TGUIWindow ) ) )
		pWidget = TSTATICCAST( TGUIWindow, pWidget )->GetAutoFocus();

	if ( pWidget == m_pFocusedWidget )
		return;

	TGUIWidget* pOldFocus = m_pFocusedWidget;
	TGUIWidget* pCommonAncestor;
	s_iFocusPathSize = 0;

	if ( pOldFocus )
	{
		pCommonAncestor = FindCommonAncestor( pOldFocus, pWidget );

		for ( TGUIWidget* pLosing = pOldFocus; pLosing != pCommonAncestor; pLosing = pLosing->GetParentWidget() )
		{
			pLosing->SetState( State_Focused, 0u );
			pLosing->NotifyFocusLost( pWidget, pOldFocus );
		}
	}
	else
	{
		pCommonAncestor = this;
	}

	m_pFocusedWidget = pWidget;
	m_pFocusedWindow = pWidget->GetWindow();

	if ( pWidget != pCommonAncestor )
	{
		for ( TGUIWidget* pGaining = pWidget; pGaining != pCommonAncestor; pGaining = pGaining->GetParentWidget() )
			s_apFocusPath[ s_iFocusPathSize++ ] = pGaining;
	}
	else
	{
		s_apFocusPath[ s_iFocusPathSize++ ] = pWidget;
	}

	while ( s_iFocusPathSize != 0 )
	{
		TGUIWidget* pGaining = s_apFocusPath[ --s_iFocusPathSize ];
		pGaining->SetState( State_Focused, State_Focused );
		pGaining->NotifyFocusGained( pWidget, pOldFocus );
	}

	if ( m_pInputContext )
		m_pInputContext->UpdateStatusDisplay();

	TGUIFocusChangeEvent oEvent;
	oEvent.pOldFocus = pOldFocus;
	oEvent.pNewFocus = m_pFocusedWidget;
	m_oFocusChangeEmitter.Throw( &oEvent );
}

// $NicktoonsPS2: FUNCTION 00368288
// $JPOG: FUNCTION TGuiInterface.dll+00013d00
TBOOL TGUIScreen::MoveFocus( TGUIWidget::NeighbourDirection a_eDirection )
{
	if ( !m_pFocusedWidget || m_pFocusedWidget == this )
		return TFALSE;

	TGUIWidget* pNeighbour = m_pFocusedWindow->FindFocusNeighbour( m_pFocusedWidget, a_eDirection );

	if ( !pNeighbour || pNeighbour == m_pFocusedWidget )
		return TFALSE;

	m_pAudioContext->PlaySound( m_iNavigateSound );
	ChangeWidgetFocus( pNeighbour, TTRUE );

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00013de0
void TGUIScreen::GrabFocus( TGUIWidget* a_pWidget )
{
	if ( a_pWidget->AcceptsFocus() && a_pWidget->CanFocus() )
	{
		m_aFocusGrabs.Push( a_pWidget );
		ChangeWidgetFocus( a_pWidget, TFALSE );
	}
}

// $NicktoonsPS2: FUNCTION 00368398
// $JPOG: FUNCTION TGuiInterface.dll+00013f70
void TGUIScreen::ReleaseFocus( TGUIWidget* a_pWidget )
{
	for ( TINT i = m_aFocusGrabs.Size() - 1; i >= 0; i-- )
	{
		if ( m_aFocusGrabs[ i ] == a_pWidget )
		{
			for ( TINT j = i; j < m_aFocusGrabs.Size() - 1; j++ )
				m_aFocusGrabs[ j ] = m_aFocusGrabs[ j + 1 ];

			m_aFocusGrabs.SetSize( m_aFocusGrabs.Size() - 1 );
			return;
		}
	}
}

// The widgets between the parent of the grab and the widget under the pointer get the pointer enter/exit
// $NicktoonsPS2: FUNCTION 00368310
// $JPOG: FUNCTION TGuiInterface.dll+00013d70
void TGUIScreen::ReleasePointer( TGUIWidget* a_pWidget )
{
	if ( a_pWidget != m_pPointerGrabWidget )
		return;

	TGUIWidget* pParent = a_pWidget->GetParentWidget();

	if ( !m_bRolloverUpdated )
		GetInputManager()->DoRolloverUpdate();

	WalkUpDownTree( pParent, m_pWidgetUnderPointer, pointer_out_callback, pointer_in_callback );
	m_pPointerGrabWidget = TNULL;
}

// The focus leaves the widget that can't be focused anymore (and its descendants)
// NOTE: the rollover event gets the widget as the new one (as in the original)
// $NicktoonsPS2: FUNCTION 00368e60
// $JPOG: FUNCTION TGuiInterface.dll+00014800
void TGUIScreen::NotifyDescendantCantFocus( TGUIWidget* a_pWidget )
{
	RemoveFocusGrabs( a_pWidget );

	if ( m_pFocusedWidget && IsWidgetOrDescendant( m_pFocusedWidget, a_pWidget ) )
		ChangeWidgetFocus( a_pWidget->GetParentWidget(), TFALSE );

	if ( m_pWidgetUnderPointer && IsWidgetOrDescendant( m_pWidgetUnderPointer, a_pWidget ) )
	{
		WalkUpDownTree( m_pWidgetUnderPointer, a_pWidget->GetParentWidget(), pointer_out_callback, pointer_in_callback );

		TGUIRolloverEvent oEvent;
		oEvent.pOldWidget     = m_pWidgetUnderPointer;
		oEvent.pNewWidget     = a_pWidget;
		m_pWidgetUnderPointer = a_pWidget->GetParentWidget();
		m_oRolloverEmitter.Throw( &oEvent );
	}
}

// $NicktoonsPS2: FUNCTION 00368ff0
// $JPOG: FUNCTION TGuiInterface.dll+00015260
void TGUIScreen::NotifyDescendantCanFocus( TGUIWidget* a_pWidget )
{
}

// Clamps the pointer to the screen, returns TTRUE if it moved
// $NicktoonsPS2: FUNCTION 00368a80
// $JPOG: FUNCTION TGuiInterface.dll+00013fe0
TBOOL TGUIScreen::SetPointerPosition( TINT a_iX, TINT a_iY )
{
	TINT iX = TMath::Max( TMath::Min( a_iX, m_iWidth - 1 ), 0 );
	TINT iY = TMath::Max( TMath::Min( a_iY, m_iHeight - 1 ), 0 );

	if ( iX == m_iPointerX && iY == m_iPointerY )
		return TFALSE;

	m_iPointerX = iX;
	m_iPointerY = iY;
	return TTRUE;
}

// Moves the pointer with a relative move (accelerated) or to an absolute position
// NOTE: the absolute positions with float magnitudes use an integer for the vertical axis (as in the original)
// $NicktoonsPS2: FUNCTION 003688a8
// $JPOG: FUNCTION TGuiInterface.dll+00014460
TBOOL TGUIScreen::UpdatePointerPosition( TInputInterface::InputEvent* a_pEvent, TUINT a_uiActions )
{
	if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
	{
		TINT iX = 0;
		TINT iY = 0;

		if ( a_pEvent->IsMagnitudeFloat() )
		{
			iX = TINT( a_pEvent->GetMagnitudeFloat( 0 ) );
			iY = TINT( a_pEvent->GetMagnitudeFloat( 1 ) );
		}
		else
		{
			iX = a_pEvent->GetMagnitudeInt( 0 );
			iY = a_pEvent->GetMagnitudeInt( 1 );
		}

		ApplyPointerAccel( iX, iY );
		iX = TINT( TFLOAT( iX ) * m_fPointerSpeed );
		iY = TINT( TFLOAT( iY ) * m_fPointerSpeed );

		return SetPointerPosition( m_iPointerX + iX, m_iPointerY + iY );
	}
	else if ( a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_VIRTUAL )
	{
		if ( a_pEvent->IsMagnitudeFloat() )
		{
			TINT iX = m_iPointerX + TINT( a_pEvent->GetMagnitudeFloat( 0 ) * m_fPointerSpeed );
			TINT iY = m_iPointerY + TINT( TFLOAT( a_pEvent->GetMagnitudeInt( 1 ) ) * m_fPointerSpeed );
			return SetPointerPosition( iX, iY );
		}

		return SetPointerPosition( TINT( TFLOAT( a_pEvent->GetMagnitudeInt( 0 ) ) * m_fPointerSpeed ), TINT( TFLOAT( a_pEvent->GetMagnitudeInt( 1 ) ) * m_fPointerSpeed ) );
	}

	return TFALSE;
}

// a_pWidget is the new widget under the pointer (a_iX, a_iY in it), the pointer move goes to it and to its parents
// (to the pointer grab only if there's one)
// $NicktoonsPS2: FUNCTION 00368b98
// $JPOG: FUNCTION TGuiInterface.dll+000142f0
TBOOL TGUIScreen::RefreshPointerPosition( TGUIWidget* a_pWidget, TINT a_iX, TINT a_iY )
{
	if ( m_pPointerGrabWidget )
	{
		if ( a_pWidget != m_pWidgetUnderPointer )
		{
			if ( a_pWidget == m_pPointerGrabWidget )
				m_pPointerGrabWidget->OnPointerEnter();
			else if ( m_pWidgetUnderPointer == m_pPointerGrabWidget )
				m_pPointerGrabWidget->OnPointerExit();

			TGUIRolloverEvent oEvent;
			oEvent.pOldWidget     = m_pWidgetUnderPointer;
			oEvent.pNewWidget     = a_pWidget;
			m_pWidgetUnderPointer = a_pWidget;
			m_oRolloverEmitter.Throw( &oEvent );
		}

		m_pPointerGrabWidget->TransformDeviceToWidget( m_iPointerX, m_iPointerY, a_iX, a_iY );
		return m_pPointerGrabWidget->OnPointerMove( a_iX, a_iY );
	}

	TGUIWidget* pWidget = a_pWidget;
	TGUIWidget* pGrab   = m_aFocusGrabs[ m_aFocusGrabs.Size() - 1 ];

	if ( pGrab && !pGrab->IsAncestorOf( pWidget ) )
		pWidget = this;

	if ( pWidget != m_pWidgetUnderPointer )
	{
		WalkUpDownTree( m_pWidgetUnderPointer, pWidget, pointer_out_callback, pointer_in_callback );

		TGUIRolloverEvent oEvent;
		oEvent.pOldWidget     = m_pWidgetUnderPointer;
		oEvent.pNewWidget     = pWidget;
		m_pWidgetUnderPointer = pWidget;
		m_oRolloverEmitter.Throw( &oEvent );
	}

	for ( TGUIWidget* pTarget = m_pWidgetUnderPointer; pTarget; pTarget = pTarget->GetParentWidget() )
	{
		if ( pTarget->OnPointerMove( a_iX, a_iY ) == TTRUE )
			return TTRUE;

		a_iX += pTarget->GetXPos();
		a_iY += pTarget->GetYPos();
	}

	return TFALSE;
}

// Doubles the moves above the threshold (twice with the mode 2)
// $NicktoonsPS2: FUNCTION 00368d40
// $JPOG: FUNCTION TGuiInterface.dll+000150c0
void TGUIScreen::ApplyPointerAccel( TINT& a_rX, TINT& a_rY )
{
	if ( ( a_rX < 0 ? -a_rX : a_rX ) > m_iPointerAccelThreshold1 && m_iPointerAccelMode != 0 )
		a_rX *= 2;

	if ( ( a_rX < 0 ? -a_rX : a_rX ) > m_iPointerAccelThreshold1 && m_iPointerAccelMode == 2 )
		a_rX *= 2;

	if ( ( a_rY < 0 ? -a_rY : a_rY ) > m_iPointerAccelThreshold1 && m_iPointerAccelMode != 0 )
		a_rY *= 2;

	if ( ( a_rY < 0 ? -a_rY : a_rY ) > m_iPointerAccelThreshold1 && m_iPointerAccelMode == 2 )
		a_rY *= 2;
}

// $NicktoonsPS2: FUNCTION 00368b38
TBOOL TGUIScreen::SetPointerVisible( TBOOL a_bVisible )
{
	if ( a_bVisible == m_bPointerVisible )
		return TTRUE;

	m_bPointerVisible = a_bVisible;

	if ( a_bVisible == TTRUE )
		return SetPointerPosition( m_iWidth / 2, m_iHeight / 2 );

	return TTRUE;
}

// $JPOG: FUNCTION TGuiInterface.dll+00014b00
void TGUIScreen::AllowRolloverUpdate( TBOOL a_bAllow )
{
	if ( a_bAllow == m_bAllowRolloverUpdate )
		return;

	m_bAllowRolloverUpdate = a_bAllow;

	if ( a_bAllow )
		GetInputManager()->DoRolloverUpdate();
}

// $JPOG: FUNCTION TGuiInterface.dll+000147e0
void TGUIScreen::InvalidateWidgetUnderPointer()
{
	if ( m_bAllowRolloverUpdate )
		GetInputManager()->DoRolloverUpdate();
}

// $JPOG: FUNCTION TGuiInterface.dll+0000f840
TGUIWidget* TGUIScreen::GetWidgetUnderPointer()
{
	if ( !m_bRolloverUpdated )
		GetInputManager()->DoRolloverUpdate();

	return m_pWidgetUnderPointer;
}

// $NicktoonsPS2: FUNCTION 00367d90
// $JPOG: FUNCTION TGuiInterface.dll+00013a20
void TGUIScreen::SetInputContext( TGUIInputContext* a_pInputContext )
{
	if ( m_pInputContext )
		m_pInputContext->SetScreen( TNULL );

	m_pInputContext = a_pInputContext;

	if ( a_pInputContext )
		a_pInputContext->SetScreen( this );
}

// $NicktoonsPS2: FUNCTION 00368e10
// $JPOG: FUNCTION TGuiInterface.dll+00015160
void __stdcall TGUIScreen::pointer_out_callback( TGUIWidget* a_pWidget )
{
	a_pWidget->OnPointerExit();
}

// $NicktoonsPS2: FUNCTION 00368e38
// $JPOG: FUNCTION TGuiInterface.dll+00015170
void __stdcall TGUIScreen::pointer_in_callback( TGUIWidget* a_pWidget )
{
	a_pWidget->OnPointerEnter();
}

TOSHI_NAMESPACE_END
