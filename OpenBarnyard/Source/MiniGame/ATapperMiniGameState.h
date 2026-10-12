#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2FloatHudElement.h"
#include "GUI/AGUI2FadingText.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AGUI2RowLifeHud.h"
#include "GUI/AGUI2ImagePopup.h"
#include "GUI/AGUI2ElementPopup.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2MessagePopupFlare.h"
#include "GUI/AGUI2AnythingPopup.h"
#include "HUD/AMilkFillHudElement.h"
#include "Render/ASkinSwap.h"
#include "Animation/AAnimationBreakpointUnitAction.h"

#include <Input/TInputDeviceController.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2SList.h>
#include <Toshi/TPString8.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class AAnimatedUnit;
class AUnit;
class AInteriorNetwork;

//-----------------------------------------------------------------------------
// BarnTender (minigame "ATapperMiniGameState", the Tapper-style milk bar game).
// TClass 007832a4, vtable 00720ef0, size 0x5aa8.
//
// The player serves milk at up to 4 bars: holding Shoot at the tap fills a glass
// (the longer, the faster it slides), releasing throws it along the bar. The
// drinkers walk towards the player and slide back when they catch a glass, then
// throw the empty glass back which the player has to catch. Glasses that fall off
// a bar cost a life, a drinker reaching the end of a bar loses the level. The
// drinkers leave tips (barnbucks) that the player collects by running along the
// bars. Wild Mike (Freddy) sometimes gets on the tables and makes the drinkers
// dance. The level ends when enough drinkers have been served.
//
// Data: Data/MinigameData/Tapper/tapper.trb ("Ints", "Floats", "LCount", "Levels"),
//       terrain Terrain_Tappers, interior network "tapper", music "tappa".
// The drinkers are "tap_drinker" units (32, 8 per lane) on the interior network.
//-----------------------------------------------------------------------------
// Handles the queued input commands of ATapperMiniGameState (a single virtual
// interface, its vtable is folded with the other pure interfaces of the game)
class ATapperCommandHandler
{
public:
	virtual TBOOL HandleCommand( AInputCommand a_eCommand, TINT a_iEventType, TBOOL& a_rbHandled ) = 0; // +0x0
};

