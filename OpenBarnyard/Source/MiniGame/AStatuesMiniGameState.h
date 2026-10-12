#pragma once
#include "AMiniGameState.h"
#include "HUD/AStatuesHud.h"
#include "Sound/ASoundLegacy.h"

#include <Toshi/TEvent.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>

class AAnimatableObject;

//-----------------------------------------------------------------------------
// Tease the Mailman (minigame "AStatuesMiniGameState") and Tease Mrs Beady
// (minigame "ATeaseMrsBeadyGame"). Both minigames use this state class;
// m_bIsMrsBeady picks the variant (terrain, actors, watcher AI, sounds, scores).
// TClass 007828c4, vtable 0071e4a8, size 0x2840.
//
// The player types the key combo of a row to make the statue strike that pose
// and earns stars/points while the pose animation plays. Down (Space) drops the
// player onto all fours (the quad model). If the watcher (the mailman, or Mr
// Beady) looks while the player isn't on all fours the game is lost.
// All actors are plain AAnimatableObjects (no units, AI or FSM).
// Data: Data/AStatuesMiniGameState.trb / ATeaseMrsBeadyGame.trb (libraries),
//       terrain Terrain_Statue / Terrain_EnvBeadyInt,
//       positional sounds Data/Sound/Statues.trb / MrsB.trb.
//-----------------------------------------------------------------------------
class AStatuesMiniGameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( AStatuesMiniGameState, AMiniGameState );

	static constexpr TINT NUM_ROWS             = AStatuesHud::NUM_ROWS;
	static constexpr TINT NUM_POSES            = 16;  // poses used
	static constexpr TINT MAX_POSES            = 32;  // pose slots
	static constexpr TINT MAX_ROW_POSES        = 16;  // m_aRowPoseOrder entries per row, -1 terminated
	static constexpr TINT MAX_OTHER_CHARACTERS = 8;   // Mailman: pig, freddy and otis
	static constexpr TINT NO_POSE              = -69; // m_iCurrentPose when no pose is playing

	static constexpr TFLOAT TIME_LIMIT        = 120.0f;
	static constexpr TINT   TARGET_SCORE      = 100; // Mailman
	static constexpr TINT   TARGET_SCORE_MRSB = 200; // Mrs Beady
	static constexpr TINT   CAUGHT_PENALTY    = -100;

	// Locale strings
	static constexpr TINT LOCALE_ROW_SCORE     = 1013; // 1013..1015, results row of each pose row
	static constexpr TINT LOCALE_CAUGHT        = 1016; // results row when caught
	static constexpr TINT LOCALE_INSTRUCTIONS  = 1017;
	static constexpr TINT HIGHSCORE_TABLE      = 40; // AHighScoreManager::AddScore, Mailman
	static constexpr TINT HIGHSCORE_TABLE_MRSB = 38; // AHighScoreManager::AddScore, Mrs Beady

	enum PLAYERSTATE : TINT
	{
		PLAYERSTATE_STANDING,     // 0
		PLAYERSTATE_DROPPING,     // 1
		PLAYERSTATE_ON_ALL_FOURS, // 2, the quad model is shown
		PLAYERSTATE_GETTING_UP,   // 3
		PLAYERSTATE_POSING,       // 4, a pose animation is playing
		PLAYERSTATE_GAME_OVER,    // 5
		PLAYERSTATE_NUMOF
	};

	// 0..7 are the watcher's state machine (UpdateMailman / UpdateMrsBeady), 6 = looking
	enum WATCHERSTATE : TINT
	{
		WATCHERSTATE_WAITING        = 0, // the mailman is searching, Mrs Beady is waiting
		WATCHERSTATE_TAKING_OUT     = 1, // the mailman takes out the mail
		WATCHERSTATE_SORTING        = 2, // the mailman sorts the mail
		WATCHERSTATE_BEADY_SEES     = 2, // Mrs Beady has seen the player
		WATCHERSTATE_PUTTING_IN     = 3, // the mailman puts the mail in
		WATCHERSTATE_SUSPECTING     = 4, // the mailman suspects something
		WATCHERSTATE_MR_BEADY_WAKES = 4, // Mr Beady wakes up
		WATCHERSTATE_TURNING        = 5, // the mailman turns around
		WATCHERSTATE_LOOKING        = 6,
		WATCHERSTATE_NOTHING_TO_SEE = 7,
		WATCHERSTATE_CAUGHT         = 8, // the watcher saw the player, game lost
		WATCHERSTATE_ENDED          = 9,
	};

	// Data of a pose as set by OnInsertion (the original writes every field inline).
	struct PoseDef
	{
		STATUESKEY aKeys[ AStatuesPose::MAX_KEYS ];
		TINT       aScores[ AStatuesPose::MAX_STARS ];
		TINT       iLocaleName;
		ASoundId   eSoundMailman;
		ASoundId   eSoundMrsBeady;
		TBOOL      bUnk;
	};

	// NOTE: the animations are, in order: ragingbull_1, airguitar_1, starjumps, thinker,
	// cleanandjerk, rudeboy, ragingbull_2, thepharaoh, golfswing, muscleflex, ninja,
	// robot, ballet, moonwalk, gradeabeef, hammertime (TPS8, see OnInsertion).
	static constexpr PoseDef POSES[ NUM_POSES ] = {
		/* ragingbull_1 */ { { STATUESKEY_UP, STATUESKEY_UP, STATUESKEY_UP, STATUESKEY_NONE, STATUESKEY_NONE }, { 3, 8, 15, 0, 0 }, 995, 201, 561, TFALSE },
		/* airguitar_1  */ { { STATUESKEY_UP, STATUESKEY_RIGHT, STATUESKEY_RIGHT, STATUESKEY_NONE, STATUESKEY_NONE }, { 2, 5, 10, 0, 0 }, 997, 196, 545, TTRUE },
		/* starjumps    */ { { STATUESKEY_UP, STATUESKEY_LEFT, STATUESKEY_LEFT, STATUESKEY_NONE, STATUESKEY_NONE }, { 2, 5, 10, 0, 0 }, 1000, ASOUNDID_INVALID, ASOUNDID_INVALID, TFALSE },
		/* thinker      */ { { STATUESKEY_UP, STATUESKEY_UP, STATUESKEY_LEFT, STATUESKEY_NONE, STATUESKEY_NONE }, { 2, 5, 10, 0, 0 }, 1001, 203, 554, TFALSE },
		/* cleanandjerk */ { { STATUESKEY_UP, STATUESKEY_RIGHT, STATUESKEY_LEFT, STATUESKEY_NONE, STATUESKEY_NONE }, { 3, 8, 15, 0, 0 }, 996, ASOUNDID_INVALID, ASOUNDID_INVALID, TFALSE },
		/* rudeboy      */ { { STATUESKEY_LEFT, STATUESKEY_LEFT, STATUESKEY_RIGHT, STATUESKEY_LEFT, STATUESKEY_NONE }, { 3, 8, 15, 30, 0 }, 999, ASOUNDID_INVALID, ASOUNDID_INVALID, TFALSE },
		/* ragingbull_2 */ { { STATUESKEY_LEFT, STATUESKEY_LEFT, STATUESKEY_RIGHT, STATUESKEY_UP, STATUESKEY_NONE }, { 2, 5, 10, 20, 0 }, 1002, 226, 551, TFALSE },
		/* thepharaoh   */ { { STATUESKEY_LEFT, STATUESKEY_UP, STATUESKEY_LEFT, STATUESKEY_UP, STATUESKEY_NONE }, { 2, 5, 10, 20, 0 }, 1003, 200, 550, TTRUE },
		/* golfswing    */ { { STATUESKEY_LEFT, STATUESKEY_LEFT, STATUESKEY_RIGHT, STATUESKEY_RIGHT, STATUESKEY_NONE }, { 2, 5, 10, 20, 0 }, 1004, 197, 547, TFALSE },
		/* muscleflex   */ { { STATUESKEY_LEFT, STATUESKEY_RIGHT, STATUESKEY_LEFT, STATUESKEY_LEFT, STATUESKEY_NONE }, { 3, 8, 15, 30, 0 }, 1005, ASOUNDID_INVALID, ASOUNDID_INVALID, TFALSE },
		/* ninja        */ { { STATUESKEY_RIGHT, STATUESKEY_RIGHT, STATUESKEY_UP, STATUESKEY_UP, STATUESKEY_LEFT }, { 2, 5, 10, 20, 40 }, 1006, ASOUNDID_INVALID, ASOUNDID_INVALID, TFALSE },
		/* robot        */ { { STATUESKEY_RIGHT, STATUESKEY_LEFT, STATUESKEY_UP, STATUESKEY_RIGHT, STATUESKEY_RIGHT }, { 3, 8, 15, 30, 50 }, 1008, 205, 552, TFALSE },
		/* ballet       */ { { STATUESKEY_RIGHT, STATUESKEY_UP, STATUESKEY_LEFT, STATUESKEY_UP, STATUESKEY_UP }, { 3, 8, 15, 30, 50 }, 1009, 206, 546, TTRUE },
		/* moonwalk     */ { { STATUESKEY_RIGHT, STATUESKEY_LEFT, STATUESKEY_UP, STATUESKEY_RIGHT, STATUESKEY_UP }, { 2, 5, 10, 20, 40 }, 1011, 227, 562, TFALSE },
		/* gradeabeef   */ { { STATUESKEY_RIGHT, STATUESKEY_RIGHT, STATUESKEY_RIGHT, STATUESKEY_UP, STATUESKEY_UP }, { 2, 5, 10, 20, 40 }, 998, 212, 556, TFALSE },
		/* hammertime   */ { { STATUESKEY_RIGHT, STATUESKEY_RIGHT, STATUESKEY_LEFT, STATUESKEY_UP, STATUESKEY_UP }, { 3, 8, 15, 30, 50 }, 1012, 228, 563, TFALSE },
	};

