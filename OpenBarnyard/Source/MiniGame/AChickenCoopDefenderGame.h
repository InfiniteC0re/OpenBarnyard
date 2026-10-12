#pragma once
#include "Units/AUnitProxy.h"

#include <File/TTRB.h>
#include <Toshi/T2DList.h>
#include <Toshi/T2Vector.h>
#include <Toshi/TRefCounted.h>
#include <Math/TVector4.h>
#include <Math/TMathInline.h>

//-----------------------------------------------------------------------------
// State of Chicken Coop Defender shared by AChickenCoopDefender and its units
// (AChickenCoopDefenderHen, AChickenCoopDefenderEgg, AChickenCoopDefenderThief)
// and projectiles.
// NOTE: retail has these as members of AChickenCoopDefender (Game at +0xbc0);
// the names of the structs are not known.
//-----------------------------------------------------------------------------

class AUnit;
class AInteriorNetwork;
class AGUI2MiniMap;
class AGUI2LifeHud;
class AGUI2TargetCounter;
class ASysMesh;
class ASysMaterial;
class AModelInstance;
struct AChickenCoopDefenderGame;

//-----------------------------------------------------------------------------
// Data/MinigameData/ChickenCoopDefender/Data.trb
//-----------------------------------------------------------------------------

// A wave: the number of thieves of each kind (AChickenCoopDefenderThief::KIND_) coming at once, 4 bytes
struct AChickenCoopDefenderWaveData
{
	TINT8 iNumCoyotes;  // 0x0
	TINT8 iNumRaccoons; // 0x1
	TINT8 iNumFreddies; // 0x2
	TINT8 iPadding;     // 0x3
};

// "ThiefData": one per kind of thief (AChickenCoopDefenderThief::KIND_), 0x40 bytes
struct AChickenCoopDefenderThiefData
{
	TFLOAT       fWalkAnimSpeed;      // 0x00, reference speed of walk_lp
	TFLOAT       fRunAnimSpeed;       // 0x04, run_lp
	TFLOAT       fCarryWalkAnimSpeed; // 0x08, walk_lp while carrying a hen or an egg
	TFLOAT       fCarryRunAnimSpeed;  // 0x0c, run_lp while carrying
	TFLOAT       fScaredAnimSpeed;    // 0x10, scared
	TFLOAT       fWalkSpeed;          // 0x14
	TFLOAT       fRunSpeed;           // 0x18
	TFLOAT       fCarryWalkSpeed;     // 0x1c
	TFLOAT       fCarryRunSpeed;      // 0x20
	TFLOAT       fFleeSpeed;          // 0x24, STATE_FLEE and STATE_SCARED
	TFLOAT       fUnk28;              // 0x28, unused
	TFLOAT       fUnk2c;              // 0x2c, unused
	const TCHAR* szCarryBone;         // 0x30, of the thief, the hen/egg is attached to it
	const TCHAR* szCarriedBone;       // 0x34, of the hen/egg
	TINT         iNumHits;            // 0x38, milk hits before the thief runs away instead of fleeing (-1: never)
	TINT         iPadding;            // 0x3c
};

TSTATICASSERT( sizeof( AChickenCoopDefenderThiefData ) == 0x40 );

// "GameData": one per variant ("NumGameVariants"), 0x4c bytes
struct AChickenCoopDefenderGameData
{
	TFLOAT                              fTimeLimit;    // 0x00
	TINT                                iNumToDefend;  // 0x04, hens (or eggs) in the coop at the start
	TINT                                iLives;        // 0x08, -1: no lives (no life HUD)
	const AChickenCoopDefenderWaveData* pWaves;        // 0x0c
	TINT                                iNumWaves;     // 0x10
	TFLOAT                              fWaveInterval; // 0x14
	const TCHAR*                        szCounterIcon; // 0x18, of AChickenCoopDefender::m_oCounter ("Egg", "Chick")

	// Kinds of hot spots (AChickenCoopDefenderHotSpot::iKind)
	struct HotSpotKind
	{
		TFLOAT fRadius;  // 0x00
		TINT   iValue;   // 0x04, score of a hen or an egg thrown from a hot spot of this kind
		TINT   iNumOpen; // 0x08, open hot spots of this kind at the same time
	};

