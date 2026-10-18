#include "pch.h"
#include "AInteriorNetworkPlacementState.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include <Render/TCameraObject.h>
#  include "AGameStateController.h"
#  include "ASimState.h"
#  include "AMovieState.h"
#  include "AMessagePopupState.h"
#  include "ATerrainLoadingStallState.h"
#  include "SaveLoadSKU.h"
#  include "ASSeqState.h"
#  include "AMiniGameInstructionState.h"
#  include "AHintDisplayState.h"
#  include "ADebugMenuState.h"
#  include "Units/ASSeqController.h"
#  include "Helpers/AGameLoader.h"
#  include "Tasks/ARootTask.h"
#  include "ACheatActivator.h"
#  include "GUI/AGUI2MessageBoxState.h"
#  include "Cameras/ACamera.h"
#  include "Cameras/ACameraManager.h"
#  include "GUI/AGUISystem.h"
#  include "Render/ARenderer.h"
#  include "Render/AWorldDebugDraw.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorArea.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorGoal.h"

#  include <TGUI/TGUIManager.h>
#  include <TGUI/TGUIScreen.h>
#  include <TGUI/TGUIStyle.h>
#  include <TGUI/TGUIDisplayContext.h>
#  include <TGUI/TGUIColour.h>
#  include <Plugins/PGUI/PGUIButton.h>
#  include <Plugins/PGUI/PGUIToggleButton.h>
#  include <Plugins/PGUI/PGUISliderBar.h>
#  include <Input/TInputDeviceKeyboard.h>
#  include <Input/TInputDeviceMouse.h>
#  include <Render/TRenderInterface.h>
#  include <Toshi/TScheduler.h>
#  include <Toshi/TSystem.h>

#  include "Physics/ACollisionManager.h"

#  include <stdio.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AInteriorNetworkPlacementState );

TPSTRING8_DECLARE( AInteriorNetworkPlacementState );

// The camera commands held down (ProcessCommand starts their repeats, see UpdateCamera)
enum HELDCOMMAND_
{
	HELDCOMMAND_LEFT       = BITFLAG( 0 ),
	HELDCOMMAND_RIGHT      = BITFLAG( 1 ),
	HELDCOMMAND_FORWARD    = BITFLAG( 2 ),
	HELDCOMMAND_BACKWARD   = BITFLAG( 3 ),
	HELDCOMMAND_UP         = BITFLAG( 4 ),
	HELDCOMMAND_DOWN       = BITFLAG( 5 ),
	HELDCOMMAND_LOOK_LEFT  = BITFLAG( 6 ),
	HELDCOMMAND_LOOK_RIGHT = BITFLAG( 7 ),
	HELDCOMMAND_LOOK_UP    = BITFLAG( 8 ),
	HELDCOMMAND_LOOK_DOWN  = BITFLAG( 9 ),
};

static const TCHAR* s_aPanelNames[ AInteriorNetworkPlacementState::PANEL_NUMOF ] = {
	"OptionsPan",
	"OtherPan",
	"AreaPan",
	"PointPan",
};

static const TWCHAR* s_aPanelHelp[ AInteriorNetworkPlacementState::PANEL_NUMOF ] = {
	L"Grid and snap options, Save and Quit. W/A/S/D and Q/E fly, the arrows turn, C frees the camera.",
	L"Move All is not available.",
	L"Click an area to select it, < > change its cost.",
	L"Click a corner or a goal, hold the button to drag it. Del (numpad) deletes the selected corner.",
};

// Nicktoons: the labels of the link types (off_46c940)
static const struct
{
	const TWCHAR* wszName;
	TFLOAT        fX;
	TFLOAT        fZ;
} s_aGoalFacings[] = {
	{ L"Facing: POSX", 1.0f, 0.0f },
	{ L"Facing: POSZ", 0.0f, 1.0f },
	{ L"Facing: NEGX", -1.0f, 0.0f },
	{ L"Facing: NEGZ", 0.0f, -1.0f },
};

// $NicktoonsPS2: FUNCTION 002aca68
AInteriorNetworkPlacementState::AInteriorNetworkPlacementState()
    : m_eMode( MODE_SELECT )
    , m_pCamera( TNULL )
    , m_vCameraForward( TVector4::VEC_POSZ )
    , m_iHoverArea( -1 )
    , m_iSelectedArea( -1 )
    , m_bSnapToGrid( TFALSE )
    , m_bFreeCamera( TFALSE )
    , m_bDirty( TFALSE )
    , m_bQuit( TFALSE )
    , m_pNetwork( TNULL )
    , m_pOldCameraHelper( TNULL )
    , m_vCursor( TVector4::VEC_ZERO )
    , m_bCursorValid( TFALSE )
    , m_vDragStart( 0.0f, 0.0f )
    , m_iHeldCommands( 0 )
    , m_fGridSize( GRID_SIZE )
    , m_fDrawDistance( 1.0f )
    , m_fCostStep( 1.0f )
    , m_bCostHighlight( TFALSE )
    , m_pDialog( TNULL )
    , m_pDialog2( TNULL )
    , m_ePanel( PANEL_OPTIONS )
    , m_iNumButtons( 0 )
    , m_iNumMainButtons( 0 )
    , m_iNumTouchedAreas( 0 )
    , m_pfCornerHeights( TNULL )
    , m_piCornerOffsets( TNULL )
    , m_iNumEdits( 0 )
    , m_iNumUndone( 0 )
{
	for ( TINT i = 0; i < NUM_POINTER_BUTTONS; i++ )
	{
		m_abPointerButton[ i ]     = TFALSE;
		m_afPointerButtonTime[ i ] = 0.0f;
	}

	for ( TINT i = 0; i < NUM_HELD_COMMANDS; i++ )
	{
		m_apHeldDevices[ i ] = TNULL;
		m_aiHeldDoodads[ i ] = 0;
	}

	for ( TINT i = 0; i < LABEL_NUMOF; i++ )
		m_awszLabels[ i ][ 0 ] = L'\0';

	// Nicktoons: the input map "ANetworkEditState"
	SetInputMap( TPS8( AInteriorNetworkPlacementState ) );
}

// $NicktoonsPS2: FUNCTION 002acb88
AInteriorNetworkPlacementState::~AInteriorNetworkPlacementState()
{
	// NOTE: Nicktoons clears the list of the entities Toggle Entity Vis hid (Barnyard has no such list)
	delete[] m_pfCornerHeights;
	delete[] m_piCornerOffsets;
}

// $NicktoonsPS2: FUNCTION 002ad198
TBOOL AInteriorNetworkPlacementState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	AGUISystem* pGUISystem = AGUISystem::GetSingleton();
	TGUIScreen* pScreen    = pGUISystem->GetTGUIScreen();
	TBOOL       bUsed      = TTRUE;

	// The widgets get the events over them
	if ( pScreen && pScreen != pScreen->GetWidgetUnderPointer() )
		return BaseClass::ProcessInput( a_pInputEvent );

	if ( m_bFreeCamera )
	{
		// NOTE: Nicktoons gives the event to the camera (its free camera turns with the pad), here the mouse turns it
		if ( a_pInputEvent->GetDoodad() == TInputDeviceMouse::AXIS_CURSOR && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_MOVED )
		{
			TurnCamera( TFLOAT( a_pInputEvent->GetMagnitudeInt( 0 ) ) / 256.0f, TFLOAT( a_pInputEvent->GetMagnitudeInt( 1 ) ) / 256.0f );
			return TTRUE;
		}
	}

	// NOTE: Nicktoons also checks here that none of its buttons is under the pointer (GetWidgetUnderPointer covers it)

	switch ( a_pInputEvent->GetDoodad() )
	{
		case TInputDeviceKeyboard::KEY_DECIMAL:
			if ( m_eMode != MODE_SELECT )
				bUsed = TFALSE;
			else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
				OnDeleteSelected();
			break;
		case TInputDeviceMouse::BUTTON_1:
		case TInputDeviceMouse::BUTTON_2:
		{
			TINT iButton = ( a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_1 ) ? 0 : 1;

			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				m_abPointerButton[ iButton ] = TFALSE;
				OnPointerButtonUp( iButton );
			}
			else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_REPEAT )
			{
				OnPointerButtonHeld( iButton );
			}
			else if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				m_abPointerButton[ iButton ] = TTRUE;
				OnPointerButtonDown( iButton );
				a_pInputEvent->StartRepeat( 0.0f, 0.0f );
			}
			else
			{
				bUsed = TFALSE;
			}

			break;
		}
		case TInputDeviceKeyboard::KEY_C:
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
			{
				TBOOL bWasFree = m_bFreeCamera;
				m_bFreeCamera  = !m_bFreeCamera;

				if ( pScreen )
				{
					AGUISystem::TGUIMemoryScope oMemoryScope;
					pScreen->SetPointerVisible( bWasFree );
				}

				if ( !m_bFreeCamera )
				{
					m_vCameraForward = m_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
					m_vCameraForward.Normalise();
				}
			}

			break;
		default:
			bUsed = TFALSE;
			break;
	}

	if ( bUsed )
		return TTRUE;

	return BaseClass::ProcessInput( a_pInputEvent );
}

// $NicktoonsPS2: FUNCTION 002ad408
TBOOL AInteriorNetworkPlacementState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	// NOTE: Nicktoons moves the camera by one step (dt * 6.0) per event and repeats the commands every frame. Here the
	// commands only set what is held, UpdateCamera moves the camera every frame while the key that set it is down
	// (the input map sends no command for the up events).
	if ( !m_bFreeCamera )
	{
		TINT iHeld = 0;

		switch ( a_eInputCommand )
		{
			case AInputCommand_Left: iHeld = HELDCOMMAND_LEFT; break;
			case AInputCommand_Right: iHeld = HELDCOMMAND_RIGHT; break;
			case AInputCommand_Forward: iHeld = HELDCOMMAND_FORWARD; break;
			case AInputCommand_Backward: iHeld = HELDCOMMAND_BACKWARD; break;
			case AInputCommand_Up: iHeld = HELDCOMMAND_UP; break;
			case AInputCommand_Down: iHeld = HELDCOMMAND_DOWN; break;
			case AInputCommand_LookLeft: iHeld = HELDCOMMAND_LOOK_LEFT; break;
			case AInputCommand_LookRight: iHeld = HELDCOMMAND_LOOK_RIGHT; break;
			case AInputCommand_LookUp: iHeld = HELDCOMMAND_LOOK_UP; break;
			case AInputCommand_LookDown: iHeld = HELDCOMMAND_LOOK_DOWN; break;
			default: break;
		}

		if ( iHeld != 0 )
		{
			if ( a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_UP )
			{
				m_iHeldCommands &= ~iHeld;
			}
			else
			{
				// The mouse wheel has no up event: one step
				if ( a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_WHEEL_FORWARD || a_pInputEvent->GetDoodad() == TInputDeviceMouse::BUTTON_WHEEL_BACKWARD )
				{
					m_pCamera->GetMatrix().GetTranslation().Add( ACamera::sm_vWorldUp * ( ( iHeld == HELDCOMMAND_UP ) ? 1.0f : -1.0f ) );
				}
				else
				{
					m_iHeldCommands |= iHeld;

					// Remember the key: UpdateCamera releases the command when it's up
					TINT iBit = 0;
					while ( ( 1 << iBit ) != iHeld ) iBit++;

					m_apHeldDevices[ iBit ] = a_pInputEvent->GetSource();
					m_aiHeldDoodads[ iBit ] = a_pInputEvent->GetDoodad();
				}
			}
		}

		// NOTE: Nicktoons moves the selected node up and down (commands 50, 51, by 0.05), Barnyard's input map has no
		// such commands
	}

	// NOTE: Barnyard's input map has Quit (Escape), Nicktoons only quits with its Quit button
	if ( a_eInputCommand == AInputCommand_Quit && a_pInputEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN )
	{
		OnQuit();
		return TTRUE;
	}

	return BaseClass::ProcessCommand( a_eInputCommand, a_pInputEvent, a_rStopEvents );
}

