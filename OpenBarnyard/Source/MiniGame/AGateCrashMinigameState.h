#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2FloatHudElement.h"
#include "Units/AUnitProxy.h"

#include <Toshi/TEvent.h>
#include <Toshi/TDList.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>
#include <Math/TMathInline.h>

class AAnimatableObject;
class AParticleEffectHandle;
class AUnit;

//-----------------------------------------------------------------------------
// Mud Jumpers (a.k.a. Gatecrasher): players stand in a ring around a pipe
// (the "gate") that swings round; jump over it or kick it back.
// TClass 00782768, vtable 0071d9e0, size 0x215c.
// Data: Data/MiniGameData/MudJumpers/muddata.trb (AIData, Scores, Lives),
//       terrain Terrain_GateCrash, intro SSMJ_Main.
//-----------------------------------------------------------------------------
class AGateCrashMinigameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( AGateCrashMinigameState, AMiniGameState );

	static constexpr TINT NUM_SPECTATORS  = 3;   // gen cow, goat, horse
	static constexpr TINT NUM_CROWD       = 3;   // charity variant: barnbuck crowd
	static constexpr TINT NUM_POSTS       = 6;   // charity variant: barnbuck "posts" following the gate
	static constexpr TINT MAX_PIPES       = 216; // gcpipe segments
	static constexpr TINT MAX_CROWD_BUCKS = 240;

	// muddata.trb "Scores": points per dodge, gate hit, knock out and life left.
	enum SCORE : TINT
	{
		SCORE_DODGE,
		SCORE_HIT,
		SCORE_KO,
		SCORE_LIFE,
		SCORE_NUMOF
	};

	// Default values of muddata.trb (Scores @0x40 and Lives @0x50).
	static constexpr TINT DEFAULT_SCORES[ SCORE_NUMOF ] = { 1, 2, 10, 20 };
	static constexpr TINT DEFAULT_LIVES[ MAX_PLAYERS ]  = { 5, 3, 2, 2 };

	// Cue played when a character gets knocked out, indexed by character (table 0071d930).
	static constexpr TINT KO_CUES[ 5 ] = { 639, 680, 78, 79, 80 };

	// Jump cues (table 0071d944).
	static constexpr TINT JUMP_CUES[ 4 ] = { -1, -1, 64, 69 };

	// Camera offset from the pipe while the start HUD is shown (0x00773838).
	static constexpr TFLOAT PRESTART_CAMERA_OFFSET[ 4 ] = { -8.3f, -3.4f, 0.0f, 1.0f };

	static constexpr TFLOAT GATE_SPEED_DECAY     = 0.03f; // per second
	static constexpr TFLOAT GATE_MIN_SPEED_BASE  = 3.0f;
	static constexpr TFLOAT GATE_MIN_SPEED_RAMP  = 0.05f; // per second
	static constexpr TFLOAT GATE_MIN_SPEED_ALIVE = 0.5f;  // added per knocked out player
	static constexpr TFLOAT GATE_HIT_SPEED_SCALE = -0.8f;
#ifdef BARNYARD_LATEST
	// Easy difficulty of the latest version (Wii): a lower minimum speed and a maximum speed, both + GATE_MIN_SPEED_ALIVE
	// per knocked out player. (The Wii plays Normal with GATE_MIN_SPEED_BASE_WII and GATE_MAX_SPEED_BASE_NORMAL too.)
	static constexpr TFLOAT GATE_MIN_SPEED_BASE_WII    = 2.0f;
	static constexpr TFLOAT GATE_MAX_SPEED_BASE_EASY   = 3.5f;
	static constexpr TFLOAT GATE_MAX_SPEED_BASE_NORMAL = 4.0f;