	HotSpotKind aHotSpotKinds[ 4 ]; // 0x1c
};

TSTATICASSERT( sizeof( AChickenCoopDefenderGameData ) == 0x4c );

// "HotSpotData", how the hot spot markers are drawn (AChickenCoopDefenderHotSpot::PostRender)
struct AChickenCoopDefenderHotSpotData
{
	TFLOAT fFadeStart;  // 0x00, distance (XZ) to the camera at which the marker starts to appear
	TFLOAT fFadeRange;  // 0x04, ... and over which it fades in
	TFLOAT fMaxOpacity; // 0x08
	TFLOAT fScale;      // 0x0c, horizontal scale of the marker, times the hot spot's radius
	TFLOAT fHeight;     // 0x10, vertical scale of the marker
	TFLOAT fUnk14;      // 0x14, unused
	TFLOAT fUnk18;      // 0x18, unused
};

//-----------------------------------------------------------------------------
// A hot spot (area of the "chasingchicks_scoring" network) the player throws the
// hens and the eggs into the coop from. Size 0x50. Lives in
// AChickenCoopDefenderGame::aOpenHotSpots/aClosedHotSpots of its kind.
// NOTE: a copy of AChasingChicksHotSpot without its fourth mesh, the functions
// are byte-identical copies, except Create (other kinds per goal name) and
// SetClosed (closing also flashes the hot spot).
//-----------------------------------------------------------------------------
struct AChickenCoopDefenderHotSpot
    : public Toshi::T2DList<AChickenCoopDefenderHotSpot>::Node
{
	enum STATE_ : TINT
	{
		STATE_IDLE,   // 0
		STATE_INSIDE, // 1, the player stands in the hot spot
		STATE_SCORED, // 2, flashes for a second
	};

	enum MESH_
	{
		MESH_RING,  // "HUD\Golf_Ball_Ring.tga", spins
		MESH_GLOW,  // "HUD\Context_Glow.tga", pulses
		MESH_ARROW, // "HUD\Chick_Arrow.tga", created but not drawn
		MESH_NUMOF
	};

	AChickenCoopDefenderHotSpot();
	~AChickenCoopDefenderHotSpot();

	// Sets the hot spot up at the area a_iArea of the scoring network (kind and size by the area's name)
	void Create( AChickenCoopDefenderGame* a_pGame, TINT a_iArea );

	// Moves the hot spot to the closed (TTRUE, it flashes) or the open (TFALSE) hot spots of its kind.
	// Returns TFALSE if it's already there.
	TBOOL SetClosed( TBOOL a_bClosed );

	// NOTE: AChasingChicksHotSpot::SetState (0050c7f0) is called by retail for the hot spots of this game too
	void SetState( STATE_ a_eState );

	void Render( TFLOAT a_fDeltaTime );
	void PostRender( TFLOAT a_fDeltaTime );

	// Places the minimap icon a_iIcon at the hot spot, visible while the player carries a hen or an egg
	void Update( TFLOAT a_fDeltaTime, TINT a_iIcon );

	TINT                      iKind;                  // 0x08, AChickenCoopDefenderGameData::aHotSpotKinds, -1
	TBOOL                     bClosed;                // 0x0c, NOTE: a TINT in retail
	STATE_                    eState;                 // 0x10
	TFLOAT                    fScoredTime;            // 0x14, STATE_SCORED, from 1 to 0
	TFLOAT                    fRadius;                // 0x18
	TINT                      iValue;                 // 0x1c, of the hens/eggs thrown from this hot spot
	TINT                      iArea;                  // 0x20, -1
	Toshi::TVector4           vPosition;              // 0x24
	TFLOAT                    fThrowAngle;            // 0x34, pitch from the hot spot up to its goal (of the same name)
	TFLOAT                    fSpin;                  // 0x38, MESH_RING
	AChickenCoopDefenderGame* pGame;                  // 0x3c
	ASysMesh*                 apMeshes[ MESH_NUMOF ]; // 0x40
	AModelInstance*           pModel;                 // 0x4c, "ccd_hotspotmarker"

	// Shared by all hot spots (ms_iNumInstances), one per mesh
	static ASysMaterial* ms_apMaterials[ MESH_NUMOF ]; // 0x00783698
	static TINT          ms_iNumInstances;             // 0x007836a4
};