// $NicktoonsPS2: FUNCTION 002acc20
TBOOL AInteriorNetworkPlacementState::OnUpdate( TFLOAT a_fDeltaTime )
{
	if ( m_bQuit )
	{
		AGameStateController::GetSingleton()->PopState( this );
		return TTRUE;
	}

	// NOTE: Nicktoons updates its network with the camera here (the nodes to draw). The draw options of the dialog:
	// the Draw Distance text box (times DRAW_DISTANCE) and the Highlight Cost toggle
	TCHAR szDrawDistance[ 32 ];

	if ( GetWidgetLabel( "DrawDist", szDrawDistance, sizeof( szDrawDistance ) ) )
	{
		TFLOAT fDrawDistance = TFLOAT( atof( szDrawDistance ) );

		if ( fDrawDistance > 0.0f )
			m_fDrawDistance = fDrawDistance;
	}

	m_bCostHighlight = IsToggled( "CostHighlight" );

	ACameraManager::GetSingleton()->DetachCameraHelpers();
	UpdateCamera( a_fDeltaTime );

	if ( m_eMode == MODE_SELECT && !m_bFreeCamera )
		UpdateHover();

	if ( !m_bFreeCamera )
	{
		m_bCursorValid = PickGround( m_vCursor, TNULL, TNULL );
		UpdatePositionLabel( m_bCursorValid ? &m_vCursor : TNULL );
	}

	for ( TINT i = 0; i < NUM_POINTER_BUTTONS; i++ )
	{
		if ( m_abPointerButton[ i ] )
			m_afPointerButtonTime[ i ] += a_fDeltaTime;
		else
			m_afPointerButtonTime[ i ] = 0.0f;
	}

	// NOTE: Nicktoons tells its entities that the editor is updated (Barnyard has no counterpart)
	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 002acfa0
void AInteriorNetworkPlacementState::OnInsertion()
{
	CreateDialogs();

	// NOTE: Nicktoons hides the buttons of the selection (Delete Link, Delete Node, Flood Check, Teleport) and the
	// labels of the link (Auto Built, Walkable Link) here, SelectArea( -1 ) does it for the area labels
	SelectArea( -1 );

	BaseClass::OnInsertion();
}

// $NicktoonsPS2: FUNCTION 002ad028
void AInteriorNetworkPlacementState::OnRemoval()
{
	DestroyDialogs();
	BaseClass::OnRemoval();
}

// $NicktoonsPS2: FUNCTION 002ad058
void AInteriorNetworkPlacementState::OnActivate()
{
	BaseClass::OnActivate();

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	m_pCamera                      = pCameraManager->GetCurrentCamera();
	m_vCameraForward               = m_pCamera->GetMatrix().AsBasisVector4( BASISVECTOR_FORWARD );
	m_vCameraForward.Normalise();

	// NOTE: Barnyard's camera helpers move the camera every frame, they're detached while the editor runs
	m_pOldCameraHelper = pCameraManager->GetCurrentCameraHelper();
	m_oOldCameraMatrix = m_pCamera->GetMatrix();
	pCameraManager->DetachCameraHelpers();

	// NOTE: Nicktoons turns the rendering of its network on here (and restores it in OnDeactivate), Render draws it
	for ( TINT i = 0; i < NUM_POINTER_BUTTONS; i++ )
	{
		m_abPointerButton[ i ]     = TFALSE;
		m_afPointerButtonTime[ i ] = 0.0f;
	}

	AGUISystem* pGUISystem = AGUISystem::GetSingleton();

	if ( pGUISystem->GetTGUIScreen() )
	{
		AGUISystem::TGUIMemoryScope oMemoryScope;
		pGUISystem->SetTGUIPointerMode( TTRUE );
		pGUISystem->GetTGUIScreen()->SetPointerVisible( TTRUE );
	}

	// NOTE: Nicktoons tells its entities that the editor starts (Barnyard has no counterpart)
	SetNetwork( m_pNetwork ? m_pNetwork : AInteriorNetworkManager::GetSingleton()->GetAnyNetwork() );

#  ifdef TRENDERINTERFACE_DX8
	// NOTE: the network is drawn in the world by the network layer of the world debug drawing
	if ( AWorldDebugDraw::IsSingletonCreated() )
		AWorldDebugDraw::GetSingleton()->SetNetworkEditor( this );
#  endif
}

// $NicktoonsPS2: FUNCTION 002ad118
void AInteriorNetworkPlacementState::OnDeactivate()
{
	m_pCamera = TNULL;
	BaseClass::OnDeactivate();

	AGUISystem* pGUISystem = AGUISystem::GetSingleton();

	if ( pGUISystem->GetTGUIScreen() )
	{
		AGUISystem::TGUIMemoryScope oMemoryScope;
		pGUISystem->SetTGUIPointerMode( TFALSE );
		pGUISystem->GetTGUIScreen()->SetPointerVisible( TFALSE );
	}

	ACameraManager* pCameraManager = ACameraManager::GetSingleton();
	pCameraManager->GetCurrentCamera()->SetMatrix( m_oOldCameraMatrix );

	if ( m_pOldCameraHelper )
	{
		pCameraManager->SetCameraHelper( m_pOldCameraHelper );
		m_pOldCameraHelper = TNULL;
	}

	m_iHeldCommands = 0;

#  ifdef TRENDERINTERFACE_DX8
	if ( AWorldDebugDraw::IsSingletonCreated() )
		AWorldDebugDraw::GetSingleton()->SetNetworkEditor( TNULL );
#  endif

	// NOTE: Nicktoons tells its entities that the editor stops (Barnyard has no counterpart)
}

// Shows the tools of the mode (Nicktoons shows its buttons in the select mode only)
// $NicktoonsPS2: FUNCTION 002ad8e0
void AInteriorNetworkPlacementState::SetMode( MODE a_eMode )
{
	// NOTE: Nicktoons turns the flood fill display of its network on in MODE_FLOOD_FILL and off when it leaves it

	if ( a_eMode == MODE_SELECT )
	{
		if ( m_pDialog )
		{
			AGUISystem::TGUIMemoryScope oMemoryScope;
			m_pDialog->SetEnabled( TTRUE );
		}

		// Nicktoons: the buttons of the selection are only shown if something is selected
		UpdateSelectionInfo();
	}
	else if ( m_pDialog )
	{
		AGUISystem::TGUIMemoryScope oMemoryScope;

		// NOTE: Nicktoons hides its buttons, the dialog is disabled (it stays in place)
		m_pDialog->SetEnabled( TFALSE );
	}

	m_eMode = a_eMode;
}

// Nicktoons creates its 18 buttons (style "window_edge", added to the GUI layer of the screen) with a callback each
// $NicktoonsPS2: FUNCTION 002acd60
void AInteriorNetworkPlacementState::CreateDialogs()
{
	AGUISystem*                 pGUISystem = AGUISystem::GetSingleton();
	AGUISystem::TGUIMemoryScope oMemoryScope;

	if ( !pGUISystem->CreateTGUI() )
		return;

	// NOTE: the dialogs are in Data/GUI/main.gui, loaded once (also by "-tguidemo")
	if ( !TGUIManager::GetSingleton()->FindTopLevelInstance( TPS8D( "AInteriorNetworkPlacementDialog" ), 0xffffffff ) )
		pGUISystem->LoadTGUIDefinitionFile( "Data/GUI/main.gui" );

	m_pDialog = pGUISystem->ShowTGUIDialog( TPS8D( "AInteriorNetworkPlacementDialog" ) );

	if ( !m_pDialog )
	{
		TWARN( "[networkedit] no dialog AInteriorNetworkPlacementDialog in Data/GUI/main.gui\n" );
		return;
	}

	TGUIScreen* pScreen = pGUISystem->GetTGUIScreen();

	// The screen grows (the GUI is drawn smaller) when the dialog doesn't fit, the dialog goes on the left
	TFLOAT fScale = TMath::Max( 1.0f, TFLOAT( m_pDialog->GetHeight() + 20 ) / AGUISystem::TGUI_SCREEN_HEIGHT );
	pScreen->SetFixedSize( TINT( AGUISystem::TGUI_SCREEN_WIDTH * fScale ), TINT( AGUISystem::TGUI_SCREEN_HEIGHT * fScale ) );
	m_pDialog->SetPosition( 10, 10 );
	pScreen->SetPointerPosition( pScreen->GetWidth() / 2, pScreen->GetHeight() / 2 );

	// NOTE: main.gui has a cursor texture section ("cursor_default") but its texture (HUD\hud1.tga) isn't loaded in the
	// sim (it's drawn white): the pointer is drawn by OnScreenPaint
	m_iNumButtons = 0;
	m_pDialog->RecurseTree( FindButtonsCallback, TNULL, TNULL, this );
	m_iNumMainButtons = m_iNumButtons;
	m_oPaintListener.Connect( pScreen->GetScreenPaintEmitter(), this, OnScreenPaint, 0 );

	ShowPanel( PANEL_POINT );

	TINFO( "[networkedit] dialog %dx%d, %d buttons, screen %dx%d\n", m_pDialog->GetWidth(), m_pDialog->GetHeight(), m_iNumButtons, pScreen->GetWidth(), pScreen->GetHeight() );
}

// $NicktoonsPS2: FUNCTION 002acf28
void AInteriorNetworkPlacementState::DestroyDialogs()
{
	AGUISystem*                 pGUISystem = AGUISystem::GetSingleton();
	AGUISystem::TGUIMemoryScope oMemoryScope;

	for ( TINT i = 0; i < m_iNumButtons; i++ )
		m_aButtonListeners[ i ].Disconnect();

	m_iNumButtons = 0;
	m_oPaintListener.Disconnect();

	if ( m_pDialog2 )
	{
		pGUISystem->HideTGUIDialog( m_pDialog2 );
		m_pDialog2 = TNULL;
	}

	if ( m_pDialog )
	{
		pGUISystem->HideTGUIDialog( m_pDialog );
		m_pDialog = TNULL;
	}

	if ( pGUISystem->GetTGUIScreen() )
		pGUISystem->GetTGUIScreen()->SetFixedSize( AGUISystem::TGUI_SCREEN_WIDTH, AGUISystem::TGUI_SCREEN_HEIGHT );
}

// HideUI: the main dialog is swapped with AInteriorNetworkPlacementDialog2 (a "Show UI" button) and back
void AInteriorNetworkPlacementState::ShowMainDialog( TBOOL a_bShow )
{
	AGUISystem*                 pGUISystem = AGUISystem::GetSingleton();
	AGUISystem::TGUIMemoryScope oMemoryScope;

	if ( !m_pDialog )
		return;

	m_pDialog->SetVisible( a_bShow );

	if ( a_bShow )
	{
		if ( m_pDialog2 )
		{
			// The listeners of its buttons are after the ones of the main dialog
			for ( TINT i = m_iNumMainButtons; i < m_iNumButtons; i++ )
				m_aButtonListeners[ i ].Disconnect();

			m_iNumButtons = m_iNumMainButtons;
			pGUISystem->HideTGUIDialog( m_pDialog2 );
			m_pDialog2 = TNULL;
		}
	}
	else if ( !m_pDialog2 )
	{
		m_pDialog2 = pGUISystem->ShowTGUIDialog( TPS8D( "AInteriorNetworkPlacementDialog2" ) );

		if ( m_pDialog2 )
		{
			m_pDialog2->SetPosition( 10, 10 );
			m_pDialog2->RecurseTree( FindButtonsCallback, TNULL, TNULL, this );
		}
	}
}

// The Options, Other, Area and Point buttons show one of the panels of the tools
void AInteriorNetworkPlacementState::ShowPanel( PANEL a_ePanel )
{
	AGUISystem::TGUIMemoryScope oMemoryScope;
	m_ePanel = a_ePanel;

	for ( TINT i = 0; i < PANEL_NUMOF; i++ )
	{
		TGUIWidget* pPanel = FindWidget( s_aPanelNames[ i ] );

		if ( pPanel )
			pPanel->SetVisible( i == a_ePanel );
	}

	SetWidgetLabel( "HelpText", s_aPanelHelp[ a_ePanel ] );

	if ( m_pDialog )
	{
		m_pDialog->Layout( -1, -1 );
		m_pDialog->Pack();
	}
}

// The ground under the pointer, TFALSE if the pointer ray doesn't hit it
// $NicktoonsPS2: FUNCTION 002ae2d0
TBOOL AInteriorNetworkPlacementState::PickGround( TVector4& a_rOutPosition, TVector4* a_pOutRayStart, TVector4* a_pOutRayDirection )
{
	TGUIScreen* pScreen = AGUISystem::GetSingleton()->GetTGUIScreen();

	if ( !pScreen || !m_pCamera )
		return TFALSE;

	TINT iPointerX;
	TINT iPointerY;
	pScreen->GetPointerPosition( iPointerX, iPointerY );

	// NOTE: Nicktoons PS2 reads the pointer position but the ray stays at zero (the code that makes the ray from the
	// screen position isn't in the PS2 build, so its picking never hits): the ray goes from the camera through the pointer
	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();

	TVector2 vScreenPos(
	    TFLOAT( iPointerX ) * TFLOAT( pDisplayParams->uiWidth ) / TFLOAT( pScreen->GetWidth() ),
	    TFLOAT( iPointerY ) * TFLOAT( pDisplayParams->uiHeight ) / TFLOAT( pScreen->GetHeight() )
	);

	TVector4 vRayStart;
	TVector4 vRayDirection;
	ARenderer::GetSingleton()->GetScreenRay( m_pCamera, vScreenPos, vRayStart, vRayDirection );

	if ( a_pOutRayStart )
		*a_pOutRayStart = vRayStart;

	if ( a_pOutRayDirection )
		*a_pOutRayDirection = vRayDirection;

	TFLOAT fDistance = PICK_DISTANCE;

	if ( !ACollisionManager::GetSingleton()->CollideRayWithTerrain( ARayData( vRayStart, vRayDirection, PICK_DISTANCE ), &fDistance, 0 ) )
		return TFALSE;

	a_rOutPosition = vRayStart;
	a_rOutPosition.x += vRayDirection.x * fDistance;
	a_rOutPosition.y += vRayDirection.y * fDistance;
	a_rOutPosition.z += vRayDirection.z * fDistance;

	if ( m_bSnapToGrid )
		SnapToGrid( a_rOutPosition );

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 002ae8e0
void AInteriorNetworkPlacementState::SnapToGrid( TVector4& a_rPosition )
{
	// NOTE: Nicktoons uses a fixed size of 4.0, here the one of the GridSlider
	TFLOAT fGrid = m_fGridSize;

	a_rPosition.x = TFLOAT( TMath::Round( a_rPosition.x / fGrid ) ) * fGrid;
	a_rPosition.z = TFLOAT( TMath::Round( a_rPosition.z / fGrid ) ) * fGrid;

	// The ground at the new position: up then down by 5 (Barnyard's world is y down)
	TFLOAT fDistance = 5.0f;
	TVector4 vDirection = ACamera::sm_vWorldUp * -1.0f;
	vDirection.w        = 1.0f;

	if ( ACollisionManager::GetSingleton()->CollideRayWithTerrain( ARayData( a_rPosition, ACamera::sm_vWorldUp, 5.0f ), &fDistance, 0 ) )
	{
		vDirection = ACamera::sm_vWorldUp;
	}
	else if ( !ACollisionManager::GetSingleton()->CollideRayWithTerrain( ARayData( a_rPosition, vDirection, 5.0f ), &fDistance, 0 ) )
	{
		return;
	}

	a_rPosition.x += vDirection.x * fDistance;
	a_rPosition.y += vDirection.y * fDistance;
	a_rPosition.z += vDirection.z * fDistance;
}

// Ray against an axis aligned box of half size a_fSize around a_rcCentre (Nicktoons: 00396788)
static TBOOL RayHitsBox( const TVector4& a_rcStart, const TVector4& a_rcDirection, const TVector4& a_rcCentre, TFLOAT a_fSize, TFLOAT& a_rfDistance )
{
	TFLOAT fNear = -1.0e10f;
	TFLOAT fFar  = a_rfDistance;

	for ( TINT i = 0; i < 3; i++ )
	{
		TFLOAT fStart = ( &a_rcStart.x )[ i ];
		TFLOAT fDir   = ( &a_rcDirection.x )[ i ];
		TFLOAT fMin   = ( &a_rcCentre.x )[ i ] - a_fSize;
		TFLOAT fMax   = ( &a_rcCentre.x )[ i ] + a_fSize;

		if ( TMath::Abs( fDir ) < 1.1920929e-7f )
		{
			if ( fStart < fMin || fStart > fMax )
				return TFALSE;

			continue;
		}

		TFLOAT fT1 = ( fMin - fStart ) / fDir;
		TFLOAT fT2 = ( fMax - fStart ) / fDir;

		if ( fT1 > fT2 )
		{
			TFLOAT fTemp = fT1;
			fT1          = fT2;
			fT2          = fTemp;
		}

		fNear = TMath::Max( fNear, fT1 );
		fFar  = TMath::Min( fFar, fT2 );

		if ( fNear > fFar || fFar < 0.0f )
			return TFALSE;
	}

	a_rfDistance = fNear;
	return TTRUE;
}

// The node (corner or goal) and the link (area) under the pointer
// $NicktoonsPS2: FUNCTION 002aea28
void AInteriorNetworkPlacementState::UpdateHover()
{
	m_oHover     = Element();
	m_iHoverArea = -1;

	if ( !m_pNetwork )
		return;

	TVector4 vGround;
	TVector4 vRayStart;
	TVector4 vRayDirection;
	TBOOL    bHit = PickGround( vGround, &vRayStart, &vRayDirection );

	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;
	const TVector4&             rcCamera  = m_pCamera->GetMatrix().GetTranslation();
	TFLOAT                      fMaxDist  = DRAW_DISTANCE * m_fDrawDistance;

	// The nodes the pointer ray goes through (Nicktoons: the closest 255 nodes within 500 of the camera)
	TFLOAT fClosest = PICK_DISTANCE;

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];

		if ( TMath::Abs( rcArea.m_fOriginX - rcCamera.x ) > fMaxDist || TMath::Abs( rcArea.m_fOriginZ - rcCamera.z ) > fMaxDist )
			continue;

		for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
		{
			Element oPoint;
			oPoint.eType  = ELEMENT_POINT;
			oPoint.iArea  = iArea;
			oPoint.iIndex = i;

			TFLOAT fDistance = fClosest;

			if ( RayHitsBox( vRayStart, vRayDirection, GetElementPosition( oPoint ), NODE_PICK_SIZE, fDistance ) && fDistance < fClosest )
			{
				fClosest = fDistance;
				m_oHover = oPoint;
			}
		}
	}

	for ( TINT i = 0; i < pINetwork->m_iNumGoals; i++ )
	{
		Element oGoal;
		oGoal.eType  = ELEMENT_GOAL;
		oGoal.iIndex = i;

		TFLOAT fDistance = fClosest;

		if ( RayHitsBox( vRayStart, vRayDirection, GetElementPosition( oGoal ), NODE_PICK_SIZE * 2.0f, fDistance ) && fDistance < fClosest )
		{
			fClosest = fDistance;
			m_oHover = oGoal;
		}
	}

	if ( m_oHover.IsValid() || !bHit )
		return;

	// The closest node to the ground under the pointer, a link going from it in the pointer's direction otherwise.
	// NOTE: Barnyard's links are areas, the area under the pointer is used when no node is close.
	Element oClosest;
	TFLOAT  fClosestSq = NODE_CLOSE_DISTANCE * NODE_CLOSE_DISTANCE;

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];

		for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
		{
			TFLOAT fDX = rcArea.m_pArcBounds[ i ].m_fX - vGround.x;
			TFLOAT fDZ = rcArea.m_pArcBounds[ i ].m_fZ - vGround.z;

			if ( fDX * fDX + fDZ * fDZ < fClosestSq )
			{
				fClosestSq      = fDX * fDX + fDZ * fDZ;
				oClosest.eType  = ELEMENT_POINT;
				oClosest.iArea  = iArea;
				oClosest.iIndex = i;
			}
		}
	}

	for ( TINT i = 0; i < pINetwork->m_iNumGoals; i++ )
	{
		TFLOAT fDX = pINetwork->m_pGoals[ i ].m_fPositionX - vGround.x;
		TFLOAT fDZ = pINetwork->m_pGoals[ i ].m_fPositionZ - vGround.z;

		if ( fDX * fDX + fDZ * fDZ < fClosestSq )
		{
			fClosestSq      = fDX * fDX + fDZ * fDZ;
			oClosest.eType  = ELEMENT_GOAL;
			oClosest.iArea  = -1;
			oClosest.iIndex = i;
		}
	}

	if ( oClosest.IsValid() )
	{
		m_oHover = oClosest;
		return;
	}

	m_iHoverArea = m_pNetwork->FindAreaContainingPointXZ( TVector2( vGround.x, vGround.z ) );
}

