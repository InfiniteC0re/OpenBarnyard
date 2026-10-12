#pragma once
#include "AMiniGameState.h"
#include "AChasingChicksGame.h"
#include "GUI/AGUI2MilkAmmoBar.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AMilkReticleManager.h"
#include "HUD/AInteractionHudElement.h"
#include "FX/AGameTimeFX.h"
#include "Helpers/AMilkBottlePickups.h"
#include "Units/APlayerControlComponent.h"
#include "Units/Projectiles/ALaunchUnitAmmo.h"
#include "Units/AUnitProxy.h"

#include <GUI/T2GUIRectangle.h>
#include <Toshi/TString16.h>
#include <Math/TVector4.h>

class AUnitPlayer;
class AInteriorNetwork;
class AGameTimeFXManager;

//-----------------------------------------------------------------------------
// Chasing Chicks (minigame "AChasingChicksMiniGame", "Chicks", 1 player).
// TClass at 0x7093e0 (parent AMiniGameState), vtables 007224c8 (AMiniGameState),
// 00722a44 (AInventoryAniEffect::CallBackInterface), 0072256c
// (AUnitActionReceiver), size 0x9244.
//
// The player and one (variant 0: Otis) or three (variant 1: Bud, Eddy and
// Igg, AChasingChicksAI) rivals chase ten chicks (AChasingChicksChick) around
// the farmyard, pick them up and throw them into the pens (hot spots) that
// light up; each kind of pen scores differently. Milk squirts and thrown chicks
// stun a rival, who drops the chick he is carrying. Hanna
// (AChasingChicksHanna) watches. The best score when the time is up wins.
//-----------------------------------------------------------------------------
class AChasingChicksMiniGame
    : public AMiniGameState
    , public AUnitActionReceiver
{
public:
	TDECLARE_CLASS( AChasingChicksMiniGame, AMiniGameState );

	static constexpr TINT NUM_ROWS = 4;

	enum PHASE_ : TINT
	{
		PHASE_PLAYING,      // 0
		PHASE_FOLLOW_CHICK, // 1, the camera follows the chick thrown by the player, the clock is stopped
		PHASE_ENDING,       // 2, waits for the chicks to land and the throws to finish
		PHASE_ENDED,        // 3, winner/loser animations, then the results
	};

public:
	AChasingChicksMiniGame();
	~AChasingChicksMiniGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual ASoundId GetSound() OVERRIDE { return 0; } // +0x2c, RETURNS_ZERO 0054d8b0
	virtual void     OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void     OnPostRender( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void     OnPause() OVERRIDE;
	virtual void     OnInsertion() OVERRIDE;
	virtual void     OnRemoval() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void  Pause() OVERRIDE;
	virtual void  PlayLoseSequence() OVERRIDE;
	virtual void  PlayWinSequence() OVERRIDE;
	virtual void  OnPreStart() OVERRIDE;
	virtual TBOOL IsGameOver() OVERRIDE;
	virtual void  UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  UpdateGameOver( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual S1*   GetResults() OVERRIDE;
	virtual void  OnStartHudFinished() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AUnitActionReceiver
	//-----------------------------------------------------------------------------
	virtual void OnUnitAction( AAnimationBreakpointUnitAction::Event* a_pEvent ) OVERRIDE;

	// a_pUnit has been hit (milk, thrown chick): stuns the player owning it
	void OnPlayerHit( Toshi::TRef<AUnitProxy> a_pUnit );

	AChasingChicksGame& GetGame() { return m_oGame; }

private:
	void LoadData();
	void CreatePlayers();
	void DestroyPlayers();
	void CreateHanna();
	void CreateChicks();
	void DestroyChicks();
	void CreateHotSpots();
	void DestroyHotSpots();
	void SetupCamera();
	void ResetCamera();
	void UpdateEnd();
	void ShowInteraction( const Toshi::TVector4& a_rcPosition, const TWCHAR* a_wszText );

	// Stuns the player a_iPlayer (unit a_pUnit) and makes him drop his chick
	void HitPlayer( AUnitPlayer* a_pUnit, TINT a_iPlayer );

	// Plays an animation of a player and updates his state ("pickup", "throw", "throw2", "yuk")
	static TBOOL PlayPlayerAnim( AChasingChicksPlayer* a_pPlayer, const Toshi::TPString8& a_rcAnim );

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay: the player catches the nearest chick and throws it into the nearest pen
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	AChasingChicksGame     m_oGame;                     // 0x0bc0
	AUnitPlayer*           m_pPlayer;                   // 0x0ec8, "player_chicks"
	AInteriorNetwork*      m_pNetwork;                  // 0x0ecc, "chasingchicks"
	TBOOL                  m_bTimeUp;                   // 0x0ed0
	TFLOAT                 m_fEndTimer;                 // 0x0ed4
	TINT                   m_iUnked8;                   // 0x0ed8
	TWCHAR                 m_wszInteraction[ 64 ];      // 0x0edc
	AInteractionHudElement m_oInteractionHud;           // 0x0f5c
	AGUI2Portrait          m_aPortraits[ MAX_PLAYERS ]; // 0x146c
	ALaunchUnitAmmo        m_oAmmo;                     // 0x2b8c
	AGameTimeFXManager*    m_pGameTimeFXManager;        // 0x2ba4
	AGameTimeFX            m_oFXFrom;                   // 0x2ba8
	AGameTimeFX            m_oFXTo;                     // 0x2d20
	TFLOAT                 m_fOldNearClip;              // 0x2e98
	AMilkReticleManager    m_oReticles;                 // 0x2e9c
	AGUI2MiniMap           m_oMap;                      // 0x88c0
	Toshi::T2GUIElement    m_oAmmoRoot;                 // 0x8de0
	Toshi::T2GUIRectangle  m_oAmmoBacking;              // 0x8e34
	AGUI2MilkAmmoBar       m_oAmmoBar;                  // 0x8eac
	Toshi::TString16       m_aRowNames[ NUM_ROWS ];     // 0x9164, results
	TBOOL                  m_bRenderReticles;           // 0x9194
	AMilkBottlePickups     m_oMilkBottles;              // 0x9198
};

TSTATICASSERT( sizeof( AChasingChicksMiniGame ) == 0x9244 );
