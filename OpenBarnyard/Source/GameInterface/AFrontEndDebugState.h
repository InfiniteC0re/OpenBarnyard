#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"

//-----------------------------------------------------------------------------
// Purpose: the developer front end menu ("DEBUG MENU"). "Debug MiniGames" opens
// the character selection followed by the minigame list in debug mode (all
// minigames, AFrontEndMiniGameState2( TFALSE )), "Preview Scripted Sequences"
// the character selection followed by AFrontEndDebugSSeqState.
// PC retail only has the TClass (size 0xbe4, parent ABYardMenuState) and the
// labels (unreferenced at 0x0077263c). Recovered from the PS2 build (SLUS_212.77,
// vtable 0x505878, size 0xbfc), which keeps every virtual but not the
// constructor (see Docs/PS2DeadCode.md).
// Reachable in community builds with "-devsseq" (main menu, Antics button).
//-----------------------------------------------------------------------------
class AFrontEndDebugState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndDebugState, ABYardMenuState );

	enum ITEM
	{
		ITEM_DEBUG_MINIGAMES,
		ITEM_PREVIEW_SSEQ,
		ITEM_NUMOF_ORIGINAL,
#ifdef BARNYARD_COMMUNITY_PATCH
		// NOTE: not in any build, added to reach AFrontEndDebugPrevTerrainState
		ITEM_PREVIEW_TERRAIN = ITEM_NUMOF_ORIGINAL,
		ITEM_NUMOF,
#else
		ITEM_NUMOF = ITEM_NUMOF_ORIGINAL,
#endif
	};

public:
	AFrontEndDebugState();
	~AFrontEndDebugState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ABYardMenuState
	//-----------------------------------------------------------------------------
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	AGUI2Button m_aButtons[ ITEM_NUMOF ]; // PS2: 0x7e0
	TINT        m_iSelectedItem;          // PS2: 0xbf8
};

// The PC TClass size, with the 2 original buttons
TSTATICASSERT( sizeof( ABYardMenuState ) + AFrontEndDebugState::ITEM_NUMOF_ORIGINAL * sizeof( AGUI2Button ) + sizeof( TINT ) == 0xbe4 );