// $NicktoonsPS2: FUNCTION 002ae438
void AInteriorNetworkPlacementState::OnPointerButtonDown( TINT a_iButton )
{
	if ( a_iButton != 0 )
		return;

	// NOTE: Nicktoons adds a node where the pointer is in MODE_NEW_NODE and starts a flood fill from the closest node in
	// MODE_FLOOD_FILL, MODE_NEW_LINK selects the first node like MODE_SELECT
	if ( m_eMode != MODE_SELECT )
		return;

	if ( m_oHover.IsValid() )
	{
		m_oSelected = m_oHover;
		SelectArea( -1 );
	}
	else if ( m_iHoverArea != -1 )
	{
		m_oSelected = Element();
		SelectArea( m_iHoverArea );
	}
	else
	{
		m_oSelected = Element();
		SelectArea( -1 );
	}

	UpdateSelectionInfo();
}

// The button is held (its repeats)
// $NicktoonsPS2: FUNCTION 002ae6c0
void AInteriorNetworkPlacementState::OnPointerButtonHeld( TINT a_iButton )
{
	if ( a_iButton != 0 )
		return;

	if ( m_eMode == MODE_DRAG )
	{
		TVector4 vGround;

		if ( PickGround( vGround, TNULL, TNULL ) )
		{
			// NOTE: Nicktoons also unlinks and relinks the moved node in its network builder
			MoveElement( m_oDragged, vGround.x, vGround.z, TFALSE );
		}
	}
	else if ( m_eMode == MODE_SELECT )
	{
		// Held on the selected node: it's dragged
		if ( m_oSelected.IsValid() && m_oSelected == m_oHover && m_afPointerButtonTime[ 0 ] > DRAG_HOLD_TIME )
		{
			TVector4 vStart = GetElementPosition( m_oSelected );
			m_oDragged      = m_oSelected;
			m_vDragStart    = TVector2( vStart.x, vStart.z );
			SetMode( MODE_DRAG );
		}
	}

	// NOTE: Nicktoons draws the new link to the node under the pointer in MODE_NEW_LINK
}

