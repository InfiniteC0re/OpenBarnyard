#pragma once
#include "Units/AUnitProxy.h"

#include <File/TTRB.h>
#include <Toshi/T2DList.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>

//-----------------------------------------------------------------------------
// State of Chasing Chicks shared by AChasingChicksMiniGame and its units
// (AChasingChicksAI, AChasingChicksChick, AChasingChicksHanna...).
// NOTE: retail has these as members of AChasingChicksMiniGame (Game at +0xbc0);
// the names of the structs are not known.
//
// Each player (the human player and one or three AI rivals) owns a pen
// (a HotSpot). Ten chicks run around the farmyard; the players catch them,
// carry them and drop them into their own pen, or throw them.
//-----------------------------------------------------------------------------

class AUnitPlayer;
class AAnimatableObject;
class AInteriorNetwork;
class AGUI2MiniMap;
class AMilkBottlePickups;
class ASimplePlayerControlComponent;
class AChasingChicksChick;
class AChasingChicksHanna;
class ASysMesh;
class ASysMaterial;
class AModelInstance;
struct AChasingChicksGame;

namespace Toshi
{
class TMesh;
}

//-----------------------------------------------------------------------------
// Data/MinigameData/ChasingChicks/Data.trb
//-----------------------------------------------------------------------------

// Tuning of the AI rivals (AChasingChicksAI), pointed to by AChasingChicksGameData
struct AChasingChicksAIData
{
	TFLOAT fUnk0;                    // 0x00
	TFLOAT fRamDistance;             // 0x04, rams (AInputCommand_Shoot) a visible rival closer than this
	TFLOAT fSquirtDistance;          // 0x08, squirts (aimed launcher) a visible rival in front closer than this
	TFLOAT fUnkC;                    // 0x0c
	TFLOAT fLeadingRate;             // 0x10, how fast the AI turns defensive while leading (per point of lead)
	TFLOAT fLeadingTime;             // 0x14, ... doubled when less time than this is left
	TFLOAT fTrailingRate;            // 0x18, how fast the AI turns aggressive while trailing (per point behind)
	TFLOAT fTrailingTime;            // 0x1c, ... doubled until this much time has elapsed
	TFLOAT fAggressiveChickDistance; // 0x20
	TFLOAT fAggressiveRivalDistance; // 0x24
	TFLOAT fChickDistance;           // 0x28, normal / low ammo
	TFLOAT fRivalDistance;           // 0x2c, normal / low ammo
	TFLOAT fDefensiveRivalDistance;  // 0x30
	TFLOAT fAttackCooldown;          // 0x34, between rams/squirts
	TINT   iPenSelection;            // 0x38, how the AI picks a pen: 1 closest, 2 random of the own pens, otherwise random
};

// "GameData": one per variant ("NumGameVariants"), 0x48 bytes
struct AChasingChicksGameData
{
	AChasingChicksAIData* pAIData;         // 0x00
	TFLOAT                fTimeLimit;      // 0x04
	TFLOAT                fPlayerStunTime; // 0x08, "yuk" time of the player when hit
	TFLOAT                fAIStunTime;     // 0x0c, "yuk" time of the AI rivals when hit
	TFLOAT                fFXStart;        // 0x10, AGameTimeFX lerp (dusk -> night) at the start
	TFLOAT                fFXEnd;          // 0x14, ... at the end

	// Kinds of chicks by score value (a chick's AChasingChicksChick::m_iValue)
	struct ChickKind
	{
		TFLOAT fPenRadius; // 0x00, AChasingChicksHotSpot::fRadius of the pens of this kind
		TINT   iValue;     // 0x04, compared with the value of a pocketed chick
		TINT   iNumOpen;   // 0x08, open pens of this kind (AChasingChicksHotSpot::iKind) at the same time
	};

	ChickKind aChickKinds[ 4 ]; // 0x18
};

TSTATICASSERT( sizeof( AChasingChicksGameData ) == 0x48 );

// "HotSpotData", how the pen markers are drawn (AChasingChicksHotSpot::PostRender)
struct AChasingChicksHotSpotData
{
	TFLOAT fFadeStart;  // 0x00, distance (XZ) to the camera at which the marker starts to appear
	TFLOAT fFadeRange;  // 0x04, ... and over which it fades in
	TFLOAT fMaxOpacity; // 0x08
	TFLOAT fScale;      // 0x0c, horizontal scale of the marker, times the pen's radius
	TFLOAT fHeight;     // 0x10, vertical scale of the marker
};

