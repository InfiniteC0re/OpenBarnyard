#pragma once
#include "GameInterface/ASplitScreen.h"
#include "GameInterface/AGameState.h"
#include "FX/AInventoryAniEffect.h"
#include "HUD/AMiniGamePlayerFloat.h"
#include "HUD/AStartMiniGameHudElement.h"
#include "HUD/ASimPlayerPickUpHudElement.h"
#include "HUD/ATimeLeftHudElement.h"
#include "AMinigamePlayerAi.h"

#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>
#include <Math/TVector4.h>

class AAnimatableObject;
class ACollisionObjectSimple;

//-----------------------------------------------------------------------------
// Base of every minigame game state (TClass 007826d0, vtable 0071d768).
//
// Flow (see AMiniGameState::OnUpdate 0046b930):
//  - AMiniGameInstructionState::Dismiss calls OnStart(): m_bIsActive = TRUE,
//    OnPreStart(), Pause() (which by default calls AGameState::OnPause, the
//    "begin play" hook; gatecrash pushes its intro cut-scene instead).
//  - Every frame: UpdateAlways(); while the start HUD is visible
//    UpdatePreStart(); the first frame after it hides OnStartHudFinished();
//    then, unless ARootTask is paused, m_fElapsedTime += dt, the AI's Update()
//    and UpdateGame(); finally UpdateGameOver() once IsGameOver() or the time
//    limit has run out.
//  - When the game is over the state pushes AAnticGameOverState( GetResults() ).
//-----------------------------------------------------------------------------
class AMiniGameState
    : public AGameState
    , public AInventoryAniEffect::CallBackInterface
{
public:
	TDECLARE_CLASS( AMiniGameState, AGameState );

	static constexpr TINT MAX_PLAYERS     = 4;
	static constexpr TINT MAX_RESULT_ROWS = 5;

	// One line of the results table ("Dodges  3").
	struct ResultRow
	{
		const TWCHAR* wszName = TNULL; // 0x0
		TINT          iScore  = 0;     // 0x4
	};

	// Results of one player.
	struct PlayerInfo
	{
		TBOOL                                       bUnk1 = TFALSE;       // 0x00
		TINT                                        iUnk2 = 0;            // 0x04
		TINT                                        iUnk3 = 0;            // 0x08
		Toshi::T2Vector<ResultRow, MAX_RESULT_ROWS> vecRows;              // 0x0c
		const TWCHAR*                               wszTotalName = TNULL; // 0x38, locale 0x2c3
		TINT                                        iTotal       = 0;     // 0x3c, sum of vecRows, clamped to >= 0
	};

	struct PlayerList
	{
		PlayerInfo aInfos[ MAX_PLAYERS ];

		// $Barnyard: FUNCTION 00469150
		PlayerList() {}

		PlayerInfo& operator[]( TINT a_iIndex )
		{
			TASSERT( a_iIndex < MAX_PLAYERS );
			return aInfos[ a_iIndex ];
		}
	};

	// Minigame results. Built by GetResults(), handed to AAnticGameOverState and
	// copied into AMiniGameManager (AMiniGameManager::StoreResults -> AMiniGameManager::CopyResults) on removal.
	struct S1
	{
		PlayerList       oPlayerList;           // 0x000
		TINT             iResult          = -1; // 0x100, 0 = won, -1 = lost/quit
		TINT             iMiniGameIndex   = -1; // 0x104
		TINT             iMiniGameVariant = -1; // 0x108
		Toshi::TPString8 strUnk;                // 0x10c
		TBOOL            bFlag1 = TFALSE;       // 0x110
		TBOOL            bFlag2 = TFALSE;       // 0x111
		TBOOL            bWon   = TFALSE;       // 0x112
		TBOOL            bFlag4 = TTRUE;        // 0x113
		TBOOL            bFlag5 = TFALSE;       // 0x114
		TBOOL            bFlag6 = TFALSE;       // 0x115

		S1();

		// Sums the rows of a player into its total.
		void ComputeTotal( TINT a_iPlayer );
	};

public:
	AMiniGameState();
	AMiniGameState( AMinigamePlayerAi* a_pPlayerAi );
	virtual ~AMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnStart() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AInventoryAniEffect::CallBackInterface
	//-----------------------------------------------------------------------------
	virtual void OnObjectCollected( AAnimatableObject* a_pObject ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own methods
	//-----------------------------------------------------------------------------

	// m_bIsActive = TFALSE and calls AGameState::OnPause (+0x3c).
	virtual void Pause(); // +0x68

	virtual void PlayLoseSequence() {} // DOES_NOTHING, +0x6c
	virtual void PlayWinSequence() {}  // DOES_NOTHING, +0x70

	// Called by OnStart before Pause().
	virtual void OnPreStart() {} // DOES_NOTHING, +0x74

	virtual void  OnResultsShown() {}          // DOES_NOTHING, +0x78
	virtual TBOOL Unk_0x7c() { return TTRUE; } // RETURNS_TRUE, +0x7c

	virtual TBOOL IsGameOver() { return TFALSE; } // RETURNS_FALSE, +0x80

	// Called every frame.
	virtual void UpdateAlways( TFLOAT a_fDeltaTime ) {} // DOES_NOTHING, +0x84

	// Called every frame after the start HUD has gone, unless ARootTask is paused.
	virtual void UpdateGame( TFLOAT a_fDeltaTime ) {} // DOES_NOTHING, +0x88

	// Called every frame once IsGameOver() or the time limit has run out.
	virtual void UpdateGameOver( TFLOAT a_fDeltaTime ) {} // DOES_NOTHING, +0x8c

	// Called every frame while the start HUD is visible.
	virtual void UpdatePreStart( TFLOAT a_fDeltaTime ) {} // DOES_NOTHING, +0x90

	// Fills m_oResults and returns it.
	virtual S1* GetResults() = 0; // +0x94

	// Detaches the start, time left and pick up HUD elements.
	virtual void OnDetachHUD(); // +0x98

	// Called once, the first frame after the start HUD has hidden.
	virtual void OnStartHudFinished(); // +0x9c

	// Shows the start HUD for the next player.
	virtual void OnSetupPlayers(); // +0xa0

	//-----------------------------------------------------------------------------
	// Charity variant helpers (Everet and Daisy standing next to the arena)
	//-----------------------------------------------------------------------------
	void CreateCharityActors( TBOOL a_bWithCollision );
	void DestroyCharityActors();
	void SetCharityActorsSkeletonUpdating( TBOOL a_bUpdating );
	void PlaceCharityActors( const Toshi::TVector4& a_rcEveretPos, const Toshi::TVector4& a_rcDaisyPos, const Toshi::TVector4& a_rcEveretDir, Toshi::TVector4 a_vDaisyDir );

	// Flies a_pObject into the pick up HUD (ASwirlTrackHudAniEffect + AInventoryAniEffect) and plays cue 0xa5.
	// a_iCount > 0 adds the pick ups to APlayerSimProfile::AddMiniGamePickups, a_fSpeed is the speed of the throw.
	void StartCollectEffect( AAnimatableObject* a_pObject, TINT a_iCount, TFLOAT a_fSpeed );

	AMinigamePlayerAi* GetPlayerAi() const { return m_pPlayerAi; }
	TFLOAT             GetElapsedTime() const { return m_fElapsedTime; }
	TFLOAT             GetTimeLimit() const { return m_fTimeLimit; }
	void               SetActive( TBOOL a_bActive ) { m_bIsActive = a_bActive; }

protected:
	S1                         m_oResults;                     // 0x0a8
	Toshi::TVector4            m_vCameraPos;                   // 0x1c0
	TFLOAT                     m_fElapsedTime;                 // 0x1d0
	TFLOAT                     m_fTimeLimit;                   // 0x1d4, 0 = no limit
	AMinigamePlayerAi*         m_pPlayerAi;                    // 0x1d8
	TINT                       m_iMusicCue;                    // 0x1dc
	TBOOL                      m_bUnk1;                        // 0x1e0
	TBOOL                      m_bWon;                         // 0x1e1
	AMiniGamePlayerFloat       m_aPlayerFloats[ MAX_PLAYERS ]; // 0x1e4
	TBOOL                      m_bShowPlayerFloats;            // 0x594
	TBOOL                      m_bIsActive;                    // 0x595
	AStartMiniGameHudElement   m_oStartHudElement;             // 0x598
	ASimPlayerPickUpHudElement m_oPickUpHudElement;            // 0x770
	TBOOL                      m_bStartHudWasHidden;           // 0x95c
	TBOOL                      m_bSubmitHighScore;             // 0x95d
	ATimeLeftHudElement        m_oTimeLeftHudElement;          // 0x960
	TBOOL                      m_bCountUp;                     // 0xb94, the clock shows elapsed instead of remaining time
	AAnimatableObject*         m_pCharityEveret;               // 0xb98
	AAnimatableObject*         m_pCharityDaisy;                // 0xb9c
	ACollisionObjectSimple*    m_pCharityEveretCollision;      // 0xba0
	ACollisionObjectSimple*    m_pCharityDaisyCollision;       // 0xba4
	TBOOL                      m_bStarted;                     // 0xba8, the start HUD has gone and the game is running
};

TSTATICASSERT( sizeof( AMiniGameState::PlayerInfo ) == 0x40 );
TSTATICASSERT( sizeof( AMiniGameState::S1 ) == 0x118 );
TSTATICASSERT( sizeof( AMiniGameState ) == 0xbac );
