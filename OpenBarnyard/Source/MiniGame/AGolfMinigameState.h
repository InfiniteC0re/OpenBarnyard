#pragma once
#include "AMiniGameState.h"
#include "AGolfReticle.h"
#include "GUI/T2GUIString.h"
#include "GUI/AGUI2AnythingPopup.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2Timer.h"
#include "GUI/AGUI2TextBox.h"
#include "HUD/AButtonSlamHudElement.h"
#include "Helpers/AWindmillHelper.h"
#include "Animation/AAnimationBreakpointCallback.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TEvent.h>
#include <Toshi/TRefCounted.h>
#include <File/TTRB.h>
#include <GUI/T2GUIRectangle.h>
#include <Render/TRenderInterface.h>
#include <Render/TViewport.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class AUnit;
class ASysMaterial;
class ASysMesh;

//-----------------------------------------------------------------------------
// Gopher Golf (minigame "AGolfMiniGameState", class "AGolfMinigameState",
// "Golf", 1-4 players). TClass 007827ec, vtable 0071dd80
// (AInventoryAniEffect::CallBackInterface 00722a44), size 0x4298.
//
// The players ("player_golf", the cow with a club) take turns on the holes of
// the course at the golf course terrain (Terrain_Golf1..4). Every player has a
// golf ball ("golfball", AUnitGolfProjectile with an ABallSteer) and a gopher
// ("golfgopher", AGolfGopherUnit) that tunnels to the ball. The power is set
// with the AButtonSlamHudElement power bar. The data come from
// Data\MinigameData\Golf\golfdata.trb: the course of the variant
// ("MiniGameLevel1".."MiniGameLevel8", CourseData) and "HoleData".
//
// NOTE: OnUpdate (00478800) replaces AMiniGameState::OnUpdate and runs its own
// state machine (m_eGameState).
//-----------------------------------------------------------------------------
class AGolfMinigameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( AGolfMinigameState, AMiniGameState );

	static constexpr TINT NUM_POWER_CHUNKS = 36;
	static constexpr TINT NUM_BUCKS        = 30; // charity variant ("barnbuck")
	static constexpr TINT MAX_HOLES        = 18; // per course (hole numbers 1..18)

	// Per hole physics of the ball (HoleData[ hole number - 1 ])
	struct HoleData
	{
		TFLOAT fBounce;          // 0x0, ABallSteer::SetBounce (0.25 if 0)
		TFLOAT fBounceFriction;  // 0x4, ABallSteer::SetBounceFriction (0.4 if 0)
		TFLOAT fMinBounceRatio;  // 0x8, ABallSteer::SetMinBounceRatio (0.2 if 0)
		TFLOAT fRollingFriction; // 0xc, ABallSteer::SetRollingFriction (0.3 if 0)
	};

	// A course ("MiniGameLevel1".."MiniGameLevel8" of golfdata.trb)
	struct CourseData
	{
		TINT   aHoles[ MAX_HOLES ];       // 0x00, hole numbers (1..18)
		TFLOAT afTimeLimits[ MAX_HOLES ]; // 0x48, m_fTimeLimit of each hole
		TINT   iNumHoles;                 // 0x90
		TBOOL  bStrokeLimit;              // 0x94, the course ends after a number of strokes (else after the time limit)
		TBOOL  bCharity;                  // 0x95, Everet and Daisy with the bucks
	};

	enum GAMESTATE : TINT
	{
		GAMESTATE_INTRO,      // 0, hole camera (ABoneAttachCameraHelper) flies over the hole
		GAMESTATE_TEE,        // 1, waiting for the swing of the tee shot
		GAMESTATE_POWER_UP,   // 2, the power bar goes up
		GAMESTATE_POWER_DOWN, // 3, the power bar goes down (accuracy)
		GAMESTATE_SWING,      // 4, swing animation, the ball is hit towards a zone locator
		GAMESTATE_TEE_FLIGHT, // 5, the tee shot is flying
		GAMESTATE_PLAY,       // 6, AGolfMinigameState::UpdatePlay
		GAMESTATE_END,        // 7, win/lose animation, then AAnticGameOverState
	};

	// Player::iGopherState (GAMESTATE_PLAY)
	enum GOPHERSTATE : TINT
	{
		GOPHERSTATE_FREE,    // 0, moved with the stick
		GOPHERSTATE_HOLDING, // 1, holds a ball (Player::iHeldBall) and turns to aim
		GOPHERSTATE_WINDUP,  // 2, winds up, strikes once the swing button is released
	};

	// Player::iBallState (GAMESTATE_PLAY)
	enum BALLSTATE : TINT
	{
		BALLSTATE_ON_COURSE, // 0
		BALLSTATE_HELD,      // 1, by a gopher
		BALLSTATE_SAVED,     // 2, the swimming gopher (Player::pGopherGolf) throws it back ("ballsave")
		BALLSTATE_IN_WATER,  // 3, drifts to the bank
		BALLSTATE_HOLED,     // 4, holed out or given up
	};

	// A player (0x79c)
	struct Player
	{
		Player();
		~Player();

		AGolfReticle            oReticle;                               // 0x000, landing spot of the shot
		ASysMesh*               pPowerPointerMesh;                      // 0x690, "particles\gopher_power_pointer.tga"
		ASysMesh*               apPowerChunkMeshes[ NUM_POWER_CHUNKS ]; // 0x694, "particles\gopher_power_chunk.tga"
		Toshi::TGenericListener oAnimListener;                          // 0x724
		Toshi::TVector4         vBallPosition;                          // 0x738, of the last frame; in the water
		Toshi::TVector4         vBallVelocity;                          // 0x748, drift in the water, then the throw of the swimming gopher
		Toshi::TRef<AUnitProxy> pGopher;                                // 0x758, "golfgopher"
		Toshi::TRef<AUnitProxy> pBall;                                  // 0x75c, "golfball"
		AAnimatableObject*      pGopherGolf;                            // 0x760, "gopher_golf" with the ball, snorkel and floaties
		AAnimatableObject*      pBallModel;                             // 0x764, copy of the ball model (attached to pGopherGolf)
		TFLOAT                  fStickX;                                // 0x768
		TFLOAT                  fStickY;                                // 0x76c
		TFLOAT                  fDPadX;                                 // 0x770, -1 / 1 (commands 18 / 19)
		TFLOAT                  fDPadY;                                 // 0x774, 1 / -1 (commands 16 / 17)
		TFLOAT                  fWindupTime;                            // 0x778, GOPHERSTATE_WINDUP, the strength of the strike
		TFLOAT                  fPower;                                 // 0x77c, power bar 0..1
		TFLOAT                  fWaterTime;                             // 0x780, BALLSTATE_IN_WATER
		TINT                    iHeldBall;                              // 0x784, player whose ball the gopher holds (or has just struck), -1
		TINT                    iStrokes;                               // 0x788
		TINT                    iScore;                                 // 0x78c
		TINT                    iGopherState;                           // 0x790, GOPHERSTATE
		TINT                    iBallState;                             // 0x794, BALLSTATE
		TBOOL                   bSwingPressed;                          // 0x798, command 55 (button down)
		TBOOL                   bSwingReleased;                         // 0x799, command 55 (button up)
	};

	// List of animation listeners connected to the same caller and callback (0x10).
	// The nodes are allocated (0x20) and freed by the list.
	struct AnimListeners
	{
		struct Node
		    : public Toshi::TDList<Node>::TNode
		{
			Node( Toshi::TGenericEmitter* a_pEmitter, void* a_pCaller, Toshi::TGenericListener::t_Callback a_fnCallback, TINT a_iPriority );
			~Node();

			Toshi::TGenericListener oListener; // 0x08
			Toshi::TGenericEmitter* pEmitter;  // 0x1c
		};

		~AnimListeners();

		// Disconnects and frees every node
		void Clear();

		// Adds a node listening to a_pEmitter
		Node* Add( Toshi::TGenericEmitter* a_pEmitter, TINT a_iPriority );

		void*                               pCaller;    // 0x0
		Toshi::TGenericListener::t_Callback fnCallback; // 0x4
		Toshi::TDList<Node>                 llNodes;    // 0x8
	};

