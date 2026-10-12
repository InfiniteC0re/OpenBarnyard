#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2MilkAmmoBar.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2FadingText.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AGUI2TargetIndicator.h"
#include "GUI/AGUI2TargetCounter.h"
#include "GUI/AGUI2FloatHudElement.h"
#include "GUI/AMilkReticleManager.h"
#include "Helpers/AMilkBottlePickups.h"
#include "Helpers/AWindmillHelper.h"
#include "Helpers/ASpline.h"
#include "Units/APlayerControlComponent.h"
#include "Units/APrecisionJersyCow.h"
#include "Units/ASpawnPoint.h"

#include <File/TTRB.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/T2DList.h>
#include <Math/TVector4.h>

class AUnitPlayer;
class AAnimatableObject;
class AInteriorNetwork;
class AInteriorSteer;
class ASimplePlayerControlComponent;
class APrecisionCoyote;
class APrecisionRacoon;
class APrecisionScarecrow;
class APrecisionTargetItem;

//-----------------------------------------------------------------------------
// Data/MinigameData/SharpSquirter/Data.trb
//-----------------------------------------------------------------------------

// "GameData": one per variant ("NumGameVariants"), 28 bytes
struct APrecisionSquirtGameData
{
	struct Round
	{
		TFLOAT fTimeLimit;  // 0x0
		TINT   iNumTargets; // 0x4, tin cans placed at the "target" goals
	};

	Round aRounds[ 3 ]; // 0x00
	TINT  iUnk18;       // 0x18
};

TSTATICASSERT( sizeof( APrecisionSquirtGameData ) == 0x1c );

