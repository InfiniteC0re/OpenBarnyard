#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TPString8.h>
#include <Toshi/T2Vector.h>
#include <File/TTRB.h>
#include <Math/TMatrix44.h>

#include "GameInterface/AHackedQuickSave.h"
#include "Units/AUnitModelStaticData.h"

class AUnit;
class AModelInstance;
class AAnimatableObject;
class AQuest;
struct ATerrainLocator;

namespace Toshi
{
class T2GUIElement;
}

//-----------------------------------------------------------------------------
// Spawns and removes the NPCs of the main game (the farm animals, the story
// characters...) as described by Data/Units/AnimalSpawn.trb. Which entries are
// spawned depends on the time of day (the spawn flags), the story progress
// (quest tiers) and the assigned quests. Also manages the "!", "?" and arrow
// markers floating above the units that have something to say.
//-----------------------------------------------------------------------------
class MODAPI AAnimalPopulationManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AAnimalPopulationManager>
{
public:
	TDECLARE_CLASS( AAnimalPopulationManager, Toshi::TTask );

	static constexpr TINT MAX_NUM_MARKERS        = 30;
	static constexpr TINT MAX_NUM_DEFERRED_SPAWN = 16;
	static constexpr TINT TRB_MEMORY_SIZE        = 0x4000;

	// Indices into the StringMappingTables symbol
	using STRINGTABLE = TINT;
	enum STRINGTABLE_ : STRINGTABLE
	{
		STRINGTABLE_UNITTYPE,    // 0x00
		STRINGTABLE_UNITNAME,    // 0x08
		STRINGTABLE_QUEST,       // 0x10
		STRINGTABLE_ANIMSET,     // 0x18
		STRINGTABLE_ANIM,        // 0x20
		STRINGTABLE_NETWORK,     // 0x28
		STRINGTABLE_AREA,        // 0x30
		STRINGTABLE_GOAL,        // 0x38
		STRINGTABLE_LOCATOR,     // 0x40
		STRINGTABLE_BEHAVIOUR,   // 0x48
		STRINGTABLE_INTERACTION, // 0x50
		STRINGTABLE_NUMOF
	};

	// The time of day (and other conditions) a spawn is valid for
	using SPAWNFLAGS = TUINT8;
	enum SPAWNFLAGS_ : SPAWNFLAGS
	{
		SPAWNFLAGS_NONE     = 0,
		SPAWNFLAGS_DAY      = BITFLAG( 0 ),
		SPAWNFLAGS_NIGHT    = BITFLAG( 1 ),
		SPAWNFLAGS_UNK2     = BITFLAG( 2 ), // set in both of the masks the game populates with
		SPAWNFLAGS_UNK3     = BITFLAG( 3 ),
		SPAWNFLAGS_MARKER   = BITFLAG( 4 ), // the spawned units get a marker
		SPAWNFLAGS_DESTROY  = BITFLAG( 5 ), // the units are destroyed when the spawn is removed
		SPAWNFLAGS_SPAWNED  = BITFLAG( 6 ),
		SPAWNFLAGS_DEFERRED = BITFLAG( 7 ), // placed by the deferred spawn list
	};

	// Parts of PlaceUnit to apply, also the fields OverrideSpawns copies
	using PLACEFLAGS = TUINT16;
	enum PLACEFLAGS_ : PLACEFLAGS
	{
		PLACEFLAGS_NONE        = 0,
		PLACEFLAGS_ANIMSET     = BITFLAG( 0 ),
		PLACEFLAGS_ANIM        = BITFLAG( 1 ), // plays the animation (or idle_lp)
		PLACEFLAGS_NETWORK     = BITFLAG( 2 ),
		PLACEFLAGS_AREA        = BITFLAG( 3 ),
		PLACEFLAGS_GOAL        = BITFLAG( 4 ),
		PLACEFLAGS_BEHAVIOUR   = BITFLAG( 5 ),
		PLACEFLAGS_INTERACTION = BITFLAG( 6 ),
		PLACEFLAGS_LOCATOR     = BITFLAG( 7 ),
		PLACEFLAGS_STATE       = BITFLAG( 8 ), // AnimalSpawn::uiState, only used by OverrideSpawns
		PLACEFLAGS_UNK9        = BITFLAG( 9 ), // checked by the deferred spawns
		PLACEFLAGS_ALL         = 0x1ff,
	};

	// State of an entry of the AnimalSpawnSet
	using SPAWNSTATE = TUINT16;
	enum SPAWNSTATE_ : SPAWNSTATE
	{
		SPAWNSTATE_DISABLED     = BITFLAG( 0 ),
		SPAWNSTATE_ENABLED      = BITFLAG( 1 ),
		SPAWNSTATE_QUESTTIER    = BITFLAG( 3 ), // the quest tier spawn of the entry is used instead
		SPAWNSTATE_QUESTSPAWNED = BITFLAG( 4 ), // the quest tier spawn has to be removed
	};