class ATapperMiniGameState
    : public AMiniGameState
    , public ATapperCommandHandler
    , public Toshi::TSingleton<ATapperMiniGameState>
{
public:
	TDECLARE_CLASS( ATapperMiniGameState, AMiniGameState );

	static constexpr TINT NUM_BARS              = 4;
	static constexpr TINT NUM_TABLES            = 4;
	static constexpr TINT NUM_SPLASHES          = 4;
	static constexpr TINT DRINKERS_PER_BAR      = 8;
	static constexpr TINT MAX_DRINKERS          = NUM_BARS * DRINKERS_PER_BAR;
	static constexpr TINT MAX_GLASSES           = 25;
	static constexpr TINT MAX_POOL_GLASSES      = 1000;
	static constexpr TINT MAX_LEVELS            = 10;
	static constexpr TINT MAX_FLOAT_TEXTS       = 64;
	static constexpr TINT NUM_BONUS_LIFE_POPUPS = 3;
	static constexpr TINT NUM_INT_PARAMS        = 19;
	static constexpr TINT NUM_FLOAT_PARAMS      = 70;

	static constexpr TFLOAT FOV            = 38.0f; // 00773b6c, degrees
	static constexpr TFLOAT TIP_SPIN_SPEED = 2.5f;  // 00773be8

	// Locale strings
	static constexpr TINT LOCALE_BONUS_LIFE      = 597;
	static constexpr TINT LOCALE_MILK_SERVED     = 603;
	static constexpr TINT LOCALE_GLASSES_DROPPED = 604;
	static constexpr TINT LOCALE_TIME_BONUS      = 606;
	static constexpr TINT LOCALE_LEVEL           = 994; // "Level %1!"

	static constexpr TINT HIGHSCORE_TABLE = 59; // + the difficulty (59..62)

	// Indices of m_aiParams ("Ints" of tapper.trb)
	enum INTPARAM : TINT
	{
		INTPARAM_MAX_LIVES            = 0,
		INTPARAM_GLASS_SCORE_1        = 1,  // 1..4, score of a served glass by how fast it was thrown
		INTPARAM_MISSED_GLASS_PENALTY = 9,  // a full glass falls off the end of a bar (< 0 loses the game)
		INTPARAM_MISSED_EMPTY_PENALTY = 10, // an empty glass falls off the player's end of a bar
		INTPARAM_CATCH_SCORE          = 11, // catching an empty glass
		INTPARAM_WILDMIKE_SCORE       = 13, // score needed between two visits of Wild Mike
		INTPARAM_NUM_LEVELS_EASY      = 14, // 14..17, number of levels by difficulty
		INTPARAM_BONUS_LIFE_SCORE     = 18,
	};

	// Indices of m_afParams ("Floats" of tapper.trb)
	enum FLOATPARAM : TINT
	{
		FLOATPARAM_BAR_END_DISTANCE         = 0,
		FLOATPARAM_WALK_SPEED_RANDOM        = 1,
		FLOATPARAM_DRINKER_SPACING          = 2,
		FLOATPARAM_WALK_ANIM_SPEED          = 3,
		FLOATPARAM_IDLE_ANIM_SPEED          = 4,
		FLOATPARAM_DRINKER_SPEED            = 5,
		FLOATPARAM_SPEED_INCREASE           = 7,
		FLOATPARAM_SLIDE_SPEED              = 8,
		FLOATPARAM_SLIDEBACK_ANIM_SPEED     = 9,
		FLOATPARAM_EMPTY_GLASS_SPEED        = 10,
		FLOATPARAM_DODGE_SPEED              = 11,
		FLOATPARAM_DODGE_DISTANCE           = 12,
		FLOATPARAM_ANGRY2_DISTANCE          = 13,
		FLOATPARAM_ANGRY1_DISTANCE          = 14,
		FLOATPARAM_DANCE_SPEED              = 15,
		FLOATPARAM_SLIDEBACK_SPEED_BAR1     = 16, // 16..19, by lane
		FLOATPARAM_WALK_SPEED_BAR1          = 20, // 20..23, by lane
		FLOATPARAM_EMPTY_GLASS_SPEED_BAR1   = 24, // 24..27, by lane
		FLOATPARAM_GRAVITY                  = 29,
		FLOATPARAM_GLASS_BREAK_SPEED        = 30,
		FLOATPARAM_CATCH_DISTANCE           = 31,
		FLOATPARAM_GLASS_FRICTION           = 32,
		FLOATPARAM_FILL_MIN                 = 33, // also the minimum speed of a glass
		FLOATPARAM_FILL_MAX                 = 34,
		FLOATPARAM_FILL_SPEED               = 35,
		FLOATPARAM_JUMP_SPEED               = 36,
		FLOATPARAM_RUN_SPEED                = 37,
		FLOATPARAM_TIP_RADIUS               = 38,
		FLOATPARAM_END_TIME                 = 39,
		FLOATPARAM_CURTAIN_TIME             = 40,
		FLOATPARAM_SWITCH_BAR_TIME          = 41,
		FLOATPARAM_SMASH_RUMBLE             = 42,
		FLOATPARAM_SMASH_RUMBLE_TIME        = 43,
		FLOATPARAM_FILL_RUMBLE              = 44,
		FLOATPARAM_DRIP_INTERVAL            = 45,
		FLOATPARAM_GLASS_SPLASH_INTERVAL    = 46,
		FLOATPARAM_GLASS_DRIP_INTERVAL      = 47,
		FLOATPARAM_ROOT_ANGRY_DISTANCE      = 48,
		FLOATPARAM_WILDMIKE_INTERVAL        = 49,
		FLOATPARAM_WILDMIKE_WARNING_TIME    = 50,
		FLOATPARAM_WILDMIKE_DURATION        = 51,
		FLOATPARAM_WILDMIKE_WALK_SPEED      = 52,
		FLOATPARAM_WILDMIKE_TURN_SPEED      = 53,
		FLOATPARAM_WILDMIKE_JUMP_TURN_SPEED = 54,
		FLOATPARAM_WILDMIKE_WALK_TIME       = 55,
		FLOATPARAM_WILDMIKE_WALK_TIME_RANGE = 56,
		FLOATPARAM_WILDMIKE_SWAY_TIME       = 57,
		FLOATPARAM_WILDMIKE_SWAY_TIME_RANGE = 58,
		FLOATPARAM_WILDMIKE_SWAY            = 59,
		FLOATPARAM_WILDMIKE_SWAY_SPEED      = 60,
		FLOATPARAM_WILDMIKE_JUMP_TIME       = 61,
		FLOATPARAM_WILDMIKE_SPIN_TIME_MIN   = 62,
		FLOATPARAM_WILDMIKE_SPIN_TIME_MAX   = 63,
		FLOATPARAM_WILDMIKE_DANCE_TIME_MIN  = 64,
		FLOATPARAM_WILDMIKE_DANCE_TIME_MAX  = 65,
		FLOATPARAM_FILL_HUD_OFFSET          = 68,
		FLOATPARAM_FILL_HUD_HEIGHT          = 69,
	};

	enum ENDSTATE : TINT
	{
		ENDSTATE_NONE,
		ENDSTATE_WON,  // the level is done
		ENDSTATE_LOST, // the player has lost the level
	};

	enum PLAYERSTATE : TINT
	{
		PLAYERSTATE_IDLE,
		PLAYERSTATE_FILLING,
		PLAYERSTATE_2,
		PLAYERSTATE_THROWING,
		PLAYERSTATE_GLASS_THROWN,
	};

	// Names from the (unused) name table at 00773b88
	enum DRINKERSTATE : TINT
	{
		DRINKERSTATE_APPROACH,
		DRINKERSTATE_WAITING,
		DRINKERSTATE_DRINKING, // caught a glass and slides back
		DRINKERSTATE_DRUNK,    // throws the empty glass back
		DRINKERSTATE_THROWBACK,
		DRINKERSTATE_RUNAWAY,
		DRINKERSTATE_STOPPED,
		DRINKERSTATE_WAITFORWALK,
		DRINKERSTATE_WALKUPTO,
		DRINKERSTATE_9,  // walking up after the level has started
		DRINKERSTATE_10, // dodged a sliding drinker
		DRINKERSTATE_11, // dodging a sliding drinker
	};

	enum MOVEMENT : TINT
	{
		MOVEMENT_WALK,
		MOVEMENT_RUN,
		MOVEMENT_WAITING,
	};

	enum GLASSSTATE : TINT
	{
		GLASSSTATE_HELD,     // the player fills it
		GLASSSTATE_DRINKING, // a drinker has it
		GLASSSTATE_FULL,     // slides to the drinkers
		GLASSSTATE_EMPTY,    // slides back to the player
		GLASSSTATE_FALLING,
		GLASSSTATE_FREE,
	};

	enum WILDMIKESTATE : TINT
	{
		WILDMIKESTATE_NONE,
		WILDMIKESTATE_GETTING_ON,
		WILDMIKESTATE_WALKING,
		WILDMIKESTATE_DANCING,
		WILDMIKESTATE_SPINNING,
		WILDMIKESTATE_JUMPING_FORWARD,
		WILDMIKESTATE_JUMPING_BACK,
		WILDMIKESTATE_LEAVING,
	};

	enum TIPSTATE : TINT
	{
		TIPSTATE_NONE,
		TIPSTATE_ACTIVE,
		TIPSTATE_COLLECTED,
	};

	enum TABLE : TINT
	{
		TABLE_3_ADD,
		TABLE_4_ADD,
		TABLE_3_SUB,
		TABLE_4_SUB,
	};

	enum EFFECT : TINT
	{
		EFFECT_SPLASH,
		EFFECT_DRIP,
		EFFECT_SPLASH_DRINK,
		EFFECT_SPLASH_FOLLOW,
		EFFECT_SPLASH_BREAK,
		EFFECT_DUSTPUFF,
		EFFECT_NUMOF
	};

	// A level of tapper.trb ("Levels")
	struct Level
	{
		TINT   iNumInitialDrinkers;    // 0x00
		TINT   iMaxBarDrinkers;        // 0x04
		TINT   iNumBars;               // 0x08
		TINT   iMaxActiveDrinkers;     // 0x0c
		TINT   iNumDrinkers;           // 0x10, the drinkers to get rid of to finish the level
		TINT   iUnk14;                 // 0x14
		TINT   iMaxSpawns;             // 0x18
		TINT   iMaxServed;             // 0x1c, glasses a drinker takes before leaving
		TFLOAT fWalkSpeed;             // 0x20
		TFLOAT fDrinkerDelayMin;       // 0x24
		TFLOAT fDrinkerDelayMax;       // 0x28
		TFLOAT fApproachTimeMin;       // 0x2c
		TFLOAT fApproachTimeMax;       // 0x30
		TFLOAT fStopTimeMin;           // 0x34
		TFLOAT fStopTimeMax;           // 0x38
		TFLOAT fSpawnDelay;            // 0x3c
		TFLOAT fSpawnDelayAfterRemove; // 0x40
		TFLOAT fApproachDistance;      // 0x44
		TFLOAT fTimeBonusTime;         // 0x48
		TFLOAT fTimeBonusScale;        // 0x4c
		TFLOAT afLaneWeights[ 4 ];     // 0x50
	};

	struct Levels
	{
		void Clear();

		TINT  iNumLevels;            // 0x0
		Level aLevels[ MAX_LEVELS ]; // 0x4
	};

	struct CommandQueue
	{
		static constexpr TINT MAX_COMMANDS = 51;

		struct Command
		{
			AInputCommand eCommand;
			TINT          iEventType;
		};

		void Push( TBOOL a_bReset, TBOOL& a_rbHandled, AInputCommand a_eCommand, TINT a_iEventType );
		void Process( ATapperCommandHandler* a_pHandler );

		void Reset()
		{
			iWrite = 0;
			iRead  = 0;
		}

		Command aCommands[ MAX_COMMANDS ]; // 0x000
		TINT    iRead;                     // 0x198
		TINT    iWrite;                    // 0x19c
	};

	struct PlayerData
	{
		TINT iUnk0;           // 0x00
		TINT iScore;          // 0x04
		TINT iBonusLives;     // 0x08
		TINT iLives;          // 0x0c
		TINT eTeam;           // 0x10, APLAYERTEAM
		TINT iUnk14;          // 0x14
		TINT iLevel;          // 0x18
		TINT iMilkServed;     // 0x1c
		TINT iGlassesDropped; // 0x20
		TINT iTips;           // 0x24
		TINT iTipsCollected;  // 0x28
		TINT iTimeBonus;      // 0x2c
	};

	struct Glass : public Toshi::T2SList<Glass>::Node
	{
		// Moves the glass by its velocity
		void UpdateMotion( TFLOAT a_fDeltaTime );

		AAnimatableObject* pFull;      // 0x04, "tap_drink"
		AAnimatableObject* pEmpty;     // 0x08, "tap_drink_empty"
		Toshi::TVector4    vVelocity;  // 0x0c
		Toshi::TVector4    vDirection; // 0x1c
		GLASSSTATE         eState;     // 0x2c
		TINT               iBar;       // 0x30, 1-based
		TINT               iScore;     // 0x34
		TFLOAT             fDripTimer; // 0x38
		Toshi::TVector4    vPrevPos;   // 0x3c
		TINT               iCue;       // 0x4c
	};

	struct Drinker : public Toshi::T2SList<Drinker>::Node
	{
		// $Barnyard: FUNCTION 004de860
		~Drinker() {}

		ASkinSwap       oSkin;       // 0x04
		AAnimatedUnit*  pUnit;       // 0x18
		TINT            iUnk1C;      // 0x1c
		TFLOAT          fTimer;      // 0x20
		TFLOAT          fSlideTime;  // 0x24
		TFLOAT          fSpeed;      // 0x28
		TFLOAT          fDance;      // 0x2c, dances while > 0
		TINT            iBar;        // 0x30, 1-based
		TINT            iNumServed;  // 0x34
		TINT            iLane;       // 0x38, 0-based, the drinker always belongs to this lane
		DRINKERSTATE    eState;      // 0x3c
		DRINKERSTATE    ePrevState;  // 0x40
		MOVEMENT        eMovement;   // 0x44
		Toshi::TVector4 vDodgeStart; // 0x48
		Toshi::TVector4 vPrevPos;    // 0x58
		TBOOL           bDancing;    // 0x68
	};

	struct Bar
	{
		// $Barnyard: FUNCTION 004df4c0
		Bar() {}

		// $Barnyard: FUNCTION 004df4d0
		~Bar() {}

		TINT                    aiUnk[ 3 ]; // 0x00
		Toshi::T2SList<Drinker> llActive;   // 0x0c
		Toshi::T2SList<Drinker> llWaiting;  // 0x10
	};

	struct GlassPools
	{
		static constexpr TINT NUM_POOLS = 1;
	};

	struct GlassPool
	{
		// $Barnyard: FUNCTION 004df520
		GlassPool() {}

		// $Barnyard: FUNCTION 004df530
		~GlassPool() {}

		TINT                  iNumGlasses; // 0x0
		Toshi::T2SList<Glass> llUsed;      // 0x4
		Toshi::T2SList<Glass> llFree;      // 0x8
	};

	struct Tip
	{
		TIPSTATE           eState;  // 0x0
		AAnimatableObject* pObject; // 0x4, "barnbuck"
	};

public:
	ATapperMiniGameState();
	~ATapperMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL  ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual TBOOL  CanDismissInstructions() OVERRIDE;
	virtual void   OnInstructionsShown() OVERRIDE;
	virtual void   OnStart() OVERRIDE;
	virtual void   OnPause() OVERRIDE;
	virtual TFLOAT GetFOV() OVERRIDE;
	virtual void   OnInsertion() OVERRIDE;
	virtual void   OnRemoval() OVERRIDE;
	virtual void   OnActivate() OVERRIDE;
	virtual void   OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AInventoryAniEffect::CallBackInterface
	//-----------------------------------------------------------------------------
	virtual void OnObjectCollected( AAnimatableObject* a_pObject ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void Pause() OVERRIDE;
	virtual void PlayLoseSequence() OVERRIDE;
	virtual void PlayWinSequence() OVERRIDE;
	virtual void OnPreStart() OVERRIDE;
	virtual void OnResultsShown() OVERRIDE;
	virtual void UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*  GetResults() OVERRIDE;
	virtual void OnStartHudFinished() OVERRIDE;

	//-----------------------------------------------------------------------------
	// ATapperCommandHandler
	//-----------------------------------------------------------------------------
	virtual TBOOL HandleCommand( AInputCommand a_eCommand, TINT a_iEventType, TBOOL& a_rbHandled ) OVERRIDE;

private:
	const Level& GetLevel() const { return m_oLevels.aLevels[ m_iLevel ]; }
	PlayerData&  GetCurrentPlayer() { return m_aPlayers[ m_iCurrentPlayer ]; }

	// The locators are indexed by the 1-based bar index
	const Toshi::TVector4& GetBarTap( TINT a_iBar ) const { return m_avBarTaps[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetBarEnd( TINT a_iBar ) const { return m_avBarEnds[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetEnemyStart( TINT a_iBar ) const { return m_avEnemyStart[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetEnemyEnd( TINT a_iBar ) const { return m_avEnemyEnd[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetPlayerBeerStart( TINT a_iBar ) const { return m_avPlayerBeerStart[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetPlayerBeerEnd( TINT a_iBar ) const { return m_avPlayerBeerEnd[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetEnemyBeerStart( TINT a_iBar ) const { return m_avEnemyBeerStart[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetEnemyBeerEnd( TINT a_iBar ) const { return m_avEnemyBeerEnd[ a_iBar - 1 ]; }
	const Toshi::TVector4& GetSplash( TINT a_iBar ) const { return m_avSplashes[ a_iBar - 1 ]; }

	void                              SelectGlassPool();                                                                                                                       // 004de950
	Toshi::TVector4&                  GetPlayerGlassStart( Toshi::TVector4& a_rOutPos, Toshi::TVector4& a_rOutDir );                                                           // 004de980
	void                              GetDrinkerGlassPos( Drinker* a_pDrinker, Toshi::TVector4& a_rOutPos, Toshi::TVector4& a_rOutDir );                                       // 004deaf0
	void                              AddScoreFloat( const Toshi::TVector4& a_rcPosition, TINT a_iScore );                                                                     // 004dece0
	Toshi::TVector4&                  GetPlayerTargetPos( Toshi::TVector4& a_rOutPos, TINT a_iBar );                                                                           // 004dedd0
	TFLOAT                            GetDistanceSqToTarget( TINT a_iBar );                                                                                                    // 004dee50
	void                              HideTips();                                                                                                                              // 004dee80
	void                              AddLives( PlayerData& a_rPlayer, TINT a_iCount );                                                                                        // 004deec0
	void                              AddBonusLife( PlayerData& a_rPlayer );                                                                                                   // 004def10
	TBOOL                             IsDrinkerBehindGlassLine( Drinker* a_pDrinker, TFLOAT a_fDistance );                                                                     // 004def60
	void                              CreateTips();                                                                                                                            // 004df580
	void                              AddCurtainToModelRenderer();                                                                                                             // 004df5e0
	void                              DetachCarriedGlass();                                                                                                                    // 004df690
	void                              CreateGlasses();                                                                                                                         // 004df700
	void                              CreateDrinkers();                                                                                                                        // 004dfa60
	void                              DestroyGlasses();                                                                                                                        // 004dfd70
	void                              DestroyDrinkers();                                                                                                                       // 004dfdd0
	TBOOL                             IsPlayerAtTarget();                                                                                                                      // 004dff00
	void                              StopDrinker( Drinker* a_pDrinker );                                                                                                      // 004dff90
	TBOOL                             IsMusicIntense();                                                                                                                        // 004e0060
	void                              UpdateMusic( TFLOAT a_fDeltaTime );                                                                                                      // 004e00b0
	void                              ReturnHeldGlass();                                                                                                                       // 004e0200
	void                              CancelGlass();                                                                                                                           // 004e02e0
	TBOOL                             IsHoldingFullGlass();                                                                                                                    // 004e0330
	void                              ThrowHeldGlass();                                                                                                                        // 004e0370
	void                              ResetGlasses();                                                                                                                          // 004e0420
	TBOOL                             LoadData();                                                                                                                              // 004e04a0
	void                              PlayJumpDust();                                                                                                                          // 004e05d0
	void                              AddScore( PlayerData& a_rPlayer, TINT a_iScore );                                                                                        // 004e0700
	void                              OnCurtainClosed();                                                                                                                       // 004e08a0
	void                              CreateHUD();                                                                                                                             // 004e0b80
	void                              SetupCamera();                                                                                                                           // 004e0e30
	Drinker*                          FindClosestDrinker( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMaxDistance, Drinker* a_pIgnore );                                    // 004e0ec0
	Drinker*                          FindClosestDrinkerOnLine( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMaxDistance, Drinker* a_pDrinker, TBOOL a_bIgnoreLeaving );     // 004e0f90
	Toshi::T2SList<Glass>::Iterator   FreeGlass( Glass* a_pGlass, TINT a_iPool );                                                                                              // 004e1350
	TINT                              GetNumMovingGlasses();                                                                                                                   // 004e1410
	void                              RandomiseDanceDelays();                                                                                                                  // 004e14b0
	TBOOL                             IsAnyDrinkerInState( DRINKERSTATE a_eState );                                                                                            // 004e1560
	Drinker*                          FindDrinkerForGlass( const Toshi::TVector4& a_rcGlassStart, const Toshi::TVector4& a_rcGlassEnd, TFLOAT& a_rfOutDistance, TINT a_iBar ); // 004e17d0
	Drinker*                          FindDrinker( AUnit* a_pUnit );                                                                                                           // 004e1a00
	Glass*                            FindGlass( AAnimatableObject* a_pObject );                                                                                               // 004e1a60
	Glass*                            FindGlass( AAnimatableObject* a_pObject, TINT& a_rOutPool );                                                                             // 004e1ac0
	void                              AddScoreAtPlayer( TINT a_iScore );                                                                                                       // 004e1b30
	void                              CollectTip( TINT a_iTip );                                                                                                               // 004e1c00
	void                              StopGlassSounds();                                                                                                                       // 004e1d70
	Drinker*                          FindDrinkerInFront( Drinker* a_pDrinker, TFLOAT a_fMaxDistance );                                                                        // 004e1df0
	void                              CreateRoot();                                                                                                                            // 004e2390
	void                              CreateCurtain();                                                                                                                         // 004e2520
	void                              CreateTables();                                                                                                                          // 004e2630
	void                              CreateWildMike();                                                                                                                        // 004e2940
	void                              SetupInteriorNetwork();                                                                                                                  // 004e2b30
	void                              FacePlayer( TFLOAT a_fAngle );                                                                                                           // 004e2c10
	void                              AttachCarriedGlass();                                                                                                                    // 004e2d00
	void                              PlayPlayerIdle();                                                                                                                        // 004e2de0
	void                              ResetPlayer();                                                                                                                           // 004e2e20
	void                              UpdatePlayer( TFLOAT a_fDeltaTime );                                                                                                     // 004e2f40
	Toshi::T2SList<Drinker>::Iterator RemoveDrinker( Drinker* a_pDrinker );                                                                                                    // 004e3720
	Glass*                            FindClosestGlass( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMaxDistance );                                                          // 004e3840
	TBOOL                             UpdateDancing( TFLOAT a_fDeltaTime );                                                                                                    // 004e3920
	void                              UpdateTips( TFLOAT a_fDeltaTime );                                                                                                       // 004e3de0
	void                              WildMikeLeave();                                                                                                                         // 004e4000
	void                              UpdateWildMike( TFLOAT a_fDeltaTime );                                                                                                   // 004e4120
	TBOOL                             DrinkerCatchGlass( Drinker* a_pDrinker, Glass* a_pGlass );                                                                               // 004e53d0
	void                              TakeGlassFromPool();                                                                                                                     // 004e55b0
	Glass*                            DetachDrinkerGlass( Drinker* a_pDrinker );                                                                                               // 004e57c0
	void                              DrinkerThrowBackGlass( Drinker* a_pDrinker );                                                                                            // 004e5830
	void                              OnAnimEvent( AAnimationBreakpointUnitAction::Event* a_pEvent );                                                                          // 004e5a20
	void                              SetDrinkerWalking( Drinker* a_pDrinker );                                                                                                // 004e5e80
	Toshi::TVector4&                  GetRandomGoalPosition( Toshi::TVector4& a_rOutPos, TINT a_iBar, const TCHAR* a_szPrefix );                                               // 004e6000
	void                              StartPouring();                                                                                                                          // 004e6100
	void                              AddServedScore( Drinker* a_pDrinker, TINT a_iScore );                                                                                    // 004e61a0
	void                              AddGlassScore( Glass* a_pGlass, TINT a_iScore );                                                                                         // 004e62d0
	TBOOL                             SwitchBar( TINT a_iDirection );                                                                                                          // 004e6400
	TBOOL                             StartFilling();                                                                                                                          // 004e6600
	Tip*                              SpawnTip( const Toshi::TVector4& a_rcPosition );                                                                                         // 004e6960
	void                              LoseGame( Drinker* a_pDrinker );                                                                                                         // 004e6b30
	Toshi::TVector4&                  GetClosestGlassPosition( Toshi::TVector4& a_rOutPos );                                                                                   // 004e6c30
	void                              UpdatePlayerLook( TFLOAT a_fDeltaTime );                                                                                                 // 004e6d10
	TINT                              PickRandomBar();                                                                                                                         // 004e7a60
	void                              SendDrinkerToBar( Drinker* a_pDrinker );                                                                                                 // 004e7b40
	void                              DetachDrinkerGlasses();                                                                                                                  // 004e7c90
	void                              SendDrinkerToStart( Drinker* a_pDrinker );                                                                                               // 004e7de0
	void                              ResetLevel();                                                                                                                            // 004e7e60
	Tip*                              SpawnTipAtDrinker( Drinker* a_pDrinker );                                                                                                // 004e8010
	void                              WinLevel();                                                                                                                              // 004e8060
	TINT                              PickSpawnLane();                                                                                                                         // 004e81c0
	void                              CreatePlayer();                                                                                                                          // 004e86a0
	Toshi::TVector4&                  GetDrinkerDestination( Toshi::TVector4& a_rOutPos, Drinker* a_pDrinker );                                                                // 004e8a90
	TBOOL                             ServeDrinker( Drinker* a_pDrinker, Glass* a_pGlass );                                                                                    // 004e8ad0
	void                              SpawnDrinker( TINT a_iLane, TBOOL a_bWhistle );                                                                                          // 004e8f30
	TBOOL                             UpdateDrinkers( TFLOAT a_fDeltaTime );                                                                                                   // 004e9470
	TBOOL                             UpdateGlasses( TFLOAT a_fDeltaTime );                                                                                                    // 004eac20
	void                              UpdateRoot( TFLOAT a_fDeltaTime );                                                                                                       // 004eb6e0
	void                              StartLevel();                                                                                                                            // 004eb880
	void                              EndPlayerTurn( TBOOL a_bLevelsDone );                                                                                                    // 004ebe00
	void                              NextLevel();                                                                                                                             // 004ec0d0

	void DisconnectAnimEventListener();
	void ShowWildMikeIcon(); // 004df1f0
	void HideWildMikeIcon(); // 004df210

	// Picks idle_lp3 or (rarely) idle_lp2
	static const Toshi::TPString8& GetRandomIdleAnim(); // 004dff50

	// Distance between the segment the glass moved along and the one the player moved along
	static TFLOAT GetGlassToPlayerDistance( const Toshi::TVector4& a_rcGlassPrevPos, const Toshi::TVector4& a_rcGlassPos, const Toshi::TVector4& a_rcPlayerPrevPos, const Toshi::TVector4& a_rcPlayerPos ); // 004de880

	static void __stdcall OnAnimEventCallback( ATapperMiniGameState* a_pCaller, AAnimationBreakpointUnitAction::Event* a_pEvent ); // 004e6df0

	// Empty function (0054df00) that is called with the rumble and the main controller
	static void StopRumble( TINT a_iRumble, Toshi::TInputDeviceController* a_pController ) {} // DOES_NOTHING

private:
	static Toshi::TVector4         s_vCameraPosition;  // 00773bb8
	static Toshi::TVector4         s_vCameraDirection; // 00773bc8
	static Toshi::TVector4         s_vPlayerStart;     // 00773bd8
	static const Toshi::TPString8* s_apIdleAnims[ 4 ]; // 00773b70, idle_lp3, idle_lp2, angry1, angry2

	TINT                                     m_aiParams[ NUM_INT_PARAMS ];                // 0x0bb0
	TFLOAT                                   m_afParams[ NUM_FLOAT_PARAMS ];              // 0x0bfc
	AGUI2FloatHudElement                     m_oFloatHud;                                 // 0x0d14
	AGUI2FadingText                          m_oLevelText;                                // 0x0f08
	AMilkFillHudElement                      m_oMilkFill;                                 // 0x107c
	AGUI2Portrait                            m_oPortrait;                                 // 0x1408
	AGUI2RowLifeHud                          m_oLivesHud;                                 // 0x19d0
	AGUI2ImagePopup                          m_oWildMikeIcon;                             // 0x2190
	AGUI2ElementPopup                        m_oBonusLifePopup;                           // 0x22e8
	AGUI2TextBox                             m_oBonusLifeTextBox;                         // 0x2358
	AGUI2MessagePopupFlare                   m_aBonusLifePopups[ NUM_BONUS_LIFE_POPUPS ]; // 0x23d0
	Levels                                   m_oLevels;                                   // 0x3084
	TINT                                     m_aiNumBarDrinkers[ NUM_BARS ];              // 0x3448
	Toshi::TVector4                          m_vWildMikeJumpTarget;                       // 0x3458
	Toshi::TVector4                          m_vWildMikeJumpStart;                        // 0x3468
	WILDMIKESTATE                            m_eWildMikeState;                            // 0x3478
	AAnimatableObject*                       m_pWildMike;                                 // 0x347c, "tap_wildmike"
	TFLOAT                                   m_fWildMikeTime;                             // 0x3480
	TFLOAT                                   m_fWildMikeTurn;                             // 0x3484
	TFLOAT                                   m_fWildMikeWalkTimer;                        // 0x3488
	TFLOAT                                   m_fWildMikeSwayTimer;                        // 0x348c
	TFLOAT                                   m_fUnk3490;                                  // 0x3490
	TFLOAT                                   m_fWildMikeSway;                             // 0x3494
	TFLOAT                                   m_fWildMikeJumpTimer;                        // 0x3498
	TFLOAT                                   m_fWildMikeSpinTimer;                        // 0x349c
	TFLOAT                                   m_fWildMikeDanceTimer;                       // 0x34a0
	TBOOL                                    m_bWildMikeJumping;                          // 0x34a4
	TBOOL                                    m_bAnyDancing;                               // 0x34a5
	AGUI2AnythingPopup                       m_oMessagePopup;                             // 0x34a8
	PlayerData                               m_aPlayers[ MAX_PLAYERS ];                   // 0x3568
	TINT                                     m_aiUnk3628[ NUM_BARS ];                     // 0x3628, 2, 3, 4, 5
	TINT                                     m_aiUnk3638[ NUM_BARS ];                     // 0x3638, 2, 3, 4, 5
	Bar                                      m_aBars[ NUM_BARS ];                         // 0x3648, by lane
	GlassPool                                m_aGlassPools[ GlassPools::NUM_POOLS ];      // 0x3698
	Glass                                    m_aGlasses[ MAX_GLASSES ];                   // 0x36a4
	Drinker                                  m_aDrinkers[ MAX_DRINKERS ];                 // 0x3e74
	AAnimatableObject*                       m_pCarriedGlass;                             // 0x4bf4, the empty glass the player carries
	AAnimatableObject*                       m_pRoot;                                     // 0x4bf8, "tap_root"
	AAnimatableObject*                       m_pCurtain;                                  // 0x4bfc, "tap_curtain", rendered by AGUI2ModelRendererScreen
	AAnimatableObject*                       m_apTables[ NUM_TABLES ];                    // 0x4c00, TABLE
	Tip                                      m_aTips[ NUM_BARS ];                         // 0x4c10
	Toshi::TVector4                          m_vPlayerPos;                                // 0x4c30
	Toshi::TVector4                          m_vPlayerVelocity;                           // 0x4c40
	Toshi::TVector4                          m_vTapDirection;                             // 0x4c50
	Toshi::TVector4                          m_avBarTaps[ NUM_BARS ];                     // 0x4c60, "player_tapN"
	Toshi::TVector4                          m_avBarEnds[ NUM_BARS ];                     // 0x4ca0, "player_endN"
	Toshi::TVector4                          m_avEnemyStart[ NUM_BARS ];                  // 0x4ce0, "enemy_startN"
	Toshi::TVector4                          m_avEnemyEnd[ NUM_BARS ];                    // 0x4d20, "enemy_endN"
	Toshi::TVector4                          m_avPlayerBeerStart[ NUM_BARS ];             // 0x4d60, "playerbeer_startN"
	Toshi::TVector4                          m_avPlayerBeerEnd[ NUM_BARS ];               // 0x4da0, "playerbeer_endN"
	Toshi::TVector4                          m_avEnemyBeerStart[ NUM_BARS ];              // 0x4de0, "enemybeer_startN"
	Toshi::TVector4                          m_avEnemyBeerEnd[ NUM_BARS ];                // 0x4e20, "enemybeer_endN"
	Toshi::TVector4                          m_avWildMikeStart[ NUM_BARS ];               // 0x4e60, "wildmike_startN"
	Toshi::TVector4                          m_avWildMikeEnd[ NUM_BARS ];                 // 0x4ea0, "wildmike_endN"
	Toshi::TVector4                          m_avSplashes[ NUM_SPLASHES ];                // 0x4ee0, "splash0N"
	Toshi::TVector4                          m_vWildMikeOnTableDirection;                 // 0x4f20, the direction Wild Mike turns to
	Toshi::TVector4                          m_vWildMikeDirection;                        // 0x4f30
	Toshi::TVector4                          m_vPlayerPrevPos;                            // 0x4f40
	AAnimatedUnit*                           m_pPlayer;                                   // 0x4f50, "tap_player_m" / "tap_player_fe"
	AInteriorNetwork*                        m_pNetwork;                                  // 0x4f54, "tapper"
	AAnimationBreakpointUnitAction::Listener m_oAnimEventListener;                        // 0x4f58
	TFLOAT                                   m_fSpawnTimer;                               // 0x4f68
	TINT                                     m_iUnk4F6C;                                  // 0x4f6c
	TFLOAT                                   m_fFill;                                     // 0x4f70
	TFLOAT                                   m_fRunDirection;                             // 0x4f74, > 0 runs to the end of the bar, < 0 back to the tap
	TFLOAT                                   m_fLookTimer;                                // 0x4f78
	TFLOAT                                   m_fFillStartTime;                            // 0x4f7c
	TFLOAT                                   m_fEndTimer;                                 // 0x4f80
	TFLOAT                                   m_fSwitchBarTimer;                           // 0x4f84
	TFLOAT                                   m_fSpeedScale;                               // 0x4f88
	TFLOAT                                   m_fDripTimer;                                // 0x4f8c
	TFLOAT                                   m_fLevelTime;                                // 0x4f90
	TFLOAT                                   m_fNextWildMikeTime;                         // 0x4f94
	TFLOAT                                   m_fBonusLifeTextTimer;                       // 0x4f98
	TINT                                     m_iUnk4F9C;                                  // 0x4f9c
	TINT                                     m_iNumPlayers2;                              // 0x4fa0
	TINT                                     m_iCurrentPlayer;                            // 0x4fa4
	TINT                                     m_iGlassPool;                                // 0x4fa8
	TINT                                     m_iNumPlayers;                               // 0x4fac
	TINT                                     m_iNumTurns;                                 // 0x4fb0
	TINT                                     m_iNumDestGoals;                             // 0x4fb4
	TINT                                     m_iUnk4FB8;                                  // 0x4fb8
	TINT                                     m_iFloatText;                                // 0x4fbc
	TINT                                     m_iBar;                                      // 0x4fc0, 1-based
	TINT                                     m_iLevel;                                    // 0x4fc4
	TINT                                     m_iNumActiveDrinkers;                        // 0x4fc8
	TINT                                     m_iNumApproaching;                           // 0x4fcc
	TINT                                     m_iNumSpawned;                               // 0x4fd0
	TINT                                     m_iNumDrinkersGone;                          // 0x4fd4
	TINT                                     m_aiLaneSpawns[ NUM_BARS ];                  // 0x4fd8
	TINT                                     m_iTotalScore;                               // 0x4fe8
	TINT                                     m_iJumpDirection;                            // 0x4fec
	TINT                                     m_iRumble;                                   // 0x4ff0
	TINT                                     m_iWildMikeScore;                            // 0x4ff4
	TINT                                     m_iWildMikeTable;                            // 0x4ff8
	TINT                                     m_iWildMikeWalkDirection;                    // 0x4ffc
	TINT                                     m_iNextSkin;                                 // 0x5000
	TINT                                     m_iUnk5004;                                  // 0x5004
	TINT                                     m_iBonusLifePopup;                           // 0x5008
	TINT                                     m_iDifficulty;                               // 0x500c, the minigame variant
	TINT                                     m_iShootDoodad;                              // 0x5010
	TBOOL                                    m_bUnk5014;                                  // 0x5014
	TBOOL                                    m_bSwitchKeyReleased;                        // 0x5015
	TBOOL                                    m_bRunning;                                  // 0x5016
	TBOOL                                    m_bTowardsEnd;                               // 0x5017
	TBOOL                                    m_bRunKeyHeld;                               // 0x5018
	TBOOL                                    m_bStickSwitchedBar;                         // 0x5019
	TBOOL                                    m_bWaitForThrow;                             // 0x501a
	TBOOL                                    m_bIntroStarted;                             // 0x501b
	TBOOL                                    m_bOverfilled;                               // 0x501c
	TINT                                     m_iFillCue;                                  // 0x5020
	ENDSTATE                                 m_eEndState;                                 // 0x5024
	PLAYERSTATE                              m_ePlayerState;                              // 0x5028
	CommandQueue                             m_oCommandQueue;                             // 0x502c
	TWCHAR                                   m_awszFloatTexts[ MAX_FLOAT_TEXTS ][ 16 ];   // 0x51cc
	TUINT8                                   PADDING1[ 0x80 ];                            // 0x59cc
	TBOOL                                    m_bUnk5A4C;                                  // 0x5a4c
	TINT                                     m_aiEffects[ EFFECT_NUMOF ];                 // 0x5a50
	TWCHAR                                   m_wszLevelText[ 32 ];                        // 0x5a68
};

TSTATICASSERT( sizeof( ATapperMiniGameState::Level ) == 0x60 );
TSTATICASSERT( sizeof( ATapperMiniGameState::Glass ) == 0x50 );
TSTATICASSERT( sizeof( ATapperMiniGameState::Drinker ) == 0x6c );
TSTATICASSERT( sizeof( ATapperMiniGameState::PlayerData ) == 0x30 );
TSTATICASSERT( sizeof( ATapperMiniGameState::CommandQueue ) == 0x1a0 );
TSTATICASSERT( sizeof( ATapperMiniGameState ) == 0x5aa8 );
