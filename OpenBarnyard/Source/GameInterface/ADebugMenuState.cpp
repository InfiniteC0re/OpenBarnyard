#include "pch.h"
#include "ADebugMenuState.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "AGameStateController.h"
#  include "AMovieState.h"
#  include "ATerrainLoadingStallState.h"
#  include "SaveLoadSKU.h"
#  include "AInteriorNetworkPlacementState.h"
#  include "ASimState.h"
#  include "ACheatActivator.h"
#  include "GUI/AGUISystem.h"
#  include "Helpers/AGameLoader.h"
#  include "Helpers/AGameTimeManager.h"
#  include "Helpers/AWeatherManager.h"
#  include "Player/APlayerProfileManager.h"
#  include "Render/ADebugOverlay.h"
#  include "Render/AWorldDebugDraw.h"
#  include "Tasks/ARootTask.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

#  include <TGUI/TGUIManager.h>
#  include <TGUI/TGUIScreen.h>
#  include <TGUI/TGUIStyle.h>
#  include <TGUI/TGUIStyleState.h>
#  include <TGUI/TGUIStyleRootComponent.h>
#  include <TGUI/TGUIComponent.h>
#  include <TGUI/TGUIComponentInfo.h>
#  include <TGUI/TGUIWidgetInfo.h>
#  include <TGUI/TGUIElement.h>
#  include <Plugins/PGUI/PGUIMenuTreeItem.h>
#  include <Plugins/PGUI/PGUIMenuTreeItemStyle.h>
#  include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ADebugMenuState );

// The cheats, as ACheatActivator::CHEAT
static const TCHAR* s_aCheatNames[ ACheatActivator::CHEAT_NUMOF ] = {
	"Unlock Mud Jump",
	"Unlock Barn Pool",
	"Unlock Bike Race",
	"Unlock Chasing Chicks",
	"Unlock Veggie Patch",
	"Unlock Cow Tipping",
	"Unlock Barntender",
	"Money",
	"Items",
	"Unlock bonus items",
	"Unlock antics",
};

static const TCHAR* s_aDayPhaseNames[ AGameTimeManager::DAYPHASE_NUMOF ] = {
	"Dawn",
	"Morning",
	"Afternoon",
	"Dusk",
	"Night",
};

static const TCHAR* s_aWeatherNames[ AWeatherManager::WEATHERTYPE_NUMOF ] = {
	"Normal",
	"Heatwave",
	"Light rain",
	"Heavy rain",
	"Rainbow",
	"Lightning storm",
};

ADebugMenuState::ADebugMenuState( AInputMap* a_pUnderInputMap )
    : m_pUnderInputMap( a_pUnderInputMap )
    , m_pMenuTree( TNULL )
    , m_iNumItems( 0 )
    , m_ePendingAction( ACTION_NONE )
    , m_iPendingValue( 0 )
    , m_bWasPaused( TFALSE )
    , m_bClose( TFALSE )
{
}

ADebugMenuState::~ADebugMenuState()
{
	DestroyMenu();
}

TBOOL ADebugMenuState::CanOpen( AGameState* a_pGameState )
{
	ARootTask* pRootTask = ARootTask::GetSingleton();

	// A loaded world, nothing loading or changing
	if ( !a_pGameState || !pRootTask->IsGameSystemCreated() || !pRootTask->IsRenderWorld() || pRootTask->IsTransitioning() ||
	     AGameLoader::IsChangingTerrain() || AGameLoader::IsReturningToFrontEnd() || AGameStateController::GetSingleton()->IsBusy() )
		return TFALSE;

	// Not on top of the screens that come and go by themselves, nor on top of the network editor (it has the camera)
	if ( a_pGameState->IsA( &TGetClass( AMovieState ) ) || a_pGameState->IsA( &TGetClass( ATerrainLoadingStallState ) ) ||
	     a_pGameState->IsA( &TGetClass( SaveLoadSKU ) ) || a_pGameState->IsA( &TGetClass( AInteriorNetworkPlacementState ) ) )
		return TFALSE;

	return AGUISystem::IsSingletonCreated();
}

TBOOL ADebugMenuState::IsDebugMenuEvent( AInputMap* a_pInputMap, const TInputInterface::InputEvent* a_pEvent )
{
	if ( !a_pInputMap )
		return a_pEvent->GetDoodad() == TInputDeviceKeyboard::KEY_GRAVE;

	AInputMapManager* pInputManager = AInputMapManager::GetSingleton();
	pInputManager->PushInputMap( a_pInputMap );

	AInputCommandArray oCommands;
	pInputManager->GetEventCommands( a_pEvent, oCommands );
	pInputManager->PopInputMap();

	TBOOL bDebugMenu = TFALSE;

	for ( TINT i = 0; i < oCommands.iNumCommands; i++ )
	{
		if ( oCommands.aCommands[ i ] == AInputCommand_DebugMenu )
			bDebugMenu = TTRUE;
	}

	oCommands.Clear();
	return bDebugMenu;
}

