#pragma once
#include "FSM/AFunctionTable.h"
#include "GameInterface/AHackedQuickSave.h"

#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2SList.h>
#include <Toshi/TPString8.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Math/TMatrix44.h>
#include <File/TTRB.h>

class AInteriorArea;
class ATerrainLocatorList;
class AParticleEffectHandle;

//-----------------------------------------------------------------------------
// Purpose: The things buried in the farm that the player can dig up with the
// treasure hunt micro game (ATreasueHuntMicroGame): bucks (TYPE_FLOWERS), tin
// cans, gopher traps and the leaking pipes of the "Miles Raccoon Sabotage"
// quests (TYPE_LEAK, fixed with the action button, see
// APlayerControlComponent::FixLeak). The treasures respawn in the areas of the
// "flowers", "cans" and "traps" interior networks over time (see
// Data/Treasure/Treasure.trb).
// TClass 0x007839f0 (version 1.0, size 0x1598), singleton at 0x007839ec,
// vtable 0x00723600.
//-----------------------------------------------------------------------------
class ATreasureManager
    : public Toshi::TObject
    , public Toshi::TSingleton<ATreasureManager>
{
public:
	TDECLARE_CLASS( ATreasureManager, Toshi::TObject );

	using TYPE = TINT;
	enum TYPE_ : TYPE
	{
		TYPE_FLOWERS = 0, // bucks
		TYPE_CANS    = 1, // tin cans
		TYPE_TRAPS   = 2, // gopher traps
		TYPE_LEAK    = 3, // leaking pipes of the sabotage quests
		TYPE_NONE    = 4, // free slot
		TYPE_NUMOF   = TYPE_NONE,
	};

	// Which sabotage quest the leaks are of (m_iSabotageStage)
	using SABOTAGE = TINT;
	enum SABOTAGE_ : SABOTAGE
	{
		SABOTAGE_NONE   = 0,
		SABOTAGE_FIRST  = 1, // "Miles Raccoon Sabotage", the "leakNNB..." leaks
		SABOTAGE_SECOND = 2, // "Miles Raccoon Sabotage 2"
	};

	static constexpr TINT MAX_NUM_TREASURES = 180;
	static constexpr TINT MAX_NUM_PATCHES   = 5;

	// Bit of m_uiEnabledTypes set while the leaks of the sabotage quests are in the world
	static constexpr TUINT8 FLAG_LEAKS_ACTIVE = BITFLAG( 3 );

	struct Treasure
	    : public Toshi::T2SList<Treasure>::Node
	{
		// Number of bucks the treasure gives, more the longer it's been buried
		TINT GetNumBucks();

		Toshi::TVector4 vecPosition; // 0x04
		TFLOAT          fDugTime;    // 0x14, game time the treasure was dug up (TREASURE_NEVER_DUG if never)
		TINT8           eType;       // 0x18, TYPE
		TINT8           iLeakId;     // 0x19, number of the leak (TYPE_LEAK)
	};

	// Limits of the treasures of a type in an area, sorted by m_iMinRegrowths in descending order.
	struct Patch
	{
		TINT iMaxTreasures; // 0x0, number of treasures the area can have...
		TINT iMinRegrowths; // 0x4, ...when it has more than this number of regrowth objects of the type
	};

	struct PatchList
	{
		void Clear();

		TINT  iNumPatches;                 // 0x00
		Patch aPatches[ MAX_NUM_PATCHES ]; // 0x04
	};

	// Settings of a treasure type ("Other" of Treasure.trb)
	struct TypeSettings
	{
		TFLOAT fNumPerDay;  // 0x0, treasures added per game day (0.0f: added once)
		TINT   iMaxAdded;   // 0x4
		TINT   iMaxInWorld; // 0x8
	};

public:
	ATreasureManager();
	~ATreasureManager();

	virtual AFunctionTable* GetFunctionTable(); // +0x0c

	// Fills the free list, called once after the manager is created
	TBOOL Initialise();

	// Starts placing the treasures (loads the settings)
	void Enable();

	// Removes every treasure and stops placing them
	void Disable();

	void Update( TFLOAT a_fDeltaTime );

	TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer );

	// Adds a treasure of a_eType at a_rcPosition if the limits of the type allow it
	void AddTreasure( const Toshi::TVector4& a_rcPosition, TYPE a_eType );

	// Adds up to a_iCount treasures to random areas of the interior network a_rcNetwork.
	// a_uiRegrowthType is the regrowth object type (ARegrowthManager) the limits of the patches count, 0xff to use the number in the area's name.
	void AddTreasuresToNetwork( const Toshi::TPString8& a_rcNetwork, TFLOAT a_fHeightOffset, TINT a_iCount, TUINT8 a_uiRegrowthType, TYPE a_eType );

	// The closest treasure that can be dug up within a_fRadius (on the XZ plane) of a_rcPosition
	Treasure* FindClosestTreasure( const Toshi::TVector4& a_rcPosition, TFLOAT a_fRadius );

	// The closest treasure of a_eType (TYPE_NONE: any) in front of a_rcPosition (a_rcDirection is the XZ direction)
	Treasure* FindTreasureInFront( const Toshi::TVector4& a_rcPosition, const Toshi::TVector2& a_rcDirection, TFLOAT a_fMaxDistance, TYPE a_eType );

	// Moves a_pTreasure to the free list (or fixes the leak)
	TBOOL RemoveTreasure( Treasure* a_pTreasure );
	void  RemoveAllOfType( TYPE a_eType );

	TINT CountType( TYPE a_eType );
	TINT CountInArea( AInteriorArea* a_pArea, TYPE a_eType );

	void SetTypeEnabled( TYPE a_eType, TBOOL a_bEnabled );

	// FSM natives
	void AddTreasureFromScript( const Toshi::TPString8& a_rcNetwork, TINT a_iCount, const Toshi::TPString8& a_rcRegrowthType, TINT a_eType );
	void AddGopherTraps();
	void RemoveGopherTraps();

	// Adds the treasures that respawned since the last call (ATreasueHuntMicroGame calls it every frame)
	void UpdateRespawns();

	TBOOL    IsEnabled() const { return m_bEnabled; }
	TINT     GetNumAdded( TYPE a_eType ) const { return m_aiNumAdded[ a_eType ]; }
	SABOTAGE GetSabotageStage() const { return m_iSabotageStage; }

