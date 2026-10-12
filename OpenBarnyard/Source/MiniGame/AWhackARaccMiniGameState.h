#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2FadingText.h"
#include "GUI/AGUI2FloatHudElement.h"
#include "GUI/AGUI2Portrait.h"

#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class AGUI2AnythingPopup;
class AGUI2LifeHud;

namespace Toshi
{
class TModelInstance;
}

//-----------------------------------------------------------------------------
// Whack-a-Racc (minigame "AWhackARaccMiniGameState", "Whack").
// TClass 0078d04c, vtable 00720998 (AInventoryAniEffect::CallBackInterface 00720990), size 0x3760.
//
// Raccoons, barn bucks and four special characters (Freddy, Peck, an egg and a
// coyote) pop out of a 3x3 grid of holes. The player moves a ring over the
// holes and swings the mallet: raccoons, Peck, the egg and the coyote give
// points, barn bucks are collected, hitting Freddy costs a life. A level is
// passed when its target score is reached before its time runs out; the
// dials on the machine show the score of the level. Up to 4 players take turns.
// The levels (when and what pops out, in which formations) come from
// Data/MinigameData/WhackARac/whackarac.trb. All the actors are plain
// AAnimatableObjects, there are no units.
//-----------------------------------------------------------------------------
class AWhackARaccMiniGameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( AWhackARaccMiniGameState, AMiniGameState );

	static constexpr TINT NUM_HOLES      = 9;
	static constexpr TINT NUM_ROWS       = 3;
	static constexpr TINT NUM_RACCOONS   = 9;
	static constexpr TINT NUM_BUCKS      = 20;
	static constexpr TINT NUM_SPECIALS   = 4;
	static constexpr TINT NUM_DIALS      = 4;
	static constexpr TINT MAX_LEVELS     = 10;
	static constexpr TINT MAX_FORMATIONS = 100;
	static constexpr TINT NUM_FLOATS     = 3;

	static constexpr TINT HIGHSCORE_TABLE = 42; // AHighScoreManager::AddScore

	// Locale strings
	static constexpr TINT LOCALE_RACCOONS  = 579;
	static constexpr TINT LOCALE_COYOTES   = 578;
	static constexpr TINT LOCALE_EGGS      = 682;
	static constexpr TINT LOCALE_PECKS     = 683;
	static constexpr TINT LOCALE_GAME_OVER = 509;
	static constexpr TINT LOCALE_PERFECT   = 511;
	static constexpr TINT LOCALE_LEVEL     = 994; // "Level {0}"

	// whackarac.trb "Ints"
	enum INT_ : TINT
	{
		INT_RACCOON_SCORE, // 0
		INT_COYOTE_SCORE,  // 1
		INT_PECK_SCORE,    // 2
		INT_EGG_SCORE,     // 3
		INT_FREDDY_SCORE,  // 4, only checked for > 0
		INT_LIVES,         // 5
		INT_NUMOF
	};

	// whackarac.trb "Floats"
	enum FLOAT_ : TINT
	{
		FLOAT_BAD_RING_TIME,    // 0, the red ring after Freddy was hit
		FLOAT_MESSAGE_TIME,     // 1, the "perfect" message and the pause after it
		FLOAT_SHAKE_AMOUNT,     // 2
		FLOAT_SHAKE_INTERVAL,   // 3
		FLOAT_SHAKE_DURATION,   // 4
		FLOAT_GAME_OVER_UPTIME, // 5, how long the raccoons stay up when the game is lost
		FLOAT_HOLE_COOLDOWN,    // 6, before the same hole can be whacked again
		FLOAT_WHACK_COOLDOWN,   // 7, before the mallet can be swung again
		FLOAT_NUMOF
	};

	// The special characters
	enum SPECIAL_ : TINT
	{
		SPECIAL_FREDDY, // costs a life
		SPECIAL_PECK,
		SPECIAL_EGG,
		SPECIAL_COYOTE,
		SPECIAL_NUMOF
	};

	// What a hole contains
	enum KIND_ : TINT
	{
		KIND_RACCOON,
		KIND_SPECIAL,
		KIND_BUCK,
	};

	enum MOLESTATE_ : TINT
	{
		MOLESTATE_HIDDEN, // 0
		MOLESTATE_HIT,    // 1
		MOLESTATE_FREE,   // 2, frees the hole
		MOLESTATE_RISING, // 3, the "up" animation
		MOLESTATE_UP,     // 4, barn bucks are flying
		MOLESTATE_HIDING, // 5, the "down" animation
	};

	enum BUCKSTATE_ : TINT
	{
		BUCKSTATE_NONE,
		BUCKSTATE_FLYING,  // 1
		BUCKSTATE_HOPPING, // 2, about to disappear
		BUCKSTATE_FALLING, // 3, falls through the ground
	};

	enum MALLETSTATE_ : TINT
	{
		MALLETSTATE_IDLE,         // 0
		MALLETSTATE_SWING,        // 1
		MALLETSTATE_IMPACT,       // 2, "hole_N_imp"
		MALLETSTATE_IMPACT_DONE,  // 3
		MALLETSTATE_RECOVER,      // 4, "hole_N_rec"
		MALLETSTATE_RECOVER_DONE, // 5
		MALLETSTATE_RESET,        // 6, "hole_N_rst"
	};

	enum EFFECT_ : TINT
	{
		EFFECT_MISS,     // "whackmiss"
		EFFECT_PUFF,     // "whackpuff"
		EFFECT_DUST,     // "whackdust"
		EFFECT_BAD,      // "whackbad"
		EFFECT_EGGSPLAT, // "eggsplat"
		EFFECT_EGGSHELL, // "eggshell"
		EFFECT_EGGYOLK,  // "eggyolk"
		EFFECT_NUMOF
	};

	// A raccoon, a barn buck or a special character
	struct Mole
	{
		AAnimatableObject* pObject; // 0x00
		Toshi::TVector4    vPos;    // 0x04
		TINT               eState;  // 0x14, MOLESTATE_
		TFLOAT             fTimer;  // 0x18, time left up
		TINT               iHole;   // 0x1c
		TUINT8             PADDING1[ 8 ];
		TINT               eBuckState; // 0x28, BUCKSTATE_
		TFLOAT             fFallTime;  // 0x2c
		TFLOAT             fVelocity;  // 0x30, Y goes down
		Toshi::TVector4    vGround;    // 0x34
		TBOOL              bLanded;    // 0x44
	};

	struct Hole
	{
		Toshi::TVector4 vPos;      // 0x00, locator "hole_N"
		TBOOL           bOccupied; // 0x10
		TINT            iObject;   // 0x14, index into the array of eKind
		TINT            eKind;     // 0x18, KIND_
		TFLOAT          fCooldown; // 0x1c
	};

	// A dial showing a digit of the level score
	struct Dial
	{
		Toshi::TVector4    vPos;    // 0x00, locator "score_N"
		AAnimatableObject* pObject; // 0x10
		TFLOAT             fAngle;  // 0x14
	};

	// A pop out of a level
	struct Form
	{
		// Form::iType: >= 0 count of random holes, one of the specials, otherwise -1 - formation
		enum TYPE_ : TINT
		{
			TYPE_FREDDY = -100,
			TYPE_EGG    = -200,
			TYPE_COYOTE = -300,
			TYPE_PECK   = -400,
		};

		TFLOAT fTime;      // 0x00, time into the level
		TINT   iType;      // 0x04
		TFLOAT fStayTime;  // 0x08, 0 = random
		TINT   bTriggered; // 0x0c
		TINT   eKind;      // 0x10, KIND_
	};

	struct Level
	{
		TINT   iNumForms;    // 0x00
		TINT   iTargetScore; // 0x04
		TFLOAT fTime;        // 0x08
		TFLOAT fMaxStayTime; // 0x0c, of the random stay times
		TINT   aUnk[ 4 ];    // 0x10
		Form*  pForms;       // 0x20
	};

	struct Formation
	{
		TINT aHoles[ NUM_HOLES ]; // 0x00
		TINT iCount;              // 0x24
	};

	struct Player
	{
		TINT            iLives;          // 0x00
		TINT            iRow;            // 0x04
		TINT            iCol;            // 0x08
		TINT            iHole;           // 0x0c, under the ring, iCol + iRow * 3
		TINT            iPrevLevelScore; // 0x10
		TINT            iLevelScore;     // 0x14
		TINT            iNumMisses;      // 0x18
		TINT            iScore;          // 0x1c
		TINT            iNumMisses2;     // 0x20
		TINT            iMalletHole;     // 0x24, the hole the mallet goes for
		TINT            eMalletState;    // 0x28, MALLETSTATE_
		TUINT8          PADDING1[ 8 ];   // 0x2c
		Toshi::TVector4 vUnk1;           // 0x34, (0, 0, -1)
		Toshi::TVector4 vBadRingPos;     // 0x44
		Toshi::TVector4 vUnk2;           // 0x54, (0, 0, -1)
		TFLOAT          fRingPulse;      // 0x64, phase of the ring pulse
		TBOOL           bFailed;         // 0x68, the level wasn't passed
		TINT            iLevel;          // 0x6c
		TINT            iRaccoonScore;   // 0x70
		TINT            iEggScore;       // 0x74
		TINT            iPeckScore;      // 0x78
		TINT            iCoyoteScore;    // 0x7c
		TBOOL           bPerfect;        // 0x80, nothing got away
		TINT            iNumFreddyHits;  // 0x84
	};

