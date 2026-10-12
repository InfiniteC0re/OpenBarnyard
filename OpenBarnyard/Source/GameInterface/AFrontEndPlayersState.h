#pragma once
#include "ABYardMenuState.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2TextBox.h"

//-----------------------------------------------------------------------------
// Purpose: a state that allows selection of the number of players that can
// join into a multiplayer game (Barnyard Antics with 2-4 controllers).
// NOTE: recovered from the original source. This is an abandoned developer test
// that didn't ship on any platform: PC retail only has its TClass registration
// (0x00706580) and the PS2 builds only have the class name. Only reachable with
// the BARNYARD_COMMUNITY_PATCH "-multiplayerantics" switch.
//-----------------------------------------------------------------------------
class AFrontEndPlayersState
    : public ABYardMenuState
{
public:
	TDECLARE_CLASS( AFrontEndPlayersState, ABYardMenuState );

	// NOTE: only Item_NUMOF is known from the original source
	enum Item
	{
		Item_2Players,
		Item_3Players,
		Item_4Players,
		Item_NUMOF
	};

	// MARK is this required ???
	static const TINT sa_iMenuItemLocaleIDs[ Item_NUMOF ];

public:
	//! Default constructor
	AFrontEndPlayersState();

	//! Destructor.
	~AFrontEndPlayersState();

	// AGameState
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	// ABYardMenuState
	virtual void OnMenuItemActivated( AGUI2MenuItem* a_pMenuItem ) OVERRIDE;
	virtual void OnMenuItemFocused( AGUI2MenuItem& a_rOldFocus, AGUI2MenuItem& a_rNewFocus ) OVERRIDE;
	virtual void OnMenuClose() OVERRIDE;

private:
	AGUI2Button  m_aMenuItems[ Item_NUMOF ];
	TINT         m_iSelectedItem;
	AGUI2TextBox m_oControllersPrompt;
};