	using MARKERTYPE = TINT;
	enum MARKERTYPE_ : MARKERTYPE
	{
		MARKERTYPE_EXCLAMATION = 1,
		MARKERTYPE_QUESTION    = 2,
		MARKERTYPE_ARROW       = 4,
	};

	// The locators FindQuestLocator looks for
	using QUESTLOCATOR = TINT;
	enum QUESTLOCATOR_ : QUESTLOCATOR
	{
		QUESTLOCATOR_DEFAULT      = 0, // "ql_a<act>_default_<name>"
		QUESTLOCATOR_CONVERSATION = 1, // "ql_a<act>t<quest>_conv_<name>", rotated by 90 degrees
		QUESTLOCATOR_QUEST        = 2, // "ql_a<act>t<quest>_<name>"
	};

	// Story changes already applied to the spawns
	using STORYFLAGS = TUINT8;
	enum STORYFLAGS_ : STORYFLAGS
	{
		STORYFLAGS_NONE          = 0,
		STORYFLAGS_BEN_DEAD      = BITFLAG( 0 ), // ben disabled, ben_grave enabled
		STORYFLAGS_IGG_BUD_EDDIE = BITFLAG( 1 ), // igg, bud and eddie enabled
		STORYFLAGS_NB_BULL       = BITFLAG( 2 ), // nb_bull enabled (the barn is open at night)
		STORYFLAGS_NO_COYOTES    = BITFLAG( 3 ), // dag and generic_coyote disabled (story complete)
		STORYFLAGS_ENDING        = BITFLAG( 4 ), // end_otis and end_daisy enabled
		STORYFLAGS_NO_OTIS       = BITFLAG( 5 ), // otis disabled (story complete)
		STORYFLAGS_NO_DAISY      = BITFLAG( 6 ), // daisy disabled (story complete)
	};

	using FLAGS = TUINT;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE    = 0,
		FLAGS_MARKERS = BITFLAG( 0 ), // the markers are enabled
	};

	using MARKERFLAGS = TUINT;
	enum MARKERFLAGS_ : MARKERFLAGS
	{
		MARKERFLAGS_ACTIVE    = BITFLAG( 0 ),
		MARKERFLAGS_TEMPORARY = BITFLAG( 1 ), // removed after the next render
		MARKERFLAGS_HIDDEN    = BITFLAG( 2 ),
	};

	// Where, how many and in which state units are spawned (0x16 bytes)
	struct SpawnData
	{
		SPAWNFLAGS uiFlags;       // 0x00
		TUINT8     uiCount;       // 0x01
		TUINT8     uiAnimSet;     // 0x02, STRINGTABLE_ANIMSET
		TUINT8     uiAnim;        // 0x03, STRINGTABLE_ANIM
		TINT16     iType;         // 0x04, STRINGTABLE_UNITTYPE
		TINT16     iName;         // 0x06, STRINGTABLE_UNITNAME
		TINT16     iNetwork;      // 0x08, STRINGTABLE_NETWORK
		TINT16     iArea;         // 0x0a, STRINGTABLE_AREA
		TINT16     iGoal;         // 0x0c, STRINGTABLE_GOAL
		TINT16     iLocator;      // 0x0e, STRINGTABLE_LOCATOR
		TINT16     iBehaviour;    // 0x10, STRINGTABLE_BEHAVIOUR
		TINT16     iInteraction;  // 0x12, STRINGTABLE_INTERACTION
		TUINT16    uiMarkerFlags; // 0x14, MARKERTYPE_QUESTION, MARKERTYPE_EXCLAMATION, otherwise arrow
	};

	// Entry of the AnimalSpawnSet symbol (0x1a bytes)
	struct AnimalSpawn
	{
		SPAWNSTATE uiState;         // 0x00
		TINT16     iQuestTierSpawn; // 0x02, index into the QuestTierSpawnSet, 0 = none
		SpawnData  oSpawn;          // 0x04
	};

	// Entry of the QuestTierSpawnSet symbol (0x18 bytes), the first one is unused
	struct QuestTierSpawn
	{
		TUINT8    uiTier;  // 0x00, absolute quest tier
		TUINT8    uiQuest; // 0x01, STRINGTABLE_QUEST
		SpawnData oSpawn;  // 0x02
	};

	// Entry of the SaveableUnitSet symbol (0x4 bytes)
	struct SaveableUnit
	{
		TUINT8 uiType;          // 0x00, STRINGTABLE_UNITTYPE
		TUINT8 uiName;          // 0x01, STRINGTABLE_UNITNAME
		TUINT8 bCheckInventory; // 0x02, not saved if the player carries it
		TUINT8 uiPadding;       // 0x03
	};

	template <class T>
	struct Set
	{
		T*   pEntries;
		TINT iNumEntries;
	};

	struct StringTable
	{
		const TCHAR** ppStrings;
		TINT          iNumStrings;
	};

	// A marker floating above a unit (0x14 bytes)
	struct Marker
	{
		AUnit*      pUnit;   // 0x00
		MARKERFLAGS uiFlags; // 0x04
		MARKERTYPE  eType;   // 0x08
		Marker*     pPrev;   // 0x0c
		Marker*     pNext;   // 0x10
	};

	// A spawn whose placement waits until its unit is out of sight (0x8 bytes)
	struct DeferredSpawn
	{
		SpawnData* pSpawn;  // 0x00
		PLACEFLAGS uiFlags; // 0x04
	};

