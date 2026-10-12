#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2MilkAmmoBar.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2ImagePopup.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AMilkReticleManager.h"
#include "HUD/AInteractionHudElement.h"
#include "FX/AGameTimeFX.h"
#include "Helpers/AMilkBottlePickups.h"
#include "Physics/ACollisionObjectMesh.h"
#include "Units/APlayerControlComponent.h"
#include "Units/Projectiles/ALaunchUnitAmmo.h"
#include "Units/AUnitProxy.h"

#include <File/TTRB.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/T2SList.h>
#include <Toshi/T2DList.h>
#include <Toshi/TEvent.h>
#include <Toshi/TRefCounted.h>
#include <Toshi/TString16.h>
#include <Math/TMatrix44.h>
#include <Math/TVector4.h>

class AUnitPlayer;
class ASimplePlayerControlComponent;
class AAnimatableObject;
class AGameTimeFXManager;
class AInteriorNetwork;
class AVeggiePatchVeggie;
class AVeggiePatchThief;

class AModelInstance;

//-----------------------------------------------------------------------------
// Data/MinigameData/VeggiePatchDefender/Data.trb
//-----------------------------------------------------------------------------

// A veggie locator ("carrot_01"...), 12 bytes
struct AVeggiePatchVeggieData
{
	const TCHAR* szLocator; // 0x0
	TINT         iKind;     // 0x4, filled at load time (AVeggiePatchDefenderGame::VEGGIE_)
	void*        pLocator;  // 0x8, filled at load time (ATerrainInterface::FindLocator)
};

// A piece of the fence around the patch ("fncveg01_01"...), 16 bytes
struct AVeggiePatchFenceData
{
	TCHAR szLocator[ 12 ]; // 0x0
	TINT  iGateHealth;     // 0xc, < 0: plain fence; otherwise a gate the raccoons can open
};

// "Ints" of a wave, 16 bytes
struct AVeggiePatchWaveData
{
	TINT   eType;     // 0x0, AVeggiePatchDefenderGame::WAVE_
	TINT   iCount;    // 0x4, raccoons spawned at once (WAVE_SPAWN), bits of the sniper locators (WAVE_SNIPERS)
	TFLOAT fUnk8;     // 0x8
	TFLOAT fDuration; // 0xc, also the time between the spawns of WAVE_SPAWN
};

// Parameters of the raccoons (8 floats per variant)
struct AVeggiePatchThiefParams
{
	TFLOAT fSlingshotAimTime; // 0x00, STATE_SLINGSHOT_AIM
	TFLOAT fFenceBreakTime;   // 0x04, time between two hits of a closed gate (STATE_BREAK_FENCE)
	TFLOAT fMilkedTime;       // 0x08, STATE_MILKED
	TFLOAT fScaredTime;       // 0x0c, STATE_SCARED
	TFLOAT fSeekVeggieSpeed;  // 0x10, STATE_SEEK_VEGGIE
	TFLOAT fSeekFenceSpeed;   // 0x14, STATE_SEEK_FENCE, STATE_FLEE
	TFLOAT fCarrySpeed;       // 0x18, STATE_CARRY_VEGGIE, multiplied by the speed of the veggie kind
	TFLOAT fScaredSpeed;      // 0x1c, STATE_SCARED, STATE_WANDER
};

// "ThiefAnimData": reference speeds of the raccoon animations
struct AVeggiePatchThiefAnimData
{
	TFLOAT fWalkSpeed;    // 0x0, walk_lp
	TFLOAT fRunSpeed;     // 0x4, run_lp
	TFLOAT fItemRunSpeed; // 0x8, itemrun_lp
	TFLOAT fScaredSpeed;  // 0xc, scared
};