public:
	AWhackARaccMiniGameState();
	~AWhackARaccMiniGameState();

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
	// AInventoryAniEffect::CallBackInterface
	//-----------------------------------------------------------------------------
	virtual void OnObjectCollected( AAnimatableObject* a_pObject ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void  Pause() OVERRIDE;            // plays the intro cut-scene
	virtual void  PlayLoseSequence() OVERRIDE; // plays the lose cut-scene
	virtual void  PlayWinSequence() OVERRIDE;  // plays the win cut-scene
	virtual void  OnPreStart() OVERRIDE;
	virtual TBOOL IsGameOver() OVERRIDE;
	virtual void  UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*   GetResults() OVERRIDE;
	virtual void  OnStartHudFinished() OVERRIDE {} // DOES_NOTHING

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	Player& GetCurrentPlayer() { return m_aPlayers[ m_iCurrentPlayer ]; }

private:
	void LoadData();                             // 004d8a40
	void ShowMessage( const TWCHAR* a_wszText ); // 004d8f40
	void ResetLevel( TBOOL a_bNewGame );         // 004d8fb0
	void EndGame();                              // 004d9ab0
	void RemoveBuck( Mole* a_pBuck );            // 004d9bd0
	void StartTurn( TBOOL a_bNewGame );          // 004da480
	void UpdateSpecials( TFLOAT a_fDeltaTime );  // 004da6f0
	void Whack( TINT a_iPlayer );                // 004da930
	void UpdateDials( TFLOAT a_fDeltaTime );     // 004db0b0
	void UpdateMallet( TFLOAT a_fDeltaTime );    // 004db2b0
	void UpdateRaccoons( TFLOAT a_fDeltaTime );  // 004db600
	void LaunchBuck( Mole* a_pBuck );            // 004db830
	void UpdateBucks( TFLOAT a_fDeltaTime );     // 004db9e0
	void UpdateForms( TFLOAT a_fDeltaTime );     // 004dbe00
	void StartGame();                            // 004dc1d0

	// NOTE: not separate functions in the original, these are repeated inline
	void HideHud();
	void ShowHud();
	void ResetDials();

	void RenderRing( Toshi::TModelInstance* a_pInstance );    // 004d8ca0
	void RenderBadRing( Toshi::TModelInstance* a_pInstance ); // 004d8dd0

	static void RenderRingCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );                   // 004d8f10
	static void RenderBadRingCallback( Toshi::TModelInstance* a_pInstance, void* a_pUserData );                // 004d8ef0
	static void OnIntroCueReset( TINT a_iType, TINT a_iCueIndex, void* a_pUserData );                          // 004d98c0
	static TBOOL __stdcall OnMalletImpact( AWhackARaccMiniGameState* a_pThis, void* a_pOwner, void* a_pData ); // 004dc1b0