public:
	AGolfMinigameState();
	~AGolfMinigameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE; // +0x10
	virtual void     OnRenderOverlay( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                         // +0x20
	virtual ASoundId GetSound() OVERRIDE;                                                                                                                     // +0x2c
	virtual TBOOL    OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;                                                                                                // +0x4c
	virtual void     OnInsertion() OVERRIDE;                                                                                                                  // +0x50
	virtual void     OnRemoval() OVERRIDE;                                                                                                                    // +0x54
	virtual void     OnActivate() OVERRIDE;                                                                                                                   // +0x60
	virtual void     OnDeactivate() OVERRIDE;                                                                                                                 // +0x64

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void Pause() OVERRIDE {}    // +0x68, DOES_NOTHING (the golf state is never paused this way)
	virtual S1*  GetResults() OVERRIDE; // +0x94

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// Strokes after which the hole ends (by its number: 9, 12, 15 or 18)
	TINT GetMaxStrokes();

	// Shows a_wszText in the popup
	void ShowPopup( const TWCHAR* a_wszText );

	// The course ends after the time limit, or after GetMaxStrokes() strokes of the first player
	TBOOL IsCourseOver();

	// Starts the turn of the next player. Returns TTRUE once every player has played.
	TBOOL NextPlayer();

	// Called by ProcessCommand (AGolfMinigameState::ProcessCommand) etc.
	Player& GetPlayer( TINT a_iIndex ) { return m_aPlayers[ a_iIndex ]; }
	Player& GetCurrentPlayer() { return m_aPlayers[ m_iCurrentPlayer ]; }

	const CourseData* GetCourse() const { return m_pCourse; }
	const HoleData*   GetHoleData() const { return m_ppHoleData[ m_pCourse->aHoles[ m_iCurrentHole ] - 1 ]; }

	AUnit* GetPlayerUnit() const { return m_pPlayerUnit ? m_pPlayerUnit->GetUnit() : TNULL; }

	//-----------------------------------------------------------------------------
	// AGolfMinigameState_Flow.cpp
	//-----------------------------------------------------------------------------
	// Reticles, timer, popup, alpha of the timer/strokes box and the fading hole text
	void UpdateHud( TFLOAT a_fDeltaTime ); // 00471bf0

	// Resets the players, the tee, the coins and the HUD for the current hole and starts the hole camera fly-over (GAMESTATE_INTRO)
	void SetupPlayerTurn(); // 00474830
	void SetupTee();        // inlined into SetupPlayerTurn

	// After the tee shots: shows the balls and the gophers, starts the clock (GAMESTATE_PLAY)
	void StartPlay(); // 00474360

	// Multi camera on the gophers (and the balls/the hole)
	void SetupHoleCamera(); // 00473d60

	// The hole is over (m_oPlayListeners): updates the lives of the single player
	void OnPlayFinished(); // 00473d00

	// "golf_start_attach" puts the ball in the hand of the player, "golf_start_detach" puts it on the tee
	TBOOL OnSwingAnimEvent( AAnimationBreakpointCallback* a_pBreakpoint, AAnimationBreakpointCallback::Event* a_pEvent ); // 00473b30

	// The gopher of a player throws the ball (Player::oAnimListener)
	TBOOL OnGopherAnimEvent( AAnimationBreakpointCallback* a_pBreakpoint, AAnimationBreakpointCallback::Event* a_pEvent ); // 00473f10

	static TBOOL __stdcall OnSwingAnimEventCallback( AGolfMinigameState* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData );  // 00474330
	static TBOOL __stdcall OnPlayFinishedCallback( AGolfMinigameState* a_pThis, void* a_pOwner, void* a_pData );                                                           // 00474350
	static TBOOL __stdcall OnGopherAnimEventCallback( AGolfMinigameState* a_pThis, AAnimationBreakpointCallback* a_pOwner, AAnimationBreakpointCallback::Event* a_pData ); // 00474530

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay" runs
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

	//-----------------------------------------------------------------------------
	// AGolfMinigameState_Play.cpp
	//-----------------------------------------------------------------------------
	// GAMESTATE_PLAY: the gophers carry the balls to the hole and strike them, holing out, scoring
	void UpdatePlay( TFLOAT a_fDeltaTime ); // 00476350

	// Once every ball is holed (or the course is over) ends the hole: next hole, retry or GAMESTATE_END
	void OnHoleFinished( TFLOAT a_fDeltaTime ); // 00475c30

	// Loads the terrain, sounds and locators of the next hole of the course
	void LoadHole(); // 004754f0

	// The balls in the water drift to the bank where the swimming gopher throws them out
	void UpdateBall( TFLOAT a_fDeltaTime ); // 00475030

private:
	// NOTE: parts of UpdatePlay (00476350), one function in retail
	void UpdatePlayCamera();
	void UpdateGophers( TFLOAT a_fDeltaTime );
	void CollectBucks();
	void UpdateBalls();
	void UpdateBallSaves();

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay: the gopher of a_iPlayer fetches its ball, aims at the hole and strikes
	void DevAutoPlayGopher( TINT a_iPlayer );
#endif

public:
protected:
	Toshi::T2GUIString      m_oHoleText;                  // 0x0bac
	TWCHAR                  m_wszHoleText[ 16 ];          // 0x0c10, OnActivate shows m_oHoleText if it isn't empty
	TFLOAT                  m_fHoleTextTime;              // 0x0c30, time left before m_oHoleText fades out
	TBOOL                   m_bHoleTextFadeIn;            // 0x0c34
	TWCHAR                  m_wszInstructions2[ 64 ];     // 0x0c36, locale 0x206 with the input tags
	Toshi::T2GUIString      m_oInstructionsText;          // 0x0cb8
	Toshi::TViewport        m_oViewport;                  // 0x0d1c
	Toshi::TTRB             m_oGolfData;                  // 0x0d34, Data\MinigameData\Golf\golfdata.trb
	Player                  m_aPlayers[ MAX_PLAYERS ];    // 0x0e74
	Toshi::TGenericListener m_oSwingListener;             // 0x2ce4
	AnimListeners           m_oPlayListeners;             // 0x2cf8, OnPlayFinishedCallback
	AButtonSlamHudElement   m_oPowerBar;                  // 0x2d08
	AGUI2AnythingPopup      m_oPopup;                     // 0x31d4
	AGUI2LifeHud            m_oLifeHud;                   // 0x3294, single player: Score_Wood_Thin, 5 lives
	AGUI2Timer              m_oTimer;                     // 0x363c, single player, time limit courses
	Toshi::T2GUIRectangle   m_oStrokesBack;               // 0x385c, single player, stroke limit courses ("Golf")
	Toshi::T2GUIString      m_oStrokesText;               // 0x38d4
	TWCHAR                  m_wszStrokes[ 4 ];            // 0x3938, the stroke limit of the hole
	Toshi::T2GUIElement*    m_pTimerOrStrokes;            // 0x3940, m_oTimer or m_oStrokesBack
	AGUI2TextBox            m_oTextBox;                   // 0x3944
	TWCHAR                  m_wszInstructions[ 64 ];      // 0x39bc, locale 0x202 with the input tags
	AGolfReticle            m_oFlagReticle;               // 0x3a3c, "Putt_Flag"
	ASysMaterial*           m_pPowerPointerMaterial;      // 0x40cc
	ASysMaterial*           m_pPowerChunkMaterial;        // 0x40d0
	Toshi::TVector4         m_vHolePosition;              // 0x40d4, the flag
	Toshi::TVector4         m_vPlayerPosition;            // 0x40e4, spawn position of the player unit
	Toshi::TVector4         m_vPlayerDirection;           // 0x40f4
	Toshi::TRef<AUnitProxy> m_pPlayerUnit;                // 0x4104, "player_golf"
	CourseData*             m_pCourse;                    // 0x4108
	HoleData**              m_ppHoleData;                 // 0x410c, "HoleData"
	AAnimatableObject*      m_apBucks[ NUM_BUCKS ];       // 0x4110, charity variant
	TINT                    m_iNumBucks;                  // 0x4188
	TINT                    m_iStartPickups;              // 0x418c, APlayerSimProfile::GetUnk10 at the start of the hole (charity)
	AAnimatableObject*      m_pTee;                       // 0x4190, "golf_tee"
	AAnimatableObject*      m_pFlag;                      // 0x4194, "golf_flag"
	AAnimatableObject*      m_pSockFlag;                  // 0x4198, "sock_flag"
	TINT                    m_iUnk419C;                   // 0x419c
	TFLOAT                  m_fIdleTimer;                 // 0x41a0
	TFLOAT                  m_fHoleEndTime;               // 0x41a4, since every ball is holed
	TFLOAT                  m_fCourseOverTime;            // 0x41a8, since IsCourseOver, the balls give up
	TFLOAT                  m_fTimer;                     // 0x41ac
	TFLOAT                  m_fPowerError;                // 0x41b0, distance of the power from the ideal power
	TFLOAT                  m_fSavedTreeLODDistance;      // 0x41b4
	TFLOAT                  m_fSavedTreeRenderDistance;   // 0x41b8
	TFLOAT                  m_fSavedTreeBlendingDistance; // 0x41bc
	TFLOAT                  m_fSavedFarDistance;          // 0x41c0
	TINT                    m_iUnk41C4;                   // 0x41c4
	TINT                    m_iUnk41C8;                   // 0x41c8
	TFLOAT                  m_fCameraTime;                // 0x41cc, since the strike, the single player camera follows the ball after 1s
	TINT                    m_iNumPlayers;                // 0x41d0
	TINT                    m_iCurrentPlayer;             // 0x41d4
	TINT                    m_iCurrentHole;               // 0x41d8, index into m_pCourse->aHoles
	TINT                    m_iLives;                     // 0x41dc, single player (5), 20 points each
	TUINT8                  m_aUsedZones[ 4 ];            // 0x41e0, used "zone%1d_%02d" locators of each accuracy (bits)
	GAMESTATE               m_eGameState;                 // 0x41e4
	TBOOL                   m_bHoleEndNotified;           // 0x41e8, the end of the hole popup/cue was played
	TBOOL                   m_bReloadHole;                // 0x41e9, OnUpdate reloads the hole (LoadHole) and returns
	TBOOL                   m_bHoleLoaded;                // 0x41ea, the first OnUpdate has loaded the hole
	TBOOL                   m_bPlayFinished;              // 0x41eb, OnPlayFinished was called
	TBOOL                   m_bCourseFinished;            // 0x41ec, m_oResults winner is valid
	TBOOL                   m_bLoseLife;                  // 0x41ed, OnPlayFinished takes a life of the single player
	TBOOL                   m_bCommand31;                 // 0x41ee, command 31 this frame
	TBOOL                   m_bCommand30;                 // 0x41ef, command 30 this frame
	TBOOL                   m_bCameraOnBall;              // 0x41f0, single player camera targets the ball instead of the hole
	TBOOL                   m_bCourseOverCamera;          // 0x41f1, the camera targets were set when the course was over
	TINT                    m_iWaterSplashAEffect;        // 0x41f4, "watersplash2a"
	TINT                    m_iWaterSplashBEffect;        // 0x41f8, "watersplash2b"
	TINT                    m_iWaterRippleEffect;         // 0x41fc, "waterripple"
	AWindmillHelper         m_oWindmillHelper;            // 0x4200
};
