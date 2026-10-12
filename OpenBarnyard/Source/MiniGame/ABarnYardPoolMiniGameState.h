#pragma once
#include "AMiniGameState.h"
#include "ABarnYardPoolAI.h"
#include "HUD/ABarnYardPoolHud.h"
#include "HUD/AGUI2NewPoolPower.h"
#include "FX/AParticleEmitLine.h"
#include "Units/AUnitProxy.h"

#include <Render/TViewport.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>
#include "SoundBank/music.h"

class AAnimatableObject;
class AAnimatedUnit;
struct AParticleEffect;
class AParticleEffectHandle;
class APoolCamera;
class AUnitPoolBall;
class AUnitPoolBfa;
class AUnitPoolCue;

//-----------------------------------------------------------------------------
// Barn Pool (minigame "ABarnYardPoolMiniGameState", "Pool", 2-4 players).
// TClass 00708f10, vtable 007204f8 (AInventoryAniEffect::CallBackInterface 00722a44), size 0x20714.
//
// Eight ball against Duke (the computer, ABarnYardPoolAI) or other players in
// two teams. The balls are AUnitPoolBall units moved by APoolBallSteer, the cue
// is an AUnitPoolCue (rendered again on top of the world, see OnRenderOverlay)
// and the aim target is an AUnitPoolBfa. A dotted line (AParticleEmitLine) and
// an arrow show where the cue ball goes. The first ball potted decides the
// groups (ball types 2 and 3), the team that has potted its 7 balls has to pot
// the black ball. Potting the cue ball gives ball in hand to the other team.
// The camera is an APoolCamera with 7 views (4 around the table, 3 behind the
// cue). Variants 0-4 are played on the boat table (envpoolboat, with the props
// that fade out near the cue; the variant is the difficulty of the computer),
// variant 5 on the pool table (envpooltable) with smaller pockets.
// NOTE: the name of the class is from the TClass, the members are named after
// what they do.
//-----------------------------------------------------------------------------
class ABarnYardPoolMiniGameState
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( ABarnYardPoolMiniGameState, AMiniGameState );

	static constexpr TINT NUM_BALLS   = 16;
	static constexpr TINT NUM_POCKETS = 6;
	static constexpr TINT NUM_PROPS   = 4; // the fish, the float and the two oarlocks next to the table
	static constexpr TINT NUM_VIEWS   = 7;
	static constexpr TINT NUM_TEAMS   = 2;
	static constexpr TINT TEXT_SIZE   = 256;

	static constexpr TINT CUE_BALL   = 0;
	static constexpr TINT BLACK_BALL = 8;

	// Value of Team::GetCurrentPlayer for the computer
	static constexpr TINT PLAYER_AI = 666;

	// AUnitPoolBall::GetBallType values
	enum BALLTYPE : TINT
	{
		BALLTYPE_CUE     = 0,
		BALLTYPE_BLACK   = 1,
		BALLTYPE_GROUP_A = 2, // balls 1-7, yellow on the HUD
		BALLTYPE_GROUP_B = 3, // balls 9-15, red on the HUD
		BALLTYPE_NONE    = 4, // the ball shadows; groups not decided yet / no ball hit
	};

	// m_eState
	enum STATE : TINT
	{
		STATE_NONE             = -1, // used by EndShot while it decides the next state
		STATE_START            = 0,
		STATE_GAME_DECIDED     = 1, // the black ball has been potted legally (or not)
		STATE_UNK_2            = 2, // states 2, 6, 11, 25 and 26 are never set, UpdateAimGuide checks them
		STATE_GAME_OVER_WAIT   = 3,
		STATE_SHOW_RESULTS     = 4,
		STATE_BALLS_ROLLING    = 5, // waits for every ball to stop, then EndShot
		STATE_UNK_6            = 6,
		STATE_START_TURN       = 7,
		STATE_HUMAN_START      = 8,
		STATE_HUMAN_AIM        = 9,
		STATE_HUMAN_POWER      = 10, // Shoot is held, the power goes up and down
		STATE_UNK_11           = 11,
		STATE_HUMAN_SHOT       = 12,
		STATE_AI_PLAN          = 13,
		STATE_AI_WAIT          = 14,
		STATE_AI_SHOW_CUE      = 15,
		STATE_AI_WAIT_CAMERA   = 16,
		STATE_AI_POWER         = 17,
		STATE_AI_SHOT          = 18,
		STATE_SHOOT            = 19,
		STATE_CUE_STRIKE       = 20, // the cue pulls back and hits the cue ball
		STATE_CUE_FOLLOW       = 21, // the cue fades out
		STATE_BALL_IN_HAND     = 22,
		STATE_HUMAN_PLACE_BALL = 23,
		STATE_AI_PLACE_BALL    = 24,
		STATE_UNK_25           = 25,
		STATE_UNK_26           = 26,
		STATE_RESULTS          = 27, // AAnticGameOverState is shown, the game restarts once it's gone
		STATE_MESSAGE          = 28, // waits 2 seconds, then goes to m_eNextState
		STATE_MESSAGE_POTTED   = 29, // same, after the player has potted a ball
	};

	// m_eBallsState
	enum BALLSSTATE : TINT
	{
		BALLSSTATE_MOVING  = 25,
		BALLSSTATE_STOPPED = 26,
	};

	static constexpr TINT HIGHSCORE_TABLE_BARN     = 66; // variants 0 and 2
	static constexpr TINT HIGHSCORE_TABLE_VARIANT1 = 65;
	static constexpr TINT HIGHSCORE_TABLE_VARIANT3 = 67;
	static constexpr TINT HIGHSCORE_TABLE_VARIANT4 = 68;
	static constexpr TINT HIGHSCORE_TABLE_VARIANT5 = 69;

	// Locale strings
	static constexpr TINT LOCALE_DUKE             = 132;
	static constexpr TINT LOCALE_PLAYER_WINS      = 628; // "{0} wins"
	static constexpr TINT LOCALE_OPPONENT_WINS    = 629;
	static constexpr TINT LOCALE_PLAYER_TURN      = 630; // "{0}'s turn"
	static constexpr TINT LOCALE_OPPONENT_TURN    = 631;
	static constexpr TINT LOCALE_RESULT_WON       = 634;
	static constexpr TINT LOCALE_RESULT_LOST      = 635;
	static constexpr TINT LOCALE_RESULT_ACCURACY  = 636;
	static constexpr TINT LOCALE_POTTED           = 641;
	static constexpr TINT LOCALE_TURN_OVER        = 642;
	static constexpr TINT LOCALE_FOUL_NO_HIT      = 643;
	static constexpr TINT LOCALE_FOUL_CUE_BALL    = 645;
	static constexpr TINT LOCALE_FOUL_BLACK_BALL  = 646;
	static constexpr TINT LOCALE_GROUP_A          = 647;
	static constexpr TINT LOCALE_GROUP_B          = 648;
	static constexpr TINT LOCALE_FOUL_HIT_BLACK   = 649;
	static constexpr TINT LOCALE_FOUL_HIT_GROUP_A = 650;
	static constexpr TINT LOCALE_FOUL_HIT_GROUP_B = 651;
	static constexpr TINT LOCALE_HOW_TO_SHOOT     = 652;

	//-----------------------------------------------------------------------------
	// The players of a team and the balls it has potted (inlined struct, no vtable).
	// ctor 004d7960 (and 004d7a80 for the eh vector iterator of the state), size 0x20.
	//-----------------------------------------------------------------------------
	struct Team
	{
		static constexpr TINT MAX_PLAYERS = 2;

		Team();

		void Reset();

		// The group of the other team: BALLTYPE_GROUP_A unless a_eBallType is BALLTYPE_GROUP_B
		void SetOppositeBallType( TINT a_eBallType );

		// Counts a potted ball of the type a_eBallType (the black one counts as the 8th ball)
		void OnBallPotted( TINT a_eBallType );

		// The next player of the team plays, returns the index of the player
		TINT NextPlayer();

		void SetNumPlayers( TINT a_iNumPlayers );

		// $Barnyard: FUNCTION 004d7a40
		void SetPlayer( TINT a_iIndex, TINT a_iPlayer ) { aPlayers[ a_iIndex ] = a_iPlayer; }

		// $Barnyard: FUNCTION 004d7a50
		TINT GetCurrentPlayer() const { return aPlayers[ iCurrentPlayer ]; }

		TINT GetNextPlayer() const;

		// The ball type the team has to hit: the black ball once all 7 are potted
		TINT GetTargetBallType() const;

		// NOTE: folded with other getters of offset 0 (006bbd30)
		TINT GetNumPotted() const { return iNumPotted; }

		// NOTE: folded with other getters of offset 8 (004ffb80)
		TINT GetBallType() const { return eBallType; }

		// NOTE: folded with other setters of offset 8 (PGUITRRenderObject::SetViewport)
		void SetBallType( TINT a_eBallType ) { eBallType = a_eBallType; }

		TINT iNumPotted;              // 0x00
		TINT iNumShots;               // 0x04
		TINT eBallType;               // 0x08, BALLTYPE
		TINT iNumPlayers;             // 0x0c, 1 or 2
		TINT iCurrentPlayer;          // 0x10
		TINT iUnk14;                  // 0x14, never used
		TINT aPlayers[ MAX_PLAYERS ]; // 0x18, player slot or PLAYER_AI
	};

	// One of the props next to the table that fade out when the cue gets close.
	struct Prop
	{
		AAnimatableObject* pObject;        // 0x0
		TFLOAT             fTime;          // 0x4, 0..1 of the fade
		TFLOAT             fTargetOpacity; // 0x8
		TFLOAT             fStartOpacity;  // 0xc
	};

	struct View
	{
		Toshi::TVector4 vPosition; // 0x00
		Toshi::TVector4 vLookAt;   // 0x10
	};