// $NicktoonsPS2: FUNCTION 002ae7d0
void AInteriorNetworkPlacementState::OnPointerButtonUp( TINT a_iButton )
{
	if ( a_iButton == 1 )
	{
		// The right button cancels the modes of the graph tools (Nicktoons: new link, new node, flood fill)
		if ( m_eMode == MODE_NEW_LINK || m_eMode == MODE_NEW_NODE || m_eMode == MODE_FLOOD_FILL )
			SetMode( MODE_SELECT );

		return;
	}

	if ( a_iButton != 0 )
		return;

	if ( m_eMode == MODE_DRAG )
	{
		// NOTE: ours, the move can be undone
		TVector4 vEnd = GetElementPosition( m_oDragged );
		RecordEdit( m_oDragged, m_vDragStart, TVector2( vEnd.x, vEnd.z ) );
		m_bDirty = TTRUE;

		TINFO( "[networkedit] moved %s %d (area %d) from (%.2f, %.2f) to (%.2f, %.2f)\n", m_oDragged.eType == ELEMENT_GOAL ? "goal" : "point", m_oDragged.iIndex, m_oDragged.iArea, m_vDragStart.x, m_vDragStart.y, vEnd.x, vEnd.z );

		m_oDragged = Element();
		SetMode( MODE_SELECT );
	}

	// NOTE: Nicktoons links the selected node to the node under the pointer in MODE_NEW_LINK (Barnyard's areas have no
	// such tool)
}

// Nicktoons: shows the labels of the selected link (Auto/Manual linked, Walkable)
// $NicktoonsPS2: FUNCTION 002ae1e8
void AInteriorNetworkPlacementState::SelectArea( TINT a_iArea )
{
	m_iSelectedArea = a_iArea;
	UpdateSelectionInfo();
}

void AInteriorNetworkPlacementState::UpdateSelectionInfo()
{
	if ( !m_pDialog )
		return;

	AGUISystem::TGUIMemoryScope oMemoryScope;

	AInteriorNetwork::INetwork* pINetwork = m_pNetwork ? m_pNetwork->m_pINetwork : TNULL;
	TINT                        iArea     = m_iSelectedArea;

	if ( m_oSelected.eType == ELEMENT_POINT )
		iArea = m_oSelected.iArea;

	if ( pINetwork && iArea >= 0 && iArea < pINetwork->m_iNumAreas )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];
		TCHAR                szName[ 64 ];
		TStringManager::String8Format( szName, sizeof( szName ), "%d %s", iArea, ( rcArea.m_strName && rcArea.m_strName->GetString() ) ? rcArea.m_strName->GetString() : "" );
		TStringManager::StringCharToUnicode( m_awszLabels[ LABEL_AREA ], szName, -1 );
		SetWidgetLabel( "AreaType", m_awszLabels[ LABEL_AREA ] );

		TStringManager::String16Format( m_awszLabels[ LABEL_COST ], LABEL_LENGTH, L"%.1f", rcArea.m_fPathHeuristicScale );
		SetWidgetLabel( "CostFactor", m_awszLabels[ LABEL_COST ] );
	}
	else
	{
		SetWidgetLabel( "AreaType", L"" );
		SetWidgetLabel( "CostFactor", L"-" );
	}

	if ( pINetwork && m_oSelected.eType == ELEMENT_GOAL )
	{
		const AInteriorGoal& rcGoal  = pINetwork->m_pGoals[ m_oSelected.iIndex ];
		TINT                 iFacing = 0;
		TFLOAT               fBest   = -2.0f;

		for ( TINT i = 0; i < TARRAYSIZE( s_aGoalFacings ); i++ )
		{
			TFLOAT fDot = rcGoal.m_vForward.x * s_aGoalFacings[ i ].fX + rcGoal.m_vForward.z * s_aGoalFacings[ i ].fZ;

			if ( fDot > fBest )
			{
				fBest   = fDot;
				iFacing = i;
			}
		}

		SetWidgetLabel( "GoalFacing", s_aGoalFacings[ iFacing ].wszName );
	}

	TGUIWidget* pFacing = FindWidget( "GoalFacing" );

	if ( pFacing )
		pFacing->SetEnabled( m_oSelected.eType == ELEMENT_GOAL );

	TGUIWidget* pDelete = FindWidget( "DeletePoint" );

	if ( pDelete )
		pDelete->SetEnabled( m_oSelected.eType == ELEMENT_POINT );
}

// The Position label: "(x,z) Area n" of the ground under the pointer
void AInteriorNetworkPlacementState::UpdatePositionLabel( const TVector4* a_pPosition )
{
	if ( !m_pDialog )
		return;

	AGUISystem::TGUIMemoryScope oMemoryScope;

	if ( a_pPosition && m_pNetwork )
	{
		TINT iArea = m_pNetwork->FindAreaContainingPointXZ( TVector2( a_pPosition->x, a_pPosition->z ) );
		TStringManager::String16Format( m_awszLabels[ LABEL_POSITION ], LABEL_LENGTH, L"(%.2f,%.2f) Area %d", a_pPosition->x, a_pPosition->z, iArea );
	}
	else
	{
		TStringManager::String16Format( m_awszLabels[ LABEL_POSITION ], LABEL_LENGTH, L"-" );
	}

	SetWidgetLabel( "Position", m_awszLabels[ LABEL_POSITION ] );
}

void AInteriorNetworkPlacementState::SetNetwork( AInteriorNetwork* a_pNetwork )
{
	m_pNetwork      = a_pNetwork;
	m_oHover        = Element();
	m_oSelected     = Element();
	m_oDragged      = Element();
	m_iHoverArea    = -1;
	m_iSelectedArea = -1;
	m_iNumEdits     = 0;
	m_iNumUndone    = 0;
	CreateCornerHeights();

	if ( m_pNetwork )
	{
		TStringManager::StringCharToUnicode( m_awszLabels[ LABEL_NETWORK ], m_pNetwork->GetName().GetString(), -1 );
		SetWidgetLabel( "UnitName", m_awszLabels[ LABEL_NETWORK ] );

		TINFO( "[networkedit] network %s: %d areas, %d goals\n", m_pNetwork->GetName().GetString(), m_pNetwork->m_pINetwork->m_iNumAreas, m_pNetwork->m_pINetwork->m_iNumGoals );
	}
	else
	{
		TWARN( "[networkedit] no network to edit\n" );
	}

	UpdateSelectionInfo();
}

// World position of a corner (at the height of its area) or a goal
TVector4 AInteriorNetworkPlacementState::GetElementPosition( const Element& a_rcElement )
{
	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;

	if ( a_rcElement.eType == ELEMENT_POINT )
	{
		const AInteriorArcBounds& rcCorner = pINetwork->m_pAreas[ a_rcElement.iArea ].m_pArcBounds[ a_rcElement.iIndex ];
		return TVector4( rcCorner.m_fX, GetCornerHeight( a_rcElement.iArea, a_rcElement.iIndex ), rcCorner.m_fZ, 1.0f );
	}

	if ( a_rcElement.eType == ELEMENT_GOAL )
	{
		TVector4 vPosition;
		pINetwork->m_pGoals[ a_rcElement.iIndex ].GetPosition( vPosition );
		return vPosition;
	}

	return TVector4::VEC_ZERO;
}

