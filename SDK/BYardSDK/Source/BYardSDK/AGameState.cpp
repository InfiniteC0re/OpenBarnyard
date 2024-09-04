#include "pch.h"
#include "AGameState.h"
#include "AGUI2.h"

#include "AGameStateController.h"

#include <Input/TInputDeviceKeyboard.h>

TOSHI_NAMESPACE_USING

AGameState::AGameState()
{
	m_eOverlayColorIndex = 0;
	m_pInputMap          = TNULL;
	m_pOwnerState        = TNULL;
	m_bWasInserted       = TFALSE;
	m_bIsActivated       = TFALSE;
	m_fFOV               = Toshi::TMath::DegToRad( 60.0f );

	auto pRootElement = AGUI2::GetRootElement();
	m_GUIElement.SetDimensions( pRootElement->GetWidth(), pRootElement->GetHeight() );
	pRootElement->AddChildTail( m_GUIElement );
	m_GUIElement.Hide();
}

TBOOL AGameState::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent )
{
	return CALL_THIS( 0x00428700, AGameState*, TBOOL, this, const Toshi::TInputInterface::InputEvent*, a_pInputEvent );
}

TBOOL AGameState::ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents )
{
	return TFALSE;
}

TBOOL AGameState::Unknown1( void* a_pUnk1, void* a_pUnk2 )
{
	return ExecuteForOneChildState( &AGameState::ChildState_Unknown1, 0, a_pUnk1, a_pUnk2 );
}

TFLOAT AGameState::GetFOV()
{
	return CALL_THIS( 0x0045b650, AGameState*, TFLOAT, this );
}

TBOOL AGameState::OnUpdate( TFLOAT a_fDeltaTime )
{
	ExecuteForAllChildStates( &AGameState::ChildState_OnUpdate, 0, a_fDeltaTime );
	return TTRUE;
}

void AGameState::OnInsertion()
{
	m_bWasInserted = TTRUE;
	ExecuteForAllChildStates( &AGameState::ChildState_OnInsertion, 0 );
}

void AGameState::OnRemoval()
{
	ExecuteForAllChildStates( &AGameState::ChildState_OnRemoval, 0 );
}

void AGameState::OnSuspend()
{
	ExecuteForAllChildStates( &AGameState::ChildState_OnSuspend, 0 );
}

void AGameState::OnResume( AGameState* a_pPreviousState )
{
	ExecuteForAllChildStates( &AGameState::ChildState_OnResume, 0, a_pPreviousState );
}

void AGameState::OnActivate()
{
	m_bIsActivated = TTRUE;
	ExecuteForAllChildStates( &AGameState::ChildState_OnActivate, 0 );
}

void AGameState::OnDeactivate()
{
	m_bIsActivated = TFALSE;
	ExecuteForAllChildStates( &AGameState::ChildState_OnDeactivate, 0 );
}

TBOOL AGameState::SendInputCommands( const Toshi::TInputInterface::InputEvent* a_pEvent )
{
	return CALL_THIS( 0x00428730, AGameState*, TBOOL, this, const Toshi::TInputInterface::InputEvent*, a_pEvent );
}

void AGameState::SetInputMap( const Toshi::TPString8& a_MapName )
{
	CALL_THIS( 0x00428320, AGameState*, void, this, const Toshi::TPString8&, a_MapName );
}

void AGameState::Remove()
{
	CALL_THIS( 0x004282d0, AGameState*, void, this );
}