TBOOL ADebugMenuState::ProcessDebugMenuInput( AGameState* a_pGameState, const TInputInterface::InputEvent* a_pEvent )
{
	if ( a_pEvent->GetEventType() != TInputInterface::EVENT_TYPE_GONE_DOWN || !a_pGameState )
		return TFALSE;

	if ( a_pGameState->IsA( &TGetClass( ADebugMenuState ) ) )
	{
		ADebugMenuState* pMenu = TSTATICCAST( ADebugMenuState, a_pGameState );

		if ( !IsDebugMenuEvent( pMenu->m_pUnderInputMap, a_pEvent ) )
			return TFALSE;

		pMenu->m_bClose = TTRUE;
		return TTRUE;
	}

	if ( !IsDebugMenuEvent( a_pGameState->GetInputMap(), a_pEvent ) || !CanOpen( a_pGameState ) )
		return TFALSE;

	TINFO( "[debugmenu] opening the debug menu on top of %s\n", a_pGameState->GetClass()->GetName() );
	AGameStateController::GetSingleton()->PushState( new ADebugMenuState( a_pGameState->GetInputMap() ) );
	return TTRUE;
}

TBOOL ADebugMenuState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	// The states under the menu get nothing (the menu tree gets its input from AGUISystem first)
	return TTRUE;
}

TBOOL ADebugMenuState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ACTION eAction   = m_ePendingAction;
	m_ePendingAction = ACTION_NONE;

	// The actions that leave the menu pop it themselves
	if ( eAction != ACTION_NONE && DoAction( eAction, m_iPendingValue ) )
		return TTRUE;

	if ( m_bClose || !m_pMenuTree )
	{
		AGameStateController::GetSingleton()->PopState( this );
		return TTRUE;
	}

	return TTRUE;
}

void ADebugMenuState::OnInsertion()
{
	// Paused like the pause menu
	m_bWasPaused = ARootTask::GetSingleton()->IsPaused();
	ARootTask::GetSingleton()->SetPaused( TTRUE );

	if ( !CreateMenu() )
		TWARN( "[debugmenu] the menu templates of Data/GUI/main.gui aren't there\n" );

	BaseClass::OnInsertion();
}

void ADebugMenuState::OnRemoval()
{
	DestroyMenu();

	if ( !m_bWasPaused )
		ARootTask::GetSingleton()->SetPaused( TFALSE );

	TINFO( "[debugmenu] closed\n" );
	BaseClass::OnRemoval();
}