// World height (y) of a corner, resolved like the network does for the units (kept until the corner moves)
TFLOAT AInteriorNetworkPlacementState::GetCornerHeight( TINT a_iArea, TINT a_iCorner )
{
	const AInteriorArea& rcArea = m_pNetwork->m_pINetwork->m_pAreas[ a_iArea ];
	TFLOAT*              pCache = m_pfCornerHeights ? &m_pfCornerHeights[ m_piCornerOffsets[ a_iArea ] + a_iCorner ] : TNULL;

	if ( pCache && *pCache != TMath::MAXFLOAT )
		return *pCache;

	TVector4 vPosition( rcArea.m_pArcBounds[ a_iCorner ].m_fX, 0.0f, rcArea.m_pArcBounds[ a_iCorner ].m_fZ, 1.0f );
	m_pNetwork->ResolvePointHeight( a_iArea, vPosition, 0 );

	if ( pCache )
		*pCache = vPosition.y;

	return vPosition.y;
}

void AInteriorNetworkPlacementState::InvalidateCornerHeights( TINT a_iArea )
{
	if ( !m_pfCornerHeights )
		return;

	for ( TINT i = 0; i < m_pNetwork->m_pINetwork->m_pAreas[ a_iArea ].m_iNumArcBounds; i++ )
		m_pfCornerHeights[ m_piCornerOffsets[ a_iArea ] + i ] = TMath::MAXFLOAT;
}

void AInteriorNetworkPlacementState::CreateCornerHeights()
{
	delete[] m_pfCornerHeights;
	delete[] m_piCornerOffsets;
	m_pfCornerHeights = TNULL;
	m_piCornerOffsets = TNULL;

	if ( !m_pNetwork )
		return;

	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;
	m_piCornerOffsets                     = new TINT[ pINetwork->m_iNumAreas ];
	TINT iNumCorners                      = 0;

	for ( TINT i = 0; i < pINetwork->m_iNumAreas; i++ )
	{
		m_piCornerOffsets[ i ] = iNumCorners;
		iNumCorners += pINetwork->m_pAreas[ i ].m_iNumArcBounds;
	}

	m_pfCornerHeights = new TFLOAT[ TMath::Max( iNumCorners, 1 ) ];

	for ( TINT i = 0; i < iNumCorners; i++ )
		m_pfCornerHeights[ i ] = TMath::MAXFLOAT;
}

void AInteriorNetworkPlacementState::MoveElement( const Element& a_rcElement, TFLOAT a_fX, TFLOAT a_fZ, TBOOL a_bRecord )
{
	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;

	if ( a_rcElement.eType == ELEMENT_POINT )
	{
		AInteriorArcBounds& rCorner = pINetwork->m_pAreas[ a_rcElement.iArea ].m_pArcBounds[ a_rcElement.iIndex ];

		// The Points toggle snaps the corner to another corner close to it
		if ( IsToggled( "PointSnap" ) )
		{
			TFLOAT fClosestSq = POINT_SNAP_DISTANCE * POINT_SNAP_DISTANCE;

			for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
			{
				const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];

				for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
				{
					const AInteriorArcBounds& rcOther = rcArea.m_pArcBounds[ i ];

					// Not the corners at the moved position
					if ( TMath::Abs( rcOther.m_fX - rCorner.m_fX ) < 0.01f && TMath::Abs( rcOther.m_fZ - rCorner.m_fZ ) < 0.01f )
						continue;

					TFLOAT fDX = rcOther.m_fX - a_fX;
					TFLOAT fDZ = rcOther.m_fZ - a_fZ;

					if ( fDX * fDX + fDZ * fDZ < fClosestSq )
					{
						fClosestSq = fDX * fDX + fDZ * fDZ;
						a_fX       = rcOther.m_fX;
						a_fZ       = rcOther.m_fZ;
					}
				}
			}
		}

		TVector2 vFrom( rCorner.m_fX, rCorner.m_fZ );
		MovePoints( rCorner.m_fX, rCorner.m_fZ, a_fX, a_fZ );

		if ( a_bRecord )
			RecordEdit( a_rcElement, vFrom, TVector2( a_fX, a_fZ ) );
	}
	else if ( a_rcElement.eType == ELEMENT_GOAL )
	{
		AInteriorGoal& rGoal = pINetwork->m_pGoals[ a_rcElement.iIndex ];
		TVector2       vFrom( rGoal.m_fPositionX, rGoal.m_fPositionZ );

		rGoal.m_fPositionX = a_fX;
		rGoal.m_fPositionZ = a_fZ;
		rGoal.m_fPositionY = TMath::MAXFLOAT;

		TINT iArea = m_pNetwork->FindAreaContainingPointXZ( TVector2( a_fX, a_fZ ) );

		if ( iArea != -1 )
			rGoal.m_iAreaIndex = iArea;

		if ( a_bRecord )
			RecordEdit( a_rcElement, vFrom, TVector2( a_fX, a_fZ ) );
	}
}

// Moves every corner at a_fFromX, a_fFromZ (the areas share their corners): the network stays connected
void AInteriorNetworkPlacementState::MovePoints( TFLOAT a_fFromX, TFLOAT a_fFromZ, TFLOAT a_fToX, TFLOAT a_fToZ )
{
	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		AInteriorArea& rArea  = pINetwork->m_pAreas[ iArea ];
		TBOOL          bMoved = TFALSE;

		for ( TINT i = 0; i < rArea.m_iNumArcBounds; i++ )
		{
			AInteriorArcBounds& rCorner = rArea.m_pArcBounds[ i ];

			if ( TMath::Abs( rCorner.m_fX - a_fFromX ) < 0.01f && TMath::Abs( rCorner.m_fZ - a_fFromZ ) < 0.01f )
			{
				rCorner.m_fX = a_fToX;
				rCorner.m_fZ = a_fToZ;
				bMoved       = TTRUE;

				if ( m_pfCornerHeights )
					m_pfCornerHeights[ m_piCornerOffsets[ iArea ] + i ] = TMath::MAXFLOAT;
			}
		}

		if ( bMoved )
		{
			UpdateArea( iArea );
			m_aiTouchedAreas[ m_iNumTouchedAreas++ % MAX_TOUCHED_AREAS ] = iArea;
		}
	}

	// The corners at both ends of the moved edges
	for ( TINT i = 0; i < TMath::Min( m_iNumTouchedAreas, MAX_TOUCHED_AREAS ); i++ )
		UpdateAreaCornerNormals( m_aiTouchedAreas[ i ] );

	m_iNumTouchedAreas = 0;
}

// Updates the edge and corner normals, the bounds and the origin of an area after its corners changed
void AInteriorNetworkPlacementState::UpdateArea( TINT a_iArea )
{
	AInteriorArea& rArea      = m_pNetwork->m_pINetwork->m_pAreas[ a_iArea ];
	TINT           iNumCorner = rArea.m_iNumArcBounds;

	if ( iNumCorner < 3 )
		return;

	// The winding of the area: its normals point inside
	TFLOAT fArea = 0.0f;

	for ( TINT i = 0; i < iNumCorner; i++ )
	{
		const AInteriorArcBounds& rcA = rArea.m_pArcBounds[ i ];
		const AInteriorArcBounds& rcB = rArea.m_pArcBounds[ ( i + 1 ) % iNumCorner ];
		fArea += rcA.m_fX * rcB.m_fZ - rcB.m_fX * rcA.m_fZ;
	}

	TFLOAT fSide = ( fArea >= 0.0f ) ? 1.0f : -1.0f;

	rArea.m_fMinX = rArea.m_fMaxX = rArea.m_pArcBounds[ 0 ].m_fX;
	rArea.m_fMinZ = rArea.m_fMaxZ = rArea.m_pArcBounds[ 0 ].m_fZ;

	TFLOAT fOriginX = 0.0f;
	TFLOAT fOriginZ = 0.0f;

	for ( TINT i = 0; i < iNumCorner; i++ )
	{
		AInteriorArcBounds&       rA  = rArea.m_pArcBounds[ i ];
		const AInteriorArcBounds& rcB = rArea.m_pArcBounds[ ( i + 1 ) % iNumCorner ];

		TVector2 vEdge( rcB.m_fX - rA.m_fX, rcB.m_fZ - rA.m_fZ );
		TFLOAT   fLength = vEdge.Magnitude();

		if ( fLength > 0.0001f )
		{
			rA.m_fNormalX = -vEdge.y / fLength * fSide;
			rA.m_fNormalZ = vEdge.x / fLength * fSide;
		}

		rArea.m_fMinX = TMath::Min( rArea.m_fMinX, rA.m_fX );
		rArea.m_fMaxX = TMath::Max( rArea.m_fMaxX, rA.m_fX );
		rArea.m_fMinZ = TMath::Min( rArea.m_fMinZ, rA.m_fZ );
		rArea.m_fMaxZ = TMath::Max( rArea.m_fMaxZ, rA.m_fZ );

		fOriginX += rA.m_fX;
		fOriginZ += rA.m_fZ;
	}

	// NOTE: the origins of the areas aren't their centres (a point inside of the area picked by the tools), it's only
	// moved to the average of the corners if it leaves the area
	TVector4 vOrigin( rArea.m_fOriginX, 0.0f, rArea.m_fOriginZ, 1.0f );

	if ( !rArea.CheckPositionInsideArea( vOrigin ) )
	{
		rArea.m_fOriginX = fOriginX / TFLOAT( iNumCorner );
		rArea.m_fOriginZ = fOriginZ / TFLOAT( iNumCorner );
	}
}

// The corner normal of the corners at a_fX, a_fZ: the average of the normals of the outer edges (no neighbour) of all
// the areas that start or end there, zero if there's none.
// NOTE: rule found from the data (park2: 26 of the 30 checked corners match, the others look stale in the file)
void AInteriorNetworkPlacementState::UpdateCornerNormals( TFLOAT a_fX, TFLOAT a_fZ )
{
	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;
	TVector2                    vNormal( 0.0f, 0.0f );

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];

		for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
		{
			const AInteriorArcBounds& rcA = rcArea.m_pArcBounds[ i ];
			const AInteriorArcBounds& rcB = rcArea.m_pArcBounds[ ( i + 1 ) % rcArea.m_iNumArcBounds ];

			if ( rcA.m_iNeighborArea != -1 )
				continue;

			TBOOL bStart = TMath::Abs( rcA.m_fX - a_fX ) < 0.01f && TMath::Abs( rcA.m_fZ - a_fZ ) < 0.01f;
			TBOOL bEnd   = TMath::Abs( rcB.m_fX - a_fX ) < 0.01f && TMath::Abs( rcB.m_fZ - a_fZ ) < 0.01f;

			if ( bStart || bEnd )
			{
				vNormal.x += rcA.m_fNormalX;
				vNormal.y += rcA.m_fNormalZ;
			}
		}
	}

	TFLOAT fLength = vNormal.Magnitude();

	if ( fLength > 0.0001f )
	{
		vNormal.x /= fLength;
		vNormal.y /= fLength;
	}
	else
	{
		vNormal = TVector2( 0.0f, 0.0f );
	}

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		AInteriorArea& rArea = pINetwork->m_pAreas[ iArea ];

		for ( TINT i = 0; i < rArea.m_iNumArcBounds; i++ )
		{
			AInteriorArcBounds& rCorner = rArea.m_pArcBounds[ i ];

			if ( TMath::Abs( rCorner.m_fX - a_fX ) < 0.01f && TMath::Abs( rCorner.m_fZ - a_fZ ) < 0.01f )
			{
				rCorner.m_fCornerNormalX = vNormal.x;
				rCorner.m_fCornerNormalZ = vNormal.y;
			}
		}
	}
}