//-----------------------------------------------------------------------------
// A pen (scoring area, "Goal0".."Goal3" areas of the "chasingchicks_scoring"
// network). Size 0x54. Lives in AChasingChicksGame::aOpenPens/aClosedPens of
// its kind. Drawn as a spinning ring, a pulsing glow (in the colour of the
// kind) and the "ccd_hotspotmarker" model.
// NOTE: Chicken Coop Defender has byte-identical copies of these functions
// (AChickenCoopDefenderHotSpot::Render, PostRender, Create...), they are marked there.
//-----------------------------------------------------------------------------
struct AChasingChicksHotSpot
    : public Toshi::T2DList<AChasingChicksHotSpot>::Node
{
	enum STATE_ : TINT
	{
		STATE_IDLE,   // 0
		STATE_INSIDE, // 1, the player stands in the pen
		STATE_SCORED, // 2, a chick was thrown into the pen: flashes for a second
	};

	enum MESH_
	{
		MESH_RING,   // "HUD\Golf_Ball_Ring.tga", spins
		MESH_GLOW,   // "HUD\Context_Glow.tga", pulses
		MESH_ARROW,  // "HUD\Chick_Arrow.tga", created but not drawn by the pen
		MESH_UNUSED, // never created (its shared material neither)
		MESH_NUMOF
	};

	AChasingChicksHotSpot();
	~AChasingChicksHotSpot();

	// Sets the pen up at the area a_iArea of the scoring network (kind and size by the area's name).
	void Create( AChasingChicksGame* a_pGame, TINT a_iArea );

	// Moves the pen to the closed (TTRUE) or the open (TFALSE) pens of its kind. Returns TFALSE if it's already there.
	TBOOL SetClosed( TBOOL a_bClosed );

	void SetState( STATE_ a_eState );

	// Spins/pulses the ring and glow meshes and draws them (AChasingChicksMiniGame::OnRender through the thunk AChasingChicksHotSpot::Render).
	void Render( TFLOAT a_fDeltaTime );

	// Draws the marker model lit in the colour of the kind and counts STATE_SCORED down (AChasingChicksMiniGame::OnPostRender through the thunk AChasingChicksHotSpot::PostRender).
	void PostRender( TFLOAT a_fDeltaTime );

	// Places the minimap icon a_iIcon at the pen, visible while the player carries a chick (a_fDeltaTime isn't used).
	void Update( TFLOAT a_fDeltaTime, TINT a_iIcon );

	TINT                iKind;                  // 0x08, AChasingChicksGameData::aChickKinds, -1
	TBOOL               bClosed;                // 0x0c, TTRUE: in aClosedPens (NOTE: a TINT in retail)
	STATE_              eState;                 // 0x10
	TFLOAT              fScoredTime;            // 0x14, STATE_SCORED, from 1 to 0
	TFLOAT              fRadius;                // 0x18
	TINT                iValue;                 // 0x1c, of the chicks scored in this pen
	TINT                iArea;                  // 0x20, -1
	Toshi::TVector4     vPosition;              // 0x24
	TFLOAT              fThrowAngle;            // 0x34, pitch from the pen up to its goal (of the same name)
	TFLOAT              fSpin;                  // 0x38, MESH_RING
	AChasingChicksGame* pGame;                  // 0x3c
	ASysMesh*           apMeshes[ MESH_NUMOF ]; // 0x40
	AModelInstance*     pModel;                 // 0x50, "ccd_hotspotmarker"

	// Shared by all pens (ms_iNumInstances), one per mesh
	static ASysMaterial* ms_apMaterials[ MESH_NUMOF ]; // 0x007835a4
	static TINT          ms_iNumInstances;             // 0x007835b4
};

TSTATICASSERT( sizeof( AChasingChicksHotSpot ) == 0x54 );

//-----------------------------------------------------------------------------
// A player (human or AI). Size 0x3c. Constructed by AChasingChicksPlayer::AChasingChicksPlayer.
//-----------------------------------------------------------------------------
struct AChasingChicksPlayer
{
	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE     = 0,
		FLAGS_INTERACT = BITFLAG( 0 ), // the interact button was pressed (the AI sets it too)
		FLAGS_THROWING = BITFLAG( 2 ), // throwing the chick into pHotSpot
		FLAGS_AI       = BITFLAG( 3 ), // the AI controls the player (AUnit flag 0x200 follows it)
		FLAGS_HIT      = BITFLAG( 4 ), // hit while jumping/throwing, stunned when possible
	};

	enum STATE_ : TINT
	{
		STATE_FREE,    // 0
		STATE_PICKUP,  // 1, "pickup"
		STATE_CARRY,   // 2, holds a chick
		STATE_AIM,     // 3, turns to the centre before the throw
		STATE_THROW,   // 4, "throw" / "throw2"
		STATE_STUNNED, // 5, "yuk"
	};

	AChasingChicksPlayer();
	~AChasingChicksPlayer();

	Toshi::TRef<AUnitProxy>        oUnit;                 // 0x00
	AAnimatableObject*             pSunglasses;           // 0x04, "sunglasses1" on the head
	TFLOAT                         fUnk8;                 // 0x08
	TFLOAT                         fUnkC;                 // 0x0c
	FLAGS                          uiFlags;               // 0x10
	STATE_                         eState;                // 0x14
	TINT                           iScore;                // 0x18
	TINT                           aNumChicksOfKind[ 4 ]; // 0x1c, pocketed chicks per AChasingChicksGameData::aChickKinds
	AChasingChicksChick*           pChick;                // 0x2c, the chick held by the player
	ASimplePlayerControlComponent* pControl;              // 0x30
	TFLOAT                         fStunTime;             // 0x34, STATE_STUNNED
	AChasingChicksHotSpot*         pHotSpot;              // 0x38, the open pen the player stands in
};

