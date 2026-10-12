#pragma once
#include "GameInterface/AGameState.h"
#include "GUI/AGUI2TextBox.h"

#include <GUI/T2GUIRectangle.h>

class AUnit;
class ASimPlayerMainHudElement;

//-----------------------------------------------------------------------------
// Purpose: the super squirt (high squirt) micro game. The player puts on the
// sunglasses and squirts milk as far as possible from one of the
// "highsquirt_NN" spots. ASimpleProjectileManager tracks the distance into
// the high score record of the closest spot, and a panel above the hint shows
// the best score of that record (face, name and distance).
// Pushed by name (interaction data), see ASpecialEventRequirement::IsSuperSquirtAvailable.
// TClass 00783bb8, vtable 00723b30, size 0x588.
//-----------------------------------------------------------------------------
class ASuperSquirtMicroGame
    : public AGameState
{
public:
	TDECLARE_CLASS( ASuperSquirtMicroGame, AGameState );

	static constexpr TINT NUM_HIGHSQUIRT_LOCATORS = 14;

	enum STATE : TINT
	{
		STATE_PUT_AWAY_HAND_ITEM, // waits until the hand is empty
		STATE_TAKE_OUT_SUNGLASSES,
		STATE_WAIT_FOR_TOOL, // waits until the player carries the sunglasses
		STATE_SELECT_LAUNCHER,
		STATE_PLAYING,
	};

public:
	ASuperSquirtMicroGame();
	~ASuperSquirtMicroGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	// NOTE: ProcessInput (+0x0c) is a thunk to AGameState::ProcessInput (00445300)
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

private:
	// Selects the secondary launcher, finds the record and starts tracking the projectiles
	void Start();

	// (Re)creates the best score panel of m_iRecord
	void CreateBestScorePanel();

	// Keeps the reticle of the aimed squirt unlocked at the cursor
	void UpdateReticle( TFLOAT a_fDeltaTime );

	// Returns the high score record of the "highsquirt_NN" locator closest to the player
	TINT FindClosestRecord();

private:
	AUnit*                    m_pPlayer;           // 0xa4
	TWCHAR                    m_wszHint[ 128 ];    // 0xa8
	TWCHAR                    m_wszScore[ 128 ];   // 0x1a8
	AGUI2TextBox              m_oHint;             // 0x2a8
	AGUI2TextBox              m_oName;             // 0x320, name of the best player
	AGUI2TextBox              m_oScore;            // 0x398, the best score
	Toshi::T2GUIRectangle     m_oFace;             // 0x410, face of the best player
	Toshi::T2GUIRectangle     m_oPanelRoot;        // 0x488
	Toshi::T2GUIRectangle     m_oPanel;            // 0x500, "Panel_01"
	ASimPlayerMainHudElement* m_pMainHud;          // 0x578
	TINT                      m_iRecord;           // 0x57c
	STATE                     m_eState;            // 0x580
	TBOOL                     m_bReady;            // 0x584, the player carries the sunglasses
	TBOOL                     m_bMiniMapForceHide; // 0x585, saved ASimMiniMap force hide flag
};

TSTATICASSERT( sizeof( ASuperSquirtMicroGame ) == 0x588 );
