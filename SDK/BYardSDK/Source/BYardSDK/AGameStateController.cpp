#include "pch.h"
#include "AGameStateController.h"

TOSHI_NAMESPACE_USING

TBOOL AGameStateController::OnCreate()
{
	return TTRUE;
}

TBOOL AGameStateController::OnUpdate( TFLOAT a_fDeltaTime )
{
	return TTRUE;
}

void AGameStateController::OnDestroy()
{
}

void AGameStateController::InsertGameState( AGameState* a_pGameState )
{
	CALL_THIS( 0x00429b00, AGameStateController*, void, this, AGameState*, a_pGameState );
}

void AGameStateController::ReplaceState( AGameState* a_pGameState )
{
	CALL_THIS( 0x0042a090, AGameStateController*, void, this, AGameState*, a_pGameState );
}

void AGameStateController::PushState( AGameState* a_pGameState )
{
	CALL_THIS( 0x00429f50, AGameStateController*, void, this, AGameState*, a_pGameState );
}

void AGameStateController::PopState( AGameState* a_pGameState, void* a_pUnused )
{
	CALL_THIS( 0x00429fb0, AGameStateController*, void, this, AGameState*, a_pGameState, void*, a_pUnused );
}

void AGameStateController::PopCurrentGameState()
{
	CALL_THIS( 0x0042a870, AGameStateController*, void, this );
}

void AGameStateController::UpdateScreenOverlay()
{
	CALL_THIS( 0x00429490, AGameStateController*, void, this );
}

void AGameStateController::ResetStack()
{
	CALL_THIS( 0x0042a150, AGameStateController*, void, this );
}

TBOOL AGameStateController::ProcessInput( const Toshi::TInputInterface::InputEvent* a_pEvent )
{
	return CALL_THIS( 0x004293d0, AGameStateController*, TBOOL, this, const Toshi::TInputInterface::InputEvent*, a_pEvent );
}