public:
	AStatuesMiniGameState();
	~AStatuesMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL IsGameOver() OVERRIDE;
	virtual void  UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE; // Mailman: camera follows the statue's camera bones
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdatePreStart( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*   GetResults() OVERRIDE;
	virtual void  OnStartHudFinished() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	void  ResetGame();                               // 004860e0
	void  UpdateMrBeadySleep( TFLOAT a_fDeltaTime ); // 004862a0
	void  AdvanceRowPose( TINT a_iRow );             // 004863f0
	TBOOL OnPoseFinished();                          // 004864c0
	TBOOL OthersUnhide();                            // 004866b0
	void  UpdateMailman( TFLOAT a_fDeltaTime );      // 00486e40, watcher state machine
	void  UpdateMrsBeady( TFLOAT a_fDeltaTime );     // 004872c0, watcher state machine

	// NOTE: not separate functions in the original, these are the shared "caught" tails of UpdateMailman / UpdateMrsBeady
	void UpdateMailman_Caught();
	void UpdateMrsBeady_Caught();

	void StartPose( AStatuesPose* a_pPose ); // 00487750
	void StandUp();                          // 00487820, quad -> statue
	void CreateShadows();                    // 00487a60
	void RemoveShadows();                    // 00487c40
	void BecomeQuad();                       // 004899b0, statue -> quad

	// Plays talk1 or talk2 (50/50)
	static void __stdcall PlayRandomTalk( AAnimatableObject* a_pObject ); // 00486450

	// Plays talk1/talk2 while the cue plays and stops them once it's over
	static void __stdcall UpdateTalk( AAnimatableObject* a_pObject, TINT a_iCueIndex ); // 00486710

	static void __stdcall PlayEffectAtBone( AAnimatableObject* a_pObject, TINT a_iBone, TINT a_iEffect );                         // 00487940
	static void __stdcall PlayEffectAtBoneName( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcBone, TINT a_iEffect ); // 004897c0

	// Animation breakpoint listener callbacks (__stdcall, return TTRUE)
	static TBOOL __stdcall OnPoseAnimEvent( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData );   // 00486790, pose animation ended
	static TBOOL __stdcall OnWinnerStart( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData );     // 004867a0, statue "winner_start"
	static TBOOL __stdcall OnWatcherNothing( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData );  // 00487a00, watcher "nothing"
	static TBOOL __stdcall OnWatcherStopLook( AStatuesMiniGameState* a_pThis, void* a_pOwner, void* a_pData ); // 00487a10, watcher "stoplook"

	TBOOL IsMrsBeady() const { return m_bIsMrsBeady; }

protected:
	Toshi::T2Vector<AAnimatableObject*, MAX_OTHER_CHARACTERS> m_vecOtherCharacters;                         // 0x0bac, Mailman: statues_pig/freddy/otis_anims
	AStatuesHud                                               m_oHud;                                       // 0x0bd0
	TFLOAT                                                    m_fComboTimer;                                // 0x1f70, > 0.7s resets the combo
	TFLOAT                                                    m_fWatcherTimer;                              // 0x1f74
	TFLOAT                                                    m_fSortingTimer;                              // 0x1f78
	TFLOAT                                                    m_fGameOverTimer;                             // 0x1f7c, -1 = not started
	TFLOAT                                                    m_fSleepTimer;                                // 0x1f80, Mrs Beady
	TBOOL                                                     m_bAsleep;                                    // 0x1f84, Mrs Beady
	TFLOAT                                                    m_fNextYellTime;                              // 0x1f88, Mrs Beady
	TINT                                                      m_iScore;                                     // 0x1f8c
	TINT                                                      m_iCurrentPose;                               // 0x1f90, index into m_aPoses, NO_POSE = none
	TINT                                                      m_iTargetScore;                               // 0x1f94
	TINT                                                      m_aRowScores[ NUM_ROWS ];                     // 0x1f98
	TINT                                                      m_aRowCursors[ NUM_ROWS ];                    // 0x1fa4, index into m_aRowPoseOrder[ row ]
	TINT                                                      m_aRowPoseOrder[ NUM_ROWS ][ MAX_ROW_POSES ]; // 0x1fb0, pose indices shuffled per row, -1 terminated
	TINT                                                      m_aShuffle[ MAX_ROW_POSES ];                  // 0x2070, scratch of the shuffle in OnInsertion
	STATUESKEY                                                m_eKey;                                       // 0x20b0, key pressed this frame, STATUESKEY_NONE = none
	AStatuesPose                                              m_aPoses[ MAX_POSES ];                        // 0x20b4
	AAnimatableObject*                                        m_pStatue;                                    // 0x27b4, americanm/f_statue
	AAnimatableObject*                                        m_pQuad;                                      // 0x27b8, americanm/f_quad
	AAnimatableObject*                                        m_pWatcher;                                   // 0x27bc, mailman_anims / mrbeady_anims
	AAnimatableObject*                                        m_pMrsBeady;                                  // 0x27c0, mrsbeady_anims, TNULL for Mailman
	TFLOAT                                                    m_fStarFadeTimer;                             // 0x27c4
	TBOOL                                                     m_bComboReset;                                // 0x27c8
	TBOOL                                                     m_bIntroStarted;                              // 0x27c9
	TBOOL                                                     m_bReshuffleRows;                             // 0x27ca, set when a pose starts
	PLAYERSTATE                                               m_ePlayerState;                               // 0x27cc
	TINT                                                      m_eWatcherState;                              // 0x27d0, WATCHERSTATE
	TINT                                                      m_iPoseCue;                                   // 0x27d4, pose / intro VO
	TINT                                                      m_iWatcherCue;                                // 0x27d8
	TINT                                                      m_iMrsBeadyCue1;                              // 0x27dc
	TINT                                                      m_iMrsBeadyCue2;                              // 0x27e0
	Toshi::TGenericListener                                   m_oPoseListener;                              // 0x27e4, pose animation end -> OnPoseAnimEvent
	Toshi::TGenericListener                                   m_oWatcherNothingListener;                    // 0x27f8, watcher "nothing" -> OnWatcherNothing
	Toshi::TGenericListener                                   m_oWatcherStopLookListener;                   // 0x280c, watcher "stoplook" -> OnWatcherStopLook
	Toshi::TGenericListener                                   m_oWinnerStartListener;                       // 0x2820, statue "winner_start" -> OnWinnerStart
	TBOOL                                                     m_bOthersFailed;                              // 0x2834
	TBOOL                                                     m_bIsMrsBeady;                                // 0x2835, the minigame is ATeaseMrsBeadyGame
	TINT                                                      m_iCharPuffEffect;                            // 0x2838, "statues_charpuff"
	TINT                                                      m_iDustPuffEffect;                            // 0x283c, "statues_dustpuff"
};

TSTATICASSERT( sizeof( AStatuesMiniGameState ) == 0x2840 );
