#pragma once
#include "AMiniGameState.h"
#include "ADartBoard.h"
#include "HUD/ADartCountHudElement.h"
#include "HUD/ADartFireHudElement.h"
#include "HUD/AMiniGameRoundHudElement.h"
#include "HUD/AInteractionHudElement.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AGUI2TextBox.h"
#include "Units/APlayerControlComponent.h"
#include "Helpers/AGameTimeManager.h"
#include "Player/APlayerProfile.h"

#include <File/TTRB.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/TPString8.h>

//-----------------------------------------------------------------------------
// Purpose: Data of a round of the Darts minigame
// (Data/MiniGameData/Darts/dartsdata.trb, symbol "dartsdata", 3 rounds).
// NOTE: no name in the original.
//-----------------------------------------------------------------------------
struct ADartsRoundData
{
	TINT   iUnk;              // 0x00, 1 sets ADartMiniGame::m_bRoundFlag
	TFLOAT fInnerRingSpeed;   // 0x04, degrees per second
	TFLOAT fMiddleRingSpeed;  // 0x08
	TFLOAT fOuterRingSpeed;   // 0x0c, the cogs spin if it isn't 0
	TFLOAT fAimTimeBonus;     // 0x10, human
	TFLOAT fAimTime;          // 0x14, human
	TFLOAT fAimFrequencyX;    // 0x18, also added to the time of the horizontal aim
	TFLOAT fAimFrequencyY;    // 0x1c
	TFLOAT fUnk20;            // 0x20
	TFLOAT fAiErrorY;         // 0x24, percent
	TFLOAT fAiErrorX;         // 0x28, percent
	TFLOAT fRaiseTime;        // 0x2c, time before the next dart is raised
	TFLOAT fAiFireTimeXMin;   // 0x30
	TFLOAT fAiFireTimeXMax;   // 0x34
	TFLOAT fAiFireTimeYMin;   // 0x38
	TFLOAT fAiFireTimeYMax;   // 0x3c
	TFLOAT fUnk40;            // 0x40
	TFLOAT fAiAimTime;        // 0x44
	TFLOAT fAiAimTimeBonus;   // 0x48
	TFLOAT fTimeBonusPlayer1; // 0x4c, points per second of time left
	TFLOAT fTimeBonusPlayer2; // 0x50
	TFLOAT fScoreMultiplier;  // 0x54
};

TSTATICASSERT( sizeof( ADartsRoundData ) == 0x58 );

