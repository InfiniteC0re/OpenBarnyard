#pragma once
#include "AGameState.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

//-----------------------------------------------------------------------------
// Pushed by ATerrainInterface::Update when the camera is above a terrain section
// that isn't loaded yet. Pauses the game and shows a loading message until the
// terrain is loaded.
//-----------------------------------------------------------------------------
class ATerrainLoadingStallState : public AGameState
{
public:
	TDECLARE_CLASS( ATerrainLoadingStallState, AGameState );

public:
	ATerrainLoadingStallState();
	~ATerrainLoadingStallState();

	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override;
	virtual void  OnActivate() override;
	virtual void  OnDeactivate() override;

private:
	AGUI2TextBox          m_oText;       // 0xa4
	Toshi::T2GUIRectangle m_oBackground; // 0x11c
};