TSTATICASSERT( sizeof( AChasingChicksPlayer ) == 0x3c );

//-----------------------------------------------------------------------------
// Size 0x308, AChasingChicksMiniGame + 0xbc0.
// Constructed by AChasingChicksGame::AChasingChicksGame, destructed by AChasingChicksGame::~AChasingChicksGame.
//-----------------------------------------------------------------------------
struct AChasingChicksGame
{
	static constexpr TINT NUM_CHICKS  = 10;
	static constexpr TINT MAX_PLAYERS = 4;

	// The game of the running AChasingChicksMiniGame (TNULL otherwise), set by AChasingChicksMiniGame::OnInsertion and cleared by AChasingChicksMiniGame::OnRemoval
	// NOTE: defined in AChasingChicksHotSpot.cpp
	static AChasingChicksGame* ms_pCurrent; // 0x00783664

	// Returns the catchable chick (free, fleeing or jumping) closest to a_rcPosition within a_rfRadius (minus the chick's radius),
	// a_rfRadius is set to its distance. NOTE: defined in AChasingChicksHotSpot.cpp
	static AChasingChicksChick* FindChickNear( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rfRadius );

	AChasingChicksGame( TFLOAT* a_pTimeLimit, TFLOAT* a_pElapsedTime, AGUI2MiniMap* a_pMap, AMilkBottlePickups* a_pMilkBottles );
	~AChasingChicksGame();

	// Stack of the free chicks (AChasingChicksGame::PushChick push, AChasingChicksGame::PopChick pop)
	void PushChick( const Toshi::TRef<AUnitProxy>& a_rChick );
	void PopChick();

	// Returns the unit of a player within a_rfRadius of a_rcPosition (a_riPlayer: its index, -1 if none).
	// On success a_rfRadius is set to the distance of that player. See the NOTE in the .cpp.
	AUnit* FindPlayerNear( const Toshi::TVector4& a_rcPosition, TFLOAT& a_rfRadius, TINT& a_riPlayer );

	// The closest open pen within a_fMaxDistance (XZ) of a_rcPosition, or TNULL
	AChasingChicksHotSpot* FindClosestPen( const Toshi::TVector4& a_rcPosition, TFLOAT a_fMaxDistance );

	// The first or last open pen of kind 0, else of the highest kind with open pens, or TNULL
	AChasingChicksHotSpot* PickPenOfBestKind();

	// A random open pen of a random kind. See the NOTE in the .cpp.
	AChasingChicksHotSpot* PickRandomPen();

	TINT                                  iNumFreeChicks;             // 0x000
	Toshi::TRef<AUnitProxy>               aFreeChicks[ NUM_CHICKS ];  // 0x004
	AChasingChicksPlayer                  aPlayers[ MAX_PLAYERS ];    // 0x02c
	TINT                                  iNumPlayers;                // 0x11c
	TINT                                  iNumChicks;                 // 0x120
	TINT                                  iLeader;                    // 0x124, index of the player with the highest score
	TINT                                  iRunnerUp;                  // 0x128, index of the player with the second highest score
	TFLOAT*                               pTimeLimit;                 // 0x12c
	TFLOAT*                               pElapsedTime;               // 0x130
	Toshi::TVector4                       vCentre;                    // 0x134, position of "Goal0" - 1 in Y
	Toshi::TVector4                       vCentreDir;                 // 0x144, forward of the "Goal0" AInteriorGoal
	AInteriorNetwork*                     pScoringNetwork;            // 0x154, "chasingchicks_scoring"
	AChasingChicksGameData*               pGameData;                  // 0x158
	AChasingChicksAIData*                 pAIData;                    // 0x15c, pGameData->pAIData
	AChasingChicksHotSpotData*            pHotSpotData;               // 0x160, "HotSpotData"
	Toshi::TTRB                           oData;                      // 0x164
	TINT                                  iNumSpawns;                 // 0x2a4, "spawn" goals of the network
	Toshi::TVector4*                      pSpawns;                    // 0x2a8
	AChasingChicksHotSpot*                pHotSpots;                  // 0x2ac, one per goal of the scoring network
	Toshi::T2DList<AChasingChicksHotSpot> aOpenPens[ MAX_PLAYERS ];   // 0x2b0, per kind of pen
	Toshi::T2DList<AChasingChicksHotSpot> aClosedPens[ MAX_PLAYERS ]; // 0x2d0, per kind of pen
	AGUI2MiniMap*                         pMap;                       // 0x2f0
	TINT*                                 pMapIcons;                  // 0x2f4, one per open pen
	AMilkBottlePickups*                   pMilkBottles;               // 0x2f8
	TINT                                  ePhase;                     // 0x2fc, AChasingChicksMiniGame::PHASE_ (the AI rivals don't move while it's 1..3)
	TINT                                  iUnk300;                    // 0x300
	AChasingChicksHanna*                  pHanna;                     // 0x304
};

TSTATICASSERT( sizeof( AChasingChicksGame ) == 0x308 );
