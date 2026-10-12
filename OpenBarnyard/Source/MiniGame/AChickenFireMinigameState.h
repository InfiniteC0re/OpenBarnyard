#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AGUI2Button.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2Feathers.h"
#include "HUD/ACFPBashHudElement.h"
#include "Helpers/AWindmillHelper.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TEvent.h>
#include <Toshi/T2Vector.h>
#include <Toshi/T2SortedVector.h>
#include <Toshi/TRefCounted.h>
#include <Math/TRandom.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class AAnimationBreakpointCallback;
class AAnimatedUnit;
class ACamera;
struct ACollisionModelRayHitList;
class AModelInstance;
class AParticleEffectHandle;

//-----------------------------------------------------------------------------
// Chicken Launch (minigame "AChickenFireMinigameState", "Chicken_Launch", 1-4 players).
// TClass 00782828, vtable 0071e000
// (AInventoryAniEffect::CallBackInterface 00722a44), size 0x3198.
//
// The player (in the American football outfit, "cfp_americanm/f") pulls Peck
// back on a giant slingshot at the Peck range; the bash bar (ACFPBashHudElement)
// sets the power and the left stick aims. Peck ("peckFireProgram" unit) flies
// towards the targets, can be boosted in the air and lands in the pond or the
// field. Freddy ("cfp_freddy") watches. The data come from
// Data/MinigameData/ChickenLaunch/cfp_data.trb. Up to 4 players take turns.
//
// NOTE: OnUpdate (00480360) replaces AMiniGameState::OnUpdate and runs its own
// state machine (m_eState).
//-----------------------------------------------------------------------------
class AChickenFireMinigameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( AChickenFireMinigameState, AMiniGameState );

	static constexpr TINT NUM_BUTTONS = 4;

	// One entry per player (stride 0x14 at 0xec0)
	struct PlayerInput
	{
		TFLOAT fAimX;  // 0x00, ProcessCommand: mouse/stick/dpad X
		TFLOAT fAimY;  // 0x04, ProcessCommand: mouse/stick/dpad Y
		TINT   iUnk08; // 0x08
		TINT   iUnk0C; // 0x0c
		TBOOL  bBash;  // 0x10, input command 8
		TBOOL  bCmd57; // 0x11, input command 57
		TBOOL  bUnk12; // 0x12
		TBOOL  bUnk13; // 0x13
	};

	// Sort order of the ring and barnbuck vectors (CompareObjectDepth)
	struct DepthSortResults
	{
		TINT operator()( AAnimatableObject* const& a_rcVal1, AAnimatableObject* const& a_rcVal2 ) const { return CompareObjectDepth( a_rcVal1, a_rcVal2 ); }
	};

	// NOTE: the retail instantiation's T2Vector::InsertBefore is 0047b250 (shared, marked in AGUI2FloatHudElement.cpp)
	template <TINT MaxSize>
	using DepthSortedVector = Toshi::T2SortedVector<AAnimatableObject*, Toshi::T2Vector<AAnimatableObject*, MaxSize>, DepthSortResults>;

public:
	AChickenFireMinigameState();
	~AChickenFireMinigameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void  OnStart() OVERRIDE;                                                                                                                      // +0x38
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void  OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void  OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void  OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void  OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual S1*  GetResults() OVERRIDE;         // +0x94
	virtual void OnStartHudFinished() OVERRIDE; // +0x9c