public:
	AAnimalPopulationManager();
	~AAnimalPopulationManager();

	//-----------------------------------------------------------------------------
	// Toshi::TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() override;                         // +0x14
	virtual TBOOL OnCreate() override;                      // +0x18
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) override; // +0x1c
	virtual void  OnDestroy() override {}                   // +0x24, DOES_NOTHING 0050de10

	// (Re)loads Data/Units/AnimalSpawn.trb
	void LoadSpawnData();

	// Spawns everything valid for a_uiSpawnFlags (0 keeps the current ones), a_bNPCBehaviour hands the units to ANPCBehaviourManager
	TBOOL Populate( SPAWNFLAGS a_uiSpawnFlags, TBOOL a_bNPCBehaviour );

	// Applies the spawns of the quest tier a_iTier (called by AQuestManager)
	TBOOL SetQuestTier( TINT a_iTier, TBOOL a_bSkipCompleted, SPAWNFLAGS a_uiSpawnFlags );

	// Removes the spawns of the quest tier a_iTier that belong to a_szQuest (TNULL: all of them)
	TBOOL ClearQuestTier( TINT a_iTier, const TCHAR* a_szQuest, SPAWNFLAGS a_uiSpawnFlags );

	// Spawns the quest tier spawns of the quest a_rcQuest
	TBOOL SpawnQuestUnits( const Toshi::TPString8& a_rcQuest );

	// Applies the fields of a_rcSource selected by a_uiMask to the entries of a_szType/a_szName
	void OverrideSpawns( const TCHAR* a_szType, const TCHAR* a_szName, const AnimalSpawn& a_rcSource, PLACEFLAGS a_uiMask, SPAWNFLAGS a_uiFlagsValue, SPAWNFLAGS a_uiFlagsMask );

	QuestTierSpawn* FindQuestTierSpawn( const TCHAR* a_szType, const TCHAR* a_szName, const TCHAR* a_szQuest );

	// Name of the goal the unit of the type a_szType goes to in the current quest tier
	const TCHAR* GetQuestTierGoal( const TCHAR* a_szType );

	// Collects the unit names of the quest tier spawns of a_szQuest
	void GetQuestUnitNames( const TCHAR* a_szQuest, AUnitModelStaticData::ModelNameArray& a_rNames );

	TBOOL IsSaveableUnit( const TCHAR* a_szName );

	// Markers
	void SetMarkersEnabled( TBOOL a_bEnabled );
	void SetMarker( AUnit* a_pUnit, MARKERTYPE a_eType, TBOOL a_bTemporary );
	void RemoveMarker( AUnit* a_pUnit );
	void SetMarkerActive( AUnit* a_pUnit, TBOOL a_bActive );
	void RemoveTemporaryMarkers();
	void AddMarkersToMiniMap( class ASimMiniMap* a_pMiniMap );
	void RenderMarkers( TBOOL a_bFullBright );
	void ResetExclamationView( TBOOL a_bSkip );

	// Quick save
	TBOOL SaveSpawnState( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL LoadSpawnState( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL SaveUnits( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL LoadUnits( AHackedQuickSave::Buffer* a_pBuffer );

	// $Barnyard: FUNCTION 00539d50
	Set<AnimalSpawn>* GetAnimalSpawnSet() const { return m_pAnimalSpawnSet; }

	// $Barnyard: FUNCTION 00539d60
	StringTable* GetStringMappingTables() const { return m_pStringMappingTables; }

	SPAWNFLAGS GetSpawnFlags() const { return m_uiSpawnFlags; }

	// TRUE if one of the quests of the current quest group (or a_pQuest) involves a_pUnit
	static TBOOL IsUnitInQuest( AUnit* a_pUnit, AQuest* a_pQuest, TBOOL a_bCheckTime );

	// Looks for the quest locator of a_szName, mode 0: tier default, 1: conversation, 2: quest
	static ATerrainLocator* FindQuestLocator( Toshi::TMatrix44& a_rOutTransform, QUESTLOCATOR a_iMode, const TCHAR* a_szName, TINT a_iTier, TINT a_iQuest );

	// Skin of the main characters for the current tier, -1 for the other unit types
	static TINT GetCharacterSkin( const TCHAR* a_szType );

	// Sets the animation set and animation of a spawned unit
	static void SetUnitAnimation( AUnit* a_pUnit, const TCHAR* a_szAnim, const TCHAR* a_szAnimSet, PLACEFLAGS a_uiFlags );

	class AGUI2ModelRenderer* GetExclamationView() const { return m_pExclamationView; }

private:
	const TCHAR* GetString( STRINGTABLE a_eTable, TINT a_iIndex ) const { return ( a_iIndex != 0 ) ? m_pStringMappingTables[ a_eTable ].ppStrings[ a_iIndex ] : TNULL; }

	TBOOL SpawnUnits( SpawnData* a_pSpawn, TBOOL a_bNPCBehaviour );
	TBOOL DespawnUnits( QuestTierSpawn* a_pQuestTierSpawn );
	TBOOL PlaceUnit( AUnit* a_pUnit, SpawnData* a_pSpawn, TINT a_iIndex, PLACEFLAGS a_uiFlags );
	TBOOL PrepareNPCBehaviour( AUnit* a_pUnit, SpawnData* a_pSpawn, TBOOL a_bNewUnit, TBOOL a_bForce, PLACEFLAGS a_uiFlags );
	void  UpdateDeferredSpawns( TBOOL a_bForce );
	TINT  EraseDeferredSpawn( TINT a_iIndex );

	void FreeMarker( Marker* a_pMarker );

	TBOOL SaveUnit( SaveableUnit* a_pUnit, AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL LoadUnit( AHackedQuickSave::Buffer* a_pBuffer );

	static Marker* UnlinkMarker( Marker* a_pMarker, Marker** a_ppHead );

	static void* __stdcall TRBAllocator( Toshi::TTRB::AllocType a_eAllocType, TUINT32 a_uiSize, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData );
	static void __stdcall TRBDeallocator( Toshi::TTRB::AllocType a_eAllocType, void* a_pMem, TINT16 a_iUnk1, TUINT32 a_uiUnk2, void* a_pUserData );

private:
	FLAGS                                                  m_eFlags;                      // 0x20
	TINT                                                   m_iUnk24;                      // 0x24
	SPAWNFLAGS                                             m_uiSpawnFlags;                // 0x28
	STORYFLAGS                                             m_uiStoryFlags;                // 0x29
	TINT                                                   m_iQuestGiverIcon;             // 0x2c, ASimMiniMap icon
	Toshi::TTRB                                            m_oTRB;                        // 0x30
	Set<AnimalSpawn>*                                      m_pAnimalSpawnSet;             // 0x170
	Set<QuestTierSpawn>*                                   m_pQuestTierSpawnSet;          // 0x174
	Set<SaveableUnit>*                                     m_pSaveableUnitSet;            // 0x178
	StringTable*                                           m_pStringMappingTables;        // 0x17c
	TUINT8*                                                m_pTRBMemory;                  // 0x180
	TUINT8*                                                m_pTRBMemoryCursor;            // 0x184
	Marker                                                 m_aMarkers[ MAX_NUM_MARKERS ]; // 0x188
	Marker*                                                m_pFreeMarkers;                // 0x3e0
	Marker*                                                m_pUsedMarkers;                // 0x3e4
	AModelInstance*                                        m_pExclamationModel;           // 0x3e8
	AModelInstance*                                        m_pQuestionModel;              // 0x3ec
	AModelInstance*                                        m_pArrowModel;                 // 0x3f0
	Toshi::T2Vector<DeferredSpawn, MAX_NUM_DEFERRED_SPAWN> m_vecDeferredSpawns;           // 0x3f4
	class AGUI2ModelRenderer*                              m_pExclamationView;            // 0x478, the "!" on the HUD
	AAnimatableObject*                                     m_pExclamationObject;          // 0x47c
};

TSTATICASSERT( sizeof( AAnimalPopulationManager::SpawnData ) == 0x16 );
TSTATICASSERT( sizeof( AAnimalPopulationManager::AnimalSpawn ) == 0x1a );
TSTATICASSERT( sizeof( AAnimalPopulationManager::QuestTierSpawn ) == 0x18 );
TSTATICASSERT( sizeof( AAnimalPopulationManager ) == 0x480 );