TBOOL ADebugMenuState::CreateMenu()
{
	AGUISystem*                 pGUISystem = AGUISystem::GetSingleton();
	AGUISystem::TGUIMemoryScope oMemoryScope;

	if ( !pGUISystem->CreateTGUI() )
		return TFALSE;

	// The templates are in Data/GUI/main.gui, loaded once (also by the network editor and "-tguidemo")
	TGUIManager* pManager = TGUIManager::GetSingleton();

	if ( !pManager->FindTopLevelInstance( TPS8D( "CurrentMenu" ), 0xffffffff ) )
		pGUISystem->LoadTGUIDefinitionFile( "Data/GUI/main.gui" );

	if ( !pManager->FindTopLevelInstance( TPS8D( "CurrentMenu" ), 0xffffffff ) || !pManager->FindStyleInstance( TPS8D( "Button_MenuSel" ) ) )
		return TFALSE;

	HighlightSelectedStyle();

	TGUIScreen* pScreen = pGUISystem->GetTGUIScreen();
	pScreen->SetFixedSize( AGUISystem::TGUI_SCREEN_WIDTH, AGUISystem::TGUI_SCREEN_HEIGHT );

	m_pMenuTree = new PGUIMenuTree();
	m_pMenuTree->SetScreen( pScreen );
	m_oSelectListener.Connect( m_pMenuTree->GetSelectEmitter(), this, OnMenuSelect, 0 );
	m_oCloseListener.Connect( m_pMenuTree->GetCloseEmitter(), this, OnMenuClose, 0 );

	// The current menu, the menus of the submenus open on its right (NextMenuTemplate)
	PGUIMenuTreeItemStyle* pStyle = m_pMenuTree->CreateMenuStyleGroup( "Button_MenuCurr", "Button_MenuSel", "Button_MenuNext", "CurrentPopupMenu", "CurrentMenu", "NextMenuTemplate" );
	PGUIMenuTreeItem*      pMenu  = new PGUIMenuTreeItem( pStyle, L"Debug Menu" );
	pMenu->SetState( PGUIItemData::State_Enabled | PGUIItemData::State_Visible );
	m_pMenuTree->GetRootItem()->AddItem( pMenu );

	m_iNumItems = 0;

	PGUIMenuTreeItem* pToggles = AddItem( pMenu, "Toggles", ACTION_NONE, 0 );
#  ifdef ADEBUGOVERLAY
	AddItem( pToggles, "Debug overlay", ACTION_TOGGLE_OVERLAY, 0 );

	for ( TINT i = 0; i < AWorldDebugDraw::LAYER_NUMOF; i++ )
		AddItem( pToggles, AWorldDebugDraw::GetLayerName( AWorldDebugDraw::LAYER( i ) ), ACTION_TOGGLE_LAYER, i );
#  endif

	AddItem( pToggles, "TGUI dialog demo", ACTION_TOGGLE_TGUI_DEMO, 0 );

	PGUIMenuTreeItem* pTools = AddItem( pMenu, "Tools", ACTION_NONE, 0 );
	AddItem( pTools, "Interior network editor", ACTION_NETWORK_EDITOR, 0 );

	PGUIMenuTreeItem* pCheats = AddItem( pMenu, "Cheats", ACTION_NONE, 0 );

	for ( TINT i = 0; i < ACheatActivator::CHEAT_NUMOF; i++ )
		AddItem( pCheats, s_aCheatNames[ i ], ACTION_CHEAT, i );

	PGUIMenuTreeItem* pMoney = AddItem( pMenu, "Money", ACTION_NONE, 0 );
	AddItem( pMoney, "+1000", ACTION_ADD_MONEY, 1000 );
	AddItem( pMoney, "Set to 9999", ACTION_SET_MONEY, 9999 );
	AddItem( pMoney, "Set to 0", ACTION_SET_MONEY, 0 );

	PGUIMenuTreeItem* pTime = AddItem( pMenu, "Time", ACTION_NONE, 0 );

	for ( TINT i = 0; i < AGameTimeManager::DAYPHASE_NUMOF; i++ )
		AddItem( pTime, s_aDayPhaseNames[ i ], ACTION_DAY_PHASE, i );

	AddItem( pTime, "Sim time x1", ACTION_TIME_SCALE, 1 );
	AddItem( pTime, "Sim time x10", ACTION_TIME_SCALE, 10 );
	AddItem( pTime, "Sim time x60", ACTION_TIME_SCALE, 60 );

	PGUIMenuTreeItem* pWeather = AddItem( pMenu, "Weather", ACTION_NONE, 0 );

	for ( TINT i = 0; i < AWeatherManager::WEATHERTYPE_NUMOF; i++ )
		AddItem( pWeather, s_aWeatherNames[ i ], ACTION_WEATHER, i );

	AddItem( pMenu, "Close menu", ACTION_CLOSE, 0 );

	UpdateLabels();

	// Under the lines of the debug overlay
	m_pMenuTree->InvokeMenu( pMenu, 60, 300 );

	// The menus are made for the pointer (nothing is selected), the keyboard and the pad scroll from the first item
	m_pMenuTree->SetSelectedItem( pMenu->GetFirstChild() );
	return TTRUE;
}

void ADebugMenuState::DestroyMenu()
{
	if ( !m_pMenuTree )
		return;

	AGUISystem::TGUIMemoryScope oMemoryScope;

	m_oSelectListener.Disconnect();
	m_oCloseListener.Disconnect();

	if ( m_pMenuTree->IsInvoked() )
		m_pMenuTree->CancelMenu();

	delete m_pMenuTree;
	m_pMenuTree = TNULL;
	m_iNumItems = 0;
}

PGUIMenuTreeItem* ADebugMenuState::AddItem( PGUIMenuTreeItem* a_pParent, const TCHAR* a_szLabel, ACTION a_eAction, TINT a_iValue )
{
	TASSERT( m_iNumItems < MAX_ITEMS );

	Item& rItem   = m_aItems[ m_iNumItems++ ];
	rItem.eAction = a_eAction;
	rItem.iValue  = a_iValue;
	rItem.szLabel = a_szLabel;
	TStringManager::StringCharToUnicode( rItem.wszLabel, a_szLabel, -1 );

	rItem.pItem = a_pParent->AddItem( rItem.wszLabel, PGUIItemData::State_Enabled | PGUIItemData::State_Visible );
	return rItem.pItem;
}

