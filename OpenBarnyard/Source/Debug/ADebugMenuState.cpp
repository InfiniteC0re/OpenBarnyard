#include "pch.h"
#include "ADebugMenuState.h"

#if defined( BARNYARD_COMMUNITY_PATCH ) && !defined( TOSHI_FINAL )
#  include "GameInterface/AGameStateController.h"
#  include "GameInterface/AMovieState.h"
#  include "GameInterface/ATerrainLoadingStallState.h"
#  include "GameInterface/SaveLoadSKU.h"
#  include "GameInterface/AInteriorNetworkPlacementState.h"
#  include "ACheatActivator.h"
#  include "Helpers/AGameLoader.h"
#  include "Tasks/ARootTask.h"
#  include "UnitMovement/PathfindingNetworks/AInteriorNetworkManager.h"

#  include <Menu/PDebugMenu.h>
#  include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#  include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS_NORUNTIME( ADebugMenuState );

ADebugMenuState* ADebugMenuState::ms_pOpenMenu = TNULL;

ADebugMenuState::ADebugMenuState( AInputMap* a_pUnderInputMap )
    : m_pUnderInputMap( a_pUnderInputMap )
    , m_ePendingAction( ACTION_NONE )
    , m_iPendingValue( 0 )
    , m_bWasPaused( TFALSE )
    , m_bClose( TFALSE )
{
}

ADebugMenuState::~ADebugMenuState()
{
	if ( ms_pOpenMenu == this )
		ms_pOpenMenu = TNULL;
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

	return PDebugMenu::IsStarted();
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
	if ( !a_pGameState )
		return TFALSE;

	TBOOL bGoneDown = a_pEvent->GetEventType() == TInputInterface::EVENT_TYPE_GONE_DOWN;

	if ( a_pGameState->IsA( &TGetClass( ADebugMenuState ) ) )
	{
		ADebugMenuState* pMenu = TSTATICCAST( ADebugMenuState, a_pGameState );

		if ( bGoneDown && IsDebugMenuEvent( pMenu->m_pUnderInputMap, a_pEvent ) )
			pMenu->m_bClose = TTRUE;

		// Nothing gets through the menu (it reads the keyboard itself)
		return TTRUE;
	}

	if ( !bGoneDown || !IsDebugMenuEvent( a_pGameState->GetInputMap(), a_pEvent ) || !CanOpen( a_pGameState ) )
		return TFALSE;

	TINFO( "[debugmenu] opening the debug menu on top of %s\n", a_pGameState->GetClass()->GetName() );
	AGameStateController::GetSingleton()->PushState( new ADebugMenuState( a_pGameState->GetInputMap() ) );
	return TTRUE;
}

TBOOL ADebugMenuState::Open()
{
	if ( ms_pOpenMenu )
		return TTRUE;

	AGameState* pGameState = AGameStateController::GetSingleton()->GetCurrentState();

	if ( !CanOpen( pGameState ) )
		return TFALSE;

	TINFO( "[debugmenu] opening the debug menu on top of %s\n", pGameState->GetClass()->GetName() );
	AGameStateController::GetSingleton()->PushState( new ADebugMenuState( pGameState->GetInputMap() ) );
	return TTRUE;
}

TBOOL ADebugMenuState::RequestAction( ACTION a_eAction, TINT a_iValue )
{
	if ( !ms_pOpenMenu )
		return TFALSE;

	ms_pOpenMenu->m_ePendingAction = a_eAction;
	ms_pOpenMenu->m_iPendingValue  = a_iValue;
	return TTRUE;
}

TBOOL ADebugMenuState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	// The states under the menu get nothing (ProcessDebugMenuInput gets the events first)
	return TTRUE;
}

TBOOL ADebugMenuState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ACTION eAction   = m_ePendingAction;
	m_ePendingAction = ACTION_NONE;

	// The actions that leave the menu pop it themselves
	if ( eAction != ACTION_NONE && DoAction( eAction, m_iPendingValue ) )
		return TTRUE;

	// Closed with the DebugMenu command, Close or Home
	if ( m_bClose || !PDebugMenu::IsVisible() )
	{
		AGameStateController::GetSingleton()->PopState( this );
		return TTRUE;
	}

	return TTRUE;
}

void ADebugMenuState::OnInsertion()
{
	ms_pOpenMenu = this;

	// Paused like the pause menu
	m_bWasPaused = ARootTask::GetSingleton()->IsPaused();
	ARootTask::GetSingleton()->SetPaused( TTRUE );

	if ( !PDebugMenu::IsVisible() )
		PDebugMenu::SetVisible( TTRUE );

	BaseClass::OnInsertion();
}

void ADebugMenuState::OnRemoval()
{
	if ( ms_pOpenMenu == this )
		ms_pOpenMenu = TNULL;

	if ( PDebugMenu::IsVisible() )
		PDebugMenu::SetVisible( TFALSE );

	if ( !m_bWasPaused )
		ARootTask::GetSingleton()->SetPaused( TFALSE );

	TINFO( "[debugmenu] closed\n" );
	BaseClass::OnRemoval();
}

TBOOL ADebugMenuState::DoAction( ACTION a_eAction, TINT a_iValue )
{
	AGameStateController* pController = AGameStateController::GetSingleton();

	switch ( a_eAction )
	{
		case ACTION_CLOSE:
			m_bClose = TTRUE;
			return TFALSE;
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
		default:
			break;
	}

	// The menu stays open
	return TFALSE;
}

#endif // BARNYARD_COMMUNITY_PATCH && !TOSHI_FINAL