TSTATICASSERT( sizeof( AChickenCoopDefenderHotSpot ) == 0x50 );

//-----------------------------------------------------------------------------
// Size 0x544, AChickenCoopDefender + 0xbc0.
// Constructed by AChickenCoopDefenderGame::AChickenCoopDefenderGame, destructed by AChickenCoopDefenderGame::~AChickenCoopDefenderGame.
//
// The hens and the eggs are units created up front and kept in pools: a thief
// reaching the coop takes one out of its pool and carries it away. Milked, it
// drops it: the hen runs around (the loose hens), the egg bounces on the ground
// (the loose eggs). The player picks it up and throws it back into the coop
// from a hot spot, it goes back to its pool and scores.
//-----------------------------------------------------------------------------
struct AChickenCoopDefenderGame
{
	static constexpr TINT NUM_HENS         = 15;
	static constexpr TINT NUM_EGGS         = 15;
	static constexpr TINT NUM_THIEVES      = 30;
	static constexpr TINT NUM_KIND_THIEVES = 10;
	static constexpr TINT NUM_GOALS        = 3;
	static constexpr TINT NUM_KINDS        = 4;

	using UnitList15 = Toshi::T2Vector<Toshi::TRef<AUnitProxy>, 15>;
	using UnitList10 = Toshi::T2Vector<Toshi::TRef<AUnitProxy>, 10>;
	using UnitList30 = Toshi::T2Vector<Toshi::TRef<AUnitProxy>, 30>;

	// "Goal0" (the coop), "Goal1" (where the hens and eggs are thrown to), "Goal2" of the
	// "chickencoop" network, 0.5 higher. Set by AChickenCoopDefender::OnInsertion. 0x0078377c
	static Toshi::TVector4 ms_aGoals[ NUM_GOALS ];

	AChickenCoopDefenderGame( TFLOAT* a_pElapsedTime, AGUI2TargetCounter* a_pCounter, AGUI2LifeHud* a_pLifeHud, AGUI2MiniMap* a_pMap );
	~AChickenCoopDefenderGame();

	// Removes a_rUnit from a_rList (if it's there) and pushes it to a_rTo
	template <class FromList, class ToList>
	static TBOOL MoveUnit( FromList& a_rFrom, ToList& a_rTo, const Toshi::TRef<AUnitProxy>& a_rUnit );

	// The camera of the player follows the player again (after following a thrown hen/egg)
	void ResetCamera();

