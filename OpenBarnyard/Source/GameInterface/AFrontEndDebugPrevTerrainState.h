#pragma once
#include "AGameState.h"
#include "GUI/AGUI2Menu.h"
#include "GUI/AGUI2Button.h"

//-----------------------------------------------------------------------------
// Purpose: developer list of the 57 terrains of the game ("terrain preview").
// Picking a terrain only hides the list: nothing in any build loads it (the
// loading code was stripped or never written).
// PC retail only has the TClass (size 0x160, parent AGameState). Recovered from
// the PS2 build (SLUS_212.77, vtable 0x5056a0), which keeps every virtual but not
// the constructor (see Docs/PS2DeadCode.md).
// NOTE: in community builds it's the third item of AFrontEndDebugState.
//-----------------------------------------------------------------------------
class AFrontEndDebugPrevTerrainState
    : public AGameState
{
public:
	TDECLARE_CLASS( AFrontEndDebugPrevTerrainState, AGameState );

	static constexpr TINT NUM_TERRAINS = 57;

public:
	AFrontEndDebugPrevTerrainState();
	~AFrontEndDebugPrevTerrainState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

private:
	// AGUI2Menu activation callback (a_pUserData is the state)
	static void MenuItemActivationCallback( void* a_pUserData, AGUI2MenuItem* a_pItem );

	// Hides the list
	void OnTerrainChosen();

private:
	AGUI2Menu      m_oMenu;                   // 0xa4 (PS2: 0xa8)
	AGUI2Button*   m_pButtons;                // 0x148, NUM_TERRAINS buttons
	const TWCHAR** m_pTerrainNames;           // 0x14c, NUM_TERRAINS names
	TINT           m_iUnk150;                 // 0x150, not used by any surviving method
	TFLOAT         m_fVisibilityLOD0Distance; // 0x154, AInstanceManager values when inserted
	TFLOAT         m_fVisibilityBlendEnd;     // 0x158
	TFLOAT         m_fVisibilityBlendStart;   // 0x15c
};

TSTATICASSERT( sizeof( AFrontEndDebugPrevTerrainState ) == 0x160 );