private:
	// Reads cfp_data.trb (BashParams, TimeOuts, Misc)
	TBOOL LoadData( const TCHAR* a_szFileName );

	// Destroys the objects of m_vecObjects1, m_vecObjects2 and m_vecObjects4
	void DestroyObjects();
	// Destroys the objects of m_vecObjects3
	void DestroyObjects3();

	// Moves Peck to the "peckattachpoint" bone of the player's model
	void AttachPeckToPlayer();

	// Release breakpoint of the player's animation (sets m_bReleased)
	static TBOOL __stdcall OnReleaseBreakpoint( AChickenFireMinigameState* a_pThis, void* a_pOwner, void* a_pData );

	// Maps the "success"..."success_10" names to 5...14
	static TINT __stdcall GetSuccessIndex( const Toshi::TString8& a_rcName );

	// Copies the locator position into a_rOutPos
	static void __stdcall GetLocatorPos( Toshi::TVector4& a_rOutPos, const TCHAR* a_szLocator );

	// Shows or hides the text box with a_wszText
	void SetText( const TWCHAR* a_wszText );

	// Reflects a_rcVec about the normal a_rcNormal into this
	static void Reflect( Toshi::TVector4& a_rOut, const Toshi::TVector4& a_rcVec, const Toshi::TVector4& a_rcNormal );

	// Adds a random offset in the XZ plane of length a_fRadius to a_rVec
	void AddRandomOffset( Toshi::TVector4& a_rVec, TFLOAT a_fRadius );

	void EndTurn();

	// Sorts by the z of the model positions: -1 if a_pA is further along z
	static TINT __stdcall CompareObjectDepth( AAnimatableObject* a_pA, AAnimatableObject* a_pB );

	// Places Peck's model at a_vPosition facing a_vDirection
	void SetPeckTransform( Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vDirection );

	//-----------------------------------------------------------------------------
	// Gameplay (AChickenFireMinigameState_Game.cpp)
	//-----------------------------------------------------------------------------
	// Peck's animations played by PlayPeckAnim
	enum PECKANIM : TINT
	{
		PECKANIM_FLIGHTLOOP,
		PECKANIM_LAUNCHPULLBACK,
		PECKANIM_LAUNCH,
		PECKANIM_FLIGHTTUMBLE,
		PECKANIM_FLIGHTFLAP,
		PECKANIM_FLIGHTFLAP2,
		PECKANIM_FLIGHTFLAPFAST,
		PECKANIM_FLIGHTDIVE,
		PECKANIM_PRELAUNCH,
		PECKANIM_IMPACT,
		PECKANIM_PRELAUNCHHOLD,
		PECKANIM_POSTIMPACTSUCCESS,
		PECKANIM_POSTIMPACTFAIL,
		PECKANIM_SLIDE,
		PECKANIM_IMPACTTOTUMBLE,
		PECKANIM_TUMBLE_END,
		PECKANIM_TUMBLE_ENDHIT,
		PECKANIM_IMPACTWOBBLE,
		PECKANIM_NUMOF
	};

	// Results of CheckPeckCollision
	enum HIT : TINT
	{
		HIT_NONE         = 0,
		HIT_BOARD        = 1, // "board" collision group of the landing area
		HIT_WATER        = 2, // "water", the pond
		HIT_POWER        = 3, // "col_power" or a power pole object, Peck tumbles down
		HIT_TOWER        = 4, // "col_tower"
		HIT_SUCCESS      = 5, // "success" ... "success_10" target rings = 5..14
		HIT_SUCCESS_LAST = 14,
		HIT_BARN         = 15, // "collision_barn"
		HIT_BOUNCE       = 16, // any other terrain/object: Peck crashes, bounces or slides
	};

	// Places Peck's model at a_vPosition facing a_vDirection, rolled by m_fBankAngle (scaled by the speed), and moves the steer there
	void SetPeckFlightTransform( Toshi::TVector4 a_vPosition, Toshi::TVector4 a_vDirection );
	// Plays a_eAnim (PECKANIM) on Peck unless it's already playing
	void PlayPeckAnim( TINT a_eAnim );
	// Flight camera: follows Peck from behind, FOV widens while boosting
	void UpdateFlightCamera( TFLOAT a_fDeltaTime );
	// Flight checks: out of bounds (|x| > 43), wrong way, stalling. Returns TTRUE if m_eState changed
	TBOOL CheckFlight( TFLOAT a_fDeltaTime );
	// Stops (or fades out) the ten m_iCue_XXX cues and the forced music
	void StopSounds( TBOOL a_bFadeOut );
	// Collision of Peck with the terrain and objects along the flight step (HIT). a_fDeltaTime is unused
	TINT CheckPeckCollision( TFLOAT a_fDeltaTime, TFLOAT* a_pOutDistance );
	// Steers Peck with the stick, updates the speed, the position and the flight animations
	void UpdateFlight( TFLOAT a_fDeltaTime );
	// Creates the ten hidden "ch_scorecard_XX" objects (m_vecObjects4)
	void CreateScorecards();
	// Hides the scorecards and shows a_iIndex (-1 = none) in Freddy's hand. a_vUnused is unused
	void ShowScorecard( TINT a_iIndex, Toshi::TVector4 a_vUnused );
	// Rings (m_vecObjects1/2): passed rings fade, flying through one gives a boost and 5 points
	void UpdateRings( TFLOAT a_fDeltaTime );
	// Barnbucks (m_vecObjects3, charity variant): collects the ones Peck flies through
	void UpdateBarnbucks( TFLOAT a_fDeltaTime );
	// Peck landed on the board (HIT_BOARD) a_fDistance along the flight step: score by the distance to the target
	void OnLandedOnBoard( TFLOAT a_fDistance );
	// Same as OnLandedOnBoard for HIT_BARN, also sets m_bWon
	void OnLandedOnBarn( TFLOAT a_fDistance );
	// Peck landed in a target ring (a_iHit = HIT_SUCCESS..HIT_SUCCESS_LAST)
	void OnLandedInRing( TINT a_iHit );
	// Creates the rings at the "ring_<variant>" locators
	void CreateRings();
	// Creates the barnbucks at the "coin_" locators (charity variant)
	void CreateBarnbucks();
	// Resets everything for the next turn; a_bNextPlayer adds the score of the turn and moves to the next player
	void ResetTurn( TBOOL a_bNextPlayer );


	//-----------------------------------------------------------------------------
	// Development (AChickenFireMinigameState.cpp)
	//-----------------------------------------------------------------------------
	// -devautoplay: bashes and launches for the current player
	void DevAutoPlay( TFLOAT a_fDeltaTime );

	PlayerInput& GetCurrentInput();


	//-----------------------------------------------------------------------------
	// Update (AChickenFireMinigameState_Update.cpp)
	//-----------------------------------------------------------------------------