public:
	ABarnYardPoolMiniGameState();
	~ABarnYardPoolMiniGameState();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void     OnRenderOverlay( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual ASoundId GetSound() OVERRIDE { return soundbank::MUSIC_POOL; } // NOTE: folded with ADartMiniGame::GetSound (00522030)
	virtual void     OnPause() OVERRIDE;
	virtual void     OnInsertion() OVERRIDE;
	virtual void     OnRemoval() OVERRIDE;
	virtual void     OnActivate() OVERRIDE;
	virtual void     OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AInventoryAniEffect::CallBackInterface
	//-----------------------------------------------------------------------------
	virtual void OnObjectCollected( AAnimatableObject* a_pObject ) OVERRIDE {} // DOES_NOTHING

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void Pause() OVERRIDE;            // plays the intro cut-scene
	virtual void PlayLoseSequence() OVERRIDE; // plays the lose cut-scene
	virtual void PlayWinSequence() OVERRIDE;  // plays the win cut-scene
	virtual void OnPreStart() OVERRIDE;
	virtual void UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*  GetResults() OVERRIDE;
	virtual void OnStartHudFinished() OVERRIDE;

private:
	Team& GetCurrentTeam() { return m_aTeams[ m_iCurrentTeam ]; }

	AUnit* GetBallUnit( TINT a_iBall ) const { return m_aBalls[ a_iBall ]->GetUnit(); }

	// Sets up a new game: racks the balls, the camera views, the teams and the HUD
	void ResetGame();

	// Places the balls: the cue ball at a_vCueBall and the others in a triangle around a_vBlack
	void RackBalls( Toshi::TVector4 a_vBlack, Toshi::TVector4 a_vCueBall, TFLOAT a_fBallRadius );

	// Decides what happens after a shot once every ball has stopped (fouls, turns, end of the game)
	void EndShot();

	// The turn goes to the other team
	void NextTurn();

	// Shows the potted message and plays the sounds after a ball of the team has been potted
	void OnTeamBallPotted();

	// Moves the views behind the cue to the cue
	void UpdateCueViews();

	// Picks the view around the table that looks along a_vShot
	void ChooseShotView( Toshi::TVector4 a_vShot );

	// Moves the aim target (or the cue ball while it's placed) with the input. Returns TTRUE if nothing moved.
	TBOOL UpdateAim( TFLOAT a_fDeltaTime );

	// Shows where the cue ball goes (the dotted line and the arrow)
	void UpdateAimGuide( TFLOAT a_fDeltaTime );

	// The props next to the table (not in variant 5)
	void CreateProps();
	void DestroyProps();
	void UpdateProps( TFLOAT a_fDeltaTime );

	// Sets the flag (ASkinMaterial +0x64) of the materials of the first LOD of the model of a_pUnit
	void SetMaterialsFlag( AAnimatedUnit* a_pUnit, TBOOL a_bFlag );

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay: aims a human turn at a ball and shoots
	void UpdateDevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	View                    m_aViews[ NUM_VIEWS ];      // 0xbac, 0-3 around the table, 4-6 behind the cue
	Toshi::TVector4         m_vAimTarget;               // 0xc8c, where the cue points at (the AUnitPoolBfa)
	Toshi::TVector4         m_vUnkC9C;                  // 0xc9c, never used
	Toshi::TVector4         m_vShotVelocity;            // 0xcac
	Toshi::TVector4         m_vAIAimPoint;              // 0xcbc
	TFLOAT                  m_fAIShotPower;             // 0xccc
	Toshi::TVector4         m_avPockets[ NUM_POCKETS ]; // 0xcd0
	Toshi::TVector4         m_vCueBallStart;            // 0xd30, the "ball_white" locator
	Toshi::TVector4         m_vForward;                 // 0xd40, from the cue ball to the black ball
	TFLOAT                  m_fBallInHandRadius;        // 0xd50
	APoolCamera*            m_pCamera;                  // 0xd54
	STATE                   m_eState;                   // 0xd58
	BALLSSTATE              m_eBallsState;              // 0xd5c
	TINT                    m_iVariant;                 // 0xd60
	TINT                    m_iNumPlayers;              // 0xd64, human players
	TINT                    m_iNumAIPlayers;            // 0xd68
	TINT                    m_iDifficulty;              // 0xd6c, of the computer
	TINT                    m_iCurrentTeam;             // 0xd70
	TINT                    m_iOtherTeam;               // 0xd74
	TINT                    m_iNextTeam;                // 0xd78, plays after a shot that pots nothing
	TINT                    m_iUnkD7C;                  // 0xd7c, set to 6, never read
	TINT                    m_eFirstGroup;              // 0xd80, BALLTYPE of the first potted ball, decides the groups
	TINT                    m_eFirstHitBallType;        // 0xd84, of the last shot
	TINT                    m_iWinner;                  // 0xd88, team, 6 while playing
	TINT                    m_iPlayerWon;               // 0xd8c, 1 if the winner is the first team, 6 while playing
	TINT                    m_iSavedView;               // 0xd90
	TBOOL                   m_bPottedGroupB;            // 0xd94, in the last shot
	TBOOL                   m_bPottedGroupA;            // 0xd95
	TBOOL                   m_bPottedCueBall;           // 0xd96
	TBOOL                   m_bPottedBlackBall;         // 0xd97
	TBOOL                   m_bOpenTable;               // 0xd98, no ball potted yet, any first hit is fine
	TBOOL                   m_bAimForwardOnly;          // 0xd99, ball in hand: the aim target must be ahead of the cue ball
	TBOOL                   m_bUnkD9A;                  // 0xd9a, never read
	TBOOL                   m_bUnkD9B;                  // 0xd9b, never read
	TBOOL                   m_bMouseAim;                // 0xd9c, never read
	Toshi::TRef<AUnitProxy> m_aBalls[ NUM_BALLS ];      // 0xda0
	Toshi::TRef<AUnitProxy> m_aShadows[ NUM_BALLS ];    // 0xde0, "poolball" units of type 4 under the balls
	Toshi::TRef<AUnitProxy> m_aCue[ 1 ];                // 0xe20
	Toshi::TRef<AUnitProxy> m_pUnusedProxy;             // 0xe24, never set
	Toshi::TRef<AUnitProxy> m_pBfa;                     // 0xe28
	TFLOAT                  m_fTableMaxX;               // 0xe2c, the pockets plus a margin, never read
	TFLOAT                  m_fTableMaxZ;               // 0xe30
	TFLOAT                  m_fTableMinX;               // 0xe34
	TFLOAT                  m_fTableMinZ;               // 0xe38
	TFLOAT                  m_fGameOverTimer;           // 0xe3c
	TINT                    m_iUnkE40;                  // 0xe40, set to 0, never read
	TFLOAT                  m_fPocketRadius;            // 0xe44
	TFLOAT                  m_fAimX;                    // 0xe48, input
	TFLOAT                  m_fAimY;                    // 0xe4c
	TFLOAT                  m_fMouseX;                  // 0xe50
	TFLOAT                  m_fMouseY;                  // 0xe54
	TFLOAT                  m_fInputDelay;              // 0xe58, input is ignored until it goes below 0
	TFLOAT                  m_afAimSpeed[ 2 ];          // 0xe5c, y and x, go up while the input is held
	TFLOAT                  m_fLineOffset;              // 0xe64, moves the dots of the line
	TBOOL                   m_abAimInput[ 2 ];          // 0xe68, y and x
	AUnitPoolCue*           m_pCue;                     // 0xe6c
	AUnitPoolBfa*           m_pAimTarget;               // 0xe70
	AUnitPoolBall*          m_pCueBall;                 // 0xe74
	AUnitPoolBall*          m_pBlackBall;               // 0xe78
	Team                    m_aTeams[ NUM_TEAMS ];      // 0xe7c, the first one is the player's
	AGUI2NewPoolPower       m_oPowerBar;                // 0xebc
	ABarnYardPoolHud        m_oHud;                     // 0x1394
	TINT                    m_iShotSound;               // 0x2608
	TINT                    m_iBallSound;               // 0x260c
	TINT                    m_iCushionSound;            // 0x2610
	TINT                    m_iPocketSound;             // 0x2614
	Prop                    m_aProps[ NUM_PROPS ];      // 0x2618
	ABarnYardPoolAI         m_oAI;                      // 0x2658
	AParticleEmitLine       m_oAimLine;                 // 0x201b4
	Toshi::TViewport        m_oCueViewport;             // 0x204d8, the cue is rendered again in it
	const TWCHAR*           m_wszOpponentName;          // 0x204f0
	TWCHAR                  m_wszText[ TEXT_SIZE ];     // 0x204f4
	AParticleEffectHandle*  m_pArrowHandle;             // 0x206f4
	AParticleEffect*        m_pArrowEffect;             // 0x206f8
	TFLOAT                  m_fSavedNear;               // 0x206fc
	TFLOAT                  m_fGuideScale;              // 0x20700, the part of the line after a cushion
	TBOOL                   m_bPropsCreated;            // 0x20704
	TFLOAT                  m_fMessageTimer;            // 0x20708
	TBOOL                   m_bGroupsAnnounced;         // 0x2070c
	STATE                   m_eNextState;               // 0x20710, after STATE_MESSAGE

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay (not in retail, keep it last)
	TFLOAT m_fDevAutoPlayTimer;
	TINT   m_iDevAutoPlayShots;
#endif
};

#ifndef BARNYARD_COMMUNITY_PATCH
TSTATICASSERT( sizeof( ABarnYardPoolMiniGameState ) == 0x20714 );
#endif
TSTATICASSERT( sizeof( ABarnYardPoolMiniGameState::Team ) == 0x20 );