// The toggles show their state, the submenus an arrow
void ADebugMenuState::UpdateLabels()
{
	for ( TINT i = 0; i < m_iNumItems; i++ )
	{
		Item& rItem = m_aItems[ i ];
		TINT  iOn   = -1;

		switch ( rItem.eAction )
		{
#  ifdef ADEBUGOVERLAY
			case ACTION_TOGGLE_OVERLAY:
				iOn = ADebugOverlay::IsSingletonCreated() && ADebugOverlay::GetSingleton()->IsEnabled();
				break;
			case ACTION_TOGGLE_LAYER:
				iOn = AWorldDebugDraw::IsSingletonCreated() && AWorldDebugDraw::GetSingleton()->IsLayerEnabled( AWorldDebugDraw::LAYER( rItem.iValue ) );
				break;
#  endif
			case ACTION_TOGGLE_TGUI_DEMO:
				iOn = AGUISystem::GetSingleton()->Dev_IsTGUIDemoShown();
				break;
			case ACTION_TIME_SCALE:
				iOn = TINT( ASimState::Dev_GetTimeScale() ) == rItem.iValue;
				break;
			default:
				break;
		}

		TCHAR szLabel[ LABEL_LENGTH ];

		if ( iOn >= 0 )
			TStringManager::String8Format( szLabel, LABEL_LENGTH, "[%c] %s", iOn ? 'X' : ' ', rItem.szLabel );
		else if ( rItem.pItem->HasChildren() )
			TStringManager::String8Format( szLabel, LABEL_LENGTH, "%s  >", rItem.szLabel );
		else
			TStringManager::String8Format( szLabel, LABEL_LENGTH, "%s", rItem.szLabel );

		TStringManager::StringCharToUnicode( rItem.wszLabel, szLabel, -1 );
	}
}

TBOOL ADebugMenuState::DoAction( ACTION a_eAction, TINT a_iValue )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	switch ( a_eAction )
	{
		case ACTION_CLOSE:
			m_bClose = TTRUE;
			return TFALSE;
#  ifdef ADEBUGOVERLAY
		case ACTION_TOGGLE_OVERLAY:
			if ( ADebugOverlay::IsSingletonCreated() )
				ADebugOverlay::GetSingleton()->SetEnabled( !ADebugOverlay::GetSingleton()->IsEnabled() );
			break;
		case ACTION_TOGGLE_LAYER:
			if ( AWorldDebugDraw::IsSingletonCreated() )
			{
				AWorldDebugDraw* pDebugDraw = AWorldDebugDraw::GetSingleton();
				pDebugDraw->SetLayerEnabled( AWorldDebugDraw::LAYER( a_iValue ), !pDebugDraw->IsLayerEnabled( AWorldDebugDraw::LAYER( a_iValue ) ) );
			}
			break;
#  endif
		case ACTION_TOGGLE_TGUI_DEMO:
		{
			// The demo shares the TGUI screen: the menu closes
			TBOOL bShow = !AGUISystem::GetSingleton()->Dev_IsTGUIDemoShown();
			DestroyMenu();
			AGUISystem::GetSingleton()->Dev_ShowTGUIDemo( bShow );
			m_bClose = TTRUE;
			return TFALSE;
		}
		case ACTION_NETWORK_EDITOR:
		{
			if ( !AInteriorNetworkManager::IsSingletonCreated() || !AInteriorNetworkManager::GetSingleton()->GetAnyNetwork() )
			{
				TWARN( "[debugmenu] no interior network in this world\n" );
				break;
			}

			// The editor takes the place of the menu
			pController->PopState( this );
			TINFO( "[debugmenu] opening the network editor on top of %s\n", pController->GetCurrentState()->GetClass()->GetName() );
			pController->PushState( new AInteriorNetworkPlacementState() );
			return TTRUE;
		}
		case ACTION_CHEAT:
			// The cheat is entered in the state under the menu (the states it's allowed in)
			pController->PopState( this );
			ACheatActivator::Dev_TriggerCheat( ACheatActivator::CHEAT( a_iValue ) );
			return TTRUE;
		case ACTION_ADD_MONEY:
			TINFO( "[debugmenu] +%d pick ups\n", a_iValue );
			APlayerProfileManager::GetSingleton()->GetMainProfile()->AddPickups( a_iValue );
			break;
		case ACTION_SET_MONEY:
			TINFO( "[debugmenu] %d pick ups\n", a_iValue );
			APlayerProfileManager::GetSingleton()->GetMainProfile()->SetPickups( a_iValue, TFALSE, TTRUE );
			break;
		case ACTION_DAY_PHASE:
			if ( AGameTimeManager::IsSingletonCreated() )
			{
				TINFO( "[debugmenu] day phase %d\n", a_iValue );
				AGameTimeManager::GetSingleton()->ChangePhaseOfDay( a_iValue, TTRUE, TTRUE );
			}
			break;
		case ACTION_TIME_SCALE:
			ASimState::Dev_SetTimeScale( TFLOAT( a_iValue ) );
			break;
		case ACTION_WEATHER:
			if ( AWeatherManager::IsSingletonCreated() )
			{
				TINFO( "[debugmenu] weather %d\n", a_iValue );
				AWeatherManager::GetSingleton()->SetCurrentWeather( a_iValue );
			}
			break;
		default:
			break;
	}

	// The menu stays open, its labels change
	if ( m_pMenuTree )
	{
		AGUISystem::TGUIMemoryScope oMemoryScope;

		UpdateLabels();
		m_pMenuTree->UpdateMenu( m_pMenuTree->GetSelectedItem() );
	}

	return TFALSE;
}

