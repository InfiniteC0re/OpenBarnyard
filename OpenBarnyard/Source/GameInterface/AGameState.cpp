#include "pch.h"
#include "AGameState.h"
#include "Input/AInputHandler.h"
#include "Tasks/ARootTask.h"
#include "Cameras/ACamera.h"
#include "GUI/AGUI2.h"
#include "AppBoot.h"
#include "AGameStateController.h"

#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

TDEFINE_CLASS( AGameState );

// $Barnyard: FUNCTION 00428170
AGameState::AGameState()
{
	m_eOverlayColorIndex = OVERLAY_1;
	m_pInputMap          = TNULL;
	m_pOwnerState        = TNULL;
	m_bWasInserted       = TFALSE;
	m_bIsActivated       = TFALSE;
	m_fFOV               = TMath::DegToRad( 60.0f );

	AGUI2Element* pRootElement = AGUI2::GetRootElement();
	m_GUIElement.SetDimensions( pRootElement->GetWidth(), pRootElement->GetHeight() );

	pRootElement->AddChildTail( m_GUIElement );
	m_GUIElement.Hide();
}

// $Barnyard: FUNCTION 00428250
AGameState::~AGameState()
{
}

// $Barnyard: FUNCTION 00428700
TBOOL AGameState::ProcessInput( const TInputInterface::InputEvent* a_pInputEvent )
{
	if ( ExecuteForOneChildState( &AGameState::ProcessInput, 0, a_pInputEvent ) )
	{
		return TTRUE;
	}

	return TFALSE == ARootTask::GetSingleton()->IsGameSystemCreated();
}

TBOOL AGameState::ProcessCommand( AInputCommand a_eInputCommand, const TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	return TFALSE;
}

// $Barnyard: FUNCTION 00428870
TBOOL AGameState::Unknown1( void* a_pUnk1, void* a_pUnk2 )
{
	return ExecuteForOneChildState( &AGameState::Unknown1, 0, a_pUnk1, a_pUnk2 );
}

void AGameState::Unknown2( void* a_pUnk1 )
{
}

void AGameState::Unknown3( void* a_pUnk1 )
{
}

void AGameState::Unknown4( void* a_pUnk1 )
{
}

void AGameState::Unknown5()
{
}

TBOOL AGameState::Unknown6()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 004ee130
ASoundId AGameState::GetSound()
{
	return ASOUNDID_INVALID;
}

TBOOL AGameState::Unknown7()
{
	return TTRUE;
}

void AGameState::Unknown8()
{
}

void AGameState::OnStarted()
{
}

void AGameState::Unknown10()
{
}

void AGameState::Unknown11( void* a_pUnk1, void* a_pUnk2 )
{
}

void AGameState::Unknown12( void* a_pUnk1, void* a_pUnk2 )
{
}

// $Barnyard: FUNCTION 0045b650
TFLOAT AGameState::GetFOV()
{
	return ACamera::sm_fCurrentFOV;
}

// $Barnyard: FUNCTION 00428890
TBOOL AGameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ExecuteForAllChildStates( &AGameState::OnUpdate, 0, a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 004288b0
void AGameState::OnInsertion()
{
	m_bWasInserted = TTRUE;
	ExecuteForAllChildStates( &AGameState::OnInsertion, 0 );
}

// $Barnyard: FUNCTION 004288d0
void AGameState::OnRemoval()
{
	ExecuteForAllChildStates( &AGameState::OnRemoval, 0 );
}

// $Barnyard: FUNCTION 00428920
void AGameState::OnSuspend()
{
	ExecuteForAllChildStates( &AGameState::OnSuspend, 0 );
}

// $Barnyard: FUNCTION 004288f0
void AGameState::OnResume( AGameState* a_pPreviousState )
{
	ExecuteForAllChildStates( &AGameState::OnResume, 0, a_pPreviousState );
}

// $Barnyard: FUNCTION 004289c0
void AGameState::OnActivate()
{
	m_bIsActivated = TTRUE;
	ExecuteForAllChildStates( &AGameState::OnActivate, 0 );
}

// $Barnyard: FUNCTION 004289e0
void AGameState::OnDeactivate()
{
	m_bIsActivated = TFALSE;
	ExecuteForAllChildStates( &AGameState::OnDeactivate, 0 );
}

void AGameState::Destroy()
{
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	Deactivate();

	OnRemoval();
	delete this;
}

void AGameState::Destroy( TBOOL a_bDeactivate )
{
	TRenderInterface::GetSingleton()->BeginEndSceneHAL();

	if ( a_bDeactivate )
		Deactivate();

	OnRemoval();
	delete this;
}

// $Barnyard: FUNCTION 00428730
TBOOL AGameState::SendInputCommands( const TInputInterface::InputEvent* a_pEvent )
{
	auto pInputHandler = AInputHandler::GetSingleton();
	auto pInputManager = AInputMapManager::GetSingleton();

	pInputManager->PushInputMap( m_pInputMap );

	AInputCommandArray commands;
	pInputManager->GetEventCommands( a_pEvent, commands );

	if ( a_pEvent->GetDoodad() == TInputDeviceKeyboard::KEY_F4 && pInputHandler->GetKeyboardDevice()->IsAltDown() )
	{
		// Quit app if Alt + F4 was pressed
		TTODO( "FUN_00425980" );
		g_oTheApp.Destroy();
	}
	else
	{
		TBOOL bProcessedCommand = TFALSE;

		for ( TINT i = 0; i < commands.iNumCommands; i++ )
		{
			auto  eCommand    = commands.aCommands[ i ];
			TBOOL bStopEvents = TTRUE;

			if ( ProcessCommand( eCommand, a_pEvent, bStopEvents ) )
			{
				bProcessedCommand = TTRUE;

				if ( bStopEvents )
				{
					pInputManager->PopInputMap();

					for ( TINT k = 0; k < commands.iNumCommands; k++ )
					{
						commands.aCommands[ k ] = AInputCommand_Empty;
					}

					return TTRUE;
				}
			}
		}

		pInputManager->PopInputMap();
		commands.Clear();

		if ( !bProcessedCommand )
		{
			return ExecuteForOneChildState( &AGameState::SendInputCommands, 0, a_pEvent );
		}
	}

	return TTRUE;
}

// $Barnyard: FUNCTION 00428320
void AGameState::SetInputMap( const TPString8& a_MapName )
{
	m_pInputMap = AInputMapManager::GetSingleton()->FindMap( a_MapName );
	TASSERT( m_pInputMap != TNULL );
}

void AGameState::Activate()
{
	OnActivate();

	TGlobalEmitter<AGameStateControllerEvent>::Throw(
	    AGameStateControllerEvent( this, AGameStateControllerEvent::Type_GameStateActivated )
	);
}

void AGameState::Deactivate()
{
	OnDeactivate();

	TGlobalEmitter<AGameStateControllerEvent>::Throw(
	    AGameStateControllerEvent(
	        this,
	        AGameStateControllerEvent::Type_GameStateDeactivated
	    )
	);
}

void AGameState::Suspend()
{
	OnSuspend();
}

// $Barnyard: FUNCTION 004282d0
void AGameState::Remove()
{
	if ( m_pOwnerState == TNULL )
	{
		AGameStateController::GetSingleton()->PopCurrentGameState();
	}
	else
	{
		Node::Remove();

		if ( m_bIsActivated )
		{
			OnDeactivate();
		}

		OnRemoval();
		delete this;
	}
}