// "GameData": one per variant ("NumGameVariants")
struct AVeggiePatchGameData
{
	TFLOAT                         fTimeLimit;       // 0x00
	TINT                           iLives;           // 0x04
	AVeggiePatchVeggieData*        apVeggies[ 5 ];   // 0x08, per AVeggiePatchDefenderGame::VEGGIE_
	TUINT                          aNumVeggies[ 5 ]; // 0x1c
	const AVeggiePatchThiefParams* pThiefParams;     // 0x30
	const AVeggiePatchWaveData*    pWaves;           // 0x34
	TINT                           iNumWaves;        // 0x38
	const AVeggiePatchFenceData*   pFences;          // 0x3c, NUM_FENCES
};

TSTATICASSERT( sizeof( AVeggiePatchGameData ) == 0x40 );

//-----------------------------------------------------------------------------
// Veggie Patch Defender (minigame "AVeggiePatchDefenderGame", "Veggie").
// TClass 0077e700... vtables 007201e0 (AMiniGameState), 00722a44
// (AInventoryAniEffect::CallBackInterface), 00720284 (AUnitActionReceiver),
// size 0x9510.
//
// Raccoons (AVeggiePatchThief) come through the gates of the fence around the
// veggie patch, grab the veggies (AVeggiePatchVeggie) and run off with them.
// The player walks around the patch: he squirts the raccoons with milk
// (AAimedSquirtProjectileLauncher + AVeggieDefenderMilkProjectile), picks them
// up and throws them over the fence, and closes the gates they open. Snipers
// sometimes appear around the patch and shoot tomatoes at him (a hit costs a
// life). The game is lost when half of the veggies are stolen or all lives are
// gone, and won when the time runs out.
//-----------------------------------------------------------------------------
class AVeggiePatchDefenderGame
    : public AMiniGameState
    , public AUnitActionReceiver
{
public:
	TDECLARE_CLASS( AVeggiePatchDefenderGame, AMiniGameState );

	static constexpr TINT NUM_FENCES        = 30;
	static constexpr TINT NUM_VEGGIE_KINDS  = 5;
	static constexpr TINT NUM_THIEVES       = 10;
	static constexpr TINT NUM_SNIPERS       = 4;
	static constexpr TINT NUM_THROW_TARGETS = 5;

	static constexpr TINT HIGHSCORE_TABLE = 30;

	enum VEGGIE_ : TINT
	{
		VEGGIE_SPROUT, // just a model
		VEGGIE_CARROT,
		VEGGIE_MELON,
		VEGGIE_CABBAGE,
		VEGGIE_PUMPKIN,
	};

	enum WAVE_ : TINT
	{
		WAVE_SPAWN,   // raccoons come every fDuration
		WAVE_FLEE,    // the raccoons run away
		WAVE_SNIPERS, // the raccoon warning, snipers at the sniper locators
	};

	enum PHASE_ : TINT
	{
		PHASE_PLAYING,
		PHASE_ENDING,
		PHASE_ENDED,
	};

	// What the player is doing
	enum PLAYERSTATE_ : TINT
	{
		PLAYERSTATE_FREE,         // 0
		PLAYERSTATE_PICKUP_THIEF, // 1, turns to the raccoon, "pickup_raccoon"
		PLAYERSTATE_CARRY_THIEF,  // 2, carries it to a throw target
		PLAYERSTATE_THROW_THIEF,  // 3, "throw_raccoon"
		PLAYERSTATE_CLOSE_FENCE,  // 4, closing a gate
		PLAYERSTATE_HIT,          // 5, "yuk" / "defeat"
	};

	struct Patch;

	// A piece of the fence. Size 0x98.
	struct Fence
	{
		enum STATE_ : TINT
		{
			STATE_CLOSED,   // 0
			STATE_OPENING,  // 1, a raccoon is opening it ("fence_open")
			STATE_OPEN,     // 2
			STATE_CLOSING,  // 3, the player walks to it and closes it ("fence_close")
			STATE_PASS_IN,  // 4, a raccoon jumps in ("passfence")
			STATE_PASS_OUT, // 5, a raccoon jumps out
		};

		Fence();
		~Fence();

		// Creates the model(s) of the fence piece
		void Create( const AVeggiePatchFenceData* a_pData, Patch* a_pPatch );
		void Destroy();
		void Update( TFLOAT a_fDeltaTime, TFLOAT a_fTime );

		// Icon of the gate on the minimap, created on demand
		TINT GetMapIcon();

		void SetSkeletonUpdating( TBOOL a_bUpdating );
		void Hit();

		TBOOL Open( AVeggiePatchThief* a_pThief );
		TBOOL PassIn( AVeggiePatchThief* a_pThief );
		TBOOL PassOut( AVeggiePatchThief* a_pThief );
		TBOOL Close( AUnit* a_pPlayer );

		static TBOOL __stdcall OnCloseAnimEvent( Fence* a_pThis, void* a_pOwner, void* a_pData );

		const AVeggiePatchFenceData*  pData;            // 0x00
		Patch*                        pPatch;           // 0x04
		STATE_                        eState;           // 0x08
		AModelInstance*               pModelInstance;   // 0x0c
		AAnimatableObject*            pGate;            // 0x10, "fncveg03"
		AAnimatableObject*            pSlats;           // 0x14, "fncveg03_02"
		AAnimatableObject*            pRail;            // 0x18, "fncveg03_03"
		TINT                          iUnk1c;           // 0x1c
		Toshi::TVector4               vOutside;         // 0x20, "rac_locator"
		Toshi::TVector4               vInside;          // 0x30, "avatar_locator", on the ground
		Toshi::TVector4               vDirection;       // 0x40, from vInside to vOutside, XZ normalised
		Toshi::TVector4               vCentre;          // 0x50
		TINT                          iHealth;          // 0x60, hits before a raccoon can open it, < 0 not a gate
		AUnit*                        pUnit;            // 0x64, the raccoon passing or the player closing it
		Toshi::TGenericListener       oCloseListener;   // 0x68
		AAnimationBreakpointCallback* pCloseBreakpoint; // 0x7c, "fence_close" of the player
		TINT                          iMapIcon;         // 0x80
		TFLOAT                        fTimer;           // 0x84
		Toshi::TVector4               vPassVelocity;    // 0x88
	};

	// Where a veggie grows. Size 0x60.
	struct VeggieSlot
	    : public Toshi::T2SList<VeggieSlot>::Node
	{
		VeggieSlot() = default;

		AVeggiePatchVeggieData* pData;          // 0x04
		AVeggiePatchVeggie*     pVeggie;        // 0x08, the veggie growing there
		AModelInstance*         pModelInstance; // 0x0c
		Toshi::TMatrix44        oTransform;     // 0x10
		Toshi::TVector4         vPosition;      // 0x50
	};

	// A model of the outer fence ("fncb02"), 8 bytes
	struct FenceModel
	    : public Toshi::T2SList<FenceModel>::Node
	{
		AModelInstance* pInstance; // 0x4
	};

	// State shared with the units (AVeggiePatchThief::m_pPatch, AVeggiePatchVeggie::m_pPatch).
	// Constructed by AVeggiePatchDefenderGame::Patch::Patch. Size 0x1408.
	struct Patch
	{
		Patch( AGUI2MiniMap* a_pMap );
		~Patch();

		Toshi::TRef<AUnitProxy>            oPlayer;                        // 0x000
		PLAYERSTATE_                       ePlayerState;                   // 0x004
		AVeggiePatchThief*                 pCarriedThief;                  // 0x008, picked up by the player
		Fence*                             pNearestFence;                  // 0x00c, an open gate the player can close
		Toshi::TVector4                    vThrowDirection;                // 0x010
		TBOOL                              bPlayerHit;                     // 0x020, hit by a tomato while busy
		TINT                               iLives;                         // 0x024
		TBOOL                              bDefeated;                      // 0x028, "defeat" was played
		Toshi::TTRB                        oData;                          // 0x02c
		const AVeggiePatchGameData*        pGameData;                      // 0x16c
		const AVeggiePatchThiefParams*     pThiefParams;                   // 0x170
		const AVeggiePatchThiefAnimData*   pThiefAnimData;                 // 0x174
		const AVeggiePatchFenceData*       pFenceData;                     // 0x178
		VeggieSlot*                        apSlots[ NUM_VEGGIE_KINDS ];    // 0x17c
		Toshi::T2SList<VeggieSlot>         aSlotLists[ NUM_VEGGIE_KINDS ]; // 0x190
		Toshi::T2DList<AVeggiePatchVeggie> oVeggies;                       // 0x1a4, growing in the patch
		Toshi::T2DList<AVeggiePatchVeggie> oFallingVeggies;                // 0x1ac, dropped, fading back to their slot
		Toshi::T2DList<AVeggiePatchVeggie> oCarriedVeggies;                // 0x1b4, carried by a raccoon
		Toshi::T2DList<AVeggiePatchThief>  oFreeThieves;                   // 0x1bc
		Toshi::T2DList<AVeggiePatchThief>  oThieves;                       // 0x1c4
		AInteriorNetwork*                  pInsideNetwork;                 // 0x1cc, "veggie"
		AInteriorNetwork*                  pOutsideNetwork;                // 0x1d0, "veggie_outside"
		Toshi::TString16                   strUnk1d4;                      // 0x1d4
		TINT                               iNumVeggiesAtStart;             // 0x1e0
		TINT                               iNumVeggiesTaken;               // 0x1e4, picked up by a raccoon
		TINT                               iNumVeggiesLost;                // 0x1e8, taken out of the patch
		TINT                               iNumVeggiesSaved;               // 0x1ec, back in their slot
		TINT                               iNumMilkHits;                   // 0x1f0
		TINT                               iNumThrown;                     // 0x1f4, raccoons thrown out
		TINT                               iNumFencesClosed;               // 0x1f8
		Fence                              aFences[ NUM_FENCES ];          // 0x1fc
		TINT                               iNumGates;                      // 0x13cc
		TINT*                              pGates;                         // 0x13d0, indices of the gates in aFences
		TINT                               iMaxLost;                       // 0x13d4, half of the veggies
		TINT                               iNumInPatch;                    // 0x13d8
		TINT                               iNumFalling;                    // 0x13dc
		TINT                               iNumCarried;                    // 0x13e0
		TINT                               iNumLost;                       // 0x13e4
		TWCHAR                             wszLeft[ 8 ];                   // 0x13e8, veggies left before the game is lost
		AGUI2MiniMap*                      pMap;                           // 0x13f8
		TFLOAT                             fTime;                          // 0x13fc, pulse of the map icons
		PHASE_                             ePhase;                         // 0x1400
		TINT                               iMainGate;                      // 0x1404, "fncveg01_17"
	};

public:
	AVeggiePatchDefenderGame();
	~AVeggiePatchDefenderGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL    ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL    ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void     OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;
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

	// A tomato of a sniper has hit a_pUnit
	TBOOL OnTomatoHit( Toshi::TRef<AUnitProxy> a_pUnit );

	Patch& GetPatch() { return m_oPatch; }

private:
	void               LoadData();
	void               CreatePlayer();
	void               DestroyPlayer();
	void               SetupCamera();
	void               CreateFences();
	void               DestroyFences();
	void               CreateVeggies();
	void               DestroyVeggies();
	void               CreateThieves();
	void               DestroyThieves();
	void               CreateModels();
	void               DestroyModels();
	void               StartWave( const AVeggiePatchWaveData* a_pWave );
	void               UpdatePlayer();
	void               UpdateInteraction();
	void               HideInteraction();
	void               ShowInteraction( const Toshi::TVector4& a_rcPosition, const TWCHAR* a_wszText );
	Fence*             FindNearestOpenGate( const Toshi::TVector4& a_rcPosition, Toshi::TVector4& a_rOutPosition, TFLOAT& a_rDistance );
	AVeggiePatchThief* FindNearestMilkedThief( const Toshi::TVector4& a_rcPosition, Toshi::TVector4& a_rOutPosition, TFLOAT& a_rDistance );

#ifdef BARNYARD_COMMUNITY_PATCH
	// -devautoplay: the player squirts the raccoons, throws them out and closes the gates
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	TBOOL                          m_bUnkbc0;                            // 0x0bc0
	TINT                           m_iUnkbc4;                            // 0x0bc4
	AUnitPlayer*                   m_pPlayer;                            // 0x0bc8
	AAnimatableObject*             m_pSunglasses;                        // 0x0bcc
	ASimplePlayerControlComponent* m_pControl;                           // 0x0bd0
	Toshi::TVector4                m_aThrowTargets[ NUM_THROW_TARGETS ]; // 0x0bd4, where the player throws the raccoons
	Toshi::TVector4                m_vThrowTarget;                       // 0x0c24
	ACollisionObjectMesh           m_oGateCollision;                     // 0x0c34
	AModelInstance*                m_pGateModel;                         // 0x0c9c, "fncveggate"
	AModelInstance*                m_pArrowModel;                        // 0x0ca0
	AModelInstance*                m_pArrowModel2;                       // 0x0ca4, over the thing the player can use
	TINT                           m_aUnkca8[ 2 ];                       // 0x0ca8
	TFLOAT                         m_fOldCameraPitch;                    // 0x0cb0
	TFLOAT                         m_fOldFarClip;                        // 0x0cb4
	TBOOL                          m_bUnkcb8;                            // 0x0cb8
	TBOOL                          m_bUnkcb9;                            // 0x0cb9
	TFLOAT                         m_fSpawnTimer;                        // 0x0cbc
	Toshi::T2GUIElement            m_oAmmoRoot;                          // 0x0cc0
	Toshi::T2GUIRectangle          m_oAmmoBacking;                       // 0x0d14
	AGUI2MilkAmmoBar               m_oAmmoBar;                           // 0x0d8c
	Toshi::T2GUIElement            m_oVeggiesRoot;                       // 0x1044
	Toshi::T2GUIRectangle          m_oVeggiesIcon;                       // 0x1098, "veggies"
	Toshi::T2GUIRectangle          m_oVeggiesBacking;                    // 0x1110, "Score_Wood_Thin"
	AGUI2TextBox                   m_oVeggiesText;                       // 0x1188
	TINT                           m_iUnk1200;                           // 0x1200
	TWCHAR                         m_wszInteraction[ 128 ];              // 0x127c
	AInteractionHudElement         m_oInteractionHud;                    // 0x137c
	TINT                           m_iInteractionLine;                   // 0x188c
	ALaunchUnitAmmo                m_oAmmo;                              // 0x1890
	Patch                          m_oPatch;                             // 0x18a8
	AMilkReticleManager            m_oReticles;                          // 0x2cb0
	Toshi::T2SList<FenceModel>     m_oFenceModels;                       // 0x86d4, "fncb02"
	AGUI2MiniMap                   m_oMap;                               // 0x86d8
	AGUI2ImagePopup                m_oWarning;                           // 0x8bf8, "Raccoon_Warning"
	AGUI2LifeHud                   m_oLifeHud;                           // 0x8d50
	TFLOAT                         m_fWaveTimer;                         // 0x90f8
	TINT                           m_iWave;                              // 0x90fc
	AGameTimeFXManager*            m_pGameTimeFXManager;                 // 0x9100
	AGameTimeFX                    m_oFXFrom;                            // 0x9104
	AGameTimeFX                    m_oFXTo;                              // 0x927c
	AModelInstance*                m_pScarecrowModel;                    // 0x93f4
	ACollisionObjectMesh           m_oScarecrowCollision;                // 0x93f8
	TBOOL                          m_bRenderReticles;                    // 0x9460
	AMilkBottlePickups             m_oMilkBottles;                       // 0x9464
};
