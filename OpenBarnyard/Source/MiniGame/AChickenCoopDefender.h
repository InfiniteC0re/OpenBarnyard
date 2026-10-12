#pragma once
#include "AMiniGameState.h"
#include "AChickenCoopDefenderGame.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2MilkAmmoBar.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AGUI2Portrait.h"
#include "GUI/AGUI2TargetCounter.h"
#include "GUI/AMilkReticleManager.h"
#include "HUD/AInteractionHudElement.h"
#include "Helpers/AMilkBottlePickups.h"
#include "Render/AGlowViewport.h"
#include "Units/APlayerControlComponent.h"
#include "Units/Projectiles/ALaunchUnitAmmo.h"
#include "Units/AUnitProxy.h"

#include <GUI/T2GUIRectangle.h>
#include <Toshi/T2SList.h>
#include <Toshi/TString16.h>
#include <Math/TVector4.h>

class AUnitPlayer;
class AAnimatableObject;
class ASimplePlayerControlComponent;
class AInteriorNetwork;
class AModelInstance;

//-----------------------------------------------------------------------------
// Chicken Coop Defender (hidden minigame "AChickenCoopDefender",
// "Coop_Defender"). Played from the quests "Ben (Night) Chicken Coop 1"
// (variant 0, raccoons steal the eggs), "Otis Chicken Coop 2: Coyotes"
// (variant 1, coyotes steal the hens and attack the player) and Freddy's dream
// (variant 2, Freddy steals roast hens), and replayed from AReplayManager
// ("chick_coop_replay", "freddy_dream_replay").
// TClass 00709520 (parent AMiniGameState), vtables 00722a48 (AMiniGameState),
// 00722a44 (AInventoryAniEffect::CallBackInterface), 00722aec
// (AUnitActionReceiver), size 0x85f8.
//
// Thieves come in waves from "thiefspawn1" to the coop and take the hens/eggs
// out of it. The player squirts them with milk (they drop what they carry),
// picks the loose hens/eggs up and throws them back into the coop from the hot
// spots around it, and throws the stunned raccoons away. The game is won if
// there are hens/eggs left (and lives) when the time is up.
//-----------------------------------------------------------------------------
class AChickenCoopDefender
    : public AMiniGameState
    , public AUnitActionReceiver
{
public:
	TDECLARE_CLASS( AChickenCoopDefender, AMiniGameState );

	static constexpr TINT NUM_THROW_TARGETS = 8;
	static constexpr TINT HIGHSCORE_TABLE   = 23; // variants 0 and 1, 24 for variant 2

	enum VARIANT_ : TINT
	{
		VARIANT_RACCOONS,      // 0
		VARIANT_COYOTES,       // 1
		VARIANT_FREDDYS_DREAM, // 2
	};

	enum PHASE_ : TINT
	{
		PHASE_PLAYING, // 0
		PHASE_FOLLOW,  // 1, the camera follows a hen/egg thrown into the coop
		PHASE_ENDED,   // 2
	};

	// What the player is doing (AChickenCoopDefenderGame::ePlayerState)
	enum PLAYERSTATE_ : TINT
	{
		PLAYERSTATE_FREE,         // 0
		PLAYERSTATE_PICKUP_THIEF, // 1, "pickup_raccoon"
		PLAYERSTATE_THROW_THIEF,  // 2, "throw_raccoon"
		PLAYERSTATE_HIT,          // 3, "yuk"
		PLAYERSTATE_PICKUP_ITEM,  // 4, "pickup" a hen or an egg
		PLAYERSTATE_CARRY_EGG,    // 5
		PLAYERSTATE_CARRY_HEN,    // 6
		PLAYERSTATE_THROW_HEN,    // 7, "throw"
		PLAYERSTATE_THROW_EGG,    // 8, "throw"
		PLAYERSTATE_AIM_EGG,      // 9, turns to the coop
		PLAYERSTATE_AIM_HEN,      // 10, turns to the coop
	};

	// A model of the fence around the coop, 8 bytes
	struct FenceModel
	    : public Toshi::T2SList<FenceModel>::Node
	{
		AModelInstance* pInstance; // 0x4
	};

public:
	AChickenCoopDefender();
	~AChickenCoopDefender();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void     OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void     OnPostRender( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual ASoundId GetSound() OVERRIDE;
	virtual void     OnPause() OVERRIDE;
	virtual void     OnInsertion() OVERRIDE;
	virtual void     OnRemoval() OVERRIDE;
	virtual void     OnActivate() OVERRIDE;
	virtual void     OnDeactivate() OVERRIDE;

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

	AChickenCoopDefenderGame& GetGame() { return m_oGame; }

private:
	void LoadData();
	void CreatePlayer( AGUI2MiniMap* a_pMap );
	void DestroyPlayer();
	void SetupCamera();
	void CreateThieves();
	void DestroyThieves();
	void CreateHensAndEggs();
	void DestroyHensAndEggs();
	void CreateHanna();
	void CreateModels();
	void DestroyModels();
	void CreateHotSpots();
	void DestroyHotSpots();

	// Shows/hides the thieves of the free lists (SetVisible)
	void SetThievesVisible( TBOOL a_bVisible );

	// The player can pick up/throw something: shows the arrow and the prompt
	void ShowInteraction( const Toshi::TVector4& a_rcPosition, const TWCHAR* a_wszText );

	// Updates what the player can interact with
	void UpdateInteraction();

	// Spawns up to a_iCount thieves of a free list
	void SpawnThieves( TINT a_iCount, AChickenCoopDefenderGame::UnitList10& a_rFree, AChickenCoopDefenderGame::UnitList10& a_rSpawned );

	// Returns the index of the throw target (s_aThrowTargets) closest to the direction a_rcDirection
	// that can be reached from a_rcFrom, or -1
	static TINT FindThrowTarget( const Toshi::TVector4& a_rcFrom, const Toshi::TVector4& a_rcDirection );

	// The player stops moving/squirting (holding something)
	void DisablePlayerControl();
	void EnablePlayerControl();

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay: the player squirts the thieves and throws the hens/eggs back into the coop
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	AChickenCoopDefenderGame       m_oGame;                // 0x0bc0
	AUnitPlayer*                   m_pPlayer;              // 0x1104, "player_chickencoop"
	AAnimatableObject*             m_pSunglasses;          // 0x1108
	ASimplePlayerControlComponent* m_pControl;             // 0x110c
	AInteriorNetwork*              m_pNetwork;             // 0x1110, "chickencoop"
	AInteriorNetwork*              m_pUnitNetwork;         // 0x1114, the network of the hens and the thieves (same)
	TBOOL                          m_bRenderBottles;       // 0x1118, NOTE: set and cleared but not read
	AInteractionHudElement         m_oInteractionHud;      // 0x111c
	AGUI2MiniMap                   m_oMap;                 // 0x162c
	AGUI2TargetCounter             m_oCounter;             // 0x1b4c, hens/eggs left
	AGUI2LifeHud                   m_oLifeHud;             // 0x1d54
	AGUI2Portrait                  m_oPortrait;            // 0x20fc
	Toshi::T2GUIElement            m_oAmmoRoot;            // 0x26c4
	Toshi::T2GUIRectangle          m_oAmmoBacking;         // 0x2718
	AGUI2MilkAmmoBar               m_oAmmoBar;             // 0x2790
	TFLOAT                         m_fWaveTimer;           // 0x2a48
	AModelInstance*                m_pArrowModel;          // 0x2a4c, "chickencoop_arrow", over what the player can interact with
	AGlowViewport::GlowObject*     m_pGlowObject;          // 0x2a50, NOTE: never set, removed from AGlowViewport by DestroyPlayer
	TWCHAR                         m_wszInteraction[ 64 ]; // 0x2a54
	AMilkReticleManager            m_oReticles;            // 0x2ad4
	TFLOAT                         m_fOldNearClip;         // 0x84f8
	ALaunchUnitAmmo                m_oAmmo;                // 0x84fc
	TBOOL                          m_bRenderReticles;      // 0x8514
	TBOOL                          m_bGameOverStarted;     // 0x8515
	AMilkBottlePickups             m_oMilkBottles;         // 0x8518
	Toshi::T2SList<FenceModel>     m_oFenceModels;         // 0x85c4, "fncp01", "fncp03", "fncpbroken"
	Toshi::TString16               m_aStrings[ 4 ];        // 0x85c8, NOTE: unused
};

TSTATICASSERT( sizeof( AChickenCoopDefender ) == 0x85f8 );
