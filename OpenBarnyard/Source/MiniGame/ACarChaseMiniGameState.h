#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AGUI2FadingText.h"
#include "GUI/AGUI2Timer.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2DirectionArrow.h"
#include "GUI/AGUI2Speedometer.h"
#include "Helpers/AInstanceDebrisManager.h"
#include "GUI/AGUI2TargetIndicator.h"
#include "HUD/ARacePlacingHudElement.h"
#include "Helpers/ASpline.h"

#include <File/TTRB.h>
#include <Render/TViewport.h>
#include <GUI/T2GUIElement.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/TEvent.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class ACameraHelper;
class ACollisionObjectMesh;
class AUnit;
class AUnitCarChaseCar;
class ARacingCarSteer;
class ASteer;
class AKidBikeSteer;

//-----------------------------------------------------------------------------
// Joy Ride: the player drives Beady's car along a night-time race track
// (spline) chased by / racing against cop cars.
// TClass "ACarChaseMiniGameState", vtable 007210f0, size 0x43b0.
// Data: Data/MiniGameData/carchasedata/carchase.trb (Floats, Ints),
//       car_race.trb (track splines), per-track data TRB ("Main").
//
// Units: m_apCars[ 0 ] is the player car, [ 1 ] and [ 2 ] the AI racers
// (cop cars driven by m_aRacers), [ 3 ] the car that follows the track
// spline (m_aSplineFollowers). The kid bikes (car_kid_bike*) only exist in
// the "follow the kids" variants.
//-----------------------------------------------------------------------------
class ACarChaseMiniGameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( ACarChaseMiniGameState, AMiniGameState );

	static constexpr TINT NUM_CARS         = 4;
	static constexpr TINT NUM_RACERS       = 2;
	static constexpr TINT NUM_KID_BIKES    = 3;
	static constexpr TINT MAX_ROAD_BLOCKS  = 25;
	static constexpr TINT NUM_BARRIERS     = 6;
	static constexpr TINT NUM_COINS        = 125;
	static constexpr TINT MAX_RACER_POINTS = 100;

	// Index into m_apCars/m_apSteers.
	enum CAR : TINT
	{
		CAR_PLAYER,
		CAR_RACER1,
		CAR_RACER2,
		CAR_FOLLOWER,
		CAR_NUMOF
	};

	// Track/variant, copied from AMiniGameManager (+0x160) and clamped to 0..6 (7 -> 4).
	using TRACK = TINT;
	enum TRACK_ : TRACK
	{
		TRACK_1           = 0, // track01_spline, hotlap1.trb
		TRACK_2           = 1, // track02_spline, hotlap2.trb, road block at the start
		TRACK_3           = 2, // track03_spline, hotlap3.trb
		TRACK_4           = 3, // track04_spline, hotlap4.trb, road block at the start
		TRACK_FOLLOW_KIDS = 4, // track05_spline, follow the kid bikes without being spotted
		TRACK_STAR        = 5, // track01_spline, reach the spinning star (carstar) chased by the cop cars
		TRACK_COINS       = 6, // collect the coins before the time runs out
	};

	// m_eState, UpdateGame
	using STATE = TINT;
	enum STATE_ : STATE
	{
		STATE_INIT          = 0, // set by SetupLevel
		STATE_START_MESSAGE = 1, // shows a message (locale 665) then goes on
		STATE_START         = 2,
		STATE_RESTART       = 3, // SetupLevel after 3 seconds
		STATE_RACING        = 4,
		STATE_LOST          = 5, // see m_eSubState
		STATE_WON           = 6,
		STATE_OUTRO         = 7, // camera circles around the stopped car
		STATE_GAME_OVER     = 8,
		STATE_RESULTS       = 9, // AAnticGameOverState is shown, SetupLevel once it's gone
	};

	// m_eSubState, why the race has been lost (STATE_LOST)
	using SUBSTATE = TINT;
	enum SUBSTATE_ : SUBSTATE
	{
		SUBSTATE_NONE        = 0,
		SUBSTATE_CAUGHT      = 2, // caught by a cop car with no lives left
		SUBSTATE_OUT_OF_TIME = 3,
		SUBSTATE_SPOTTED     = 4, // too close to the kids
		SUBSTATE_KIDS_LOST   = 6, // too far from the kids
	};

	// Steering command of DriveCar (AUnitCarChaseCar::m_iUnk15C/m_iUnk160)
	using STEERCOMMAND = TINT;
	enum STEERCOMMAND_ : STEERCOMMAND
	{
		STEERCOMMAND_ANALOG = 0, // ARacingCarSteer::STEER_ABSOLUTE (m_iUnk160: no turn button held)
		STEERCOMMAND_LEFT   = 3, // ARacingCarSteer::STEER_LEFT
		STEERCOMMAND_RIGHT  = 4, // ARacingCarSteer::STEER_RIGHT
	};

	// AI racer (cop car) driving along the track spline, ctor 004eca40.
	struct Racer
	{
		Racer();

		// Racer::iMode
		enum MODE_ : TINT
		{
			MODE_IDLE,     // 0, waits (the player hasn't passed it yet)
			MODE_CHASE,    // 1, drives along the track towards the target (UpdateChase)
			MODE_FOLLOW,   // 2, follows the trail of the target (UpdateFollow)
			MODE_CAUGHT,   // 3, caught up with the target and stopped
			MODE_LOST,     // 4, the target got more than 300 away
			MODE_FINISHED, // 5
		};

		void Setup( ASteer* a_pSteer, TFLOAT a_fUnk10, TFLOAT a_fUnk14, TFLOAT a_fUnk18, TFLOAT a_fPointSpacing ); // 004ec4b0
		void SetMode( TINT a_eMode, TINT a_iUnk1c );                                                               // 004ec4e0
		void SetSpline( class ASpline* a_pSpline );                                                                // 004ec970

		// Drives a_pSteer after a_ppTargets[ 0 ] (the player). Returns the mode.
		TINT Update( ARacingCarSteer* a_pSteer, TINT a_iNumPlayers, ARacingCarSteer** a_ppTargets, AUnitCarChaseCar** a_ppCars, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fDeltaTime ); // 004ed150

	private:
		// Remembers the trail of the target (a point every fPointSpacing units, at most MAX_RACER_POINTS)
		void AddPoint( Toshi::TVector4 a_vPoint ); // 004eca90

		void UpdateChase( ARacingCarSteer* a_pSteer, ARacingCarSteer* a_pTarget, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fDeltaTime );  // 004ec510
		void UpdateFollow( ARacingCarSteer* a_pSteer, ARacingCarSteer* a_pTarget, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fDeltaTime ); // 004ecb30

		// NOTE: the points are a Toshi::T2Vector<TVector4, 100> in the original (these are its
		// instantiated methods), kept as a count and an array here.
		void ClearPoints();                // 004ec920
		void RemovePoint( TINT a_iIndex ); // 004ec990
		TINT GetLastPointIndex() const;    // 004eca10, 0 when empty

	public:
		TINT            iUnk00;                      // 0x000, 100
		TINT            iMode;                       // 0x004, see 004ec4e0
		TINT            iUnk08;                      // 0x008
		TFLOAT          fPointSpacing;               // 0x00c, 4.0
		TFLOAT          fUnk10;                      // 0x010, 20.0
		TFLOAT          fUnk14;                      // 0x014, 5.0
		TFLOAT          fUnk18;                      // 0x018, 2.5
		TINT            iUnk1c;                      // 0x01c
		TINT            iUnk20;                      // 0x020
		TFLOAT          fUnk24;                      // 0x024, 5.0
		TINT            iNumPoints;                  // 0x028
		Toshi::TVector4 aPoints[ MAX_RACER_POINTS ]; // 0x02c
		ASteer*         pSteer;                      // 0x66c
		class ASpline*  pSpline;                     // 0x670, the track
		TFLOAT          fUnk674;                     // 0x674, 10.0 when MODE_CHASE starts
		TFLOAT          fFarTime;                    // 0x678, time spent further than 50 from the target (MODE_FOLLOW)
	};

	// Follows the track spline (m_apCars[ CAR_FOLLOWER ]), ctor 004f58f0, init 004f5cb0.
	struct SplineFollower
	{
		SplineFollower();

		// Steers along the spline ahead of the car. Always returns TTRUE.
		TBOOL Update( ARacingCarSteer* a_pPlayerSteer, TINT* a_pOutPedal, TINT* a_pOutSteer, TFLOAT* a_pOutSteering, TFLOAT a_fPlayerDistance, TFLOAT a_fDeltaTime ); // 004f5940

		// Starts following a_pSpline from its beginning
		void Init( class ASpline* a_pSpline ); // 004f5cb0

		// NOTE: 0066b140 in the original, a setter of +0x28 shared by many classes (AProjectileLauncher::SetAmmo)
		void SetUnk28( TINT a_iValue ) { iUnk28 = a_iValue; }

		class ASpline*   pSpline;   // 0x00
		ARacingCarSteer* pSteer;    // 0x04
		Toshi::TVector4  vUnk08;    // 0x08
		TFLOAT           fUnk18;    // 0x18, 4.0
		TINT             iUnk1c;    // 0x1c
		TFLOAT           fProgress; // 0x20, progress of the closest point of the spline
		TINT             iUnk24;    // 0x24
		TINT             iUnk28;    // 0x28, 3
		TINT             iUnk2c;    // 0x2c
		TINT             iUnk30;    // 0x30
	};

	// car_race.trb and the track spline read from it (destructor 004ee250).
	struct RaceTrack
	{
		~RaceTrack();

		Toshi::TTRB oTRB;    // 0x000
		ASpline     oSpline; // 0x140
	};

	struct Coin
	{
		AAnimatableObject* pObject  = TNULL; // 0x0
		TINT               iMapIcon = -1;    // 0x4, icon on the minimap
		TBOOL              bCollected;       // 0x8
	};