public:
	// Values of m_eState (OnUpdate)
	enum STATE : TINT
	{
		STATE_WAIT_FOR_BASH = 0,  // Peck sits in the slingshot until the first bash
		STATE_BASHING       = 1,  // bash to pull back, the countdown runs
		STATE_OVERPOWER     = 3,  // starts the "cfp_overpwr_failcase" sequence
		STATE_UNDERPOWER    = 4,  // starts the "cfp_underpwr_failcase" sequence
		STATE_RELEASE       = 5,  // the player lets go
		STATE_RELEASING     = 6,  // waits for the "ReleaseBP" breakpoint
		STATE_UNK_7         = 7,  // not set by OnUpdate, goes to STATE_TUMBLE after 0.5s
		STATE_TUMBLE        = 8,  // Peck tumbles out of the slingshot, the camera follows
		STATE_FLYING        = 9,  // flight, bash to boost
		STATE_CRASH         = 10, // Peck hits the ground
		STATE_CRASHED       = 11, // camera shake, Peck slides
		STATE_END_CAMERA    = 13, // Freddy reacts, the score of the landing is shown
		STATE_END           = 14, // shows AAnticGameOverState
		STATE_SEQUENCE      = 15, // waits for the fail sequence (ASSeqState)
		STATE_LAUNCH        = 16, // the countdown ended, judges the power
		STATE_DIVE          = 17, // Peck turns down
		STATE_SPLASHED      = 18, // waits for the landing cue to finish
		STATE_FALLING       = 19, // waits 3s and the cue, then the results
		STATE_RESULTS       = 20, // AAnticGameOverState is up, then the next player's turn
		STATE_DESCEND       = 21, // Peck goes down until it hits water or a "coll" group
		STATE_WAIT_FOR_HUD  = 22, // waits for the start hud to go away
		STATE_ROLLING       = 23, // Peck rolls on the ground
		STATE_STOPPED       = 24, // waits for the landing cue to finish, then the results
	};

	// Pooled locale strings shown below the power dial
	static constexpr TINT LOCALE_POWER_LOW  = 1032;
	static constexpr TINT LOCALE_POWER_HIGH = 1033;
	static constexpr TINT LOCALE_POWER_GOOD = 1035;

