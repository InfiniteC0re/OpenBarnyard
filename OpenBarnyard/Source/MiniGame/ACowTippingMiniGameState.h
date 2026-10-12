#pragma once
#include "AMiniGameState.h"
#include "GUI/ATargetReticle.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AGUI2FloatHudElement.h"
#include "GUI/AGUI2AnythingPopup.h"
#include "GUI/AMilkReticleManager.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2KidCounter.h"
#include "GUI/AGUI2MessagePopupFlare.h"
#include "GUI/AGUI2ImagePopup.h"
#include "GUI/AGUI2ElementPopup.h"
#include "HUD/AButtonSlamHudElement.h"
#include "Render/ASkinSwap.h"
#include "Helpers/AWindmillHelper.h"
#include "Animation/AAnimationBreakpointUnitAction.h"

#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/TPString8.h>
#include <Toshi/TSingleton.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>

class AAnimatableObject;
class AAnimatedUnit;
class AInteriorGoal;
class AInteriorSteer;
class AInteriorNetwork;
class AGameTimeFX;
class AUnit;
class AUnitPlayer;

namespace Toshi
{
class T2GUITextureSection;
}

//-----------------------------------------------------------------------------
// Cow Tipping (minigame "ACowTippingMiniGameState", "Cow_Tipping", 1-4 players).
// TClass 00782f00 (TClass_ACowTippingMiniGameState), vtable 0071fb88
// (AInventoryAniEffect::CallBackInterface vtable 0071fb80), size 0x14d94.
// Singleton pointer 00782efc. Data: Data/MinigameData/CowTipping/CowTipping.trb
// (Floats, Ints, Levels/LCount, Stage/StageCount).
//
// NOTE: the class is split over several .cpp files (ACowTippingMiniGameState*.cpp).
// Members that aren't understood yet are named after their offset: m_xUnk_NNNNN
// (NNNNN = hex offset), FUN_xxxxxxxx (method at that address) and iUnk_<x>NNN
// (member of a nested struct). The [...] comment after a field lists which
// parts of the code (files) use it.
//-----------------------------------------------------------------------------
class ACowTippingMiniGameState
    : public AMiniGameState
    , public Toshi::TSingleton<ACowTippingMiniGameState>
{
public:
	TDECLARE_CLASS( ACowTippingMiniGameState, AMiniGameState );

	static constexpr TINT MAX_LEVELS           = 10;
	static constexpr TINT MAX_STAGES           = 7;
	static constexpr TINT MAX_PROJECTILES      = 80;
	static constexpr TINT NUM_CRATE_GROUPS     = 6;
	static constexpr TINT NUM_CRATES_PER_GROUP = 5;
	static constexpr TINT NUM_COINS            = 8;

	// CowTipping.trb "Stage" (StageCount entries).
	struct Stage
	{
		using STAGETYPE = TINT;
		enum STAGETYPE_ : STAGETYPE
		{
			STAGETYPE_KIDS          = 0, // kids push the cows
			STAGETYPE_RACCOONS      = 1, // raccoons push the cows
			STAGETYPE_JUNKYARD      = 2, // hens, Dag and Wild Mike instead of cows, gophers and raccoons
			STAGETYPE_JUNKYARD_BOSS = 3, // junkyard with the Dag power bar and the objects left counter
		};

		TINT      iNumLevels;      // 0x0
		TINT      iNetwork;        // 0x4
		TINT      iLocatorIndex;   // 0x8
		TINT      iScript;         // 0xc
		TINT      iCameraSettings; // 0x10
		STAGETYPE iStageType;      // 0x14
		TINT      iLives1;         // 0x18
		TINT      iLives2;         // 0x1c
		TINT      iMaxLives;       // 0x20
		TINT      iMaxLives2;      // 0x24
	};

	// Acceleration of the keyboard/controller aiming, CowTipping.trb "Floats" has 2 of them (Stage::iCameraSettings).
	struct AimAcceleration
	{
		TFLOAT fAccelX;   // 0x0
		TFLOAT fAccelY;   // 0x4
		TFLOAT fMaxTimeX; // 0x8
		TFLOAT fMaxTimeY; // 0xc
	};

	// Camera of a stage, CowTipping.trb "Floats" has 2 of them (Stage::iCameraSettings).
	struct CameraSettings
	{
		TFLOAT fSideStepSpeed;        // 0x0
		TFLOAT fUnk04;                // 0x4, scales m_fCameraYawDelta and m_fCameraPitchDelta
		TFLOAT fRailYawScale;         // 0x8, m_fCameraTargetYaw per unit of the rail position
		TFLOAT fTargetPitchScale;     // 0xc
		TFLOAT fMaxYaw;               // 0x10
		TFLOAT fMaxPitch;             // 0x14
		TFLOAT fMinPitch;             // 0x18
		TFLOAT fPitchOffset;          // 0x1c
		TFLOAT fLookDownY;            // 0x20
		TFLOAT fYawSpeed;             // 0x24
		TFLOAT fPitchSpeed;           // 0x28
		TUINT8 PADDING1[ 0x8 ];       // 0x2c
		TFLOAT fYawOffset;            // 0x34
		TFLOAT fLocatorForwardOffset; // 0x38
		TFLOAT fLocatorHeightOffset;  // 0x3c
		TFLOAT fTargetYawSpeed;       // 0x40
		TFLOAT fTargetTime;           // 0x44
		TUINT8 PADDING2[ 0x4 ];       // 0x48
		TFLOAT fFOV;                  // 0x4c, in degrees
		TFLOAT fMinStartPitch;        // 0x50
		TFLOAT fMaxStartPitch;        // 0x54
		TFLOAT fUnk58;                // 0x58
		TFLOAT fStartPitch;           // 0x5c
		TFLOAT fRailCentre;           // 0x60
		TFLOAT fRailHalfWidth;        // 0x64
	};

	//-----------------------------------------------------------------------------
	// [Flow] level scripts
	//-----------------------------------------------------------------------------
	static constexpr TINT MAX_LEVEL_COMMANDS = 50;
	static constexpr TINT NUM_LEVEL_SCRIPTS  = 4;
	static constexpr TINT NUM_LEVEL_COMMANDS = 18;

	// One command of a level script, run by RunLevelScript through ms_apfnLevelCommands.
	struct LevelCommand
	{
		TUINT8 uiType;   // 0x0, index of ms_apfnLevelCommands
		TUINT8 uiParam1; // 0x1, 0xff: any/random
		TCHAR  cParam2;  // 0x2, 'r': random, 'a'/'b': variant, or a signed index
		TUINT8 uiParam3; // 0x3
		TFLOAT fValue;   // 0x4
	};

	struct LevelScript
	{
		LevelCommand aCommands[ MAX_LEVEL_COMMANDS ]; // 0x0
		TINT         iNumCommands;                    // 0x190
	};

	// CowTipping.trb "Levels" (LCount entries).
	struct Level
	{
		TINT        iBonus;                        // 0x0
		TFLOAT      fRaccoonDrawTime;              // 0x4
		TFLOAT      fRaccoonReloadTime;            // 0x8
		TFLOAT      fRaccoonDownTime;              // 0xc
		TFLOAT      fRaccoonDrawTimeLarge;         // 0x10, in the stages of type 2 and 3
		TFLOAT      fRaccoonReloadTimeLarge;       // 0x14
		TFLOAT      fRaccoonDownTimeLarge;         // 0x18
		LevelScript aScripts[ NUM_LEVEL_SCRIPTS ]; // 0x1c, index Stage::iScript
	};

	// A power-up of a player (PlayerData::aPowerUps, index: the power-up).
	struct PowerUp
	{
		TFLOAT fTime;    // 0x0, active while > 0
		TINT   iCharges; // 0x4
	};

	// One per player (index m_iCurrentPlayer).
	struct PlayerData
	{
		PowerUp aPowerUps[ 6 ];      // 0x0
		TINT    iScore;              // 0x30
		TINT    iLastExtraLifeScore; // 0x34, score of the last extra life given for m_iScoreForExtraLife
		TINT    iTeam;               // 0x38
		TINT    iSelectedTipSpot;    // 0x3c, see SelectPrevious/SelectNext
		TINT    iPowerUp0Shots;      // 0x40
		TINT    iLevel;              // 0x44
		TINT    iLives;              // 0x48
		TINT    iLives2;             // 0x4c
		TINT    iCoyoteHits;         // 0x50
		TINT    iShots;              // 0x54
		TINT    iHits;               // 0x58
		TINT    iTurnShots;          // 0x5c
		TINT    iTurnHits;           // 0x60
	};

	struct Projectile;
	struct Enemy;

	// m_aProjectileLists[ 0 ]: tomatoes, [ 1 ]: walnuts (or "cowtipping_largeobject" in stages of type 2/3).
	struct ProjectileList
	{
		ProjectileList();
		~ProjectileList();

		TINT                       iAmmo;  // 0x0
		TINT                       iHits;  // 0x4
		Toshi::T2SList<Projectile> llHeld; // 0x8, flying (or held) projectiles
		Toshi::T2SList<Projectile> llFree; // 0xc, free projectiles
	};

	struct Projectile
	    : public Toshi::T2SList<Projectile>::Node
	{
		AAnimatableObject* pObject;         // 0x4, the model
		Toshi::TVector4    vLastPosition;   // 0x8, position before the last step
		Toshi::TVector4    vVelocity;       // 0x18, velocity (zero while not flying)
		Toshi::TVector4    vDirection;      // 0x28, direction it was thrown in
		AAnimatedUnit*     pTarget;         // 0x38, homing target
		TFLOAT             fScale;          // 0x3c, model scale
		AAnimatedUnit*     pThrower;        // 0x40, the unit that threw it
		Enemy*             pEnemy;          // 0x44
		TBOOL              bRecalcVelocity; // 0x48, the velocity is (re)computed on the next step
		TBOOL              bHit;            // 0x49, it hit something
		TUINT8             PADDING_j04A[ 2 ];
		TINT               iTargetBone; // 0x4c, bone of the target that is aimed at (-1: the model position)
		TINT               iGrowType;   // 0x50, 1/2: the scale grows up to m_fGrowType1MaxScale/m_fGrowType2MaxScale
	};

	// Raccoon, kid or coyote (m_aEnemies) that walks to a cow and tips it.
	// Linked in m_llActiveEnemies while it's in the field, moved to m_llDoneEnemies when it's done.
	struct Enemy
	    : public Toshi::T2SList<Enemy>::Node
	{
		// 0..5 index the per type arrays (m_aModelNames for the kids)
		using ENEMYTYPE = TINT;
		enum ENEMYTYPE_ : ENEMYTYPE
		{
			ENEMYTYPE_KID_BOY     = 0,
			ENEMYTYPE_KID_GIRL    = 1,
			ENEMYTYPE_KID_BOY_FAT = 2,
			ENEMYTYPE_RACCOON     = 3,
			ENEMYTYPE_COYOTE      = 6,
		};

		using ENEMYSTATE = TINT;
		enum ENEMYSTATE_ : ENEMYSTATE
		{
			ENEMYSTATE_ACTIVE    = 0, // walking/running in the field
			ENEMYSTATE_PUSHING   = 1, // pushing a cow
			ENEMYSTATE_PUSH_END  = 2, // finishing the push
			ENEMYSTATE_RETURNING = 3, // running back to its start goal
			ENEMYSTATE_INACTIVE  = 4, // not in the field
			ENEMYSTATE_FADING    = 5, // fading out
			ENEMYSTATE_TARGETED  = 6, // stands still while a projectile flies at it
		};

		Enemy();

		ASkinSwap       oSkinSwap;            // 0x4
		AAnimatedUnit*  pUnit;                // 0x18
		TINT            iHits;                // 0x1c
		TFLOAT          fWaitTime;            // 0x20, negative while howling/growling
		TFLOAT          fUnk24;               // 0x24
		TFLOAT          fStillTime;           // 0x28
		TINT            iCow;                 // 0x2c, index into m_aCows (-1: none), number of the 'start' goal
		TINT            iCowSide;             // 0x30, 'a' + iCowSide is the variant of the 'start' goal
		ENEMYTYPE       eType;                // 0x34, 0..5 index the per type arrays, 6 is the coyote
		TINT            iCoyoteGoal;          // 0x38, goal "coyote%d"
		TINT            iProjectilesInFlight; // 0x3c
		TINT            iNumTargeting;        // 0x40, compared with m_iAmmoPerTarget
		ENEMYSTATE      eState;               // 0x44
		TINT            iRunning;             // 0x48, 1 while it runs away (ResetEnemySpeed makes it walk again)
		TBOOL           bUnk4C;               // 0x4c
		TBOOL           bUnk4D;               // 0x4d
		TBOOL           bUnk4E;               // 0x4e
		TBOOL           bUnk4F;               // 0x4f
		TFLOAT          fTurnProgress;        // 0x50, 0..1
		Toshi::TVector4 vTurnStartFacing;     // 0x54, facing when the turning started
	};

	// A cow of the field (m_aCows).
	struct Cow
	{
		Cow();
		~Cow();

		ASkinSwap          oSkinSwap;       // 0x0
		AAnimatableObject* pObject;         // 0x14
		TFLOAT             fTipProgress;    // 0x18
		TFLOAT             fUnk1C;          // 0x1c
		TINT               iSpins;          // 0x20
		TFLOAT             fHitTime;        // 0x24
		Toshi::TVector4    vTipSpot1;       // 0x28, bone of m_iCowTipSpotBone
		Toshi::TVector4    vTipSpot2;       // 0x38, bone "ch_rac2_pos"
		Toshi::TVector4    vTipDirection1;  // 0x48
		Toshi::TVector4    vTipDirection2;  // 0x58
		Toshi::TVector4    vSpinePosition;  // 0x68
		Toshi::TVector4    vSpine1Position; // 0x78
		Toshi::TVector4    vHeadPosition;   // 0x88
		Toshi::TMatrix44   matStart;        // 0x98
		TBOOL              bActive;         // 0xd8
		TUINT8             PADDING[ 3 ];    // 0xd9
	};

	// A raccoon or a gopher that pops up from a hiding spot (m_aRaccoons, m_aGophers).
	// The active ones are linked in m_llKids (raccoons) and m_llGophers, the free ones in m_llRaccoons and m_llFreeGophers.
	struct PopUpUnit
	    : public Toshi::T2SList<PopUpUnit>::Node
	{
		using POPUPSTATE = TINT;
		enum POPUPSTATE_ : POPUPSTATE
		{
			POPUPSTATE_ACTIVE     = 0, // up
			POPUPSTATE_INACTIVE   = 1, // hidden
			POPUPSTATE_LEAVING    = 2, // raccoon: gets removed, gopher: fading out
			POPUPSTATE_HIT        = 3, // playing yuk/down
			POPUPSTATE_DRAWING    = 4, // drawing (until fire plays)
			POPUPSTATE_GOING_DOWN = 5, // raccoon: playing down
			POPUPSTATE_TAUNTING   = 6, // raccoon: playing tawnt
		};

		AAnimatedUnit* pUnit;  // 0x4
		TINT           iHits;  // 0x8
		TINT           iSpot;  // 0xc, index of the hiding spot (bit of m_auiUsedRaccoonBits)
		POPUPSTATE     iState; // 0x10, 3 when hit
	};

	using Gopher = PopUpUnit;

	struct Raccoon
	    : public PopUpUnit
	{
		TFLOAT fDrawTimer;   // 0x14
		TFLOAT fDownTimer;   // 0x18
		TBOOL  bTargeted;    // 0x1c, a projectile was thrown at it (it isn't a milk target)
		TUINT8 PADDING[ 3 ]; // 0x1d
		TINT   iShots;       // 0x20
		TINT   iType;        // 0x24, != 0: taunts the player, 1 and 2 drop a tomato
	};

	// m_eRunnerState (Wild Mike, junkyard stages)
	using RUNNERSTATE = TINT;
	enum RUNNERSTATE_ : RUNNERSTATE
	{
		RUNNERSTATE_NONE          = 0, // hidden
		RUNNERSTATE_PIPE_UP       = 1, // the box is up, waiting to be hit
		RUNNERSTATE_COMING_OUT    = 2, // coming out of the box
		RUNNERSTATE_CHASING       = 3, // chasing the cows
		RUNNERSTATE_RETURNING     = 4, // going back to the box
		RUNNERSTATE_ENTERING_PIPE = 5,
		RUNNERSTATE_PIPE_CLOSING  = 6, // the box is lowered
	};

	// m_iReloadState
	using RELOADSTATE = TINT;
	enum RELOADSTATE_ : RELOADSTATE
	{
		RELOADSTATE_NONE      = 0,
		RELOADSTATE_RELOADING = 1, // playing reload
		RELOADSTATE_HIT       = 2, // the player was hit (impact)
	};

	// m_eEndState (end of the final mission)
	using ENDSTATE = TINT;
	enum ENDSTATE_ : ENDSTATE
	{
		ENDSTATE_NONE       = 0,
		ENDSTATE_FADING_OUT = 1,
		ENDSTATE_CUTSCENE   = 2, // Dag plays end_taunt
	};

	// m_eTurnEnd
	using TURNEND = TINT;
	enum TURNEND_ : TURNEND
	{
		TURNEND_NONE         = 0,
		TURNEND_GAME_END     = 1, // EndGame, EndTurn( TTRUE ) when the timer runs out
		TURNEND_OUT_OF_LIVES = 2, // EndTurnOutOfLives, EndTurn( TFALSE ) when the timer runs out
	};

	// Part of the cow that was hit (FindHitCow, OnCowHit)
	using COWHIT = TINT;
	enum COWHIT_ : COWHIT
	{
		COWHIT_BACK  = 0,
		COWHIT_FRONT = 1,
		COWHIT_HEAD  = 2,
	};

	using LevelCommandFn = TBOOL ( ACowTippingMiniGameState::* )( const LevelCommand* a_pCommand );

	// A crate (pick up) the launcher drops into the field, m_aCrates has NUM_CRATE_GROUPS groups (one per crate type) of NUM_CRATES_PER_GROUP.
	struct Crate
	{
		using CRATESTATE = TINT;
		enum CRATESTATE_ : CRATESTATE
		{
			CRATESTATE_FREE      = 0,
			CRATESTATE_LOADING   = 1, // fading out while flying to m_iLauncherCartridge2Bone
			CRATESTATE_FADING    = 2, // fading out
			CRATESTATE_CARRIED   = 3, // carried/thrown, lands once it hits the ground
			CRATESTATE_FALLING   = 4,
			CRATESTATE_ON_GROUND = 5, // blinks before it disappears
			CRATESTATE_COLLECTED = 6, // flying to m_iLauncherCartridge1Bone
			CRATESTATE_GIVEN     = 7, // the power-up was given
		};

		Toshi::TVector4    vDirection;    // 0x0
		Toshi::TVector4    vPosition;     // 0x10, position on the previous frame
		Toshi::TVector4    vNormal;       // 0x20, ground normal
		CRATESTATE         iState;        // 0x30, 0: free, 3..5 can be hit, 6 when done
		AAnimatableObject* pObject;       // 0x34
		TINT               iHits;         // 0x38
		TINT               iBit;          // 0x3c, bit in m_uiPickupBits (-1: none)
		TFLOAT             fBlinkPhase;   // 0x40
		TFLOAT             fTimeLeft;     // 0x44, starts at m_fBounceTime, it blinks near the end
		TFLOAT             fTurnProgress; // 0x48, 0..1
		TFLOAT             fFallTime;     // 0x4c
	};

	// A barnbuck (coin) that falls and bounces (m_aCoins).
	struct Coin
	{
		using COINSTATE = TINT;
		enum COINSTATE_ : COINSTATE
		{
			COINSTATE_FREE      = 0,
			COINSTATE_BOUNCING  = 1,
			COINSTATE_ENDING    = 2, // bounces higher near the end of fTimeLeft
			COINSTATE_VANISHING = 3, // falls through the floor
			COINSTATE_COLLECTED = 4,
		};

		TINT               iUnk00;    // 0x0
		TFLOAT             fTimeLeft; // 0x4, starts at m_fBounceTime
		TFLOAT             fVelocity; // 0x8
		TFLOAT             fFallTime; // 0xc
		Toshi::TVector4    vPosition; // 0x10, y is the floor height
		COINSTATE          iState;    // 0x20, 0: free, 4 when collected
		AAnimatableObject* pObject;   // 0x24
	};

public:
	ACowTippingMiniGameState();
	~ACowTippingMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // 004bdd00
	virtual void     OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // 004a7a40
	virtual ASoundId GetSound() OVERRIDE;                                                                                                                     // 004a79f0
	virtual void     OnPause() OVERRIDE;                                                                                                                      // 004a7ab0
	virtual TFLOAT   GetFOV() OVERRIDE;                                                                                                                       // 004a7a10
	virtual void     OnInsertion() OVERRIDE;                                                                                                                  // 004c2390
	virtual void     OnRemoval() OVERRIDE;                                                                                                                    // 004b8700
	virtual void     OnActivate() OVERRIDE;                                                                                                                   // 004c3650
	virtual void     OnDeactivate() OVERRIDE;                                                                                                                 // 004a84f0

	//-----------------------------------------------------------------------------
	// AInventoryAniEffect::CallBackInterface
	//-----------------------------------------------------------------------------
	virtual void OnObjectCollected( AAnimatableObject* a_pObject ) OVERRIDE; // 004a7be0

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void  Pause() OVERRIDE;                               // 004ad090
	virtual void  PlayLoseSequence() OVERRIDE;                    // 004aae90
	virtual void  PlayWinSequence() OVERRIDE;                     // 004bccd0
	virtual void  OnPreStart() OVERRIDE;                          // 004a7af0
	virtual TBOOL IsGameOver() OVERRIDE;                          // 004a7500
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;     // 004c4380
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE; // 004a74d0
	virtual void  UpdatePreStart( TFLOAT a_fDeltaTime ) OVERRIDE; // 004c1a60
	virtual S1*   GetResults() OVERRIDE;                          // 004ac2e0
	virtual void  OnStartHudFinished() OVERRIDE;                  // 004aac90

	//-----------------------------------------------------------------------------
	// Own virtuals
	//-----------------------------------------------------------------------------
	virtual void UpdateMusicVolume( TFLOAT a_fUnk1 ); // +0xa4

	//-----------------------------------------------------------------------------
	// Own methods (provisional names, grouped by the part of the code they belong to)
	//-----------------------------------------------------------------------------
	//-----------------------------------------------------------------------------
	// [Setup]
	//-----------------------------------------------------------------------------
	void  LoadCowHudSprites();
	void  CreateHUD();
	void  CreateProjectiles();
	void  CreateCrates();
	void  FindHenTransforms();
	void  CreateLifeHuds();
	void  SetupNetwork();
	void  CreateWildMike();
	void  CreateHannaAndOtis();
	void  CreateGophers();
	void  OnAnimationEvent( void* a_pUnk1 );
	void  GetCowTransform( TINT a_iUnk1, Toshi::TMatrix44* a_pUnk2 );
	TBOOL HasCow( TINT a_iUnk1 );
	void  CreateCoins();
	void  CreateCows();
	void  CreateKids();
	void  CreateRaccoons();
	void  CreateLauncher();

	//-----------------------------------------------------------------------------
	// [Flow]
	//-----------------------------------------------------------------------------
	// Level script commands (ms_apfnLevelCommands, the index is LevelCommand::uiType)
	TBOOL Cmd_WaitTime( const LevelCommand* a_pCommand );               // 0
	TBOOL Cmd_WaitEnemyCount( const LevelCommand* a_pCommand );         // 1
	TBOOL Cmd_WaitActiveRaccoonCount( const LevelCommand* a_pCommand ); // 2
	TBOOL Cmd_WaitCrateCount( const LevelCommand* a_pCommand );         // 3
	TBOOL Cmd_WaitAllCounts( const LevelCommand* a_pCommand );          // 4
	TBOOL Cmd_SpawnEnemies( const LevelCommand* a_pCommand );           // 5
	TBOOL Cmd_ActivateRaccoon( const LevelCommand* a_pCommand );        // 6
	TBOOL Cmd_ActivateGopher( const LevelCommand* a_pCommand );         // 7
	TBOOL Cmd_RandomJump( const LevelCommand* a_pCommand );             // 8
	TBOOL Cmd_JumpIfScore( const LevelCommand* a_pCommand );            // 9
	TBOOL Cmd_EndLevelWhenClear( const LevelCommand* a_pCommand );      // 10
	TBOOL Cmd_EndLevel( const LevelCommand* a_pCommand );               // 11
	TBOOL Cmd_SetLevelTime( const LevelCommand* a_pCommand );           // 12
	TBOOL Cmd_NextStage( const LevelCommand* a_pCommand );              // 13
	TBOOL Cmd_RaccoonsShootNow( const LevelCommand* a_pCommand );       // 14
	TBOOL Cmd_JumpIfObjectsLeft( const LevelCommand* a_pCommand );      // 15
	TBOOL Cmd_PlaceCoins( const LevelCommand* a_pCommand );             // 16
	TBOOL Cmd_SpawnEnemiesAlt( const LevelCommand* a_pCommand );        // 17

	TINT   GetNumUsedCrates( TINT a_iGroup );
	void   AddUnitObstruction( Enemy* a_pUnk );
	void   DestroyCows();
	void   DestroyProjectiles();
	void   DestroyCrates();
	void   DestroyEnemies();
	void   DestroyGophers();
	TINT   GetRandomCrateGroup();
	void   SelectPrevious( TINT a_iPlayer );
	void   SelectNext( TINT a_iPlayer );
	void   DestroyCoins();
	Enemy* FindFreeEnemyOfType( TINT a_iType );
	Enemy* GetRandomFreeEnemy();
	TINT   GetNumActiveEnemies();
	TINT   GetNumActiveEnemiesOfType( TINT a_iType );
	void   DestroyRaccoons();
	Crate* TakeCrate( TINT a_iIndex, TINT a_iGroup );
	void*  FindGoal( TINT a_iIndex, const TCHAR* a_szGoalName );
	TBOOL  PlaceWildMikeBox( TINT a_iIndex );
	Coin*  TakeCoin( const Toshi::TVector4* a_pPosition );
	void   Cleanup(); // also called by Setup
	TINT   GetRandomStartIndex( TBOOL* a_pOutVariant );
	TINT   GetRandomRaccoonIndex();
	TINT   GetRandomGopherIndex();
	TBOOL  ActivateRaccoon( TINT a_iIndex, TINT a_iType );
	TBOOL  ActivateGopher( TINT a_iGroup );
	TINT   GetRandomCoinIndex();
	TBOOL  SpawnEnemy( TINT a_iType, TINT a_iIndex, TBOOL a_bVariant, TBOOL a_bUnk );

	// 00782f30, filled in by 00708ca0
	static LevelCommandFn ms_apfnLevelCommands[ NUM_LEVEL_COMMANDS ];

	//-----------------------------------------------------------------------------
	// [Game]
	//-----------------------------------------------------------------------------
	void  OnPowerUpEnded( TINT a_iPowerUp );
	void  StopRunnerPipe();
	TINT  HideRunner(); // also called by Flow,Setup
	void  UpdatePopups( TFLOAT a_fUnk1 );
	TINT  SetupLifeHuds(); // also called by Flow
	void  ReadCameraLocator();
	void  ResetCamera();
	TINT  DetachGridObjects();            // also called by Flow
	void  UpdateCamera( TFLOAT a_fUnk1 ); // also called by Flow
	void  DropHeldProjectile();
	TBOOL StartThrow(); // also called by Flow
	void  ResetProjectiles();
	void  ResetGame();                     // also called by Setup
	TBOOL ShowLevelPopups( TINT a_iUnk1 ); // also called by Flow
	TBOOL LoadData();                      // also called by Setup
	void  ResetPowerUps( TINT a_iPlayer );
	void  UsePowerUpCharges( TINT a_iPlayer );
	void  UpdatePowerUps( TFLOAT a_fUnk1 );
	TINT  RunLevelScript( TINT a_iUnk1 );
	void  ResetBounceObjects();
	void  StartEndFade();
	void  UpdateCowDust( TFLOAT a_fDeltaTime );
	void  UpdateUnkHud_0EC58( TFLOAT a_fDeltaTime );
	TINT  SetTimeOfDay( TFLOAT a_fUnk1 ); // also called by Flow
	void  FUN_004ac660();
	TINT  ResetTurn();   // also called by Setup
	TBOOL StartReload(); // also called by Flow,Setup
	void  FreezeForEndCutscene();
	void  UnfreezeAfterEndCutscene();
	TINT  ResetPlayer( TINT a_iUnk1 ); // also called by Setup
	void  PlaceOtis();
	void  UpdateAmmoHud( TFLOAT a_fDeltaTime );
	void  UpdateTimeOfDay( TFLOAT a_fUnk1 );
	void  TakeNextProjectile();
	void  FUN_004b0be0();
	void  FUN_004b0c30();
	void  FUN_004b0c80();
	void  ResetCows();
	void  AddScore( TINT a_iScore );
	TBOOL SendRunnerBack();
	void  ReleaseRunner();
	void  UpdateBounceObjects( TFLOAT a_fDeltaTime );
	void  UpdateRunner( TFLOAT a_fUnk1 );
	void  EndTurnOutOfLives();
	void  PlacePlayer(); // also called by Setup
	void  StartTurn();
	TBOOL CompleteLevel();
	void  EndTurn( TBOOL a_bNextPlayer );
	TBOOL CheckTurnEnd( TFLOAT a_fUnk1 );
	TBOOL UpdateRestart(); // also called by Flow

	//-----------------------------------------------------------------------------
	// [Player]
	//-----------------------------------------------------------------------------
	TBOOL       ShowShootHint();
	void*       FindCrateByObject( TINT a_iUnk1 );          // also called by Flow
	TINT        SelectAmmoList();                           // also called by Enemies,Game
	static void ResetCrate( void* a_pUnk1 );                // also called by Enemies
	TBOOL       SetAmmoCount( TINT a_iUnk1, TINT a_iUnk2 ); // also called by Enemies,Flow
	TINT        GetAmmoCount( TINT a_iUnk1 );               // also called by Game
	TBOOL       UpdateTomatoesAnim( TFLOAT a_fUnk1 );       // also called by Game
	void        FlagAimableUnits( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5 );
	void        RemoveFadedUnit( TINT a_iUnk1 );                    // also called by Game
	TINT        FindClosestEnemyXZ( TINT a_iUnk1, TFLOAT a_fUnk2 ); // also called by Game
	void        UpdateCowTipping( TFLOAT a_fUnk1 );                 // also called by Game
	void*       FindProjectileByObject( void* a_pUnk1 );
	void*       FindProjectileByObjectEx( void* a_pUnk1, void* a_pUnk2 );
	void        DropKidTomato( TINT a_iUnk1 );       // also called by Flow,Game
	void        RemoveKid( TINT a_iUnk1 );           // also called by Game
	void        UpdateFadingUnits( TFLOAT a_fUnk1 ); // also called by Game
	void        Reload();
	void        UpdateReload();                                     // also called by Flow
	void        LoadKidSlingshot( void** a_pUnk1 );                 // also called by Flow
	void        GetSlingshotBone( void* a_pUnk1, TFLOAT* a_pUnk2 ); // also called by Setup
	TINT        FindEnemyNearRay( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 );
	TINT        FindKidNearRay( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 );
	void*       FindCrateNearRay( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 );
	void*       FindObjectNearRay( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3, TFLOAT a_fUnk4 );
	void        UpdateAiming( TFLOAT a_fUnk1, TINT a_iUnk2 );
	void        UpdatePlayerTransform( TFLOAT a_fUnk1, TINT a_iUnk2 ); // also called by Flow,Game
	void        KidStartShooting( TINT a_iUnk1 );
	void        KidFireTomato( void* a_pUnk1 );
	void        UpdateShooter( TFLOAT a_fUnk1 );        // also called by Game
	void        UpdatePlayerControls( TFLOAT a_fUnk1 ); // also called by Flow
	void        UpdateKids( TFLOAT a_fUnk1 );           // also called by Game
	void        UpdatePlayer( TFLOAT a_fUnk1 );         // also called by Game

	//-----------------------------------------------------------------------------
	// [Enemies]
	//-----------------------------------------------------------------------------
	static TINT  BuildTransform( TFLOAT* a_pUnk1, TINT a_iUnk2, TFLOAT* a_pUnk3, TFLOAT* a_pUnk4 ); // also called by Setup
	TINT         CountActivePickups();                                                              // also called by Flow
	TINT         GetPickupGroup( TINT a_iUnk1 );
	TINT         AddLives2( TINT a_iUnk1, TINT a_iUnk2 );
	void*        GetCowSidePosition( void* a_pUnk1, void* a_pUnk2, TINT a_iUnk3, TINT a_iUnk4, TINT a_iUnk5, TINT a_iUnk6, void* a_pUnk7, void* a_pUnk8 );
	TINT         ResetOtherPowerUpTimers( TINT a_iUnk1, TINT a_iUnk2 );
	static void* WeightedChoice_Begin( void* a_pThis, void* a_pUnk1 ); // also called by Setup
	TBOOL        StartWaiting( TINT a_iUnk1, TINT a_iUnk2 );
	void*        GetEnemyExitPosition( Toshi::TVector4* a_pUnk1, void* a_pUnk2 );
	static TINT  PickWeighted( void* a_pThis );     // also called by Flow,Setup
	void         TurnFarmerTowards( TINT a_iUnk1 ); // also called by Player
	TINT         HidePickup( void* a_pUnk1 );       // also called by Flow,Game
	void*        RemoveEnemy( void* a_pUnk1 );      // also called by Game
	void*        FindClosestCow( const Toshi::TVector4* a_pUnk1, TFLOAT a_fUnk2 );
	TINT         PuffPickup( TINT a_iUnk1 );
	TINT         ResetEnemySpeed( void* a_pUnk1 );
	void*        GetEnemyStartPosition( Toshi::TVector4* a_pUnk1, void* a_pUnk2 );
	void*        GetLauncherBonePosition1( void* a_pUnk1 );
	void*        GetLauncherBonePosition2( void* a_pUnk1 );
	void*        GetLauncherBoneDirection( void* a_pUnk1 );
	TBOOL        MovePickupToLauncher( TFLOAT* a_pUnk1, TFLOAT a_fUnk2 );
	TBOOL        MovePickupToBone2( TINT a_iUnk1, TFLOAT a_fUnk2 );
	void         OnCowTipped( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 );
	TBOOL        GivePowerUp( TINT a_iUnk1 );
	TINT         PickFreeCow( TINT a_iUnk1, void* a_pUnk2 );
	void         SpawnCoyote();
	void         UpdateCoyoteTimer( TFLOAT a_fUnk1 ); // also called by Game
	TINT         UpdatePickups( TFLOAT a_fUnk1 );     // also called by Game
	void         StartPushing( TINT a_iUnk1, TINT a_iUnk2 );
	void*        GetCowSideForEnemy( void* a_pUnk1, void* a_pUnk2, TINT a_iUnk3, TFLOAT* a_pUnk4, void* a_pUnk5 );
	void*        GetCowSideAndHelper( void* a_pUnk1, void* a_pUnk2, void* a_pUnk3, TFLOAT* a_pUnk4, void* a_pUnk5 );
	void         SendEnemyToCow( TINT a_iUnk1 );
	TBOOL        UpdateEnemies( TFLOAT a_fUnk1 ); // also called by Game

	//-----------------------------------------------------------------------------
	// [Units]
	//-----------------------------------------------------------------------------
	static TBOOL     IsUnitNotYukking( TINT a_iUnk1 );                            // also called by Enemies,Game,Player,Proj
	void             TurnUnitTowardsPlayer( TINT a_iUnk1 );                       // also called by Flow
	void             OnHitCollectable( Projectile* a_pProjectile, Coin* a_pUnk ); // callback of HitTestCoins
	void             SetObjectsLeft( TINT a_iUnk1 );
	TINT             ShowMessagePopup( const TWCHAR* a_wszText );
	TINT             ShowEndPopup( void* a_pUnk1 );                                                                                                   // also called by Game
	TINT             AddLives1( TINT a_iUnk1, TINT a_iUnk2 );                                                                                         // also called by Game,Proj
	TINT             AddLifeAt( TINT a_iUnk1, TINT a_iUnk2 );                                                                                         // also called by Enemies
	static TINT      GetUnitStartBit( void* a_pThis, TINT a_iUnk1 );                                                                                  // also called by Enemies,Flow
	TBOOL            IsPowerUpActive( TINT a_iUnk1 );                                                                                                 // also called by Flow,Game,Player,Setup
	TINT             GetUnitHeldObject( TINT a_iUnk1 );                                                                                               // also called by Player
	TBOOL            MakeUnitRunAway( TINT a_iUnk1, TFLOAT a_fUnk2, TFLOAT a_fUnk3, TFLOAT a_fUnk4 );                                                 // also called by Enemies,Proj
	TBOOL            IsTargetInAim( void* a_pUnk1, TFLOAT* a_pUnk2, TFLOAT* a_pUnk3, TINT a_iUnk4, TFLOAT* a_pUnk5, TFLOAT a_fUnk6, TFLOAT a_fUnk7 ); // also called by Player,Proj
	void             ShowScoreFloat( void* a_pUnk1, TINT a_iUnk2, TINT a_iUnk3 );                                                                     // also called by Proj
	void             PlaySplatEffect( void* a_pUnk1, TINT a_iUnk2 );                                                                                  // also called by Proj
	TBOOL            SetScore( TINT a_iUnk1 );                                                                                                        // also called by Game,Proj
	TINT             GetNumUnhit13268();                                                                                                              // also called by Game,Player
	TINT             FreeProjectile( void* a_pUnk1, TINT a_iUnk2 );                                                                                   // also called by Player,Proj
	TBOOL            HitFarmer();
	static TINT      DetachUnit( TINT a_iUnk1 );             // also called by Proj
	TINT             StealUnitCarriedObject( TINT a_iUnk1 ); // also called by Enemies
	void             SendUnitBackToStart( TINT a_iUnk1 );    // also called by Enemies,Game,Proj
	TINT             SendUnitBackToStartRunning( Enemy* a_pUnit );
	Toshi::TVector4* GetUnitBonePos6( Toshi::TVector4* a_pOutPos, Enemy* a_pUnit );
	void*            GetUnitTargetPos( void* a_pUnk1, TINT a_iUnk2 );       // also called by Enemies,Game,Player,Proj
	TINT             GetUnit13268TargetPos( TINT a_iUnk1, void** a_pUnk2 ); // also called by Player,Proj
	Toshi::TVector4* GetUnit13270TargetPos( Toshi::TVector4* a_pOutPos, PopUpUnit* a_pUnit );
	static void*     GetCratePos( Toshi::TVector4* a_pUnk1, TINT a_iUnk2 ); // also called by Enemies,Player
	PopUpUnit*       FindAimed13270( const Toshi::TVector4* a_pDir, const Toshi::TVector4* a_pPos, TFLOAT* a_pOutDist, TFLOAT a_fUnk4, TFLOAT a_fMaxDist, TBOOL a_bUnk6 );
	void*            FindAimedCrate( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, TFLOAT* a_pUnk3, TFLOAT a_fUnk4, TFLOAT a_fUnk5, TFLOAT a_fUnk6 ); // also called by Player
	void*            FindAimedCoin( TFLOAT* a_pUnk1, TFLOAT* a_pUnk2, void* a_pUnk3, void* a_pUnk4, void* a_pUnk5, TFLOAT a_fUnk6 );      // also called by Player
	// Hit callbacks of HitTestCoins, HitTest13120, HitTest13268 and HitTestCrates
	// NOTE: retail passes them as member function pointers (function + this adjustment)
	using t_HitCallbackCoin  = void ( ACowTippingMiniGameState::* )( Projectile* a_pProjectile, Coin* a_pUnk );
	using t_HitCallback13120 = TBOOL ( ACowTippingMiniGameState::* )( TINT a_iProjectile, void* a_pUnit, TINT a_iDist, TINT a_iList );
	using t_HitCallback13268 = TINT ( ACowTippingMiniGameState::* )( TINT a_iProjectile, void* a_pUnit, TINT a_iDist, TINT a_iList );
	using t_HitCallbackCrate = TINT ( ACowTippingMiniGameState::* )( Projectile* a_pProjectile, Crate* a_pUnk, TFLOAT a_fDist, TINT a_iList );
	Coin*      HitTestCoins( const Toshi::TVector4* a_pDir, Projectile* a_pProjectile, const Toshi::TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallbackCoin a_fnCallback, TBOOL a_bUnk9 );
	Enemy*     HitTest13120( const Toshi::TVector4* a_pDir, Projectile* a_pProjectile, const Toshi::TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallback13120 a_fnCallback, TBOOL a_bUnk9 );
	PopUpUnit* HitTest13268( const Toshi::TVector4* a_pDir, Projectile* a_pProjectile, const Toshi::TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallback13268 a_fnCallback, TBOOL a_bUnk9 );
	Crate*     HitTestCrates( const Toshi::TVector4* a_pDir, Projectile* a_pProjectile, const Toshi::TVector4* a_pPos, TFLOAT* a_pOutDist, TINT a_iUnk5, TFLOAT a_fMaxDist, TINT a_iList, t_HitCallbackCrate a_fnCallback, TBOOL a_bUnk9 );
	void       ScareUnitsNearProjectile( const Toshi::TVector4* a_pPos, TFLOAT* a_pUnk2, TFLOAT a_fMaxDistXZ, TFLOAT a_fMaxDist );
	void       CheckNearMiss13270( const Toshi::TVector4* a_pPos, TFLOAT* a_pOutDist, TFLOAT a_fMaxDistXZ, TFLOAT a_fMaxDist );
	TINT       AddUnitScore( Enemy* a_pUnit, TINT a_iScore, TINT a_iBonus, Projectile* a_pProjectile );
	TINT       AddUnit13268Score( PopUpUnit* a_pUnit, TINT a_iScore, TINT a_iBonus );
	TBOOL      AddUnit13270Score( PopUpUnit* a_pUnit, TINT a_iScore, TINT a_iBonus );
	TINT       AddCrateScore( Crate* a_pUnk, TINT a_iScore, TINT a_iBonus );
	void       PlayProjectileSplat( void** a_pUnk1, TINT a_iUnk2 ); // also called by Proj
	TINT       PlayProjectileSplatWithSound( Projectile* a_pProjectile, void* a_pUnk2 );
	TINT       AddLifeAtUnit( PlayerData* a_pPlayer, Enemy* a_pUnit );
	TBOOL      OnUnitHit( void* a_pUnk1, TINT a_iUnk2, TINT a_iUnk3, TINT a_iUnk4 );                // also called by Game,Player
	TBOOL      OnProjectileHitUnit13120( TINT a_iUnk1, void* a_pUnk2, TINT a_iUnk3, TINT a_iUnk4 ); // also called by Proj
	void       OnProjectileHitUnit13270( Projectile* a_pProjectile, PopUpUnit* a_pUnit, TFLOAT a_fDist, TINT a_iList );
	TBOOL      TestProjectileVs13270( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 ); // also called by Proj
	TINT       OnProjectileHitCrate( Projectile* a_pProjectile, Crate* a_pUnk, TFLOAT a_fDist, TINT a_iList );
	TBOOL      TestProjectileVsCrates( void* a_pUnk1, TINT a_iUnk2, void** a_pUnk3 ); // also called by Proj
	TBOOL      TestProjectileVsCoins( TINT a_iUnk1, TINT a_iUnk2, TINT a_iUnk3 );     // also called by Proj
	void*      SendAllUnitsBackToStart();                                             // also called by Flow,Game
	TINT       EndGame();                                                             // also called by Game
	TINT       RemoveObjects( TINT a_iUnk1 );
	TINT       OnProjectileHitUnit13268( TINT a_iUnk1, void* a_pUnk2, TINT a_iUnk3, TINT a_iUnk4 ); // also called by Proj
	TBOOL      TestProjectileVsAll( TINT a_iUnk1, TFLOAT* a_pUnk2, TINT a_iUnk3 );                  // also called by Proj

	//-----------------------------------------------------------------------------
	// [Proj]
	//-----------------------------------------------------------------------------
	TBOOL                   IsPlayerDucking();                                                                             // also called by Flow
	static void             PlayEnemySound( Enemy* a_pEnemy, ASoundId a_eSound0, ASoundId a_eSound1, ASoundId a_eSound2 ); // also called by Flow
	TFLOAT                  GetThrowSpeed();
	Toshi::TVector4*        CalculateThrowVelocity( Toshi::TVector4* a_pOutVelocity, const Toshi::TVector4* a_pFrom, const Toshi::TVector4* a_pTo, TFLOAT a_fSpeed );
	Toshi::TVector4*        CalculateThrowVelocityToTarget( Toshi::TVector4* a_pOutVelocity, const Toshi::TVector4* a_pPosition );                                     // also called by Setup
	static Toshi::TVector4* CalculateThrowVelocityToTarget2( ACowTippingMiniGameState* a_pThis, Toshi::TVector4* a_pOutVelocity, const Toshi::TVector4* a_pPosition ); // also called by Player
	Toshi::TVector4*        GetClosestGoalPosition( Toshi::TVector4* a_pOutPosition, TINT a_iIndex, const TCHAR* a_szGoalName, const Toshi::TVector4* a_pPosition );   // also called by Enemies
	Toshi::TVector4*        GetCowHitPoints( Cow* a_pCow, Toshi::TVector4* a_pOut1, Toshi::TVector4* a_pOut2, Toshi::TVector4* a_pOut3, Toshi::TVector4* a_pOut4 );    // also called by Enemies,Player
	TINT                    OnPlayerHit();
	void                    ClearUnitsFlag9(); // also called by Game,Player
	TBOOL                   AimAtPathFollower( AInteriorSteer* a_pSteer, const Toshi::TVector4* a_pFrom, Toshi::TVector4* a_pOutVelocity, Toshi::TVector4* a_pOutTarget );
	void                    AimAtMovingTarget( const Toshi::TVector4* a_pTarget, const Toshi::TVector4* a_pTargetVelocity, const Toshi::TVector4* a_pFrom, Toshi::TVector4* a_pOutVelocity, Toshi::TVector4* a_pOutTarget );
	Enemy*                  FindClosestEnemy( const Toshi::TVector4* a_pPosition, TFLOAT a_fMaxDistance ); // also called by Enemies,Player
	Cow*                    FindHitCow( const Toshi::TVector4* a_pDirection, const Toshi::TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, COWHIT* a_pOutHitType, TBOOL a_bUnk );
	void                    OnProjectileReachedPlayer( Projectile* a_pProjectile );
	static void             ResetProjectileScale( Projectile* a_pProjectile ); // also called by Game
	static void             SetProjectileScale( Projectile* a_pProjectile, TFLOAT a_fScale );
	static AInteriorGoal*   FindRandomGoal( ACowTippingMiniGameState* a_pThis, TINT a_iIndex, const TCHAR* a_szGoalName );                                                                             // also called by Enemies,Setup
	Toshi::TVector4*        GetPlayerHandPosition( Toshi::TVector4* a_pOutPosition );                                                                                                                  // also called by Player
	Enemy*                  FindHitEnemy( const Toshi::TVector4* a_pDirection, const Toshi::TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, TBOOL a_bUnk );    // also called by Player
	void*                   FindHitUnk13268( const Toshi::TVector4* a_pDirection, const Toshi::TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fRadius, TFLOAT a_fMaxDistance, TBOOL a_bUnk ); // also called by Player
	void                    CheckNearMissUnk13268( const Toshi::TVector4* a_pPosition, TFLOAT* a_pOutDistance, TFLOAT a_fMaxDistanceXZ, TFLOAT a_fMaxDistance );
	TINT                    AwardCowPoints( Cow* a_pCow, TINT a_iPoints ); // also called by Enemies
	TINT                    SplatOnWorld( Projectile* a_pProjectile, const Toshi::TVector4* a_pNormal );
	void                    SplatProjectilesCollided( Projectile* a_pProjectile, Projectile* a_pOther );
	void                    SplatOnEnemy( Projectile* a_pProjectile, Enemy* a_pEnemy );
	void                    OnCowHit( Projectile* a_pProjectile, Cow* a_pCow, TFLOAT a_fDistance, TINT a_iList, COWHIT a_iHitType );
	TBOOL                   TryHitCow( TINT a_iList, const Toshi::TVector4* a_pPosition, Projectile* a_pProjectile, Cow* a_pCow, TFLOAT a_fDistance, COWHIT a_iHitType );
	Toshi::TVector4*        CalculateThrowVelocityToEnemy( Toshi::TVector4* a_pOutVelocity, const Toshi::TVector4* a_pFrom, Enemy* a_pEnemy );
	Toshi::TVector4*        GetEnemyRetreatPosition( Toshi::TVector4* a_pOutPosition, Enemy* a_pEnemy, Toshi::TVector4* a_pOutDirection ); // also called by Enemies
	TBOOL                   UpdateProjectile( Projectile* a_pProjectile, TFLOAT a_fDeltaTime, TINT a_iList );
	void                    MakeEnemyRetreat( Enemy* a_pEnemy ); // also called by Enemies,Flow
	TBOOL                   CheckHitCows( TINT a_iList, const Toshi::TVector4* a_pPosition, Projectile* a_pProjectile );
	TBOOL                   TryHitEnemy( TINT a_iList, const Toshi::TVector4* a_pPosition, Projectile* a_pProjectile, Enemy* a_pEnemy, TFLOAT a_fDistance );
	TBOOL                   TryHitUnk13268( TINT a_iList, const Toshi::TVector4* a_pPosition, Projectile* a_pProjectile, void* a_pUnk, TFLOAT a_fDistance );
	TBOOL                   CheckHitEnemies( TINT a_iList, const Toshi::TVector4* a_pPosition, Projectile* a_pProjectile );
	TBOOL                   CheckHitUnk13268( TINT a_iList, const Toshi::TVector4* a_pPosition, Projectile* a_pProjectile );
	void                    UpdateProjectiles( TFLOAT a_fDeltaTime ); // also called by Game

public:
	//-----------------------------------------------------------------------------
	// CowTipping.trb "Ints" (0xbac..0xc98, copied by LoadData)
	//-----------------------------------------------------------------------------
	TINT   m_iStartScore;             // 0xbac
	TINT   m_aiUnitTipScore[ 5 ];     // 0xbb0, per enemy type
	TINT   m_aiUnitSplatScore[ 5 ];   // 0xbc4, per enemy type
	TINT   m_iRaccoonHitScore;        // 0xbd8
	TINT   m_iGopherHitScore;         // 0xbdc
	TINT   m_iCoyoteHitScore;         // 0xbe0
	TINT   m_iCrateHitScore;          // 0xbe4
	TINT   m_iMovingHitBonus;         // 0xbe8, bonus for hitting an enemy that is on the move
	TINT   m_iCowTippedPenalty;       // 0xbec
	TINT   m_iCowHitPenalty;          // 0xbf0
	TINT   m_iCowHitPenaltyPushing;   // 0xbf4
	TUINT8 m_PADDING_00BF8[ 0x4 ];    // 0xbf8
	TINT   m_aiUnitMaxHits[ 6 ];      // 0xbfc, per enemy type
	TINT   m_iGopherHitsEnabled;      // 0xc14, gophers can only be hit if > 0
	TINT   m_iCrateMaxHits;           // 0xc18
	TINT   m_iTomatoHits;             // 0xc1c, hits of a tomato (m_aProjectileLists[ 0 ].iHits)
	TINT   m_iGrowType1Hits;          // 0xc20, hits of a projectile with iGrowType 1
	TINT   m_iGrowType2Hits;          // 0xc24, hits of a projectile with iGrowType 2
	TINT   m_aiNumKids[ 5 ];          // 0xc28, per kid character (CreateKids reads the first 3)
	TINT   m_aiPowerUpCharges[ 6 ];   // 0xc3c, per power-up, added to PowerUp::iCharges
	TINT   m_iPowerUp0Shots;          // 0xc54
	TINT   m_iStartAmmo;              // 0xc58
	TINT   m_aiPowerUpAmmo[ 6 ];      // 0xc5c, per power-up, the new m_iMaxAmmo (0: unchanged)
	TINT   m_iCowSpinsToSleep;        // 0xc74
	TINT   m_iStreakForExtraLife;     // 0xc78, extra life when m_iStreak reaches it
	TINT   m_iCoyoteHitsForExtraLife; // 0xc7c
	TINT   m_iScoreForExtraLife;      // 0xc80
	TINT   m_iAmmoPerTarget;          // 0xc84, also the max. Enemy::iUnk40 that can be locked on
	TINT   m_iMaxAmmoLimit;           // 0xc88
	TINT   m_iNumObjects;             // 0xc8c
	TINT   m_iObjectsLostOnYuk;       // 0xc90
	TINT   m_iObjectsLostOnYukObject; // 0xc94

	//-----------------------------------------------------------------------------
	// CowTipping.trb "Floats" (0xc98..0x10b8, copied by LoadData)
	//-----------------------------------------------------------------------------
	TFLOAT          m_fUnk_00C98;                 // 0xc98
	TFLOAT          m_fTurnEndDelay;              // 0xc9c
	TFLOAT          m_fMoveSpeed;                 // 0xca0
	TFLOAT          m_fPitchInputSpeed;           // 0xca4
	TFLOAT          m_fAimLockRadius;             // 0xca8
	TFLOAT          m_fAimLockDistance;           // 0xcac
	TFLOAT          m_fAimLockFollowSpeed;        // 0xcb0
	TFLOAT          m_fTargetHeight;              // 0xcb4
	TFLOAT          m_fInputAccelTime;            // 0xcb8
	TFLOAT          m_fInputDeadZone;             // 0xcbc
	TFLOAT          m_fNearRayRadius;             // 0xcc0
	TFLOAT          m_fMouseMoveSpeed;            // 0xcc4
	TFLOAT          m_fMousePitchSpeed;           // 0xcc8
	TFLOAT          m_fMouseMaxMoveX;             // 0xccc
	TFLOAT          m_fMouseMaxMoveY;             // 0xcd0
	TFLOAT          m_fMouseMinMoveX;             // 0xcd4
	TUINT8          m_PADDING_00CD8[ 0x4 ];       // 0xcd8
	TFLOAT          m_fMouseExponentX;            // 0xcdc
	TFLOAT          m_fMouseExponentY;            // 0xce0
	TUINT8          m_PADDING_00CE4[ 0x4 ];       // 0xce4
	TFLOAT          m_fAimTilt;                   // 0xce8
	TFLOAT          m_fReticleOffsetY;            // 0xcec
	TUINT8          m_PADDING_00CF0[ 0xc ];       // 0xcf0
	TFLOAT          m_fRailSpeedScale;            // 0xcfc
	TFLOAT          m_fRailSpeedExponent;         // 0xd00
	TFLOAT          m_fAimMinDot;                 // 0xd04
	TFLOAT          m_fAimMinDotPowerUp;          // 0xd08
	AimAcceleration m_aAimAcceleration[ 2 ];      // 0xd0c, index Stage::iCameraSettings
	TFLOAT          m_fThrowDelay;                // 0xd2c
	TFLOAT          m_fHitFadeTime;               // 0xd30
	TFLOAT          m_fHitShakeTime;              // 0xd34
	TFLOAT          m_fReticleSpeedMin;           // 0xd38
	TFLOAT          m_fReticleSpeedMax;           // 0xd3c
	TFLOAT          m_fUnk_00D40;                 // 0xd40
	TFLOAT          m_fUnk_00D44;                 // 0xd44
	TUINT8          m_PADDING_00D48[ 0x4 ];       // 0xd48
	TFLOAT          m_fHitShakeAmplitude;         // 0xd4c
	TFLOAT          m_fHitShakeInterval;          // 0xd50
	TFLOAT          m_fProjectileMaxRange;        // 0xd54
	TFLOAT          m_fThrowSpeed;                // 0xd58
	TFLOAT          m_fThrowSpeedLarge;           // 0xd5c
	TFLOAT          m_fGrowType1MaxScale;         // 0xd60
	TFLOAT          m_fGrowType1ShrinkSpeed;      // 0xd64
	TFLOAT          m_fScalePowerUp1;             // 0xd68
	TFLOAT          m_fHitTestScale;              // 0xd6c
	TUINT8          m_PADDING_00D70[ 0x4 ];       // 0xd70
	TFLOAT          m_fThrowAnimSpeedPowerUp1;    // 0xd74
	TFLOAT          m_fThrowAnimSpeedPowerUp0;    // 0xd78
	TFLOAT          m_fGrowType2MaxScale;         // 0xd7c
	TFLOAT          m_fAimLockRadiusScale;        // 0xd80, while power-up 2 is active
	TUINT8          m_PADDING_00D84[ 0x4 ];       // 0xd84
	TFLOAT          m_fProjectileCeilingHeight;   // 0xd88
	TFLOAT          m_fThrowAnimSpeedPowerUp2;    // 0xd8c
	TFLOAT          m_fScalePowerUp2;             // 0xd90
	TFLOAT          m_fProjectileCollideDistance; // 0xd94
	TFLOAT          m_fTomatoSpeedMin;            // 0xd98
	TFLOAT          m_fTomatoSpeedMax;            // 0xd9c
	TFLOAT          m_fHitPlayerDistanceSq;       // 0xda0
	TFLOAT          m_fHitPlayerDistanceSqLarge;  // 0xda4
	TUINT8          m_PADDING_00DA8[ 0xc ];       // 0xda8
	TFLOAT          m_fRaccoonHitDistance;        // 0xdb4
	TFLOAT          m_fRaccoonSplatOffset;        // 0xdb8
	TFLOAT          m_fRaccoonHitRadius;          // 0xdbc
	TFLOAT          m_fRaccoonNearMissDistanceXZ; // 0xdc0
	TFLOAT          m_fRaccoonNearMissDistance;   // 0xdc4
	TFLOAT          m_fRaccoonShootDelay;         // 0xdc8, start value of m_fRaccoonShootTimer
	TFLOAT          m_fCowSearchRadius;           // 0xdcc
	TFLOAT          m_fCowSideMaxDistance;        // 0xdd0
	TFLOAT          m_fTipThreshold;              // 0xdd4
	TUINT8          m_PADDING_00DD8[ 0x4 ];       // 0xdd8
	TFLOAT          m_fCowHitDistance;            // 0xddc
	TFLOAT          m_fTipRecoverSpeed;           // 0xde0
	TFLOAT          m_fTipRecoverSpeedFree;       // 0xde4
	TFLOAT          m_fCowHitRadius;              // 0xde8
	TFLOAT          m_fCowTimerMin;               // 0xdec
	TFLOAT          m_fCowTimerMax;               // 0xdf0
	TFLOAT          m_fExitRadius;                // 0xdf4
	TFLOAT          m_fEnemyHitDistance;          // 0xdf8
	TFLOAT          m_fEnemyNearMissDistance;     // 0xdfc
	TFLOAT          m_fAimRayLength;              // 0xe00
	TFLOAT          m_fEnemyHeight;               // 0xe04
	TFLOAT          m_fEnemyGoalDistanceXZ;       // 0xe08
	TFLOAT          m_fEnemyHitRadius;            // 0xe0c
	TUINT8          m_PADDING_00E10[ 0x4 ];       // 0xe10
	TFLOAT          m_fMovingBonusMinSpeed;       // 0xe14
	TFLOAT          m_fScareDistanceXZ;           // 0xe18
	TFLOAT          m_fCowSideMoveSpeed;          // 0xe1c
	TFLOAT          m_fCowSideTurnSpeed;          // 0xe20
	TFLOAT          m_fWalkSpeedRandom;           // 0xe24
	TFLOAT          m_fRunSpeedRandom;            // 0xe28
	TFLOAT          m_fUnk_00E2C;                 // 0xe2c
	TFLOAT          m_fUnk_00E30;                 // 0xe30
	TFLOAT          m_fUnk_00E34;                 // 0xe34
	TFLOAT          m_fRunAwayTimeMin;            // 0xe38
	TFLOAT          m_fRunAwayTimeMax;            // 0xe3c
	TFLOAT          m_fRetreatUnk24;              // 0xe40, Enemy::fUnk24 when it retreats
	TFLOAT          m_fRunSpeedScaleHurt;         // 0xe44
	TFLOAT          m_fWalkSpeedBase;             // 0xe48
	TFLOAT          m_fRunSpeedBonus;             // 0xe4c
	TFLOAT          m_fYukRunSpeedBonus;          // 0xe50
	TFLOAT          m_fKidMinSpeed;               // 0xe54
	TFLOAT          m_fKidMaxSpeed;               // 0xe58
	TFLOAT          m_fEnemyWaitTimeMin;          // 0xe5c
	TFLOAT          m_fEnemyWaitTimeMax;          // 0xe60
	TFLOAT          m_fRunAwayWaitMin;            // 0xe64
	TFLOAT          m_fRunAwayWaitMax;            // 0xe68
	TFLOAT          m_fEnemyWalkTimeMin;          // 0xe6c
	TFLOAT          m_fEnemyWalkTimeMax;          // 0xe70
	TFLOAT          m_fEnemyHowlTimeMin;          // 0xe74
	TFLOAT          m_fEnemyHowlTimeMax;          // 0xe78
	TFLOAT          m_fTargetStillTime;           // 0xe7c, Enemy::fStillTime of the enemy a projectile is thrown at
	TFLOAT          m_fLargeStageTimeScale;       // 0xe80
	TFLOAT          m_afUnitTipSpeed[ 5 ];        // 0xe84, per enemy type
	TFLOAT          m_afUnitRunSpeed[ 5 ];        // 0xe98, per enemy type
	TFLOAT          m_fCoyoteInterval;            // 0xeac
	TFLOAT          m_fCoyoteRunSpeed;            // 0xeb0
	TFLOAT          m_fCoyoteWaitTimeMin;         // 0xeb4
	TFLOAT          m_fCoyoteWaitTimeMax;         // 0xeb8
	TFLOAT          m_fCoyoteRunAwayTimeMin;      // 0xebc
	TFLOAT          m_fCoyoteRunAwayTimeMax;      // 0xec0
	TFLOAT          m_fCoyoteWalkTimeMin;         // 0xec4
	TFLOAT          m_fCoyoteWalkTimeMax;         // 0xec8
	TFLOAT          m_fCoyoteHowlTimeMin;         // 0xecc
	TFLOAT          m_fCoyoteHowlTimeMax;         // 0xed0
	TFLOAT          m_fGopherHitDistance;         // 0xed4
	TFLOAT          m_fCrateHitDistance;          // 0xed8
	TFLOAT          m_fUnk_00EDC;                 // 0xedc
	TFLOAT          m_fGopherHitRadius;           // 0xee0
	TFLOAT          m_fCrateHitRadius;            // 0xee4
	TFLOAT          m_fGopherNearMissDistanceXZ;  // 0xee8
	TFLOAT          m_fGopherDelay;               // 0xeec, start value of m_fGopherTimer
	TFLOAT          m_fGopherNearMissDistance;    // 0xef0
	TFLOAT          m_fPickupMoveSpeed;           // 0xef4
	TFLOAT          m_fPickupMoveSpeed2;          // 0xef8
	TFLOAT          m_fPickupLift;                // 0xefc
	TFLOAT          m_fPickupTiltSpeed;           // 0xf00
	TFLOAT          m_fPickupTurnSpeed;           // 0xf04
	TFLOAT          m_fPickupArriveDistance;      // 0xf08
	TFLOAT          m_afPowerUpTimes[ 6 ];        // 0xf0c, per power-up, PowerUp::fTime when it's given ([ 3 ]: the flare)
	TFLOAT          m_fBounceTime;                // 0xf24
	TFLOAT          m_fBounceFallTime;            // 0xf28
	TFLOAT          m_fBlinkSpeedMin;             // 0xf2c
	TFLOAT          m_fBlinkSpeedMax;             // 0xf30
	TUINT8          m_PADDING_00F34[ 0x4 ];       // 0xf34
	TFLOAT          m_fPickupOpacityA;            // 0xf38
	TFLOAT          m_fPickupOpacityB;            // 0xf3c
	TFLOAT          m_fBlinkSteps;                // 0xf40
	CameraSettings  m_aCameraSettings[ 2 ];       // 0xf44, index Stage::iCameraSettings
	TFLOAT          m_fStreakTime;                // 0x1014, start value of m_fStreakTimer
	TFLOAT          m_fShootRumbleStrength;       // 0x1018
	TFLOAT          m_fShootRumbleUnk;            // 0x101c
	TFLOAT          m_fHitRumbleStrength;         // 0x1020, see OnPlayerHit
	TFLOAT          m_fHitRumbleUnk;              // 0x1024, see OnPlayerHit
	TFLOAT          m_fRumbleStrength1;           // 0x1028
	TFLOAT          m_fRumbleDuration1;           // 0x102c
	TFLOAT          m_fRumbleStrength2;           // 0x1030
	TFLOAT          m_fRumbleDuration2;           // 0x1034
	TFLOAT          m_fEndPopupDuration;          // 0x1038
	TFLOAT          m_fEndPopupMoveDuration;      // 0x103c
	TFLOAT          m_fLevelPopupDuration;        // 0x1040
	TFLOAT          m_fLevelPopupMoveDuration;    // 0x1044
	TFLOAT          m_fBonusPopupDuration;        // 0x1048
	TFLOAT          m_fBonusPopupMoveDuration;    // 0x104c
	TFLOAT          m_fBonusPopupDelay;           // 0x1050
	TFLOAT          m_fMusicDelay;                // 0x1054
	TFLOAT          m_fMusicFadeInSpeed;          // 0x1058
	TFLOAT          m_fMusicFadeOutSpeed;         // 0x105c
	TFLOAT          m_fShakeTime2;                // 0x1060
	TFLOAT          m_fShakeInterval2;            // 0x1064
	TFLOAT          m_fShakeAmplitude2;           // 0x1068
	TFLOAT          m_fReticleTime;               // 0x106c
	TFLOAT          m_fShooterRadius;             // 0x1070
	TFLOAT          m_fRunnerChaseTime;           // 0x1074
	TFLOAT          m_fRunnerOutTime;             // 0x1078
	TFLOAT          m_fRunnerTipRadius;           // 0x107c
	TFLOAT          m_fRunnerPipeTime;            // 0x1080
	TFLOAT          m_fCoinHitRadius;             // 0x1084
	TFLOAT          m_fCoinHitDistance;           // 0x1088
	TFLOAT          m_fCoinNearRayRadius;         // 0x108c
	TFLOAT          m_fKidTargetRadius;           // 0x1090
	TFLOAT          m_fKidProjectileRadius;       // 0x1094
	TFLOAT          m_fKidShootDelayMin;          // 0x1098
	TFLOAT          m_fKidShootDelayMax;          // 0x109c
	TFLOAT          m_fLauncherFireMinDot;        // 0x10a0
	TFLOAT          m_fFarmerFacingMinDot;        // 0x10a4
	TFLOAT          m_fEnemyTurnSpeed;            // 0x10a8
	TFLOAT          m_fTrajectoryGravity;         // 0x10ac
	TFLOAT          m_fFadeSpeed;                 // 0x10b0
	TFLOAT          m_fTimeOfDayDuration;         // 0x10b4

	Toshi::TPString8                         m_aModelNames[ 5 ];                                    // 0x10b8
	Toshi::TVector4                          m_vPlayerPos;                                          // 0x10cc
	Toshi::TVector4                          m_vPlayerVelocity;                                     // 0x10dc
	Toshi::TVector4                          m_vRailDirection;                                      // 0x10ec
	Toshi::TVector4                          m_vAimDirection;                                       // 0x10fc
	TUINT8                                   m_aUnk_0110C[ 0x10 ];                                  // 0x110c
	Toshi::TVector4                          m_vAimTarget;                                          // 0x111c
	Toshi::TVector4                          m_vReticleTarget;                                      // 0x112c
	Toshi::TVector4                          m_vLockPosition;                                       // 0x113c
	Toshi::TVector4                          m_vCameraDir;                                          // 0x114c
	Toshi::TVector4                          m_vCameraPos;                                          // 0x115c
	Toshi::TVector4                          m_vCameraBasePos;                                      // 0x116c
	Toshi::TVector4                          m_vPlayerCentre;                                       // 0x117c
	Toshi::TVector4                          m_vCameraLocatorPos;                                   // 0x118c
	Toshi::TVector4                          m_vCameraLocatorDir;                                   // 0x119c
	Toshi::TVector4                          m_vUnk_011AC;                                          // 0x11ac
	TUINT8                                   m_aUnk_011BC[ 0x10 ];                                  // 0x11bc
	Toshi::TVector4                          m_vCameraPitchAxis;                                    // 0x11cc
	Toshi::TVector4                          m_aWildMikeGoalPositions[ 8 ];                         // 0x11dc
	Toshi::TVector4                          m_aWildMikeGoalDirections[ 8 ];                        // 0x125c
	Toshi::TVector4                          m_aCoinPositions[ 8 ];                                 // 0x12dc, goals "coin%d"
	TUINT8                                   m_aUnk_0135C[ 0x20 ];                                  // 0x135c
	Toshi::TVector4                          m_vFarmerToTarget;                                     // 0x137c
	Toshi::TVector4                          m_vFarmerFacing;                                       // 0x138c
	Toshi::TVector4                          m_vUnk_0139C;                                          // 0x139c
	Toshi::TMatrix44                         m_aHenTransforms[ 15 ];                                // 0x13ac
	TINT                                     m_iNumLevels;                                          // 0x176c
	Level                                    m_aLevels[ MAX_LEVELS ];                               // 0x1770
	TINT                                     m_iUnk_057A8;                                          // 0x57a8
	TUINT8                                   m_auiUsedRaccoonBits[ 4 ];                             // 0x57ac, bit per raccoon spot
	TUINT8                                   m_auiPickupBits[ 4 ];                                  // 0x57b0
	TUINT8                                   m_auiCowSideReservedBits[ 4 ];                         // 0x57b4
	TUINT8                                   m_auiCowSideTakenBits[ 4 ];                            // 0x57b8
	TUINT8                                   m_auiUsedCoinBits[ 4 ];                                // 0x57bc, bit per coin position
	AAnimatableObject*                       m_pLauncher;                                           // 0x57c0
	AGameTimeFX*                             m_pCowTipFX;                                           // 0x57c4
	AGameTimeFX*                             m_pMoonFX;                                             // 0x57c8
	AGameTimeFX*                             m_pDawnFX;                                             // 0x57cc
	AInteriorNetwork*                        m_pNetwork;                                            // 0x57d0
	AGUI2KidCounter                          m_oAmmoCounter;                                        // 0x57d4
	ATargetReticle                           m_oTargetReticle;                                      // 0x6e08
	AAnimatableObject*                       m_pWildMikeBox;                                        // 0x7170
	AAnimatedUnit*                           m_pWildMike;                                           // 0x7174
	TFLOAT                                   m_fRunnerTime;                                         // 0x7178
	RUNNERSTATE                              m_eRunnerState;                                        // 0x717c
	Enemy*                                   m_pRunnerTarget;                                       // 0x7180
	Toshi::TVector4                          m_vRunnerGoal;                                         // 0x7184..0x7194
	TFLOAT                                   m_fRunnerPipeTimer;                                    // 0x7194
	AAnimatableObject*                       m_pHanna;                                              // 0x7198
	AUnitPlayer*                             m_pOtis;                                               // 0x719c, unit type cowtipping_otis (junkyard stages)
	Toshi::T2GUIElement                      m_oHudRoot;                                            // 0x71a0
	AGUI2Portrait                            m_oPortrait;                                           // 0x71f4
	AGUI2FloatHudElement                     m_oFloatHudElement;                                    // 0x77bc
	AGUI2MessagePopupFlare                   m_aFlarePopups[ 3 ];                                   // 0x79b0
	AGUI2AnythingPopup                       m_oLevelPopup;                                         // 0x8664
	AGUI2AnythingPopup                       m_aBonusPopups[ 3 ];                                   // 0x8724
	AGUI2AnythingPopup                       m_oEndPopup;                                           // 0x8964
	AGUI2AnythingPopup                       m_oTurnEndPopup;                                       // 0x8a24
	AMilkReticleManager                      m_oMilkReticleManager;                                 // 0x8ae4
	AGUI2LifeHud                             m_oLives1Hud;                                          // 0xe508
	AGUI2LifeHud                             m_oLives2Hud;                                          // 0xe8b0
	AGUI2ImagePopup                          m_oRaccoonWarning;                                     // 0xec58
	AGUI2ImagePopup                          m_oRunnerIcon;                                         // 0xedb0
	AGUI2ElementPopup                        m_oElementPopup;                                       // 0xef08
	AButtonSlamHudElement                    m_oButtonSlamHud;                                      // 0xef78
	Toshi::T2GUIRectangle                    m_oDagMug;                                             // 0xf444
	AGUI2TextBox                             m_oDagText;                                            // 0xf4bc
	AGUI2TextBox                             m_aLivesTexts[ 2 ];                                    // 0xf534
	AGUI2ImagePopup                          m_oCowShockMarks;                                      // 0xf624
	Toshi::T2GUITextureSection**             m_ppCowSpinSprites;                                    // 0xf77c
	Toshi::T2GUITextureSection**             m_ppCowFallSprites;                                    // 0xf780
	Toshi::T2GUITextureSection**             m_ppCowShockSprites;                                   // 0xf784
	Toshi::T2GUITextureSection**             m_ppCowSleepSprites;                                   // 0xf788
	Toshi::T2GUITextureSection**             m_ppCowShockedSprites;                                 // 0xf78c
	TINT                                     m_iNumCowSpinSprites;                                  // 0xf790
	TINT                                     m_iNumCowFallSprites;                                  // 0xf794
	TINT                                     m_iNumCowShockSprites;                                 // 0xf798
	TINT                                     m_iNumCowSleepSprites;                                 // 0xf79c
	TINT                                     m_iNumCowShockedSprites;                               // 0xf7a0
	RELOADSTATE                              m_iReloadState;                                        // 0xf7a4
	ProjectileList                           m_aProjectileLists[ 2 ];                               // 0xf7a8
	Projectile                               m_aProjectiles[ MAX_PROJECTILES ];                     // 0xf7c8
	Enemy                                    m_aEnemies[ 13 ];                                      // 0x11208
	Cow                                      m_aCows[ 15 ];                                         // 0x1171c
	Raccoon                                  m_aRaccoons[ 10 ];                                     // 0x12400
	Gopher                                   m_aGophers[ 14 ];                                      // 0x12590
	Crate                                    m_aCrates[ NUM_CRATE_GROUPS ][ NUM_CRATES_PER_GROUP ]; // 0x126a8
	Stage                                    m_aStages[ MAX_STAGES ];                               // 0x13008
	Toshi::T2SList<Enemy>                    m_llActiveEnemies;                                     // 0x13120
	Toshi::T2SList<Enemy>                    m_llDoneEnemies;                                       // 0x13124
	Coin                                     m_aCoins[ NUM_COINS ];                                 // 0x13128
	Toshi::T2SList<PopUpUnit>                m_llActiveRaccoons;                                    // 0x13268, Raccoon
	Toshi::T2SList<PopUpUnit>                m_llFreeRaccoons;                                      // 0x1326c, Raccoon
	Toshi::T2SList<PopUpUnit>                m_llActiveGophers;                                     // 0x13270
	Toshi::T2SList<PopUpUnit>                m_llFreeGophers;                                       // 0x13274
	AAnimationBreakpointUnitAction::Listener m_oAnimEventListener;                                  // 0x13278
	TFLOAT                                   m_fUnk_13288;                                          // 0x13288
	TFLOAT                                   m_fTimeLimit;                                          // 0x1328c
	TFLOAT                                   m_fCameraStartPitch;                                   // 0x13290
	TFLOAT                                   m_fCameraYawDelta;                                     // 0x13294
	TFLOAT                                   m_fCameraPitchDelta;                                   // 0x13298
	TFLOAT                                   m_fCameraYaw;                                          // 0x1329c
	TFLOAT                                   m_fCameraPitch;                                        // 0x132a0
	TFLOAT                                   m_fCameraTargetYaw;                                    // 0x132a4
	TFLOAT                                   m_fCameraTargetPitch;                                  // 0x132a8
	TFLOAT                                   m_fAccelTimerX;                                        // 0x132ac
	TFLOAT                                   m_fAccelTimerY;                                        // 0x132b0
	TFLOAT                                   m_fLastInputX;                                         // 0x132b4
	TFLOAT                                   m_fLastInputY;                                         // 0x132b8
	TFLOAT                                   m_fSpeedY;                                             // 0x132bc
	TFLOAT                                   m_fCoyoteTimer;                                        // 0x132c0
	TFLOAT                                   m_fRaccoonShootTimer;                                  // 0x132c4
	TFLOAT                                   m_fGopherTimer;                                        // 0x132c8
	TFLOAT                                   m_fThrowCooldown;                                      // 0x132cc
	TFLOAT                                   m_fUnk_132D0;                                          // 0x132d0
	TFLOAT                                   m_fCameraTargetTime;                                   // 0x132d4
	TUINT8                                   m_PADDING_132D8[ 0x4 ];                                // 0x132d8
	TFLOAT                                   m_fInputX;                                             // 0x132dc
	TFLOAT                                   m_fMoveY;                                              // 0x132e0
	TFLOAT                                   m_fSavedTreeLODDistance;                               // 0x132e4
	TFLOAT                                   m_fSavedTreeRenderDistance;                            // 0x132e8
	TFLOAT                                   m_fSavedTreeBlendingDistance;                          // 0x132ec
	TFLOAT                                   m_fScriptWaitStart;                                    // 0x132f0
	TINT                                     m_iUnk_132F4;                                          // 0x132f4
	TFLOAT                                   m_fStreakTimer;                                        // 0x132f8
	TFLOAT                                   m_fMouseX;                                             // 0x132fc
	TFLOAT                                   m_fMouseY;                                             // 0x13300
	TFLOAT                                   m_fUnk_13304;                                          // 0x13304
	TFLOAT                                   m_fGameTime;                                           // 0x13308
	TFLOAT                                   m_fUnk_1330C;                                          // 0x1330c
	TFLOAT                                   m_fPlayTime;                                           // 0x13310
	TFLOAT                                   m_fLevelTimer;                                         // 0x13314
	TFLOAT                                   m_fCameraShakeX;                                       // 0x13318
	TFLOAT                                   m_fCameraShakeY;                                       // 0x1331c
	TFLOAT                                   m_fHitShakeIntervalTimer;                              // 0x13320
	TFLOAT                                   m_fShakeTime;                                          // 0x13324
	TFLOAT                                   m_fRecoilX;                                            // 0x13328
	TFLOAT                                   m_fRecoilY;                                            // 0x1332c
	TFLOAT                                   m_fShakeIntervalTimer2;                                // 0x13330
	TFLOAT                                   m_fShakeTimeLeft2;                                     // 0x13334
	TFLOAT                                   m_fTurnEndTimer;                                       // 0x13338
	TFLOAT                                   m_fFixedStepTime;                                      // 0x1333c
	TFLOAT                                   m_fUnk_13340;                                          // 0x13340
	TFLOAT                                   m_fDustTimer;                                          // 0x13344
	TFLOAT                                   m_fPopupHideTimer;                                     // 0x13348
	TFLOAT                                   m_fMusicTimer;                                         // 0x1334c
	TFLOAT                                   m_fLevelTime;                                          // 0x13350
	TFLOAT                                   m_fReticleTimer;                                       // 0x13354
	TFLOAT                                   m_fLevelProgress;                                      // 0x13358
	TFLOAT                                   m_fTimeOfDayProgress;                                  // 0x1335c
	TFLOAT                                   m_fFarmerTurnProgress;                                 // 0x13360
	TFLOAT                                   m_fKidShootTimer;                                      // 0x13364
	TINT                                     m_iThrowAnim;                                          // 0x13368
	TINT                                     m_iUnk_1336C;                                          // 0x1336c
	TINT                                     m_iNumCows;                                            // 0x13370
	TINT                                     m_iNumHumanPlayers;                                    // 0x13374
	TINT                                     m_iCurrentPlayer;                                      // 0x13378
	TINT                                     m_iProjectileType;                                     // 0x1337c
	TINT                                     m_iNumPlayers;                                         // 0x13380
	TINT                                     m_iNumTipSpots;                                        // 0x13384
	TUINT8                                   m_PADDING_13388[ 0x4 ];                                // 0x13388
	TINT                                     m_iFloatTextIndex;                                     // 0x1338c
	TINT                                     m_iNumRaccoons;                                        // 0x13390
	TINT                                     m_iNumGophers;                                         // 0x13394
	TINT                                     m_iScriptCommand;                                      // 0x13398
	TINT                                     m_iScriptLoops;                                        // 0x1339c
	TINT                                     m_iNumStages;                                          // 0x133a0
	TINT                                     m_iStreak;                                             // 0x133a4, see m_iStreakForExtraLife
	TINT                                     m_iStage;                                              // 0x133a8
	TINT                                     m_iMaxAmmo;                                            // 0x133ac
	TINT                                     m_iNextCowSkin;                                        // 0x133b0
	TINT                                     m_aiSkinIndex[ 5 ];                                    // 0x133b4, next skin per enemy type
	TINT                                     m_iObjectsLeft;                                        // 0x133c8
	TINT                                     m_iStompCrateEffect;                                   // 0x133cc
	TINT                                     m_iStompCrateFlatEffect;                               // 0x133d0
	TINT                                     m_iNumHens;                                            // 0x133d4
	TINT                                     m_iCarryBone;                                          // 0x133d8
	TINT                                     m_iNumCoins;                                           // 0x133dc
	TUINT8                                   m_PADDING_133E0[ 0x4 ];                                // 0x133e0
	Enemy*                                   m_pLockEnemy;                                          // 0x133e4, locked on by the aiming
	Raccoon*                                 m_pLockRaccoon;                                        // 0x133e8, locked on by the aiming
	Enemy*                                   m_pThrowEnemy;                                         // 0x133ec, m_pLockEnemy when the throw started
	Raccoon*                                 m_pThrowRaccoon;                                       // 0x133f0, m_pLockRaccoon when the throw started
	AAnimatedUnit*                           m_pLockUnit;                                           // 0x133f4, while power-up 2 is active
	AAnimatedUnit*                           m_pThrowUnit;                                          // 0x133f8, m_pLockUnit when the throw started
	AUnit*                                   m_pCameraTarget;                                       // 0x133fc
	TINT                                     m_iUnk_13400;                                          // 0x13400
	TBOOL                                    m_bUnk_13404;                                          // 0x13404
	TBOOL                                    m_bSideStepping;                                       // 0x13405
	TBOOL                                    m_bNoInputX;                                           // 0x13406
	TBOOL                                    m_bNoInputY;                                           // 0x13407
	TBOOL                                    m_bUnk_13408;                                          // 0x13408
	TBOOL                                    m_bUnk_13409;                                          // 0x13409
	TBOOL                                    m_bUnk_1340A;                                          // 0x1340a
	TBOOL                                    m_bLevelDone;                                          // 0x1340b
	TBOOL                                    m_bShootHintShown;                                     // 0x1340c
	TBOOL                                    m_bGameOver;                                           // 0x1340d
	TBOOL                                    m_bRaccoonsShootNow;                                   // 0x1340e
	TBOOL                                    m_bShooterTurning;                                     // 0x1340f
	TBOOL                                    m_bUnk_13410;                                          // 0x13410
	TUINT8                                   m_PADDING_13411[ 3 ];                                  // 0x13411
	ENDSTATE                                 m_eEndState;                                           // 0x13414
	Toshi::TMatrix44                         m_matDagTransform;                                     // 0x13418..0x13458
	PlayerData                               m_aPlayers[ MAX_PLAYERS ];                             // 0x13458
	TWCHAR                                   m_awszFloatTexts[ 41 ][ 64 ];                          // 0x135e8, ring of float-hud texts (index m_iFloatTextIndex)
	TWCHAR                                   m_wszLevelText[ 128 ];                                 // 0x14a68..0x14b68
	TWCHAR                                   m_wszBonusText[ 128 ];                                 // 0x14b68..0x14c68
	TFLOAT                                   m_fSavedFarDistance;                                   // 0x14c68
	AWindmillHelper                          m_oWindmillHelper;                                     // 0x14c6c
	TINT                                     m_iEnemyScore;                                         // 0x14d04, results
	TINT                                     m_iRaccoonScore;                                       // 0x14d08, results
	TINT                                     m_iCoyoteScore;                                        // 0x14d0c, results
	TINT                                     m_iScoreTotal;                                         // 0x14d10
	TINT                                     m_iTurnScore;                                          // 0x14d14
	TINT                                     m_iCrateScore;                                         // 0x14d18
	TINT                                     m_iBonusScore;                                         // 0x14d1c, results
	TINT                                     m_iLauncherGunEndBone;                                 // 0x14d20
	TINT                                     m_iLauncherBrowBone;                                   // 0x14d24
	TINT                                     m_iKidPelvisBone;                                      // 0x14d28
	TINT                                     m_iRaccoonHandBone;                                    // 0x14d2c
	TINT                                     m_iRaccoonPelvisBone;                                  // 0x14d30
	TINT                                     m_iCoyotePelvisBone;                                   // 0x14d34
	TINT                                     m_iGopherPelvisBone;                                   // 0x14d38
	TINT                                     m_iThrowTargetBone;                                    // 0x14d3c, m_iLockTargetBone when the throw started
	TINT                                     m_iLockTargetBone;                                     // 0x14d40
	TINT                                     m_iLauncherCartridge1Bone;                             // 0x14d44
	TINT                                     m_iLauncherCartridge2Bone;                             // 0x14d48
	TINT                                     m_iLauncherCrateBone;                                  // 0x14d4c
	TINT                                     m_iLastRaccoonSpot;                                    // 0x14d50
	TINT                                     m_iNextFlarePopup;                                     // 0x14d54
	TINT                                     m_iCowTipSpotBone;                                     // 0x14d58
	TINT                                     m_iCowSpineBone;                                       // 0x14d5c
	TINT                                     m_iCowSpine1Bone;                                      // 0x14d60
	TINT                                     m_iCowHeadBone;                                        // 0x14d64
	TINT                                     m_iSlingshotBone;                                      // 0x14d68
	TBOOL                                    m_bGameOverPending;                                    // 0x14d6c
	TUINT8                                   m_PADDING_14D6D[ 3 ];                                  // 0x14d6d
	TURNEND                                  m_eTurnEnd;                                            // 0x14d70
	TINT                                     m_iGameOverStep;                                       // 0x14d74
	TINT                                     m_iTomatoSplashEffect;                                 // 0x14d78
	TINT                                     m_iDustPuffEffect;                                     // 0x14d7c
	TINT                                     m_iDustPuffSmallEffect;                                // 0x14d80
	TINT                                     m_iCowZzzzEffect;                                      // 0x14d84
	TINT                                     m_iSplatSound;                                         // 0x14d88
	TFLOAT                                   m_fSavedMusicVolume;                                   // 0x14d8c
	TBOOL                                    m_bStoryEnding;                                        // 0x14d90
	TBOOL                                    m_bEndCutscene;                                        // 0x14d91
	TBOOL                                    m_bWasVisible;                                         // 0x14d92
	TUINT8                                   m_PADDING_14D93[ 1 ];                                  // 0x14d93
};

TSTATICASSERT( sizeof( ACowTippingMiniGameState::Stage ) == 0x28 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::CameraSettings ) == 0x68 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::AimAcceleration ) == 0x10 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Level ) == 0x66c );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::PlayerData ) == 0x64 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Projectile ) == 0x54 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Enemy ) == 0x64 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Cow ) == 0xdc );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::LevelCommand ) == 0x8 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::LevelScript ) == 0x194 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Crate ) == 0x50 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Coin ) == 0x28 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::PowerUp ) == 0x8 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::PopUpUnit ) == 0x14 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState::Raccoon ) == 0x28 );
TSTATICASSERT( sizeof( ACowTippingMiniGameState ) == 0x14d94 );