//-----------------------------------------------------------------------------
// Sharp Squirter (hidden minigame "APrecisionSquirt", "Sharp_Squirt"), played
// in the Miles quests "Target Squirting" / "Sharp Squirter" and replayed from
// AReplayManager (barnyard_squirt_replay, benshill_squirt_replay...).
// TClass 00709270 (parent AMiniGameState, size 0x9678), vtables 00721da8,
// 00722a44 (AInventoryAniEffect::CallBackInterface), 00721d9c (AUnitActionReceiver).
//
// The player stands at the "spawn" goal of the variant's network
// ("precision1".."precision10") and squirts milk (projectile type 7,
// AMilkPrecisionProjectile) at the tin cans (APrecisionTargetItem) placed at the
// "target" goals before the time runs out, over up to three rounds. Hitting cans
// with the same squirt builds up a combo (2 points per can and combo level).
// The coyotes, raccoons, scarecrows and Jersey cows of the network are there to
// be squirted too. The charity variant (3) has barnbuck coins along a spline.
//
// Variants: 0 barnyard, 1 Ben's hill, 2 Beady's farm, 3 charity, 4 golf course,
// 5 junkyard (night), 6..9 the woods (AMiniGame field 0x18 2 or 3 adds 6 or 7).
//-----------------------------------------------------------------------------
class APrecisionSquirt
    : public AMiniGameState
    , public AUnitActionReceiver
{
public:
	TDECLARE_CLASS( APrecisionSquirt, AMiniGameState );

	static constexpr TINT MAX_SPAWN_POINTS = 10;
	static constexpr TINT MAX_COYOTES      = 25;
	static constexpr TINT MAX_RACOONS      = 30;
	static constexpr TINT MAX_SCARECROWS   = 11;
	static constexpr TINT MAX_COWS         = 4;
	static constexpr TINT MAX_TARGETS      = 50;
	static constexpr TINT NUM_FLOAT_TEXTS  = 5;
	static constexpr TINT NUM_COINS        = 240;

	enum PHASE_ : TINT
	{
		PHASE_PLAYING,   // 0
		PHASE_UNK1,      // 1
		PHASE_UNK2,      // 2
		PHASE_ROUND_WON, // 3, the player cheers, the next round starts 5 seconds later
		PHASE_GAME_OVER, // 4
		PHASE_PRESTART,  // 5
	};

	// Result of a round (GetRoundResult)
	enum RESULT_ : TINT
	{
		RESULT_WON,      // 0, all the targets were hit in time
		RESULT_TIME_UP,  // 1
		RESULT_NO_LIVES, // 2
		RESULT_PLAYING,  // 3
	};

	// A barnbuck coin of the charity variant. 36 bytes.
	struct Coin
	    : public Toshi::T2DList<Coin>::Node
	{
		Coin( AAnimatableObject* a_pObject, const Toshi::TVector4& a_rcPosition );

		void Update( TFLOAT a_fDeltaTime );
		void SetActive( TBOOL a_bActive );
		void SetSkeletonUpdating( TBOOL a_bUpdating );

		AAnimatableObject* pObject;   // 0x08
		TFLOAT             fTime;     // 0x0c
		Toshi::TVector4    vPosition; // 0x10
		TBOOL              bFading;   // 0x20
		TBOOL              bActive;   // 0x21
	};

	// A pumpkin on the head of a scarecrow, 12 bytes
	struct Pumpkin
	{
		AUnit* pUnit;    // 0x0
		TINT   iMapIcon; // 0x4
		TBOOL  bUnk8;    // 0x8
	};

	struct PumpkinList
	{
		void Add( const Pumpkin& a_rcPumpkin );

		TINT    iCount;                      // 0x0
		Pumpkin aPumpkins[ MAX_SCARECROWS ]; // 0x4
	};

public:
	APrecisionSquirt();
	~APrecisionSquirt();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void  OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x18
	virtual void  OnPause() OVERRIDE;                                                                                                                      // +0x3c
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void  Pause() OVERRIDE;                               // +0x68
	virtual void  PlayLoseSequence() OVERRIDE {}                  // +0x6c, DOES_NOTHING
	virtual void  PlayWinSequence() OVERRIDE {}                   // +0x70, DOES_NOTHING
	virtual void  OnPreStart() OVERRIDE;                          // +0x74
	virtual TBOOL IsGameOver() OVERRIDE;                          // +0x80
	virtual void  UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE {} // +0x84, DOES_NOTHING
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;     // +0x88
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x8c
	virtual S1*   GetResults() OVERRIDE;                          // +0x94
	virtual void  OnStartHudFinished() OVERRIDE;                  // +0x9c

	//-----------------------------------------------------------------------------
	// AUnitActionReceiver
	//-----------------------------------------------------------------------------
	virtual void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// The section the player starts in, used as the current VIS group while loading
	static TINT GetTerrainSection();

	RESULT_ GetRoundResult();

	// TTRUE if the player holds a pumpkin
	TBOOL IsHoldingPumpkin();

	// Counts the targets hit by the milk and gives the points
	void UpdateTargets( TFLOAT a_fDeltaTime );

	// The player cheers or sulks once the game is over
	void UpdateResultAnimation();

	// Creates what's at the goal a_pGoal (player spawn, respawns, targets, enemies, cows, scarecrows...)
	void CreateFromGoal( AInteriorGoal* a_pGoal );

	// Creates the 50 targets
	void CreateTargets();

	// Places a_iNumTargets targets at the target positions of the round
	void PlaceTargets( TINT a_iNumTargets );

	// Sets up the HUD and the map for the round
	void SetupRoundHud();

	// Collects the coins the player runs into
	void UpdateCoins( TFLOAT a_fDeltaTime );

	// Starts the first round (a_bRestart) or the next one
	void StartRound( TBOOL a_bRestart );

	// Moves the icons of the map
	void UpdateMap( TFLOAT a_fDeltaTime );

	// Creates the coins of the charity variant along the spline
	void CreateCoins();

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" squirts at the targets
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	TINT                                m_iNumSpawnPoints;                        // 0x0bc0
	ASpawnPoint                         m_aSpawnPoints[ MAX_SPAWN_POINTS ];       // 0x0bc4
	Toshi::T2DList<Coin>                m_llCoins;                                // 0x0df4
	Toshi::T2DList<Coin>                m_llCollectedCoins;                       // 0x0dfc
	TINT                                m_iNumCoyotes;                            // 0x0e04
	APrecisionCoyote*                   m_apCoyotes[ MAX_COYOTES ];               // 0x0e08
	TINT                                m_iNumRacoons;                            // 0x0e6c
	APrecisionRacoon*                   m_apRacoons[ MAX_RACOONS ];               // 0x0e70
	TINT                                m_iNumScarecrows;                         // 0x0ee8
	APrecisionScarecrow*                m_apScarecrows[ MAX_SCARECROWS ];         // 0x0eec, standing, with a pumpkin
	TINT                                m_iNumKnockedOver;                        // 0x0f18
	APrecisionScarecrow*                m_apKnockedOver[ MAX_SCARECROWS ];        // 0x0f1c, swapped in when the standing one is hit
	PumpkinList                         m_oPumpkins;                              // 0x0f48
	TINT                                m_iNumCows;                               // 0x0fd0
	APrecisionJersyCow*                 m_apCows[ MAX_COWS ];                     // 0x0fd4
	TINT                                m_iNumTargets;                            // 0x0fe4
	APrecisionTargetItem*               m_apTargets[ MAX_TARGETS ];               // 0x0fe8
	APrecisionPositionList<MAX_TARGETS> m_oTargetPositions;                       // 0x10b0, "target"
	APrecisionPositionList<MAX_TARGETS> m_oTargetPositions2;                      // 0x13d4, "target2"
	APrecisionPositionList<MAX_TARGETS> m_oTargetPositions3;                      // 0x16f8, "target3"
	AUnit*                              m_pNearestPumpkin;                        // 0x1a1c, the pumpkin the player can pick up
	AWindmillHelper                     m_oWindmills;                             // 0x1a20
	AMilkBottlePickups                  m_oMilkBottles;                           // 0x1ab8
	Toshi::T2GUIRectangle               m_oAmmoBacking;                           // 0x1b64
	AGUI2MilkAmmoBar                    m_oAmmoBar;                               // 0x1bdc
	AGUI2TargetCounter                  m_oTargetCounter;                         // 0x1e94
	AGUI2Portrait                       m_oPortrait;                              // 0x209c
	AGUI2LifeHud                        m_oLifeHud;                               // 0x2664
	AGUI2FadingText                     m_oRoundText;                             // 0x2a0c
	AGUI2MiniMap                        m_oMap;                                   // 0x2b80
	AGUI2TargetIndicator                m_oTargetIndicator;                       // 0x30a0
	AGUI2FloatHudElement                m_oFloats;                                // 0x32f0
	AMilkReticleManager                 m_oReticles;                              // 0x34e4
	AGUI2MiniMap::Viewport              m_oMapViewport;                           // 0x9008
	AInteriorSteer*                     m_apCoyoteSteers[ MAX_COYOTES ];          // 0x9018
	AInteriorSteer*                     m_apRacoonSteers[ MAX_RACOONS ];          // 0x907c
	AInteriorSteer*                     m_apCowSteers[ MAX_COWS ];                // 0x90f4
	AInteriorNetwork*                   m_pNetwork;                               // 0x9104
	AUnitPlayer*                        m_pPlayer;                                // 0x9108
	AAnimatableObject*                  m_pSunglasses;                            // 0x910c
	ASimplePlayerControlComponent*      m_pControl;                               // 0x9110
	Toshi::TPString8                    m_strNetwork;                             // 0x9114
	Toshi::TVector4                     m_vPyramid;                               // 0x9118, "pyramid" goal, one unit up
	TFLOAT                              m_fRoundEndTimer;                         // 0x9128
	TINT                                m_iVariant;                               // 0x912c
	TINT                                m_iNumRounds;                             // 0x9130
	TINT                                m_iNumPreviousTargets;                    // 0x9134, targets of the previous rounds
	TINT                                m_iRound;                                 // 0x9138, 1..m_iNumRounds
	TINT                                m_iLives;                                 // 0x913c
	TINT                                m_iNumScarecrowsAtStart;                  // 0x9140
	TINT                                m_iNumTargetsLeft;                        // 0x9144
	TINT                                m_iNumCoinsLeft;                          // 0x9148
	TINT                                m_iNumCoins;                              // 0x914c
	TINT                                m_iNumCoinsCollected;                     // 0x9150
	TINT                                m_iUnk9154;                               // 0x9154
	TINT                                m_iNumRacoonsAtStart;                     // 0x9158
	PHASE_                              m_ePhase;                                 // 0x915c
	TINT                                m_iUnk9160;                               // 0x9160
	RESULT_                             m_eResult;                                // 0x9164
	TINT                                m_iCombo;                                 // 0x9168
	TINT                                m_iLastStreamId;                          // 0x916c
	TINT                                m_iScore;                                 // 0x9170
	TINT                                m_iTimeBonus;                             // 0x9174
	TFLOAT                              m_fComboTimer;                            // 0x9178
	Toshi::TVector4                     m_vSpawnPosition;                         // 0x917c
	TFLOAT                              m_fOldNear;                               // 0x918c
	Toshi::TTRB                         m_oData;                                  // 0x9190
	APrecisionSquirtGameData*           m_pGameData;                              // 0x92d0
	Toshi::TTRB                         m_oCoinSplineTRB;                         // 0x92d4
	ASpline                             m_oCoinSpline;                            // 0x9414
	TWCHAR                              m_awszFloatTexts[ NUM_FLOAT_TEXTS ][ 8 ]; // 0x9424, "%d" points over the targets
	TBOOL                               m_bGameOverPushed;                        // 0x9474
	TWCHAR                              m_wszRoundText[ 255 ];                    // 0x9476
};