public:
	ACarChaseMiniGameState();
	~ACarChaseMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void  OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x18
	virtual void  OnRenderOverlay( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                         // +0x20
	virtual void  OnPause() OVERRIDE;                                                                                                                      // +0x3c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void Pause() OVERRIDE;                               // +0x68
	virtual void PlayLoseSequence() OVERRIDE;                    // +0x6c
	virtual void PlayWinSequence() OVERRIDE;                     // +0x70
	virtual void OnPreStart() OVERRIDE;                          // +0x74
	virtual void UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE;   // +0x84, music
	virtual void UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;     // +0x88, 004f3270 (_Game.cpp)
	virtual void UpdatePreStart( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x90, intro camera
	virtual S1*  GetResults() OVERRIDE;                          // +0x94
	virtual void OnStartHudFinished() OVERRIDE;                  // +0x9c

	//-----------------------------------------------------------------------------
	// Own methods (ACarChaseMiniGameState.cpp)
	//-----------------------------------------------------------------------------

	// Reads Floats/Ints from carchase.trb.
	void LoadData(); // 004ee080

	// Shows or hides the race HUD.
	void SetHUDVisible( TBOOL a_bVisible ); // 004ee140

	// Pushes AAnticGameOverState.
	TBOOL ShowGameOver( TINT a_iUnused, const TINT* a_pValue ); // 004efdd0

	// Creates the coins (coin_NN locators).
	void CreateCoins(); // 004f11d0

	// Loads car_race.trb, the track data TRB and places the road blocks/barriers.
	void LoadTrack(); // 004f17b0

	// Places the cars at the start of the track and resets the race.
	void SetupLevel(); // 004f2060

	// Starts the kid bike animations.
	void StartKidBikes(); // 004ed300

	// Attaches the chase camera to the player car (inlined in the original)
	void SetupCamera();

	//-----------------------------------------------------------------------------
	// Gameplay (ACarChaseMiniGameState_Game.cpp / _AI.cpp)
	//-----------------------------------------------------------------------------

	// Racers: speed limits of the track, catching the player, putting them back on the track
	void UpdateRacers( TFLOAT a_fDeltaTime ); // 004f0320

	// The car following the track spline
	void UpdateSplineFollowers( TFLOAT a_fDeltaTime ); // 004ed3c0

	// Finish line and time limit
	void UpdateRace( TFLOAT a_fDeltaTime ); // 004ed880

	// Follow the kids: stay close to the kid bikes until they reach the end of their spline
	void UpdateFollowKids( TFLOAT a_fDeltaTime ); // 004ef250

	// Minimaps, timer, speedometer, direction arrow and race position
	void UpdateHUD( TFLOAT a_fDeltaTime ); // 004edbd0

	// Puts the racer behind (1) / ahead of (2) the player on the track and starts it
	void RespawnRacer1(); // 004ef790
	void RespawnRacer2(); // 004efad0

	// Collects the coins close to the player and spins them
	void UpdateCoins( TFLOAT a_fDeltaTime ); // 004f14c0

	// Applies the inputs to the car of a_pSteer (see ARacingCarSteer::SetPedal/SetSteering)
	static void DriveCar( ARacingCarSteer* a_pSteer, TINT a_ePedalCommand, STEERCOMMAND a_eSteerCommand, TFLOAT a_fSteering, TFLOAT a_fUnk, TINT a_iUnk ); // 004ed230

	// Speed limit of the track at a_fProgress, read from the track data (inlined in the original).
	// Returns TFALSE when there's none.
	TBOOL GetTrackSpeedLimit( TFLOAT a_fProgress, TFLOAT& a_rfOutSpeed ) const;

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay: drives the player car along the track
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

protected:
	TFLOAT                  m_afData[ 8 ];                              // 0xbac, carchase.trb "Floats"
	TINT                    m_iData;                                    // 0xbcc, carchase.trb "Ints"
	ACameraHelper*          m_apCameraHelpers[ MAX_PLAYERS ];           // 0xbd0
	TUINT8                  PADDING1[ 0x20 ];                           // 0xbe0
	Toshi::TVector4         m_vParkingPos;                              // 0xc00, far away from the track: unused road blocks and the cars of the follow the kids variant
	STATE                   m_eState;                                   // 0xc10
	TINT                    m_iUnkc14;                                  // 0xc14
	TINT                    m_iUnkc18;                                  // 0xc18
	SUBSTATE                m_eSubState;                                // 0xc1c
	TINT                    m_iFinishPlace;                             // 0xc20
	TINT                    m_iNumPlayersFinished;                      // 0xc24
	TINT                    m_iNumSplineFollowers;                      // 0xc28, 0 or 1 (depends on the track)
	TINT                    m_iNumPlayers;                              // 0xc2c
	TINT                    m_iUnkc30;                                  // 0xc30
	TBOOL                   m_bUnkc34;                                  // 0xc34
	TBOOL                   m_bInputEnabled;                            // 0xc35, ProcessCommand drives the player car while set
	TFLOAT                  m_fSavedFarClip;                            // 0xc38, restored on removal
	TFLOAT                  m_fSavedNearClip;                           // 0xc3c, restored on removal
	TFLOAT                  m_afSavedVisibilityDistances[ 3 ];          // 0xc40, AInstanceManager
	TFLOAT                  m_afSavedTreeDistances[ 3 ];                // 0xc4c, ATreeManager
	TFLOAT                  m_fUnkc58;                                  // 0xc58, 6.0
	TFLOAT                  m_fUnkc5c;                                  // 0xc5c, 35.0
	TFLOAT                  m_fUnkc60;                                  // 0xc60, 6.0
	TFLOAT                  m_fUnkc64;                                  // 0xc64, 3.0
	TFLOAT                  m_fUnkc68;                                  // 0xc68, 20.0
	TFLOAT                  m_fTime;                                    // 0xc6c
	TFLOAT                  m_afFinishTimes[ 3 ];                       // 0xc70, race time when the car last went through UpdateRace: players [ i ], followers [ 1 + i ]
	TINT                    m_iNumCoinsCollected;                       // 0xc7c
	TRACK                   m_eTrack;                                   // 0xc80
	Toshi::TVector4         m_vFinishPosition;                          // 0xc84
	TINT                    m_iUnkc94;                                  // 0xc94
	TINT                    m_iUnkc98;                                  // 0xc98
	TFLOAT                  m_fTimeLimit;                               // 0xc9c
	AAnimatableObject*      m_apRoadBlocks[ MAX_ROAD_BLOCKS ];          // 0xca0
	ACollisionObjectMesh*   m_apRoadBlockCollisions[ MAX_ROAD_BLOCKS ]; // 0xd04
	AAnimatableObject*      m_apBarriers[ NUM_BARRIERS ];               // 0xd68
	ACollisionObjectMesh*   m_apBarrierCollisions[ NUM_BARRIERS ];      // 0xd80
	TINT                    m_iNumRoadBlocks;                           // 0xd98
	TFLOAT                  m_fUnkd9c;                                  // 0xd9c, 10.0
	TINT                    m_aiMapRect[ 4 ];                           // 0xda0, x, y, width, height of the minimap (one per player)
	AGUI2MiniMap            m_aMiniMaps[ 1 ];                           // 0xdb0
	AGUI2DirectionArrow     m_oDirectionArrow;                          // 0x12d0
	AGUI2Speedometer        m_oSpeedometer;                             // 0x13b4
	TUINT8                  PADDING3[ 4 ];                              // 0x17e0
	AGUI2FadingText         m_oFadingText;                              // 0x17e4
	AGUI2Timer              m_oTimer;                                   // 0x1958
	AGUI2TargetIndicator    m_oTargetIndicator;                         // 0x1b78
	AUnitCarChaseCar*       m_apCars[ CAR_NUMOF ];                      // 0x1dc8
	ARacingCarSteer*        m_apSteers[ CAR_NUMOF ];                    // 0x1dd8
	AUnit*                  m_apKidBikes[ NUM_KID_BIKES ];              // 0x1de8
	AKidBikeSteer*          m_apKidBikeSteers[ NUM_KID_BIKES ];         // 0x1df4
	Racer                   m_aRacers[ NUM_RACERS ];                    // 0x1e00
	SplineFollower          m_aSplineFollowers[ 1 ];                    // 0x2af8
	RaceTrack               m_oRaceTrack;                               // 0x2b2c
	TWCHAR                  m_wszText[ 50 ];                            // 0x2c7c
	AInstanceDebrisManager  m_oDebrisManager;                           // 0x2ce0, not used by the state
	Toshi::TViewport        m_oViewport;                                // 0x3588
	TBOOL                   m_bNoTimeLimit;                             // 0x35a0
	ARacePlacingHudElement  m_oRacePlacingHud;                          // 0x35a4
	Toshi::T2GUIElement     m_oIconElement;                             // 0x3724
	Toshi::T2GUIRectangle   m_oIconRectangle;                           // 0x3778, "Chase_Icon_Light"
	TFLOAT                  m_fRespawnTimer;                            // 0x37f0
	Toshi::TGenericListener m_oListener;                                // 0x37f4
	AAnimatableObject*      m_pCarStar;                                 // 0x3808
	TINT                    m_iUnk380c;                                 // 0x380c
	TBOOL                   m_bStartEngines;                            // 0x3810
	TINT                    m_iNumFinishedCars;                         // 0x3814
	TBOOL                   m_bUnk3818;                                 // 0x3818
	TINT                    m_iLastGear;                                // 0x381c
	TFLOAT                  m_fGearShiftCameraTime;                     // 0x3820
	TINT                    m_iCue3824;                                 // 0x3824
	TINT                    m_iMilkSlickEffect;                         // 0x3828, "carmilkslick"
	TFLOAT                  m_fDistanceWarningTime;                     // 0x382c
	Coin                    m_aCoins[ NUM_COINS ];                      // 0x3830
	TFLOAT                  m_fKidDistance;                             // 0x3e0c
	TUINT8                  PADDING2[ 0x80 ];                           // 0x3e10
	Toshi::TTRB             m_oTrackDataTRB;                            // 0x3e90
	void*                   m_pTrackData;                               // 0x3fd0, "Main" symbol of m_oTrackDataTRB
	AGUI2LifeHud            m_oLifeHud;                                 // 0x3fd4
	TINT                    m_iLives;                                   // 0x437c
	TBOOL                   m_bTrackLoaded;                             // 0x4380
	TINT                    m_iCue4384;                                 // 0x4384
	TFLOAT                  m_fDriverVoiceTimer;                        // 0x4388
	TINT                    m_iCue438c;                                 // 0x438c
	TFLOAT                  m_fBikeVoiceTimer;                          // 0x4390
	TINT                    m_eVariant;                                 // 0x4394, AMiniGameManager variant (0..7)
	AAnimatableObject*      m_pFinishLine;                              // 0x4398
	ACollisionObjectMesh*   m_pFinishLineCollision;                     // 0x439c
	TFLOAT                  m_fOutroTime;                               // 0x43a0
	TFLOAT                  m_fSpeedScale;                              // 0x43a4, 1.0
	TBOOL                   m_bRaceRunning;                             // 0x43a8
	TINT                    m_iUnk43ac;                                 // 0x43ac, not initialised by the ctor
};

TSTATICASSERT( sizeof( ACarChaseMiniGameState::Racer ) == 0x67c );
TSTATICASSERT( sizeof( ACarChaseMiniGameState::SplineFollower ) == 0x34 );
TSTATICASSERT( sizeof( ACarChaseMiniGameState::RaceTrack ) == 0x150 );
TSTATICASSERT( sizeof( ACarChaseMiniGameState::Coin ) == 0xc );
TSTATICASSERT( sizeof( ACarChaseMiniGameState ) == 0x43b0 );
