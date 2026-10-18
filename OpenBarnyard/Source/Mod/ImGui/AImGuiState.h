#pragma once
#include "GameInterface/AGameState.h"

//-----------------------------------------------------------------------------
// Game state pushed while the ImGui settings window is open. Pauses the game
// and shows the mouse cursor.
//-----------------------------------------------------------------------------
class AImGuiState
    : public AGameState
{
public:
	TDECLARE_CLASS( AImGuiState, AGameState );

public:
	AImGuiState();

	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) override;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnInsertion() override;
	virtual void  OnRemoval() override;
	virtual void  OnActivate() override;
	virtual void  OnDeactivate() override;

private:
	TBOOL m_bWasPaused;
};