#ifdef BARNYARD_COMMUNITY_PATCH
public:
	// -devautoplay: moves the ring to the best target and swings the mallet
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	AAnimatableObject*      m_pMallet;                         // 0x0bac, "wacmallet"
	AAnimatableObject*      m_pRing;                           // 0x0bb0, "wackring"
	AAnimatableObject*      m_pBadRing;                        // 0x0bb4, "wackring", red, after Freddy was hit
	Player                  m_aPlayers[ MAX_PLAYERS ];         // 0x0bb8
	TINT                    m_aInts[ INT_NUMOF ];              // 0x0dd8, whackarac.trb "Ints"
	TFLOAT                  m_aFloats[ FLOAT_NUMOF ];          // 0x0df0, whackarac.trb "Floats"
	Level                   m_aLevels[ MAX_LEVELS ];           // 0x0e10
	Formation               m_aFormations[ MAX_FORMATIONS ];   // 0x0f78
	TUINT8                  PADDING1[ 4 ];                     // 0x1f18
	Toshi::TGenericListener m_oImpactListener;                 // 0x1f1c, mallet "impactbp" -> OnMalletImpact
	Mole                    m_aRaccoons[ NUM_RACCOONS ];       // 0x1f30, "racforwaca"
	Mole                    m_aBucks[ NUM_BUCKS ];             // 0x21b8, "barnbuck"
	Mole                    m_aSpecials[ SPECIAL_NUMOF ];      // 0x2758
	Hole                    m_aHoles[ NUM_HOLES ];             // 0x2878
	Dial                    m_aDials[ NUM_DIALS ];             // 0x2998, "wacadial"
	AAnimatableObject*      m_pArrow;                          // 0x29f8, "wacaarrow", the clock hand
	Toshi::TVector4         m_vArrowPos;                       // 0x29fc
	TINT                    m_iLevel;                          // 0x2a0c
	AGUI2FadingText         m_oLevelText;                      // 0x2a10
	TBOOL                   m_bBadHit;                         // 0x2b84, Freddy was hit, the rings are red
	TFLOAT                  m_fBadRingTime;                    // 0x2b88
	TINT                    m_iLoopCue;                        // 0x2b8c
	TINT                    m_iIntroCue;                       // 0x2b90
	TINT                    m_iUnkCue;                         // 0x2b94, only stopped
	AGUI2FloatHudElement    m_oScoreFloats;                    // 0x2b98
	AGUI2Portrait           m_oPortrait;                       // 0x2d8c
	TWCHAR                  m_awszFloats[ NUM_FLOATS ][ 128 ]; // 0x3354
	TINT                    m_iFloatIndex;                     // 0x3654
	Toshi::TPString8        m_aHoleImpAnims[ NUM_HOLES ];      // 0x3658, "hole_N_imp"
	Toshi::TPString8        m_aHoleRecAnims[ NUM_HOLES ];      // 0x367c, "hole_N_rec"
	Toshi::TPString8        m_aHoleRstAnims[ NUM_HOLES ];      // 0x36a0, "hole_N_rst"
	TINT                    m_iCurrentPlayer;                  // 0x36c4
	AGUI2AnythingPopup*     m_pMessage;                        // 0x36c8
	AGUI2LifeHud*           m_pLifeHud;                        // 0x36cc
	TFLOAT                  m_fMessageTimer;                   // 0x36d0, the game waits while > 0, -1 = no message
	TBOOL                   m_bUnk1;                           // 0x36d4
	TBOOL                   m_bTurnEnded;                      // 0x36d5
	TBOOL                   m_bRestartPending;                 // 0x36d6
	Toshi::TVector2         m_vShake;                          // 0x36d8
	TFLOAT                  m_fShakeTimer;                     // 0x36e0
	TFLOAT                  m_fShakeTimeLeft;                  // 0x36e4
	TFLOAT                  m_fWhackCooldown;                  // 0x36e8
	TINT                    m_iVoiceCue;                       // 0x36ec
	TINT                    m_iFreddyCue;                      // 0x36f0
	TINT                    m_iCrowdCue;                       // 0x36f4
	TBOOL                   m_bGameEnding;                     // 0x36f8, lost, the raccoons laugh
	TBOOL                   m_bMouseControl;                   // 0x36f9
	TBOOL                   m_bKeyboardMove;                   // 0x36fa
	TINT                    m_aEffects[ EFFECT_NUMOF ];        // 0x36fc
	TINT                    m_iNumLevels;                      // 0x3718
	TBOOL                   m_bShowLevelText;                  // 0x371c
	TWCHAR                  m_wszLevelText[ 32 ];              // 0x371e
};

TSTATICASSERT( sizeof( AWhackARaccMiniGameState ) == 0x3760 );
TSTATICASSERT( sizeof( AWhackARaccMiniGameState::Mole ) == 0x48 );
TSTATICASSERT( sizeof( AWhackARaccMiniGameState::Player ) == 0x88 );
TSTATICASSERT( sizeof( AWhackARaccMiniGameState::Level ) == 0x24 );