private:
	// NOTE: helpers for code that is inlined (or repeated) in OnUpdate
	AAnimatedUnit*     GetPeckUnit() const;
	AAnimatableObject* GetPeckObject() const;
	void               PlayPeckAnimation( const Toshi::TPString8& a_rcName );
	void               UpdatePullbackAnimation( AAnimatableObject* a_pObject, const Toshi::TPString8& a_rcName, TFLOAT& a_rfDuration, TFLOAT& a_rfTime, TFLOAT a_fDeltaTime );
	static void        TurnDirection( Toshi::TVector4& a_rDirection, const Toshi::TVector4& a_rcTarget, TFLOAT a_fProgress );
	static void        TurnCamera( ACamera* a_pCamera, const Toshi::TVector4& a_rcTarget, TFLOAT a_fProgress );
	void               GetDiveDirection( Toshi::TVector4& a_rOutDirection );
	void               MoveForward( TFLOAT a_fDeltaTime );
	void               StopFlightFeathers();
	void               ShowResults();
	void               HideHud();
	void               OrbitCamera( TFLOAT a_fDeltaTime );
	void               FollowWithCamera( TFLOAT a_fProgress );
	void               SnapToGround();
	void               LandInWater( TFLOAT a_fHitDistance, TBOOL a_bHideText );
	void               Rumble();
	static TINT        FindClosestHit( const ACollisionModelRayHitList& a_rcHitList, TFLOAT& a_rfDistance );

	void UpdateBashing( TFLOAT a_fDeltaTime );
	void UpdateTumble( TFLOAT a_fDeltaTime );
	void UpdateFlying( TFLOAT a_fDeltaTime );
	void UpdateCrashed( TFLOAT a_fDeltaTime );
	void UpdateEndCamera( TFLOAT a_fDeltaTime );
	void UpdateDive( TFLOAT a_fDeltaTime );
	void UpdateDescend( TFLOAT a_fDeltaTime );
	void UpdateRolling( TFLOAT a_fDeltaTime );

	// 0x00782824: never written in retail, so always 0.0f
	inline static TFLOAT ms_fMinFlightSpeed = 0.0f;