// The corner normals of all the corners of an area (after its edges changed)
void AInteriorNetworkPlacementState::UpdateAreaCornerNormals( TINT a_iArea )
{
	const AInteriorArea& rcArea = m_pNetwork->m_pINetwork->m_pAreas[ a_iArea ];

	for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
		UpdateCornerNormals( rcArea.m_pArcBounds[ i ].m_fX, rcArea.m_pArcBounds[ i ].m_fZ );
}

// Removes the selected corner from the areas that have it (and keep 3 corners)
TBOOL AInteriorNetworkPlacementState::DeletePoint( const Element& a_rcElement )
{
	if ( a_rcElement.eType != ELEMENT_POINT )
		return TFALSE;

	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;
	const AInteriorArcBounds&   rcCorner  = pINetwork->m_pAreas[ a_rcElement.iArea ].m_pArcBounds[ a_rcElement.iIndex ];
	TFLOAT                      fX        = rcCorner.m_fX;
	TFLOAT                      fZ        = rcCorner.m_fZ;
	TINT                        iDeleted  = 0;

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		AInteriorArea& rArea = pINetwork->m_pAreas[ iArea ];

		for ( TINT i = 0; i < rArea.m_iNumArcBounds; i++ )
		{
			if ( TMath::Abs( rArea.m_pArcBounds[ i ].m_fX - fX ) >= 0.01f || TMath::Abs( rArea.m_pArcBounds[ i ].m_fZ - fZ ) >= 0.01f )
				continue;

			if ( rArea.m_iNumArcBounds <= 3 )
				break;

			// The edge before the corner now goes to the next corner, it keeps its neighbour if both edges had the same
			TINT iPrev = ( i + rArea.m_iNumArcBounds - 1 ) % rArea.m_iNumArcBounds;

			if ( rArea.m_pArcBounds[ iPrev ].m_iNeighborArea != rArea.m_pArcBounds[ i ].m_iNeighborArea )
				rArea.m_pArcBounds[ iPrev ].m_iNeighborArea = -1;

			for ( TINT k = i; k < rArea.m_iNumArcBounds - 1; k++ )
				rArea.m_pArcBounds[ k ] = rArea.m_pArcBounds[ k + 1 ];

			InvalidateCornerHeights( iArea );

			rArea.m_iNumArcBounds--;
			UpdateArea( iArea );
			UpdateAreaCornerNormals( iArea );
			iDeleted++;
			break;
		}
	}

	TINFO( "[networkedit] deleted the corner at (%.2f, %.2f) from %d areas\n", fX, fZ, iDeleted );
	return iDeleted > 0;
}

// Save writes a text dump of the network to the "-devnetworksave" path (nothing without it)
// NOTE: Nicktoons' Save button does nothing (its callback returns at once, 002adcf8)
TBOOL AInteriorNetworkPlacementState::SaveNetwork()
{
	const TCHAR* szArg = TStringManager::String8FindString( GetCommandLineA(), "-devnetworksave " );

	if ( !szArg || !m_pNetwork )
	{
		TWARN( "[networkedit] Save: no -devnetworksave <path> on the command line, nothing written\n" );
		return TFALSE;
	}

	TCHAR szPath[ 260 ];
	szArg += TStringManager::String8Length( "-devnetworksave " );
	TINT iLength = 0;

	while ( szArg[ iLength ] != '\0' && szArg[ iLength ] != ' ' && iLength < TARRAYSIZE( szPath ) - 1 )
	{
		szPath[ iLength ] = szArg[ iLength ];
		iLength++;
	}

	szPath[ iLength ] = '\0';

	FILE* pFile = fopen( szPath, "w" );

	if ( !pFile )
	{
		TWARN( "[networkedit] Save: can't write %s\n", szPath );
		return TFALSE;
	}

	AInteriorNetwork::INetwork* pINetwork = m_pNetwork->m_pINetwork;

	fprintf( pFile, "; OpenBarnyard interior network editor dump\n" );
	fprintf( pFile, "network \"%s\" areas %d goals %d special %d\n", m_pNetwork->GetName().GetString(), pINetwork->m_iNumAreas, pINetwork->m_iNumGoals, pINetwork->m_iNumSpecialAreas );

	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];
		fprintf( pFile, "area %d \"%s\" flags 0x%x height %.3f cost %.3f corners %d\n", iArea, ( rcArea.m_strName && rcArea.m_strName->GetString() ) ? rcArea.m_strName->GetString() : "", rcArea.m_eFlags, rcArea.m_fBaseHeight, rcArea.m_fPathHeuristicScale, rcArea.m_iNumArcBounds );

		for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
			fprintf( pFile, "  corner %.3f %.3f neighbour %d\n", rcArea.m_pArcBounds[ i ].m_fX, rcArea.m_pArcBounds[ i ].m_fZ, rcArea.m_pArcBounds[ i ].m_iNeighborArea );
	}

	for ( TINT i = 0; i < pINetwork->m_iNumGoals; i++ )
	{
		const AInteriorGoal& rcGoal = pINetwork->m_pGoals[ i ];
		fprintf( pFile, "goal %d \"%s\" position %.3f %.3f facing %.3f %.3f area %d\n", i, ( rcGoal.m_strName && rcGoal.m_strName->GetString() ) ? rcGoal.m_strName->GetString() : "", rcGoal.m_fPositionX, rcGoal.m_fPositionZ, rcGoal.m_vForward.x, rcGoal.m_vForward.z, rcGoal.m_iAreaIndex );
	}

	fclose( pFile );
	m_bDirty = TFALSE;

	TINFO( "[networkedit] saved %s to %s\n", m_pNetwork->GetName().GetString(), szPath );
	return TTRUE;
}

void AInteriorNetworkPlacementState::RecordEdit( const Element& a_rcElement, const TVector2& a_rcFrom, const TVector2& a_rcTo )
{
	// A new edit drops the undone ones
	m_iNumEdits -= m_iNumUndone;
	m_iNumUndone = 0;

	if ( m_iNumEdits == MAX_EDITS )
	{
		for ( TINT i = 1; i < MAX_EDITS; i++ )
			m_aEdits[ i - 1 ] = m_aEdits[ i ];

		m_iNumEdits--;
	}

	Edit& rEdit    = m_aEdits[ m_iNumEdits++ ];
	rEdit.oElement = a_rcElement;
	rEdit.vFrom    = a_rcFrom;
	rEdit.vTo      = a_rcTo;
}

void AInteriorNetworkPlacementState::UndoEdit()
{
	if ( m_iNumUndone >= m_iNumEdits )
		return;

	m_iNumUndone++;
	const Edit& rcEdit = m_aEdits[ m_iNumEdits - m_iNumUndone ];

	if ( rcEdit.oElement.eType == ELEMENT_POINT )
		MovePoints( rcEdit.vTo.x, rcEdit.vTo.y, rcEdit.vFrom.x, rcEdit.vFrom.y );
	else
		MoveElement( rcEdit.oElement, rcEdit.vFrom.x, rcEdit.vFrom.y, TFALSE );

	TINFO( "[networkedit] undo: (%.2f, %.2f) back to (%.2f, %.2f)\n", rcEdit.vTo.x, rcEdit.vTo.y, rcEdit.vFrom.x, rcEdit.vFrom.y );
}

void AInteriorNetworkPlacementState::RedoEdit()
{
	if ( m_iNumUndone <= 0 )
		return;

	const Edit& rcEdit = m_aEdits[ m_iNumEdits - m_iNumUndone ];
	m_iNumUndone--;

	if ( rcEdit.oElement.eType == ELEMENT_POINT )
		MovePoints( rcEdit.vFrom.x, rcEdit.vFrom.y, rcEdit.vTo.x, rcEdit.vTo.y );
	else
		MoveElement( rcEdit.oElement, rcEdit.vTo.x, rcEdit.vTo.y, TFALSE );
}

// Nicktoons (ProcessCommand): moves along the forward and the side of the camera, up and down, turns around the up axis
void AInteriorNetworkPlacementState::UpdateCamera( TFLOAT a_fDeltaTime )
{
	// Release the commands whose keys are up (no command comes for the up events)
	for ( TINT i = 0; i < NUM_HELD_COMMANDS; i++ )
	{
		if ( ( m_iHeldCommands & ( 1 << i ) ) && ( !m_apHeldDevices[ i ] || !m_apHeldDevices[ i ]->IsDown( m_aiHeldDoodads[ i ] ) ) )
			m_iHeldCommands &= ~( 1 << i );
	}

	if ( !m_pCamera || m_bFreeCamera || m_iHeldCommands == 0 )
		return;

	TMatrix44& rMatrix = m_pCamera->GetMatrix();
	TVector4&  rPos    = rMatrix.GetTranslation();
	TFLOAT     fStep   = a_fDeltaTime * CAMERA_SPEED;

	TVector4 vRight;
	vRight.CrossProduct( ACamera::sm_vWorldUp, m_vCameraForward );
	vRight.Normalise();

	// Nicktoons: cross( up, forward ), Barnyard's up is -y
	if ( m_iHeldCommands & HELDCOMMAND_LEFT ) rPos.Add( vRight * -fStep );
	if ( m_iHeldCommands & HELDCOMMAND_RIGHT ) rPos.Add( vRight * fStep );
	if ( m_iHeldCommands & HELDCOMMAND_FORWARD ) rPos.Add( m_vCameraForward * fStep );
	if ( m_iHeldCommands & HELDCOMMAND_BACKWARD ) rPos.Add( m_vCameraForward * -fStep );
	if ( m_iHeldCommands & HELDCOMMAND_UP ) rPos.Add( ACamera::sm_vWorldUp * fStep );
	if ( m_iHeldCommands & HELDCOMMAND_DOWN ) rPos.Add( ACamera::sm_vWorldUp * -fStep );
	rPos.w = 1.0f;

	TFLOAT fTurn = a_fDeltaTime * CAMERA_TURN_SPEED;
	TFLOAT fYaw  = 0.0f;
	TFLOAT fPitch = 0.0f;

	if ( m_iHeldCommands & HELDCOMMAND_LOOK_LEFT ) fYaw -= fTurn;
	if ( m_iHeldCommands & HELDCOMMAND_LOOK_RIGHT ) fYaw += fTurn;

	// NOTE: ours, Nicktoons only turns around the up axis
	if ( m_iHeldCommands & HELDCOMMAND_LOOK_UP ) fPitch -= fTurn;
	if ( m_iHeldCommands & HELDCOMMAND_LOOK_DOWN ) fPitch += fTurn;

	if ( fYaw != 0.0f || fPitch != 0.0f )
		TurnCamera( fYaw, fPitch );
}