	TINT                                        ePhase;                       // 0x000, AChickenCoopDefender::PHASE_
	TINT                                        iUnk4;                        // 0x004
	TINT                                        iNumThieves;                  // 0x008, in oThieves
	TINT                                        iNumHensLost;                 // 0x00c
	TINT                                        iNumHensCarried;              // 0x010, taken out of the pool
	TINT                                        iUnk14;                       // 0x014
	TINT                                        iNumEggsCarried;              // 0x018, taken out of the pool
	TINT                                        iNumEggsLost;                 // 0x01c
	TINT                                        iNumRaccoonHits;              // 0x020, milk hits of the raccoons
	TINT                                        iNumHits;                     // 0x024, milk hits of the other thieves and raccoons thrown away
	TINT                                        iBonus;                       // 0x028, values of the hens/eggs thrown back into the coop
	TINT                                        iWave;                        // 0x02c
	TINT                                        iNumThievesAtCoop;            // 0x030
	TINT                                        iUnk34;                       // 0x034
	TINT                                        iLives;                       // 0x038, -1: no lives
	TINT                                        iNumLeft;                     // 0x03c, hens (or eggs) still in the coop
	AGUI2TargetCounter*                         pCounter;                     // 0x040
	AGUI2LifeHud*                               pLifeHud;                     // 0x044
	UnitList15                                  oHenPool;                     // 0x048, in the coop (hidden)
	UnitList15                                  oCarriedHens;                 // 0x088, by a thief or the player
	UnitList15                                  oLooseHens;                   // 0x0c8, running around
	UnitList15                                  oEggPool;                     // 0x108, in the coop (hidden)
	UnitList15                                  oCarriedEggs;                 // 0x148, by a thief or the player, or flying
	UnitList15                                  oLooseEggs;                   // 0x188, on the ground
	UnitList30                                  oThieves;                     // 0x1c8, spawned
	UnitList10                                  oFreeRaccoons;                // 0x244
	UnitList10                                  oRaccoons;                    // 0x270, spawned
	UnitList10                                  oFreeCoyotes;                 // 0x29c
	UnitList10                                  oCoyotes;                     // 0x2c8, spawned
	UnitList10                                  oFreeFreddies;                // 0x2f4
	UnitList10                                  oFreddies;                    // 0x320, spawned
	Toshi::TRef<AUnitProxy>                     oPlayer;                      // 0x34c
	TINT                                        ePlayerState;                 // 0x350, AChickenCoopDefender::PLAYERSTATE_
	AUnit*                                      pNearThief;                   // 0x354, a milked thief the player can pick up
	AUnit*                                      pItem;                        // 0x358, the hen/egg near the player or carried
	TINT                                        iThrowTarget;                 // 0x35c, index of s_aThrowTargets, -1
	Toshi::TVector4                             vThrowDirection;              // 0x360
	Toshi::TVector4                             vThrowFrom;                   // 0x370, "cb_rlocator" of the player
	Toshi::TVector4                             vUnk380;                      // 0x380
	AChickenCoopDefenderHotSpot*                pHotSpot;                     // 0x390, the hot spot the player stands in
	TFLOAT                                      fHitCooldown;                 // 0x394, the player can't be hit again before it's < 0
	TBOOL                                       bDefeated;                    // 0x398, "defeat" was played
	TBOOL                                       bPlayerHit;                   // 0x399, hit by a coyote while throwing
	const AChickenCoopDefenderThiefData*        pThiefData;                   // 0x39c
	const AChickenCoopDefenderGameData*         pGameData;                    // 0x3a0
	const AChickenCoopDefenderHotSpotData*      pHotSpotData;                 // 0x3a4
	Toshi::TTRB                                 oData;                        // 0x3a8
	TFLOAT*                                     pElapsedTime;                 // 0x4e8
	AGUI2MiniMap*                               pMap;                         // 0x4ec
	TFLOAT                                      fTime;                        // 0x4f0, pulse of the map icons
	AUnit*                                      pHanna;                       // 0x4f4, "chickencoop_hanna"
	AChickenCoopDefenderHotSpot*                pHotSpots;                    // 0x4f8, one per area of the scoring network
	Toshi::T2DList<AChickenCoopDefenderHotSpot> aOpenHotSpots[ NUM_KINDS ];   // 0x4fc, per kind
	Toshi::T2DList<AChickenCoopDefenderHotSpot> aClosedHotSpots[ NUM_KINDS ]; // 0x51c, per kind
	TINT*                                       pMapIcons;                    // 0x53c, one per open hot spot
	AInteriorNetwork*                           pScoringNetwork;              // 0x540, "chasingchicks_scoring"
};

TSTATICASSERT( sizeof( AChickenCoopDefenderGame ) == 0x544 );

// NOTE: engine TVector4 method of the original binary (006c7540), see AGTAStyleCameraHelper.cpp
inline void AChickenCoopDefenderNormaliseXZ( Toshi::TVector4& a_rVector )
{
	a_rVector.y = 0.0f;

	TFLOAT fMagSq = a_rVector.x * a_rVector.x + a_rVector.z * a_rVector.z;

	if ( fMagSq == 0.0f )
	{
		a_rVector.x = 0.0f;
		a_rVector.z = 1.0f;
		return;
	}

	TFLOAT fMag = Toshi::TMath::Sqrt( fMagSq );
	a_rVector.x = 1.0f / fMag * a_rVector.x;
	a_rVector.z = 1.0f / fMag * a_rVector.z;
}

// NOTE: retail finds the unit and erases the iterator without checking it was found
// (erasing the end iterator releases the last element instead); it's always there.
template <class FromList, class ToList>
TBOOL AChickenCoopDefenderGame::MoveUnit( FromList& a_rFrom, ToList& a_rTo, const Toshi::TRef<AUnitProxy>& a_rUnit )
{
	auto it = a_rFrom.Find( a_rUnit );

	if ( it == a_rFrom.End() )
		return TFALSE;

	a_rFrom.Erase( it );
	a_rTo.PushBack( a_rUnit );
	return TTRUE;
}
