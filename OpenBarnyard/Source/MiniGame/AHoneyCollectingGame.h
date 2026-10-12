#pragma once
#include "AMiniGameState.h"
#include "GUI/AGUI2MilkAmmoBar.h"
#include "GUI/AGUI2TextBox.h"
#include "GUI/AGUI2LifeHud.h"
#include "GUI/AGUI2MiniMap.h"
#include "GUI/AMilkReticleManager.h"
#include "HUD/AGUI2NewPoolPower.h"
#include "HUD/AHoneyCollectHudElement.h"
#include "HUD/AHoneyCountHudElement.h"
#include "Helpers/AMilkBottlePickups.h"
#include "Helpers/ASpline.h"

#include <File/TTRB.h>
#include <GUI/T2GUIRectangle.h>
#include <Toshi/T2DList.h>
#include <Toshi/TBitArray.h>
#include <Toshi/TEvent.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>

class AUnitPlayer;
class AAnimatableObject;
class AInteriorNetwork;
class AModelInstance;
class ABeeSwarm;
class ACollisionObjectSimple;
class ACollisionObjectMesh;
class AAnimationBreakpointAttachment;
class AInteriorArea;

//-----------------------------------------------------------------------------
// Honey Collecting (hidden minigame "AHoneyCollectingGame", "Honey"), played
// in Walnut Woods for Hanna's "HANNA Honey Collecting" quest and replayed from
// the farm (AReplayHoneyCollecting).
// TClass 00782b60 (parent AMiniGameState), vtables 0071e8b8 (AMiniGameState),
// 00722a44 (AInventoryAniEffect::CallBackInterface), size 0x8e50.
//
// The player starts hidden in a hollow stump. Three hives hang in the woods
// (the third one has the royal jelly) and four swarms of bees (ABeeSwarm)
// patrol around them. At a hive, the player turns the stick around the comb
// (AHoneyCollectHudElement) to empty it into the jar (AHoneyCountHudElement).
// A swarm that sees him chases and stings him (a life lost every few
// seconds), unless he swats it away (the jump button fills the swat bar) or
// hides in a stump. A squirt of milk disperses the bees. With the jar full
// (or 5 combs) he takes the honey back to the entrance of the woods.
//-----------------------------------------------------------------------------
class AHoneyCollectingGame
    : public AMiniGameState
{
public:
	TDECLARE_CLASS( AHoneyCollectingGame, AMiniGameState );

	static constexpr TINT MAX_HIVES        = 3;
	static constexpr TINT NUM_SWARMS       = 4;
	static constexpr TINT MAX_FLOWER_BEDS  = 10;
	static constexpr TINT MAX_HIDING_SPOTS = 8;
	static constexpr TINT MAX_LIVES        = 5;
	static constexpr TINT HONEY_TO_WIN     = 5;
	static constexpr TINT HIVE_HONEY       = 5;
	static constexpr TINT ROYAL_HIVE_HONEY = 6;

	// What the action button does (m_eAction)
	enum ACTION : TINT
	{
		ACTION_NONE,
		ACTION_COLLECT, // in front of a hive
		ACTION_HIDE,    // next to a hiding stump
	};

	// The hint shown at the bottom of the screen (m_eHint)
	enum HINT : TINT
	{
		HINT_NONE,
		HINT_SWAT,    // 1, locale 544
		HINT_HIDE,    // 2, locale 547
		HINT_UNHIDE,  // 3, locale 543 (while hiding)
		HINT_COLLECT, // 4, locale 548
		HINT_RUN,     // 5, locale 546
		HINT_UNUSED6,
		HINT_STOP,  // 7, locale 542 (while collecting)
		HINT_ANGRY, // 8, locale 556
	};

	// A hive ("beehive_N" locator, not "stump"), 0x3c bytes
	struct Hive
	{
		// $Barnyard: FUNCTION 0048bed0
		Hive()
		    : pHive( TNULL )
		    , pEmptyHive( TNULL )
		    , pCollision( TNULL )
		    , iHoney( 0 )
		    , iUnk1c( 0 )
		    , iUnk20( 0 )
		    , pSwarm( TNULL )
		    , bRoyalJelly( TFALSE )
		    , iUnk2c( 0 )
		    , iUnk30( -1 )
		    , pBee( TNULL )
		    , iMapIcon( -1 )
		{}

		// $Barnyard: FUNCTION 0048c020
		~Hive() {}

		AAnimatableObject*      pHive;       // 0x00, "beehive"
		AAnimatableObject*      pEmptyHive;  // 0x04, "beehive_empty"
		ACollisionObjectSimple* pCollision;  // 0x08
		TINT                    iHoney;      // 0x0c
		Toshi::TBitArray        oSections;   // 0x10, of the comb (AHoneyCollectHudElement)
		TINT                    iUnk1c;      // 0x1c, set when the hive has a swarm, never read
		TINT                    iUnk20;      // 0x20
		ABeeSwarm*              pSwarm;      // 0x24, the swarm of the hive (see OnInsertion)
		TBOOL                   bRoyalJelly; // 0x28
		TINT                    iUnk2c;      // 0x2c
		TINT                    iUnk30;      // 0x30
		AAnimatableObject*      pBee;        // 0x34, "beehive_bee", flies away when the player comes
		TINT                    iMapIcon;    // 0x38
	};

	// A flower bed ("BeeFlowerBed" locator), 0x14 bytes
	struct FlowerBed
	{
		AModelInstance* pModel;    // 0x00, "data\models\hc_beeflowerbed"
		Toshi::TVector4 vPosition; // 0x04
	};

	// A stump the player hides in ("P_hiding_" locator), 0x1c bytes
	struct HidingSpot
	{
		AModelInstance*       pModel;     // 0x00, "data\models\hc_p_hiding"
		ACollisionObjectMesh* pCollision; // 0x04
		Toshi::TVector4       vPosition;  // 0x08
		TINT                  iMapIcon;   // 0x18
	};

	// The patrol of a swarm (Data\MiniGameData\HoneyCollecting\beepatrolN.trb), 0x150 bytes
	struct BeePatrol
	{
		BeePatrol( const TCHAR* a_szFileName, const TCHAR* a_szSymbol );
		~BeePatrol();

		Toshi::TTRB oTRB;    // 0x000
		ASpline     oSpline; // 0x140
	};

	// The milk hit listener of a launcher of the player, 0x20 bytes
	struct MilkHitListener
	    : public Toshi::T2DList<MilkHitListener>::Node
	{
		MilkHitListener( Toshi::TGenericEmitter& a_rEmitter, void* a_pCaller, Toshi::TGenericListener::t_Callback a_fnCallback, TINT a_iPriority );
		~MilkHitListener();

		Toshi::TGenericListener oListener; // 0x08
		Toshi::TGenericEmitter* pEmitter;  // 0x1c
	};

	// The milk hit listeners, 0x10 bytes
	struct MilkHitListeners
	{
		MilkHitListeners( AHoneyCollectingGame* a_pOwner, Toshi::TGenericListener::t_Callback a_fnCallback );
		~MilkHitListeners();

		void Add( Toshi::TGenericEmitter& a_rEmitter, TINT a_iPriority );
		void RemoveAll();

		AHoneyCollectingGame*               pOwner;     // 0x00
		Toshi::TGenericListener::t_Callback fnCallback; // 0x04
		Toshi::T2DList<MilkHitListener>     oList;      // 0x08
	};

public:
	AHoneyCollectingGame();
	~AHoneyCollectingGame();

	//-----------------------------------------------------------------------------
	// AGameState
	//-----------------------------------------------------------------------------
	virtual TBOOL ProcessInput( const Toshi::TInputInterface::InputEvent* a_pInputEvent ) OVERRIDE;
	virtual TBOOL ProcessCommand( AInputCommand a_eInputCommand, const Toshi::TInputInterface::InputEvent* a_pInputEvent, TBOOL& a_rStopEvents ) OVERRIDE;
	virtual void  OnRender( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnPause() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;
	virtual void  OnInsertion() OVERRIDE;
	virtual void  OnRemoval() OVERRIDE;
	virtual void  OnActivate() OVERRIDE;
	virtual void  OnDeactivate() OVERRIDE;

	//-----------------------------------------------------------------------------
	// AMiniGameState
	//-----------------------------------------------------------------------------
	virtual void Pause() OVERRIDE;                             // +0x68, the intro
	virtual void PlayLoseSequence() OVERRIDE;                  // +0x6c, the defeat sequence
	virtual void PlayWinSequence() OVERRIDE;                   // +0x70, the victory sequence
	virtual void OnPreStart() OVERRIDE;                        // +0x74
	virtual void UpdateAlways( TFLOAT a_fDeltaTime ) OVERRIDE; // +0x84
	virtual void UpdateGame( TFLOAT a_fDeltaTime ) OVERRIDE;   // +0x88
	virtual S1*  GetResults() OVERRIDE;                        // +0x94
	virtual void OnStartHudFinished() OVERRIDE;                // +0x9c

	// Raises the honey of the jar (AHoneyCollectHudElement)
	void AddJarHoney( TFLOAT a_fHeight );

	TBOOL IsHiding() const { return m_bHiding; }
	TBOOL IsDefeated() const { return m_bDefeated; }
	void  ResetStingTime() { m_fStingInterval = 0.0f; }

private:
	// Spawns the swarms and their minimap icons
	void CreateSwarms();

	// Starts collecting the honey of the hive m_iHive (the player stands in front of it)
	void StartCollecting();

	// Stops collecting the honey. a_bInterrupted plays "idle_lp" instead of "honey_end"
	void StopCollecting( TBOOL a_bInterrupted );

	// Updates the hint at the bottom of the screen
	void UpdateHint();

	// Milk hit a swarm
	static TBOOL __stdcall OnMilkHit( AHoneyCollectingGame* a_pCaller, void* a_pOwner, Toshi::TRef<class AUnitProxy>* a_pUnit );

#ifdef BARNYARD_COMMUNITY_PATCH
	// Development: "-devautoplay"
	void DevAutoPlay( TFLOAT a_fDeltaTime );
#endif

private:
	TFLOAT                          m_fTime;                            // 0x0bac
	TBOOL                           m_bUnkbb0;                          // 0x0bb0
	TINT                            m_iUnkbb4;                          // 0x0bb4, 1 becomes 2 on activation
	TINT                            m_aUnkbb8[ 5 ];                     // 0x0bb8, released by TRef::Release, never used
	Hive                            m_aHives[ MAX_HIVES ];              // 0x0bcc
	ABeeSwarm*                      m_apSwarms[ NUM_SWARMS ];           // 0x0c80
	AInteriorNetwork*               m_pNetwork;                         // 0x0c90, "woods"
	TBOOL                           m_bCollecting;                      // 0x0c94
	AHoneyCollectHudElement         m_oCollectHud;                      // 0x0c98
	AHoneyCountHudElement           m_oCountHud;                        // 0x1330
	TINT                            m_iNumCombs;                        // 0x16d4, combs emptied
	TINT                            m_iUnk16d8;                         // 0x16d8
	TINT                            m_iUnk16dc;                         // 0x16dc
	AUnitPlayer*                    m_pPlayer;                          // 0x16e0, "player_hc"
	AAnimatableObject*              m_pSunglasses;                      // 0x16e4
	Toshi::TVector4                 m_avHivePositions[ MAX_HIVES ];     // 0x16e8
	Toshi::TVector4                 m_avHiveDirections[ MAX_HIVES ];    // 0x1718
	TINT                            m_iHive;                            // 0x1748, the hive in front of the player (-1 if none)
	TINT                            m_iNumStinging;                     // 0x174c
	TINT                            m_iNumAngry;                        // 0x1750
	AModelInstance*                 m_pArrow;                           // 0x1754, "data\models\honey_arrow"
	AGUI2TextBox                    m_oRoyalJellyText;                  // 0x1758, locale 553
	AGUI2TextBox                    m_oLeaveText;                       // 0x17d0, locale 554
	TINT                            m_iUnk1848;                         // 0x1848
	TINT                            m_iUnk184c;                         // 0x184c, not initialised
	TBOOL                           m_bWalkingToHive;                   // 0x1850
	TBOOL                           m_bLeavingHive;                     // 0x1851
	TBOOL                           m_bHiding;                          // 0x1852
	TBOOL                           m_bLeavingHide;                     // 0x1853
	Toshi::TVector4                 m_vHivePosition;                    // 0x1854, where the player stands to collect
	Toshi::TVector4                 m_vHiveDirection;                   // 0x1864
	AGUI2TextBox                    m_oHint;                            // 0x1874
	ACTION                          m_eAction;                          // 0x18ec
	HINT                            m_eHint;                            // 0x18f0
	TINT                            m_iNumAttacking;                    // 0x18f4
	Toshi::T2GUIFont*               m_pHintFont;                        // 0x18f8, "Rekord18"
	TINT                            m_iUnk18fc;                         // 0x18fc
	TINT                            m_aUnk1900[ 8 ];                    // 0x1900
	TINT                            m_iNumFlowerBeds;                   // 0x1920
	FlowerBed                       m_aFlowerBeds[ MAX_FLOWER_BEDS ];   // 0x1924
	TINT                            m_iNumHidingSpots;                  // 0x19ec
	HidingSpot                      m_aHidingSpots[ MAX_HIDING_SPOTS ]; // 0x19f0
	TINT                            m_iUnk1ad0;                         // 0x1ad0
	TINT                            m_aUnk1ad4[ 8 ];                    // 0x1ad4
	TFLOAT                          m_fStingTime;                       // 0x1af4, since the last sting
	TFLOAT                          m_fStingInterval;                   // 0x1af8
	TINT                            m_iCollectCue;                      // 0x1afc
	TBOOL                           m_bDefeated;                        // 0x1b00
	TINT                            m_iSectionsAtStart;                 // 0x1b04
	TINT                            m_iCombsAtStart;                    // 0x1b08
	AAnimationBreakpointAttachment* m_pHoneyGlob;                       // 0x1b0c, "honeyglob" of the "honey" animations
	TBOOL                           m_bWon;                             // 0x1b10, the jar is full
	TFLOAT                          m_fOldNear;                         // 0x1b14
	TINT                            m_iHidingSpot;                      // 0x1b18, the closest one (-1 if none)
	AInteriorArea*                  m_pInteractionArea;                 // 0x1b1c, the area of the player last frame, not initialised
	TWCHAR                          m_wszLeaveText[ 128 ];              // 0x1b20, locale 549
	TBOOL                           m_bAtExit;                          // 0x1c20
	Toshi::T2GUIRectangle           m_oUnkRectangle;                    // 0x1c24, never used
	TINT                            m_iNumHives;                        // 0x1c9c
	TINT                            m_iUnk1ca0;                         // 0x1ca0
	BeePatrol                       m_aPatrols[ NUM_SWARMS ];           // 0x1ca4
	AGUI2TextBox                    m_oUnkText;                         // 0x21e4, fades out in UpdateAlways
	TINT                            m_aUnk225c[ 16 ];                   // 0x225c
	TFLOAT                          m_fUnkTextTime;                     // 0x229c
	AGUI2MiniMap::Viewport          m_oMapViewport;                     // 0x22a0
	AGUI2MiniMap                    m_oMap;                             // 0x22b0
	MilkHitListeners                m_oMilkHitListeners;                // 0x27d0
	AMilkReticleManager             m_oReticles;                        // 0x27e0
	TINT                            m_iUnk8204;                         // 0x8204, -1, a rumble
	TFLOAT                          m_fCombX;                           // 0x8208, where the mouse cursor is held
	TFLOAT                          m_fCombY;                           // 0x820c
	TINT                            m_iDustPuffEffect;                  // 0x8210, "dustpuff"
	TBOOL                           m_bRenderReticles;                  // 0x8214
	AMilkBottlePickups              m_oMilkBottles;                     // 0x8218
	AGUI2LifeHud                    m_oLifeHud;                         // 0x82c4
	TINT                            m_iLives;                           // 0x868c
	TINT                            m_iNumEmptiedHives;                 // 0x8690
	TBOOL                           m_bRoyalJelly;                      // 0x8694, found
	Toshi::TVector4                 m_vHideExitPosition;                // 0x8698
	Toshi::TVector4                 m_vHideExitDirection;               // 0x86a8
	Toshi::TVector2                 m_vStick;                           // 0x86b8, while collecting
	Toshi::TVector2                 m_vKeys;                            // 0x86c0, while collecting
	Toshi::TVector2                 m_vLastKeys;                        // 0x86c8
	Toshi::TVector2                 m_vSmoothKeys;                      // 0x86d0
	AGUI2MilkAmmoBar                m_oAmmoBar;                         // 0x86d8
	TBOOL                           m_bMoveCamera;                      // 0x8970, the camera moves to the hive
	AGUI2NewPoolPower               m_oSwatBar;                         // 0x8974
	Toshi::TPString8                m_strWorldHoney;                    // 0x8e4c, "worldhoney"
};

TSTATICASSERT( sizeof( AHoneyCollectingGame::Hive ) == 0x3c );
TSTATICASSERT( sizeof( AHoneyCollectingGame::FlowerBed ) == 0x14 );
TSTATICASSERT( sizeof( AHoneyCollectingGame::HidingSpot ) == 0x1c );
TSTATICASSERT( sizeof( AHoneyCollectingGame::BeePatrol ) == 0x150 );
TSTATICASSERT( sizeof( AHoneyCollectingGame::MilkHitListener ) == 0x20 );
TSTATICASSERT( sizeof( AHoneyCollectingGame::MilkHitListeners ) == 0x10 );
TSTATICASSERT( sizeof( AHoneyCollectingGame ) == 0x8e50 );