#endif
	static constexpr TFLOAT GATE_TIP_DISTANCE = 3.9f; // drips
	static constexpr TFLOAT GAME_OVER_DELAY   = 5.0f;
	static constexpr TFLOAT GATE_HIT_ANGLE    = Toshi::TMath::PI / 36.0f; // 0x00782764, the gate hits (or is dodged by) a player closer than this
	static constexpr TFLOAT KICK_HIT_ANGLE    = Toshi::TMath::PI / 12.0f; // 0x00782760, a kick reaches the gate closer than this

	enum PLAYERSTATE : TINT
	{
		PLAYERSTATE_READY,    // 0, idle
		PLAYERSTATE_JUMPING,  // 1
		PLAYERSTATE_STUNNED,  // 2, hit but still has lives (0.2 s)
		PLAYERSTATE_KICKING,  // 3
		PLAYERSTATE_KO_LEFT,  // 4, knocked out by a gate going left (ms_impact_l)
		PLAYERSTATE_KO_RIGHT, // 5, knocked out by a gate going right (ms_impact_r)
		PLAYERSTATE_OUT,      // 6, KO animation finished
		PLAYERSTATE_NUMOF
	};

	struct PlayerSlot
	{
		// $Barnyard: FUNCTION 0046dca0
		PlayerSlot() {}

		// $Barnyard: FUNCTION 0046dcb0
		~PlayerSlot() {}

		Toshi::TVector4         vPosition;        // 0x00
		Toshi::TRef<AUnitProxy> pUnitProxy;       // 0x10, unit = proxy->GetUnit()
		PLAYERSTATE             eState;           // 0x14
		TFLOAT                  fKickTimer;       // 0x18
		TFLOAT                  fJumpVelocity;    // 0x1c
		TFLOAT                  fAngle;           // 0x20, around the pipe
		TFLOAT                  fStunTimer;       // 0x24
		TINT                    iLives;           // 0x28
		TBOOL                   bJumpPressed;     // 0x2c
		TBOOL                   bKickPressed;     // 0x2d
		TBOOL                   bJumpHeld;        // 0x2e
		TBOOL                   bMudSplashed;     // 0x2f
		TBOOL                   bDodging;         // 0x30
		TBOOL                   bDirectionalKick; // 0x31
		TBOOL                   bKickLeft;        // 0x32

		AUnit* GetUnit() const { return pUnitProxy.Get() ? pUnitProxy.Get()->GetUnit() : TNULL; }
	};

	// The block AGateCrashPlayerAi reads (m_pGame points at 0xbac).
	struct GameData
	{
		PlayerSlot aSlots[ MAX_PLAYERS ]; // 0x00
		TFLOAT     fGateAngle;            // 0xd0
		TFLOAT     fGateSpeed;            // 0xd4, radians per second, sign = direction
		TINT       iServingPlayer;        // 0xd8, -1 while playing
	};

	// One listener per connected animation emitter (ms_impact_l / ms_impact_r).
	struct ImpactListener
	    : public Toshi::TDList<ImpactListener>::TNode
	{
		// $Barnyard: FUNCTION 0046cdf0
		~ImpactListener() { oListener.Disconnect(); }

		Toshi::TGenericListener oListener; // 0x08
		Toshi::TGenericEmitter* pEmitter;  // 0x1c
	};

	// The impact listeners all share a caller and callback.
	struct ImpactListeners
	{
		void*                               pCaller;    // 0x00, this
		Toshi::TGenericListener::t_Callback fnCallback; // 0x04, LAB_0046dd30 -> OnImpactAnimEvent
		Toshi::TDList<ImpactListener>       oList;      // 0x08

		// $Barnyard: FUNCTION 0046e830
		~ImpactListeners() { FreeAll(); }

		// Disconnects and frees every listener.
		void FreeAll(); // 0046e210
	};