//-----------------------------------------------------------------------------
// Purpose: The Darts minigame (registered as "Darts", 2 players, 3 rounds).
// Two players (the second one is the computer, "Root") throw ten darts each
// per round at a board with rotating rings of yellow and blue targets. The
// first player aims with two bars (horizontal then vertical, each stopped
// with the action button) before the time runs out; hitting a target of
// your own colour scores its points (plus a time bonus) and knocks it off,
// a target of the other colour scores for the other player.
// TClass 0078387c (parent AMiniGameState), vtables 00722dd0 (AMiniGameState),
// 00722a44 (AInventoryAniEffect::CallBackInterface), 00722dc8
// (AUnitActionReceiver), size 0x5338.
//-----------------------------------------------------------------------------
class ADartMiniGame
    : public AMiniGameState
    , public AUnitActionReceiver
{
public:
	TDECLARE_CLASS( ADartMiniGame, AMiniGameState );

	static constexpr TINT NUM_PLAYERS = 2;
	static constexpr TINT NUM_DARTS   = 10;
	static constexpr TINT NUM_ROUNDS  = 3;

	static constexpr TINT HIGHSCORE_TABLE = 41;

	struct Player
	{
		// $Barnyard: FUNCTION 00524de0
		Player() = default;

		// $Barnyard: FUNCTION 00524e10
		~Player() = default;

		AUnit*                 pUnit;                      // 0x000
		ADartCountHudElement*  pCountHud;                  // 0x004
		AGUI2Portrait*         pPortrait;                  // 0x008
		AGUI2TextBox*          pBonusText;                 // 0x00c
		ADart                  aDarts[ NUM_DARTS ];        // 0x010
		Toshi::TPString8       strDartBone;                // 0x1f0, "dart_loc"
		TINT                   aCategoryScores[ 3 ];       // 0x1f4
		TINT                   iScore;                     // 0x200
		TINT                   iTimeBonus;                 // 0x204
		TINT                   iCurrentDart;               // 0x208
		TINT                   aRoundScores[ NUM_ROUNDS ]; // 0x20c
		TINT                   bComputer;                  // 0x218
		ADartThrow::TEAM       iTeam;                      // 0x21c
		APlayerProfile::GENDER eGender;                    // 0x220
		TWCHAR                 wszBonus[ 24 ];             // 0x224
		TFLOAT                 fBonusAlpha;                // 0x254
		TFLOAT                 afColour[ 3 ];              // 0x258
	};

public:
	ADartMiniGame();
	~ADartMiniGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual ASoundId GetSound() OVERRIDE;
	virtual void     OnPause() OVERRIDE;
	virtual void     OnInsertion() OVERRIDE;
	virtual void     OnRemoval() OVERRIDE;
	virtual void     OnActivate() OVERRIDE;
	virtual void     OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void  Pause() OVERRIDE;
	virtual void  OnPreStart() OVERRIDE;
	virtual TBOOL IsGameOver() OVERRIDE;
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdatePreStart( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*   GetResults() OVERRIDE;
	virtual void  OnStartHudFinished() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AUnitActionReceiver
	//-----------------------------------------------------------------------------
	virtual void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent ) OVERRIDE;

private:
	// Sets the ring speeds and the score multiplier of the round
	void SetupRound( TINT a_iRound );

	// Turns the current player towards the aimed point
	void UpdatePlayerFacing();

	// Spends a_fDeltaTime of the throw time and updates the clock of the current player
	void UpdateThrowTime( ADartThrow& a_rThrow, TFLOAT a_fTime, TFLOAT a_fDeltaTime );

	// Highlights the HUD of the player whose turn it is
	void SetPlayerActive( TINT a_iPlayer, TBOOL a_bActive );

	// Shows the time bonus of a player
	void ShowTimeBonus( TINT a_iPlayer, TINT a_iBonus );

	void SetVisible( TBOOL a_bVisible );

	// The ADartThrowAnimController of a player
	static class ADartThrowAnimController* GetThrowController( Player& a_rPlayer );

	// Points the camera at the board
	void SetupCamera();

	// Turns the players towards the camera
	void FacePlayersToCamera();

	// Moves the horizontal aim of the current player
	void UpdateAimX( ADartThrow& a_rThrow, TFLOAT a_fDeltaTime );

	// Moves the vertical aim of the current player
	void UpdateAimY( ADartThrow& a_rThrow, TFLOAT a_fDeltaTime );

	// Plays the comment on a throw
	void PlayHitSound( TINT a_iPlayer, ADartThrow& a_rThrow );

	// Plays the intro sequence
	void PlayIntro();

	Player& GetCurrentPlayer() { return m_aPlayers[ m_iCurrentPlayer ]; }
	ADart&  GetCurrentDart() { return GetCurrentPlayer().aDarts[ GetCurrentPlayer().iCurrentDart ]; }

	const ADartsRoundData& GetRoundData() const { return m_pData[ m_iRound ]; }

private:
	Player                     m_aPlayers[ NUM_PLAYERS ];            // 0x0bc0
	TWCHAR                     m_awszRoundNames[ NUM_ROUNDS ][ 64 ]; // 0x1088
	AMiniGameRoundHudElement   m_oRoundHud;                          // 0x1208
	ADartBoard                 m_oBoard;                             // 0x1430
	ADartThrow                 m_oThrow;                             // 0x2ee4
	ADartFireHudElement        m_oFireHud;                           // 0x2f28
	ADartCountHudElement       m_aCountHuds[ NUM_PLAYERS ];          // 0x3410
	AGUI2TextBox               m_aBonusTexts[ NUM_PLAYERS ];         // 0x3ed8
	AInteractionHudElement     m_oInteractionHud;                    // 0x3fc8
	AGUI2Portrait              m_aPortraits[ NUM_PLAYERS ];          // 0x44d8
	ADart                      m_oAimDart;                           // 0x5068, follows the thrown dart for the target lock
	Toshi::TTRB                m_oDataTRB;                           // 0x5098
	ADartsRoundData*           m_pData;                              // 0x51d8
	Toshi::TVector4            m_vReleasePosition;                   // 0x51dc
	Toshi::T2GUIRectangle      m_oUnusedRect;                        // 0x51ec
	AGUI2TextBox               m_oUnusedText;                        // 0x5264
	TINT                       m_iCurrentPlayer;                     // 0x52dc
	TINT                       m_iRound;                             // 0x52e0
	AGameTimeManager::DAYPHASE m_eSavedDayPhase;                     // 0x52e4
	TFLOAT                     m_fFlightTime;                        // 0x52e8
	TFLOAT                     m_fRaiseTimer;                        // 0x52ec
	TFLOAT                     m_fResultTimer;                       // 0x52f0
	TFLOAT                     m_fAimTimerY;                         // 0x52f4
	TFLOAT                     m_fAimTimerX;                         // 0x52f8
	TFLOAT                     m_fAiTimer;                           // 0x52fc
	TFLOAT                     m_fAiTargetX;                         // 0x5300
	TFLOAT                     m_fAiTargetY;                         // 0x5304
	TFLOAT                     m_fAiFireTimeY;                       // 0x5308
	TFLOAT                     m_fAiFireTimeX;                       // 0x530c
	TFLOAT                     m_fAiErrorX;                          // 0x5310
	TFLOAT                     m_fAiErrorY;                          // 0x5314
	TFLOAT                     m_fSavedFOV;                          // 0x5318
	TFLOAT                     m_fSavedNear;                         // 0x531c
	TFLOAT                     m_fAimTime;                           // 0x5320
	TFLOAT                     m_fAimTimeBonus;                      // 0x5324
	TFLOAT                     m_fFadeTimer;                         // 0x5328
	TBOOL                      m_bFireX;                             // 0x532c
	TBOOL                      m_bFireY;                             // 0x532d
	TBOOL                      m_bGameOver;                          // 0x532e
	TBOOL                      m_bGameOverPushed;                    // 0x532f
	TBOOL                      m_bRoundFlag;                         // 0x5330
	TBOOL                      m_bCogsSpin;                          // 0x5331
	TBOOL                      m_bFading;                            // 0x5332
	TBOOL                      m_bUnk1;                              // 0x5333
	TBOOL                      m_bUnk2;                              // 0x5334
	TBOOL                      m_bFadeIn;                            // 0x5335, never set
};
