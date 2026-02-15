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

	T2GUIElement* pRootElement = AGUI2::GetRootElement();
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
	if ( Bind_CallForOneChild( this, &AGameState::ProcessInput )( a_pInputEvent ) )
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
	return Bind_CallForOneChild( this, &AGameState::Unknown1 )( a_pUnk1, a_pUnk2 );
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

void AGameState::OnStart()
{
}

void AGameState::OnPause()
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
	Bind_CallForAllChild( this, &AGameState::OnUpdate )( a_fDeltaTime );
	return TTRUE;
}

// $Barnyard: FUNCTION 004288b0
void AGameState::OnInsertion()
{
	m_bWasInserted = TTRUE;
	Bind_CallForAllChild( this, &AGameState::OnInsertion )();
}

// $Barnyard: FUNCTION 004288d0
void AGameState::OnRemoval()
{
	Bind_CallForAllChild( this, &AGameState::OnRemoval )();
}

// $Barnyard: FUNCTION 00428920
void AGameState::OnSuspend()
{
	Bind_CallForAllChild( this, &AGameState::OnSuspend )();
}

// $Barnyard: FUNCTION 004288f0
void AGameState::OnResume( AGameState* a_pPreviousState )
{
	Bind_CallForAllChild( this, &AGameState::OnResume )( a_pPreviousState );
}

// $Barnyard: FUNCTION 004289c0
void AGameState::OnActivate()
{
	m_bIsActivated = TTRUE;
	Bind_CallForAllChild( this, &AGameState::OnActivate )();
}

// $Barnyard: FUNCTION 004289e0
void AGameState::OnDeactivate()
{
	m_bIsActivated = TFALSE;
	Bind_CallForAllChild( this, &AGameState::OnDeactivate )();
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

// $Barnyard: FUNCTION 00428a00
AGameState* AGameState::FindChildState( TClass* a_pClass )
{
	AGameState* pResult = TNULL;
	Bind_CallForOneChild( this, &AGameState::FindChildStateImpl )( a_pClass, pResult );

	return pResult;
}

// $Barnyard: FUNCTION 00428a30
TBOOL AGameState::FindChildStateImpl( TClass* a_pClass, AGameState*& a_rOutput )
{
	if ( GetClass()->IsA( a_pClass ) )
	{
		a_rOutput = this;
		return TTRUE;
	}
	
	// Look through other children states
	AGameState* pResult = TNULL;
	Bind_CallForOneChild( this, &AGameState::FindChildStateImpl )( a_pClass, pResult );

	// BUG?: shouldn't it move the result into a_rOutput?
	return pResult != TNULL;
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
			return Bind_CallForOneChild( this, &AGameState::SendInputCommands, 0 )( a_pEvent );
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