void AInteriorNetworkPlacementState::TurnCamera( TFLOAT a_fYaw, TFLOAT a_fPitch )
{
	TMatrix44& rMatrix = m_pCamera->GetMatrix();
	TVector4   vForward = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );

	// Around the up axis
	TMatrix44 oYaw;
	oYaw.Identity();
	oYaw.RotateY( a_fYaw );

	TVector4 vNewForward;
	TMatrix44::RotateVector( vNewForward, oYaw, vForward );

	// Around the side axis (kept away from straight up and down)
	TFLOAT fHorizontal = TMath::Sqrt( vNewForward.x * vNewForward.x + vNewForward.z * vNewForward.z );
	TFLOAT fAngle      = TMath::ATan2( vNewForward.y, fHorizontal ) + a_fPitch;
	fAngle             = TMath::Max( -1.4f, TMath::Min( 1.4f, fAngle ) );

	if ( fHorizontal > 0.0001f )
	{
		TFLOAT fScale = TMath::Cos( fAngle ) / fHorizontal;
		vNewForward.x *= fScale;
		vNewForward.z *= fScale;
		vNewForward.y = TMath::Sin( fAngle );
	}

	vNewForward.w = 1.0f;
	vNewForward.Normalise();

	TVector4 vPosition = rMatrix.GetTranslation();
	rMatrix.LookAtDirection( vNewForward, ACamera::sm_vWorldUp );
	rMatrix.SetTranslation( vPosition );

	m_vCameraForward = rMatrix.AsBasisVector4( BASISVECTOR_FORWARD );
	m_vCameraForward.Normalise();
}