private:
	// Clears the lists and puts every treasure to the free list
	void Reset();

	TBOOL LoadSettings();

	// Adds the treasures of the locators called a_szPrefix* (multiplied by a_rcMatrix)
	void AddTreasuresFromLocators( ATerrainLocatorList* a_pLocatorList, const Toshi::TMatrix44& a_rcMatrix, TYPE a_eType, const TCHAR* a_szPrefix );
	void AddTreasuresFromLocators( ATerrainLocatorList* a_pLocatorList, TYPE a_eType, const TCHAR* a_szPrefix );

	// Adds the leaks from the locators of the terrain
	void AddLeaks();

public:
	static constexpr TFLOAT TREASURE_NEVER_DUG = -100000000.0f;

	// DAT_00783a20
	static AFunctionTable ms_oFunctionTable;

private:
	Treasure                 m_aTreasures[ MAX_NUM_TREASURES ]; // 0x0004
	Toshi::T2SList<Treasure> m_llFreeTreasures;                 // 0x13b4
	Toshi::T2SList<Treasure> m_llTreasures;                     // 0x13b8
	TBOOL                    m_bEnabled;                        // 0x13bc
	TFLOAT                   m_fRespawnTime;                    // 0x13c0, game time before a dug up treasure can be found again
	TINT                     m_aiNumAdded[ TYPE_NUMOF ];        // 0x13c4
	TFLOAT                   m_afLastSpawnTime[ TYPE_NUMOF ];   // 0x13d4, in seconds since the first day
	TFLOAT                   m_fQuestCheckTimer;                // 0x13e4
	SABOTAGE                 m_iSabotageStage;                  // 0x13e8
	TINT                     m_iCurrentLeak;                    // 0x13ec, the leak that has the particle effect
	AParticleEffectHandle*   m_pLeakEffect;                     // 0x13f0
	PatchList                m_oPatches;                        // 0x13f4
	TypeSettings             m_aTypeSettings[ TYPE_NUMOF ];     // 0x1420
	Toshi::TTRB              m_oTRB;                            // 0x1450
	TUINT8                   m_uiEnabledTypes;                  // 0x1590, bit per TYPE + FLAG_LEAKS_ACTIVE
	TINT                     m_iLeakEffect;                     // 0x1594
};