public:
	AGateCrashMinigameState();
	~AGateCrashMinigameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual ASoundId GetSound() OVERRIDE;
	virtual void     OnPause() OVERRIDE; // begin play once the intro has finished
	virtual void     OnInsertion() OVERRIDE;
	virtual void     OnRemoval() OVERRIDE;

	// NOTE: +0x60/+0x64 in the retail vtable are 00522040/00522050, jmp thunks to
	// AMiniGameState::OnActivate/OnDeactivate. They are not overridden here.

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void  Pause() OVERRIDE;      // pushes the SSMJ_Main intro ASSeqState
	virtual void  OnPreStart() OVERRIDE; // hides the players while the intro plays
	virtual TBOOL IsGameOver() OVERRIDE;
	virtual void  UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdatePreStart( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*   GetResults() OVERRIDE;
	virtual void  OnStartHudFinished() OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------
	TINT GetGateSpeedSign() const;                                                   // 0046cd20, +1 / -1
	void OnImpactAnimEvent( void* a_pEventData );                                    // 0046d2d0, rumbles the slot whose animation threw it
	void PlayEndPoses();                                                             // 0046d3c0
	void OnGateHit( TINT a_iSlot );                                                  // 0046d4f0
	void UpdateDrips();                                                              // 0046d700
	void BuildGatePipe( TINT a_iNumSegments );                                       // 0046e250
	void UpdateGatePosition( const Toshi::TVector4& a_rcPosition, TFLOAT a_fAngle ); // 0046e610

	// Serve animation finished listener callback (0046dcd0, __stdcall, returns TTRUE).
	static TBOOL __stdcall OnServeFinished( AGateCrashMinigameState* a_pThis, void* a_pOwner, void* a_pData );

	// Impact animation event listener callback (LAB_0046dd30, __stdcall, returns TTRUE).
	static TBOOL __stdcall OnImpactAnimEventCallback( AGateCrashMinigameState* a_pThis, void* a_pOwner, void* a_pData );

	PlayerSlot& GetPlayerSlot( TINT a_iSlot ) { return m_oGame.aSlots[ a_iSlot ]; }
	GameData&   GetGameData() { return m_oGame; }

protected:
	GameData                m_oGame;                          // 0x0bac
	AAnimatableObject*      m_apSpectators[ NUM_SPECTATORS ]; // 0x0c88
	TINT                    m_iUnk1;                          // 0x0c94
	AAnimatableObject*      m_apCrowd[ NUM_CROWD ];           // 0x0c98, charity
	TFLOAT                  m_afCrowdTimers[ NUM_CROWD ];     // 0x0ca4, charity, -1 when done
	AAnimatableObject*      m_apPosts[ NUM_POSTS ];           // 0x0cb0, charity
	AGUI2LifeHud            m_aLifeHuds[ MAX_PLAYERS ];       // 0x0cc8
	Toshi::TGenericListener m_oServeListener;                 // 0x1b68
	ImpactListeners         m_oImpactListeners;               // 0x1b7c
	AGUI2FloatHudElement    m_oScorePopup;                    // 0x1b8c
	TINT                    m_iNumPipes;                      // 0x1d80
	AAnimatableObject*      m_apPipes[ MAX_PIPES ];           // 0x1d84, [ 0 ] is the head
	Toshi::TVector4         m_vPipePos;                       // 0x20e4, locator CB_pipe
	TFLOAT                  m_fPreStartTimer;                 // 0x20f4
	TFLOAT                  m_fGameOverTimer;                 // 0x20f8
	TFLOAT                  m_fMinGateSpeed;                  // 0x20fc
	TFLOAT                  m_fPrevGateAngle;                 // 0x2100
	TINT                    m_iNumPlayers;                    // 0x2104
	TINT                    m_iNumAlive;                      // 0x2108
	TINT                    m_iLastKicker;                    // 0x210c, slot that kicked the gate last, -1 after a hit
	AParticleEffectHandle*  m_pDripEffect;                    // 0x2110, "waterdrip"
	AParticleEffectHandle*  m_pMudDripEffect;                 // 0x2114, "waterdripmud"
	TINT                    m_iCrowdCount;                    // 0x2118, charity, capped at MAX_CROWD_BUCKS
	TFLOAT                  m_fPostTimer;                     // 0x211c, charity
	TINT                    m_aScores[ SCORE_NUMOF ];         // 0x2120, muddata.trb "Scores"
	TINT                    m_iNumHits;                       // 0x2130
	TINT                    m_iNumDodges;                     // 0x2134
	TINT                    m_iNumKOs;                        // 0x2138
	TBOOL                   m_bCharityVariant;                // 0x213c, minigame variant 1
	TBOOL                   m_bIntroPending;                  // 0x213d
	TINT                    m_iCreakCue;                      // 0x2140
	TINT                    m_iCue2;                          // 0x2144
	TINT                    m_iMudAmbienceCue;                // 0x2148
	TUINT8                  PADDING[ 0x10 ];                  // 0x214c, unused
};

TSTATICASSERT( sizeof( AGateCrashMinigameState::PlayerSlot ) == 0x34 );
TSTATICASSERT( sizeof( AGateCrashMinigameState::GameData ) == 0xdc );
TSTATICASSERT( sizeof( AGateCrashMinigameState::ImpactListener ) == 0x20 );
TSTATICASSERT( sizeof( AGateCrashMinigameState::ImpactListeners ) == 0x10 );
TSTATICASSERT( sizeof( AGateCrashMinigameState ) == 0x215c );