// Draws the areas (outer edges white, shared edges grey), their corners and the goals around the camera
void AInteriorNetworkPlacementState::Render( TGUIScreen* a_pScreen )
{
	if ( !m_pNetwork || !m_pCamera || !m_pDialog )
		return;

	TGUIDisplayContext*              pContext       = a_pScreen->GetDisplayContext();
	ARenderer*                       pRenderer      = ARenderer::GetSingleton();
	TRenderInterface::DISPLAYPARAMS* pDisplayParams = TRenderInterface::GetSingleton()->GetCurrentDisplayParams();
	AInteriorNetwork::INetwork*      pINetwork      = m_pNetwork->m_pINetwork;

	TFLOAT fScaleX = TFLOAT( a_pScreen->GetWidth() ) / TFLOAT( pDisplayParams->uiWidth );
	TFLOAT fScaleY = TFLOAT( a_pScreen->GetHeight() ) / TFLOAT( pDisplayParams->uiHeight );

	const TVector4& rcCamera = m_pCamera->GetMatrix().GetTranslation();
	TFLOAT          fMaxDist = DRAW_DISTANCE * m_fDrawDistance;

	auto fnProject = [ & ]( const TVector4& a_rcPosition, TINT& a_rX, TINT& a_rY ) -> TBOOL {
		TVector2 vScreen;

		if ( !pRenderer->ProjectToScreen( m_pCamera, a_rcPosition, vScreen ) )
			return TFALSE;

		a_rX = TINT( vScreen.x * fScaleX );
		a_rY = TINT( vScreen.y * fScaleY );
		return TTRUE;
	};

	auto fnCross = [ & ]( const TVector4& a_rcPosition, TINT a_iSize, const TGUIColour& a_rcColour ) {
		TINT iX, iY;

		if ( fnProject( a_rcPosition, iX, iY ) )
		{
			pContext->DrawLine( iX - a_iSize, iY - a_iSize, iX + a_iSize, iY + a_iSize, a_rcColour );
			pContext->DrawLine( iX - a_iSize, iY + a_iSize, iX + a_iSize, iY - a_iSize, a_rcColour );
		}
	};

	const TGUIColour oOuter( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );
	const TGUIColour oShared( TUINT8( 110 ), TUINT8( 140 ), TUINT8( 255 ), TUINT8( 255 ) );
	const TGUIColour oHover( TUINT8( 255 ), TUINT8( 230 ), TUINT8( 0 ), TUINT8( 255 ) );
	const TGUIColour oSelected( TUINT8( 0 ), TUINT8( 255 ), TUINT8( 0 ), TUINT8( 255 ) );
	const TGUIColour oPoint( TUINT8( 255 ), TUINT8( 80 ), TUINT8( 80 ), TUINT8( 255 ) );
	const TGUIColour oGoalColour( TUINT8( 255 ), TUINT8( 0 ), TUINT8( 255 ), TUINT8( 255 ) );
	const TGUIColour oCursor( TUINT8( 0 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );

#  ifndef TRENDERINTERFACE_DX8
	// NOTE: with DX8 the areas and the goals are drawn in the world, with depth (AWorldDebugDraw)
	for ( TINT iArea = 0; iArea < pINetwork->m_iNumAreas; iArea++ )
	{
		const AInteriorArea& rcArea = pINetwork->m_pAreas[ iArea ];

		if ( TMath::Abs( rcArea.m_fOriginX - rcCamera.x ) > fMaxDist || TMath::Abs( rcArea.m_fOriginZ - rcCamera.z ) > fMaxDist )
			continue;

		TBOOL  bArea   = ( iArea == m_iSelectedArea ) || ( iArea == m_iHoverArea );

		for ( TINT i = 0; i < rcArea.m_iNumArcBounds; i++ )
		{
			const AInteriorArcBounds& rcA = rcArea.m_pArcBounds[ i ];
			TINT                      iNext = ( i + 1 ) % rcArea.m_iNumArcBounds;
			const AInteriorArcBounds& rcB   = rcArea.m_pArcBounds[ iNext ];
			TFLOAT                    fHeightA = GetCornerHeight( iArea, i );
			TFLOAT                    fHeightB = GetCornerHeight( iArea, iNext );

			TINT iX1, iY1, iX2, iY2;

			if ( fnProject( TVector4( rcA.m_fX, fHeightA, rcA.m_fZ, 1.0f ), iX1, iY1 ) && fnProject( TVector4( rcB.m_fX, fHeightB, rcB.m_fZ, 1.0f ), iX2, iY2 ) )
			{
				const TGUIColour& rcColour = ( iArea == m_iSelectedArea ) ? oSelected : ( iArea == m_iHoverArea ) ? oHover : ( rcA.m_iNeighborArea == -1 ) ? oOuter : oShared;
				pContext->DrawLine( iX1, iY1, iX2, iY2, rcColour );

				// Thicker for the selection
				if ( bArea )
					pContext->DrawLine( iX1, iY1 + 1, iX2, iY2 + 1, rcColour );
			}

			fnCross( TVector4( rcA.m_fX, fHeightA, rcA.m_fZ, 1.0f ), 2, oPoint );
		}
	}

	for ( TINT i = 0; i < pINetwork->m_iNumGoals; i++ )
	{
		const AInteriorGoal& rcGoal = pINetwork->m_pGoals[ i ];

		if ( TMath::Abs( rcGoal.m_fPositionX - rcCamera.x ) > fMaxDist || TMath::Abs( rcGoal.m_fPositionZ - rcCamera.z ) > fMaxDist )
			continue;

		Element oGoal;
		oGoal.eType  = ELEMENT_GOAL;
		oGoal.iIndex = i;

		TVector4 vPosition = GetElementPosition( oGoal );
		TINT     iX, iY, iX2, iY2;

		if ( fnProject( vPosition, iX, iY ) )
		{
			pContext->DrawLine( iX - 4, iY, iX, iY - 4, oGoalColour );
			pContext->DrawLine( iX, iY - 4, iX + 4, iY, oGoalColour );
			pContext->DrawLine( iX + 4, iY, iX, iY + 4, oGoalColour );
			pContext->DrawLine( iX, iY + 4, iX - 4, iY, oGoalColour );

			// The facing
			TVector4 vFacing( vPosition.x + rcGoal.m_vForward.x * 0.75f, vPosition.y, vPosition.z + rcGoal.m_vForward.z * 0.75f, 1.0f );

			if ( fnProject( vFacing, iX2, iY2 ) )
				pContext->DrawLine( iX, iY, iX2, iY2, oGoalColour );
		}
	}
#  endif

	if ( m_oHover.IsValid() )
		fnCross( GetElementPosition( m_oHover ), 6, oHover );

	if ( m_oSelected.IsValid() )
		fnCross( GetElementPosition( m_oSelected ), 8, oSelected );

	if ( m_bCursorValid && !m_bFreeCamera )
		fnCross( m_vCursor, 4, oCursor );
}

TGUIWidget* AInteriorNetworkPlacementState::FindWidget( const TCHAR* a_szName ) const
{
	TGUIWidget* pWidget = m_pDialog ? m_pDialog->FindNamedChild( a_szName ) : TNULL;

	if ( !pWidget && m_pDialog2 )
		pWidget = m_pDialog2->FindNamedChild( a_szName );

	return pWidget;
}

void AInteriorNetworkPlacementState::SetWidgetLabel( TGUIWidget* a_pWidget, const TWCHAR* a_wszLabel )
{
	if ( !a_pWidget || !a_pWidget->GetStyle() )
		return;

	TINT iParameter = a_pWidget->GetStyle()->FindParameter( TPS8D( "LABEL" ) );

	if ( iParameter >= 0 )
		a_pWidget->SetParameter( iParameter, a_wszLabel );
}

void AInteriorNetworkPlacementState::SetWidgetLabel( const TCHAR* a_szName, const TWCHAR* a_wszLabel )
{
	SetWidgetLabel( FindWidget( a_szName ), a_wszLabel );
}

TBOOL AInteriorNetworkPlacementState::GetWidgetLabel( const TCHAR* a_szName, TCHAR* a_szOutText, TINT a_iMaxLength ) const
{
	a_szOutText[ 0 ] = '\0';

	TGUIWidget* pWidget = FindWidget( a_szName );

	if ( !pWidget || !pWidget->GetStyle() )
		return TFALSE;

	TINT iParameter = pWidget->GetStyle()->FindParameter( TPS8D( "LABEL" ) );

	if ( iParameter < 0 )
		return TFALSE;

	PPropertyValue oValue = pWidget->GetParameter( iParameter );

	if ( oValue.GetType() != PPropertyValue::TYPE_STRING16 || !oValue.GetString16() )
		return TFALSE;

	TStringManager::StringUnicodeToChar( a_szOutText, oValue.GetString16(), a_iMaxLength - 1 );
	a_szOutText[ a_iMaxLength - 1 ] = '\0';
	return TTRUE;
}

TBOOL AInteriorNetworkPlacementState::IsToggled( const TCHAR* a_szName ) const
{
	TGUIWidget* pWidget = FindWidget( a_szName );
	return pWidget && pWidget->IsA( &TGetClass( PGUIToggleButton ) ) && TSTATICCAST( PGUIToggleButton, pWidget )->IsToggled();
}

TBOOL __stdcall AInteriorNetworkPlacementState::FindButtonsCallback( TGUIWidget* a_pWidget, void* a_pUserData )
{
	AInteriorNetworkPlacementState* pState = TSTATICCAST( AInteriorNetworkPlacementState, a_pUserData );

	if ( a_pWidget->IsA( &TGetClass( PGUIButton ) ) && pState->m_iNumButtons < MAX_BUTTONS )
		pState->m_aButtonListeners[ pState->m_iNumButtons++ ].Connect( TSTATICCAST( PGUIButton, a_pWidget )->GetActionEmitter(), pState, OnButtonAction, 0 );

	return TTRUE;
}

TBOOL __stdcall AInteriorNetworkPlacementState::OnScreenPaint( AInteriorNetworkPlacementState* a_pState, TGUIScreen* a_pScreen, TGUIScreenPaintEvent* a_pEvent )
{
	if ( a_pEvent->ePhase == TGUIScreenPaintEvent::Phase_Begin )
	{
		a_pState->Render( a_pScreen );
	}
	else if ( a_pScreen->IsPointerVisible() )
	{
		// The pointer, an arrow above the widgets
		TGUIDisplayContext* pContext = a_pScreen->GetDisplayContext();
		const TGUIColour    oWhite( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );
		const TGUIColour    oBlack( TUINT8( 0 ), TUINT8( 0 ), TUINT8( 0 ), TUINT8( 255 ) );

		TINT iX, iY;
		a_pScreen->GetPointerPosition( iX, iY );

		for ( TINT i = 0; i < 12; i++ )
			pContext->DrawLine( iX, iY + i, iX + i * 2 / 3, iY + i, ( i == 11 ) ? oBlack : oWhite );

		pContext->DrawLine( iX - 1, iY, iX - 1, iY + 12, oBlack );
		pContext->DrawLine( iX, iY - 1, iX + 9, iY + 10, oBlack );
	}

	return TTRUE;
}

// The buttons of the dialogs
TBOOL __stdcall AInteriorNetworkPlacementState::OnButtonAction( AInteriorNetworkPlacementState* a_pState, PGUIButton* a_pButton, TGUIActionEvent* a_pEvent )
{
	const TPString8& rcName = a_pButton->GetName();
	const TCHAR*     szName = rcName.GetString() ? rcName.GetString() : "";

	TINFO( "[networkedit] button %s\n", szName );

	if ( !TStringManager::String8Compare( szName, "Quit" ) )
		a_pState->OnQuit();
	else if ( !TStringManager::String8Compare( szName, "Save" ) )
		a_pState->OnSave();
	else if ( !TStringManager::String8Compare( szName, "DeletePoint" ) )
		a_pState->OnDeleteSelected();
	else if ( !TStringManager::String8Compare( szName, "GridSnap" ) )
		a_pState->OnToggleSnapToGrid();
	else if ( !TStringManager::String8Compare( szName, "HideUI" ) )
		a_pState->ShowMainDialog( !a_pState->m_pDialog2 );
	else if ( !TStringManager::String8Compare( szName, "Unit" ) )
		a_pState->OnLoadNetwork();
	else if ( !TStringManager::String8Compare( szName, "Undo" ) )
		a_pState->UndoEdit();
	else if ( !TStringManager::String8Compare( szName, "Redo" ) )
		a_pState->RedoEdit();
	else if ( !TStringManager::String8Compare( szName, "Options" ) )
		a_pState->ShowPanel( PANEL_OPTIONS );
	else if ( !TStringManager::String8Compare( szName, "Other" ) )
		a_pState->ShowPanel( PANEL_OTHER );
	else if ( !TStringManager::String8Compare( szName, "Area" ) )
		a_pState->ShowPanel( PANEL_AREA );
	else if ( !TStringManager::String8Compare( szName, "Point" ) )
		a_pState->ShowPanel( PANEL_POINT );
	else if ( !TStringManager::String8Compare( szName, "CostInc" ) )
		a_pState->OnChangeCost( 1.0f );
	else if ( !TStringManager::String8Compare( szName, "CostDec" ) )
		a_pState->OnChangeCost( -1.0f );
	else if ( !TStringManager::String8Compare( szName, "GoalFacing" ) )
		a_pState->OnGoalFacing();
	else
	{
		// TODO: AddArea, AddGoal, DeleteArea, DeleteGoal, AddPoint (the network's arrays are in its TRB file and can't
		// grow), MoveAll, Model, ShowCollision and the collision draw options, BoneSnap (Locators), CollisionGroupPrev/Next.
		// The toggle buttons (PointSnap, CostHighlight) are read when they're used.
		if ( !a_pButton->IsA( &TGetClass( PGUIToggleButton ) ) )
			TWARN( "[networkedit] %s isn't implemented\n", szName );
	}

	return TTRUE;
}

// $NicktoonsPS2: FUNCTION 002ada48
// $NicktoonsPS2: FUNCTION 002ada70
void AInteriorNetworkPlacementState::OnQuit()
{
	// NOTE: Nicktoons shows the entities Toggle Entity Vis hid again here
	m_bQuit = TTRUE;
}

// $NicktoonsPS2: FUNCTION 002adb98
// $NicktoonsPS2: FUNCTION 002adbc0
void AInteriorNetworkPlacementState::OnDeleteSelected()
{
	// NOTE: Nicktoons has a button for the selected link (Delete Link, 002adb20) and one for the node (Delete Node),
	// Barnyard's areas can't be deleted (see OnButtonAction)
	if ( m_oSelected.eType != ELEMENT_POINT )
		return;

	if ( DeletePoint( m_oSelected ) )
	{
		m_oSelected = Element();
		m_oHover    = Element();
		m_iNumEdits = m_iNumUndone = 0; // NOTE: ours, a delete can't be undone
		UpdateSelectionInfo();
		m_bDirty = TTRUE;
	}
}

// $NicktoonsPS2: FUNCTION 002adcd0
// $NicktoonsPS2: FUNCTION 002adcf8
void AInteriorNetworkPlacementState::OnSave()
{
	SaveNetwork();
}

// $NicktoonsPS2: FUNCTION 002addf0
void AInteriorNetworkPlacementState::OnToggleSnapToGrid()
{
	// NOTE: Nicktoons flips the flag, here it follows the toggle button (and the grid size its slider)
	m_bSnapToGrid = IsToggled( "GridSnap" );

	TGUIWidget* pSlider = FindWidget( "GridSlider" );

	if ( pSlider && pSlider->IsA( &TGetClass( PGUISliderBar ) ) && TSTATICCAST( PGUISliderBar, pSlider )->GetCurrentValue() > 0.0f )
		m_fGridSize = TSTATICCAST( PGUISliderBar, pSlider )->GetCurrentValue();
	else
		m_fGridSize = GRID_SIZE;

	TINFO( "[networkedit] snap to grid %d (%.2f)\n", m_bSnapToGrid, m_fGridSize );
}

// The Load button: the network named in UnitName (loaded from Data/Networks/<name>.trb if it isn't yet)
void AInteriorNetworkPlacementState::OnLoadNetwork()
{
	TCHAR szName[ 64 ];

	if ( !GetWidgetLabel( "UnitName", szName, sizeof( szName ) ) || szName[ 0 ] == '\0' )
		return;

	AInteriorNetworkManager* pManager = AInteriorNetworkManager::GetSingleton();
	TPString8                strName( szName );
	AInteriorNetwork*        pNetwork = pManager->FindNetwork( strName );

	if ( !pNetwork )
		pNetwork = pManager->LoadNetwork( strName );

	if ( pNetwork )
		SetNetwork( pNetwork );
	else
		TWARN( "[networkedit] no network %s\n", szName );
}

// The < > buttons of the cost: the path cost of the selected area (scale of its heuristic) by the CostStep text box
void AInteriorNetworkPlacementState::OnChangeCost( TFLOAT a_fSign )
{
	TINT iArea = ( m_oSelected.eType == ELEMENT_POINT ) ? m_oSelected.iArea : m_iSelectedArea;

	if ( !m_pNetwork || iArea < 0 )
		return;

	TCHAR szStep[ 32 ];

	if ( GetWidgetLabel( "CostStep", szStep, sizeof( szStep ) ) )
		m_fCostStep = TFLOAT( atof( szStep ) );

	AInteriorArea& rArea        = m_pNetwork->m_pINetwork->m_pAreas[ iArea ];
	rArea.m_fPathHeuristicScale = TMath::Max( 0.0f, rArea.m_fPathHeuristicScale + m_fCostStep * a_fSign );
	m_bDirty                    = TTRUE;

	UpdateSelectionInfo();
}

// The facing of the selected goal turns by 90 degrees (Nicktoons: Walkable Link cycles the type of the link, 002adf78)
// $NicktoonsPS2: FUNCTION 002adec0
// $NicktoonsPS2: FUNCTION 002adf78
void AInteriorNetworkPlacementState::OnGoalFacing()
{
	if ( !m_pNetwork || m_oSelected.eType != ELEMENT_GOAL )
		return;

	AInteriorGoal& rGoal   = m_pNetwork->m_pINetwork->m_pGoals[ m_oSelected.iIndex ];
	TINT           iFacing = 0;
	TFLOAT         fBest   = -2.0f;

	for ( TINT i = 0; i < TARRAYSIZE( s_aGoalFacings ); i++ )
	{
		TFLOAT fDot = rGoal.m_vForward.x * s_aGoalFacings[ i ].fX + rGoal.m_vForward.z * s_aGoalFacings[ i ].fZ;

		if ( fDot > fBest )
		{
			fBest   = fDot;
			iFacing = i;
		}
	}

	iFacing = ( iFacing + 1 ) % TARRAYSIZE( s_aGoalFacings );

	rGoal.m_vForward.x = s_aGoalFacings[ iFacing ].fX;
	rGoal.m_vForward.z = s_aGoalFacings[ iFacing ].fZ;
	m_bDirty           = TTRUE;

	UpdateSelectionInfo();
}

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