void ADebugMenuState::HighlightSelectedStyle()
{
	static TGUIStyle* s_pHighlighted = TNULL;

	TGUIStyle* pStyle = TGUIManager::GetSingleton()->FindStyleInstance( TPS8D( "Button_MenuSel" ) );

	if ( !pStyle || pStyle == s_pHighlighted )
		return;

	s_pHighlighted = pStyle;

	// NOTE: ours: Button_MenuSel of main.gui is the same as Button_MenuCurr (yellow text on a frame), the selected item
	// gets white text on a blue frame. The black elements are the shadows.
	const TGUIWidgetInfo* pWidgetInfo  = pStyle->GetWidgetInfo();
	TINT                  iForeground  = pWidgetInfo->GetComponentIndex( TPS8D( "foreground" ) );
	const TGUIColour      oBlack( TUINT8( 0 ), TUINT8( 0 ), TUINT8( 0 ), TUINT8( 255 ) );
	const TGUIColour      oText( TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ), TUINT8( 255 ) );
	const TGUIColour      oFrame( TUINT8( 110 ), TUINT8( 170 ), TUINT8( 255 ), TUINT8( 255 ) );

	for ( TINT iRoot = 0; iRoot < pWidgetInfo->GetComponentCount(); iRoot++ )
	{
		TGUIStyleRootComponent& rRoot = pStyle->GetRootComponent( iRoot );

		for ( auto itState = rRoot.GetStates().Begin(); itState != rRoot.GetStates().End(); itState++ )
		{
			for ( auto it = itState->GetComponents().Begin(); it != itState->GetComponents().End(); it++ )
			{
				TGUIComponent* pComponent = it;
				TINT           iElements  = pComponent->GetComponentInfo()->GetElementCount();

				for ( TINT i = 0; i < iElements; i++ )
				{
					TGUIElement& rElement = pComponent->GetElement( i );

					if ( rElement.GetColour() != oBlack )
						rElement.SetColour( iRoot == iForeground ? oText : oFrame );
				}

				pComponent->NotifyElementsChanged();
			}
		}
	}
}

TBOOL __stdcall ADebugMenuState::OnMenuSelect( ADebugMenuState* a_pState, PGUIMenuTree* a_pMenuTree, TGUIActionEvent* a_pEvent )
{
	PGUIMenuTreeItem* pItem = TSTATICCAST( PGUIMenuTreeItem, a_pEvent->pItem );

	for ( TINT i = 0; i < a_pState->m_iNumItems; i++ )
	{
		if ( a_pState->m_aItems[ i ].pItem == pItem )
		{
			TINFO( "[debugmenu] selected '%s'\n", a_pState->m_aItems[ i ].szLabel );

			// Done in OnUpdate, outside of the input of the screen
			a_pState->m_ePendingAction = a_pState->m_aItems[ i ].eAction;
			a_pState->m_iPendingValue  = a_pState->m_aItems[ i ].iValue;
			break;
		}
	}

	return TTRUE;
}

TBOOL __stdcall ADebugMenuState::OnMenuClose( ADebugMenuState* a_pState, PGUIMenuTree* a_pMenuTree, PGUIMenuTree::CloseEvent* a_pEvent )
{
	// Back from the first menu
	if ( a_pEvent->bCancelled )
		a_pState->m_bClose = TTRUE;

	return TTRUE;
}

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
