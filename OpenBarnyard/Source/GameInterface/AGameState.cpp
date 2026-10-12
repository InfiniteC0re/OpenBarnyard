#include "pch.h"
#include "AGameState.h"
#include "Helpers/AGameLoader.h"
#include "Input/AInputHandler.h"
#include "Tasks/ARootTask.h"
#include "Cameras/ACamera.h"
#include "Cameras/ACameraManager.h"
#include "GUI/AGUI2.h"
#include "GUI/AGUISystem.h"
#include "AppBoot.h"
#include "AGameStateController.h"

#include <Input/TInputDeviceKeyboard.h>

//-----------------------------------------------------------------------------
// Enables memory debugging.
// Note: Should be the last include!
//-----------------------------------------------------------------------------
#include <Core/TMemoryDebugOn.h>

TOSHI_NAMESPACE_USING

// NOTE: GetClass 00428060, CreateObject 004285e0, CreateObjectInPlace 00428640
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
// $Barnyard: FUNCTION 00428680
AGameState::~AGameState()
{
#ifdef BARNYARD_COMMUNITY_PATCH
	// NOTE: a state can be destroyed with child states still attached (Destroy only calls their OnRemoval),
	// e.g. AAnticGameOverState with AAnticTrophyState when the minigame is left or restarted. Retail leaks them
	// (the list destructor does nothing). They're detached and leaked here too: one of them is usually still
	// running (the caller of AGameLoader::LeaveMiniGame/RestartMiniGame), so it can't be deleted.
	while ( !m_ChildStates.IsEmpty() )
	{
		AGameState* pChild    = m_ChildStates.PopFront();
		pChild->m_pOwnerState = TNULL;
	}
#endif
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

// DOES_NOTHING
void AGameState::OnRender( TFLOAT a_fDeltaTime )
{
}

// DOES_NOTHING
void AGameState::OnPostRender( TFLOAT a_fDeltaTime )
{
}

// DOES_NOTHING
void AGameState::OnRenderOverlay( TFLOAT a_fDeltaTime )
{
}

void AGameState::OnLoaded()
{
}

TBOOL AGameState::ShouldPlacePlayerAtStartGoal()
{
	return TTRUE;
}

// $Barnyard: FUNCTION 004ee130
ASoundId AGameState::GetSound()
{
	return ASOUNDID_INVALID;
}

TBOOL AGameState::CanDismissInstructions()
{
	return TTRUE;
}

void AGameState::OnInstructionsShown()
{
}

void AGameState::OnStart()
{
}

void AGameState::OnPause()
{
}

void AGameState::OnCollisionModelLoaded( void* a_pCollisionModel, void* a_pGroupId )
{
}

void AGameState::OnVISGroupChanged( void* a_pCurrentSection, void* a_pPreviousSection )
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
	m_HUDParams.Apply();
}

// $Barnyard: FUNCTION 004288d0
void AGameState::OnRemoval()
{
	Bind_CallForAllChild( this, &AGameState::OnRemoval )();
	m_HUDParams.Remove();
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
	m_HUDParams.Apply();
}

// $Barnyard: FUNCTION 0059ca60
void AGameState::HUDParams::Apply()
{
	AGUISystem::GetSingleton()->ApplyHUDParams();
}

// $Barnyard: FUNCTION 00428940
void AGameState::AddChildState( AGameState* a_pChildState )
{
	a_pChildState->m_pOwnerState = this;
	m_ChildStates.PushBack( a_pChildState );

	if ( m_bWasInserted )
	{
		a_pChildState->OnInsertion();

		if ( m_bIsActivated )
			a_pChildState->OnActivate();
	}
}

// $Barnyard: FUNCTION 00428af0
void AGameState::ReplaceChildState( AGameState* a_pOldChildState, AGameState* a_pNewChildState )
{
	a_pOldChildState->Node::Remove();

	if ( m_bIsActivated )
		a_pOldChildState->OnDeactivate();

	a_pOldChildState->OnRemoval();
	delete a_pOldChildState;

	a_pNewChildState->m_pOwnerState = this;
	m_ChildStates.PushBack( a_pNewChildState );

	if ( m_bWasInserted )
	{
		a_pNewChildState->OnInsertion();

		if ( m_bIsActivated )
			a_pNewChildState->OnActivate();
	}
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

// $Barnyard: FUNCTION 00428990
TUINT32 AGameState::GetHUDMask()
{
	TUINT32 uiMask = m_HUDParams.m_uiFlags;
	Bind_CallForAllChild( this, &AGameState::GetHUDMaskImpl )( uiMask );

	return uiMask;
}

// $Barnyard: FUNCTION 00428340
void AGameState::SaveCameraFOV()
{
	if ( ACameraManager::IsSingletonCreated() )
	{
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			m_fFOV = pCamera->GetFOV();
	}
}

// $Barnyard: FUNCTION 00428370
void AGameState::RestoreCameraFOV()
{
	if ( ACameraManager::IsSingletonCreated() )
	{
		if ( ACamera* pCamera = ACameraManager::GetSingleton()->GetCurrentCamera() )
			pCamera->SetFOV( m_fFOV );
	}
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

// $Barnyard: FUNCTION 00428070
void AGameState::GetHUDMaskImpl( TUINT32& a_rOutput )
{
	TUINT uiMask = m_HUDParams.m_uiFlags;

	Bind_CallForAllChild( this, &AGameState::GetHUDMaskImpl )( a_rOutput );

	a_rOutput |= uiMask;
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
		AGameLoader::UnloadForExit();
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