private:
	Toshi::TVector4                         m_vTargetPoint;                 // 0xbac, "target_point00"
	Toshi::TVector4                         m_vPeckPos;                     // 0xbbc, "player_start", later the "peckattachpoint" bone
	Toshi::TVector4                         m_vPeckPrevPos;                 // 0xbcc, m_vPeckPos before the last flight step
	Toshi::TVector4                         m_vPeckDir;                     // 0xbdc, flight direction of Peck
	Toshi::TVector4                         m_vUnk_BEC;                     // 0xbec, reset to VEC_ZERO when the flight starts
	TFLOAT                                  m_fPitchRate;                   // 0xbfc, pitch speed of the flight (stick Y), +-2 degrees
	TFLOAT                                  m_fYawRate;                     // 0xc00, yaw speed of the flight (stick X), +-1 degree
	TFLOAT                                  m_fBankAngle;                   // 0xc04, visual roll of Peck around the flight direction (stick X), +-PI/2
	TFLOAT                                  m_fPower;                       // 0xc08, bash power (0..1) while pulling back, then the flight speed
	TFLOAT                                  m_fStateTime;                   // 0xc0c, += delta every OnUpdate; reset by a bash while pulling back
	TINT                                    m_iUnk_C10;                     // 0xc10
	TFLOAT                                  m_fTargetDist1;                 // 0xc14, target_point00 .. target_point01
	TFLOAT                                  m_fTargetDist2;                 // 0xc18, target_point01 .. target_point02
	TINT                                    m_iNumPlayers;                  // 0xc1c
	TINT                                    m_iNextRing;                    // 0xc20, first ring of m_vecObjects1/2 Peck hasn't passed yet
	TINT                                    m_iNextCoin;                    // 0xc24, first barnbuck of m_vecObjects3 Peck hasn't passed yet
	TINT                                    m_iScore;                       // 0xc28, portrait score: 5 per ring, 1 per barnbuck
	TINT                                    m_iLandingScore;                // 0xc2c, scorecard of the landing (0..9, -1 = none), GetResults: locale 1039 row = 10 * ( this + 1 )
	TINT                                    m_iRingScore;                   // 0xc30, 5 per ring, GetResults: locale 1038 row
	TINT                                    m_iCoinsPassed;                 // 0xc34, barnbucks passed (collected or not)
	TBOOL                                   m_bUnk_C38;                     // 0xc38, cleared by ResetTurn
	TUINT8                                  m_aUnk_C39[ 3 ];                // 0xc39
	TINT                                    m_iCue_C3C;                     // 0xc3c, cue 127
	TINT                                    m_iCue_C40;                     // 0xc40
	TINT                                    m_iCue_C44;                     // 0xc44
	TINT                                    m_iCue_C48;                     // 0xc48
	TINT                                    m_iCue_C4C;                     // 0xc4c
	TINT                                    m_iCue_C50;                     // 0xc50, cue 135 (positional)
	TINT                                    m_iCue_C54;                     // 0xc54, cue 135 (positional)
	TINT                                    m_iCue_C58;                     // 0xc58, cue 130, paused on deactivate
	TINT                                    m_iCue_C5C;                     // 0xc5c, cue 141
	TINT                                    m_iCue_C60;                     // 0xc60, cue 160
	Toshi::TRef<AUnitProxy>                 m_oPeckProxy;                   // 0xc64, the "peckFireProgram" unit
	TINT                                    m_eState;                       // 0xc68
	ACFPBashHudElement                      m_oBashHud;                     // 0xc6c
	TBOOL                                   m_bEndCameraStarted;            // 0xebc, set by OnUpdate when the end camera starts
	TUINT8                                  m_aUnk_EBD[ 3 ];                // 0xebd
	PlayerInput                             m_aInputs[ MAX_PLAYERS ];       // 0xec0
	DepthSortedVector<200>                  m_vecObjects1;                  // 0xf10
	DepthSortedVector<200>                  m_vecObjects2;                  // 0x1234
	DepthSortedVector<240>                  m_vecObjects3;                  // 0x1558
	TINT                                    m_iUnk_191C;                    // 0x191c
	Toshi::T2Vector<AAnimatableObject*, 10> m_vecObjects4;                  // 0x1920
	AGUI2Portrait                           m_oPortrait;                    // 0x194c
	AAnimatableObject*                      m_pPlayerModel;                 // 0x1f14, "cfp_americanm" / "cfp_americanf"
	AAnimatableObject*                      m_pFreddy;                      // 0x1f18, "cfp_freddy"
	Toshi::TRandom                          m_oRandom;                      // 0x1f1c
	TFLOAT                                  m_fPlayerPullbackDuration;      // 0x1fc0, -1.0f until known
	TFLOAT                                  m_fPeckPullbackDuration;        // 0x1fc4, -1.0f until known
	TFLOAT                                  m_fPlayerPullbackTime;          // 0x1fc8, "americanm_pullback" time (restored by OnActivate)
	TFLOAT                                  m_fPeckPullbackTime;            // 0x1fcc, "launchPullBack" time (restored by OnActivate)
	TFLOAT                                  m_fStallTime;                   // 0x1fd0
	TFLOAT                                  m_fUnk_1FD4;                    // 0x1fd4
	TFLOAT                                  m_fUnk_1FD8;                    // 0x1fd8
	Toshi::TGenericListener                 m_oReleaseListener;             // 0x1fdc
	AAnimationBreakpointCallback*           m_pReleaseEmitter;              // 0x1ff0, "ReleaseBP" emitter of the player model
	TBOOL                                   m_bReleased;                    // 0x1ff4, set by OnReleaseBreakpoint
	TFLOAT                                  m_fMinBash;                     // 0x1ff8, BashParams/MinBash
	TFLOAT                                  m_fUnderPowerBash;              // 0x1ffc, BashParams/UnderPowerBash (0..1)
	TFLOAT                                  m_fOverPowerBash;               // 0x2000, BashParams/OverPowerBash (0..1)
	TFLOAT                                  m_fIdealPowerBash;              // 0x2004, BashParams/IdealPowerBash (0..1)
	TFLOAT                                  m_fOverPowerTimeout;            // 0x2008, TimeOuts/OverPowerTimeout
	TFLOAT                                  m_fUnderPowerTimeout;           // 0x200c, TimeOuts/UnderPowerTimeout
	TFLOAT                                  m_fLaunchTimeout;               // 0x2010, TimeOuts/LaunchTimeout
	TFLOAT                                  m_fBashInc;                     // 0x2014, BashParams/BashInc
	TFLOAT                                  m_fBashRed;                     // 0x2018, BashParams/BashRed
	TFLOAT                                  m_fBoostTimeout;                // 0x201c, TimeOuts/BoostTimeout
	TFLOAT                                  m_fPeckBoostAccel;              // 0x2020, Misc/PeckBoostAccel
	TBOOL                                   m_bBoosted;                     // 0x2024, a boost was used during the flight
	TBOOL                                   m_bSavedTreeCollision;          // 0x2025, ATreeManager flag restored by OnRemoval
	TFLOAT                                  m_fTimer;                       // 0x2028, launch countdown / flight time out / per-state timer
	TINT                                    m_iCountdown;                   // 0x202c, last whole second of the launch countdown (beeps)
	TFLOAT                                  m_fEndTimer;                    // 0x2030, landing and the end camera
	TFLOAT                                  m_fShakeRadius;                 // 0x2034, camera shake after a crash landing
	TFLOAT                                  m_fFlightStep;                  // 0x2038, distance moved this frame while diving
	TFLOAT                                  m_fTextTimer;                   // 0x203c, refreshes the bash hud text every 0.2s
	TINT                                    m_iNumBoosts;                   // 0x2040, boosts (wings) left
	TFLOAT                                  m_fUnk_2044;                    // 0x2044
	TFLOAT                                  m_fSavedTreeLODDistance;        // 0x2048
	TFLOAT                                  m_fSavedTreeRenderDistance;     // 0x204c
	TFLOAT                                  m_fSavedTreeBlendingDistance;   // 0x2050
	TFLOAT                                  m_fSavedFarDistance;            // 0x2054
	AParticleEffectHandle*                  m_pFlightFeathersL;             // 0x2058, "PeckFeth_flight" on the left forearm while boosting
	AParticleEffectHandle*                  m_pFlightFeathersR;             // 0x205c, "PeckFeth_flight" on the right forearm while boosting
	TBOOL                                   m_bOrbitCamera;                 // 0x2060, the end camera orbits Peck
	TFLOAT                                  m_fWrongWayTime;                // 0x2064, time flying away from the targets (locale 1044 shown)
	TINT                                    m_iVariant;                     // 0x2068, AMiniGameManager variant + 1 (4 = charity)
	AGUI2Button                             m_aButtons[ NUM_BUTTONS ];      // 0x206c
	void*                                   m_apUnk_287C[ NUM_BUTTONS ];    // 0x287c, TMalloc( 32 ) each
	TINT                                    m_iNumPlayers2;                 // 0x288c
	TUINT8                                  m_aUnk_2890[ 0x28d0 - 0x2890 ]; // 0x2890
	TWCHAR                                  m_wszBashText[ 128 ];           // 0x28d0, locale 1031 with the input tags, shown by the bash hud
	TWCHAR                                  m_wszInstructions[ 128 ];       // 0x29d0, locale 1045 with the input tags
	AGUI2TextBox                            m_oTextBox;                     // 0x2ad0
	TFLOAT                                  m_fFlightTime;                  // 0x2b58
	TINT                                    m_iTimeBonus;                   // 0x2b5c, 45 - m_fFlightTime on a target landing, GetResults: locale 606 row
	AModelInstance*                         m_pPeckModelInstance;           // 0x2b60
	TINT                                    m_iPeckLeftForearmBone;         // 0x2b64, "cg_l_forearm"
	TINT                                    m_iPeckRightForearmBone;        // 0x2b68, "cg_r_forearm"
	TINT                                    m_iFeatherFlightEffect;         // 0x2b6c, "PeckFeth_flight"
	TINT                                    m_iWaterSplashAEffect;          // 0x2b70, "watersplash2a"
	TINT                                    m_iWaterSplashBEffect;          // 0x2b74, "watersplash2b"
	TINT                                    m_iWaterRippleEffect;           // 0x2b78, "waterripple"
	TINT                                    m_iFeatherExplodeEffect;        // 0x2b7c, "PeckFeth_explode"
	TINT                                    m_iFeatherExplodeEffect2;       // 0x2b80, "PeckFeth_explode"
	TINT                                    m_iDustPuffEffect;              // 0x2b84, "dustpuff"
	TINT                                    m_iLeavesFallEffect;            // 0x2b88, "leavesfall"
	AGUI2Feathers                           m_oFeathers;                    // 0x2b8c
	TINT                                    m_iUnk_30F4;                    // 0x30f4
	TINT                                    m_iUnk_30F8;                    // 0x30f8
	AWindmillHelper                         m_oWindmillHelper;              // 0x30fc
	TFLOAT                                  m_fSavedWindDirection;          // 0x3194, AWindmillHelper::ms_fWindDirection, cleared while the minigame runs
};
