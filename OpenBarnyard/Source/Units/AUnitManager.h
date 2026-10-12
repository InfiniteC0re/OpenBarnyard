#pragma once
#include <Toshi/TTask.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TNodeList.h>
#include <Toshi/T2SList.h>
#include <Toshi/TPString8.h>
#include <Toshi/TEvent.h>
#include <Toshi/TGlobalEmitter.h>
#include <Toshi/TRefCounted.h>
#include <Core/TMemory.h>
#include <File/TTRB.h>
#include <Math/TVector4.h>

#include "AUnitProxy.h"
#include "AUnitHash.h"
#include "GameInterface/AHackedQuickSave.h"

class AUnit;
class AUnitType;
class AUnitFactory;
class AUnitTransform;
class AUIDMap;
class AUnitComponentManagerList;
class AFunctionTable;
class AInteriorNetwork;
class AInteriorGoal;
class AGoalCollisionUpdateInterface;
struct AUnitRef;

// Thrown through TGlobalEmitter right before a unit's OnDestroy.
// Listener list lives at 0x0077e364.
struct AUnitDestroyEvent
{
	AUnit* pUnit;
};

class AUnitManager
    : public Toshi::TTask
    , public Toshi::TSingleton<AUnitManager>
{
public:
	TDECLARE_CLASS( AUnitManager, Toshi::TTask );

	// Number of per-category unit lists. Purpose of the split is still unclear.
	static constexpr TINT NUM_UNIT_LISTS = 26;

	// Depth of the saved-memory-block stack used while spawning pooled units.
	static constexpr TINT MEM_BLOCK_STACK_SIZE = 32;

	// One preallocated slot of a pooled unit type: the memory block the unit is
	// constructed in and the unit currently occupying it.
	struct UnitPoolEntry : public Toshi::T2SList<UnitPoolEntry>::Node
	{
		Toshi::TMemory::MemBlock* pMemBlock; // 0x04
		AUnit*                    pUnit;     // 0x08, set to 1 while the unit is being created
	};

	// Per pooled unit type record. Indexed by the type's symbol in m_oUnitTypesTRB.
	struct UnitPool
	{
		// $Barnyard: FUNCTION 0040f5b0
		~UnitPool() {}

		void           Destroy();
		UnitPoolEntry* FindUsedEntry( AUnit* a_pUnit );

		// Oldest third of the used list: a visible unit far enough from the camera to recycle.
		UnitPoolEntry* FindStealCandidate();

		TINT                          iNumEntries; // 0x00
		Toshi::T2SList<UnitPoolEntry> llFree;      // 0x04
		Toshi::T2SList<UnitPoolEntry> llUsed;      // 0x08
		UnitPoolEntry*                pEntries;    // 0x0c
	};

public:
	AUnitManager();
	~AUnitManager();

	//-----------------------------------------------------------------------------
	// TTask
	//-----------------------------------------------------------------------------
	virtual TBOOL Reset() OVERRIDE;
	virtual TBOOL OnCreate() OVERRIDE;
	virtual TBOOL OnUpdate( TFLOAT a_fDeltaTime ) OVERRIDE;

	//-----------------------------------------------------------------------------
	// Own Methods
	//-----------------------------------------------------------------------------
	virtual AFunctionTable* GetFunctionTable(); // +0x38

	AUnitType* FindUnit( const TCHAR* a_szName );
	AUnit*     CreateUnit( AUnitType* a_pType, AUnitTransform* a_pTransform );

	AUnit* SpawnUnit( const TCHAR* a_szTypeName, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );
	AUnit* SpawnUnit( const Toshi::TPString8& a_rcTypeName, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );
	AUnit* SpawnUnit( const Toshi::TPString8& a_rcTypeName, AUnitTransform* a_pTransform );
	AUnit* SpawnUnit( AUnitType* a_pType, const Toshi::TVector4& a_rcPosition, const Toshi::TVector4& a_rcDirection );

	AUnit* SpawnUnitAtGoal( AUnitType* a_pType, AUnitTransform* a_pTransform, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface );
	AUnit* SpawnUnitAtGoal( const Toshi::TPString8& a_rcTypeName, AUnitTransform* a_pTransform, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface );
	AUnit* SpawnUnitAtGoal( const Toshi::TPString8& a_rcTypeName, const Toshi::TVector4& a_rcOffset, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface );
	AUnit* SpawnUnitAtGoal( const Toshi::TPString8& a_rcTypeName, AInteriorNetwork* a_pNetwork, AInteriorGoal* a_pGoal, AGoalCollisionUpdateInterface* a_pInterface );

	// Recycles the unit to its pool or destroys it (FSM "DestroyUnit").
	void DestroyUnit( AUnit* a_pUnit );

	// FSM natives
	void   DestroyUnitNextFrame( AUnit* a_pUnit );
	AUnit* FindUnitByTypeName( const Toshi::TPString8& a_rcTypeName );

	// Thrown with the AUnit** of every unit that has been created / with the type of the removal
	Toshi::TGenericEmitter& GetUnitCreatedEmitter() { return m_oUnitCreatedEmitter; }
	Toshi::TGenericEmitter& GetUnitsRemovedEmitter() { return m_oUnitsRemovedEmitter; }
	void                    SetUnitActive( AUnit* a_pUnit, TBOOL a_bActive );
	void                    DestroyUnitsOfType( const Toshi::TPString8& a_rcTypeName );

	AUnitHash::QueryResult* FindUnits( const Toshi::TVector4& a_rcPos, TFLOAT a_fRadius, TINT* a_pNumFound, TBOOL a_b3D, TINT a_iMaxNum, TBOOL a_bAllUnits, AUnitHash::FilterFn a_fnFilter );

	AUnitHash* GetUnitHash() const { return m_pUnitHash; }

	AUnit* FindClosestUnit( const Toshi::TVector4& a_rcPos, AUnitType* a_pType, TFLOAT a_fRadius, AUnit* a_pExclude );
	AUnit* FindNextUnitOfType( AUnitType* a_pType, Toshi::TNodeList<AUnit>::Iterator& a_rIt );
	AUnit* FindUnitById( TINT a_iId );
	AUnit* FindActorBySSeqId( TINT a_iId );
	TINT   GetTypeInstanceCount( const TCHAR* a_szTypeName );

	TBOOL ClearUIDMap();
	void  UnloadLibrary( const TCHAR* a_szLibName );

	// Marks/unmarks a pooled unit's slot as used (eligible to be stolen).
	void SetUnitPoolUsed( AUnit* a_pUnit, TBOOL a_bUsed );

	// FSM save data
	void SaveUnitRef( AHackedQuickSave::Buffer* a_pWriter, const AUnitRef* a_pRef );
	void LoadUnitRef( AHackedQuickSave::Buffer* a_pReader, AUnitRef* a_pRef );

	// Updates every unit flagged FLAGS_EXTERNAL_UPDATE.
	void Update( TFLOAT a_fDeltaTime );

	// Resets the AUnitHash query stamp of every live unit (when the stamp wraps).
	void SetUnitsQueryStamp( TUINT a_uiStamp );

	Toshi::TNodeList<AUnit>& GetUnits() { return m_oUnits; }

	// Iterators of m_oUnits (not inlined in ABarnExploreMinigameState::OnInsertion)
	Toshi::TNodeList<AUnit>::Iterator BeginUnits();
	Toshi::TNodeList<AUnit>::Iterator EndUnits();

	// Frees every unit flagged as dying, called at the end of OnUpdate (and by minigames on removal)
	void DeleteDyingUnits();

	void          RemoveAllUnits();
	AUnitFactory* GetUnitFactory() const { return m_pUnitFactory; }

private:
	// NOTE: compiled to a folded RETURNS_TRUE stub (0046cde0) called at the end of OnCreate
	TBOOL OnCreateUnk() { return TTRUE; }

	UnitPool* GetUnitPool( void* a_pTypeSymbol ) const
	{
		return &m_pUnitTypeRecords[ ( TINT( a_pTypeSymbol ) - TINT( m_pTypeSymbolBase ) ) >> 4 ];
	}

	AUnit*         CreateUnitFromTemplate( AUnitType* a_pType, AUnitTransform* a_pTransform );
	AUnit*         CreateUnitInstance( AUnitType* a_pType, AUnitTransform* a_pTransform );
	UnitPoolEntry* AcquireUnitFromPool( const TCHAR* a_szTypeName );
	UnitPoolEntry* StealPoolEntry( UnitPool* a_pPool );
	TBOOL          ReleasePoolEntry( AUnit* a_pUnit );
	void           SetPoolEntryUsed( UnitPool* a_pPool, AUnit* a_pUnit, TBOOL a_bUsed );

	void SaveUnitNames( AHackedQuickSave::Buffer* a_pWriter, AUnit* a_pUnit, const Toshi::TPString8& a_rcTypeName, const Toshi::TPString8& a_rcUnitName );

	void OnUnitCreated( AUnit* a_pUnit );

	// Re-hashes a unit into the spatial hash after it moved during Think.
	void ReinsertUnitInHash( AUnit* a_pUnit );

	// Returns the unit to its type pool if the type is pooled; TTRUE if recycled.
	TBOOL RecycleUnit( AUnit* a_pUnit );
	void  DestroyUnitImmediate( AUnit* a_pUnit );
	void  FinalizeUnitRemoval( AUnit* a_pUnit );

	void RegisterCharacterUnit( AUnit* a_pUnit );
	void UnregisterCharacterUnit( AUnit* a_pUnit );

	void ProcessPendingUnits();
	void UnloadUnitTypes();

private:
	// Weak handle to a character unit, keyed by its id.
	struct CharacterUnitNode : public Toshi::TNodeList<CharacterUnitNode>::TNode
	{
		// $Barnyard: FUNCTION 0040f810
		~CharacterUnitNode() {}

		Toshi::TRef<AUnitProxy> oUnit; // 0x0c
	};

private:
	TINT                                m_iUnk20;                       // 0x20
	AUnitFactory*                       m_pUnitFactory;                 // 0x24
	AUnitHash*                          m_pUnitHash;                    // 0x28
	AUnitComponentManagerList*          m_pComponentManagers;           // 0x2c
	AUIDMap*                            m_pUIDMap;                      // 0x30
	TINT                                m_iNextUnitId;                  // 0x34
	Toshi::TNodeList<AUnit>             m_oUnits;                       // 0x38
	Toshi::TNodeList<AUnit>             m_aUnitLists[ NUM_UNIT_LISTS ]; // 0x48, TODO: per-category split
	Toshi::TNodeList<CharacterUnitNode> m_aCharacterUnits[ 1 ];         // 0x1e8
	Toshi::TRef<AUnitProxy>             m_oUnk1f8;                      // 0x1f8, NOTE: type inferred from the refcount/Delete pattern
	TBOOL                               m_bUpdateAI;                    // 0x1fc
	TBOOL                               m_bUpdateUnits;                 // 0x1fd
	TBOOL                               m_bUpdateThink;                 // 0x1fe
	TBOOL                               m_bUnk1ff;                      // 0x1ff
	TBOOL                               m_bProcessPending;              // 0x200

	// Update throttles. Each timer accumulates delta time and fires once it
	// passes the matching ms_f*Interval, then resets to zero.
	TFLOAT m_fThinkAccumNonPlayer; // 0x204
	TFLOAT m_fAIAccumNonPlayer;    // 0x208
	TFLOAT m_fGlobalUpdateAccum;   // 0x20c
	TFLOAT m_fThinkAccumPlayer;    // 0x210
	TFLOAT m_fAIAccumPlayer;       // 0x214

	Toshi::TTRB               m_oUnitTypesTRB;                           // 0x218, pooled unit types
	TINT                      m_iNumUnitTypes;                           // 0x358
	void*                     m_pTypeSymbolBase;                         // 0x35c, base of the type-symbol region inside the TRB
	UnitPool*                 m_pUnitTypeRecords;                        // 0x360
	TINT                      m_iMemBlockStackIndex;                     // 0x364
	Toshi::TMemory::MemBlock* m_apMemBlockStack[ MEM_BLOCK_STACK_SIZE ]; // 0x368

	Toshi::TGenericEmitter m_oUnitCreatedEmitter;  // 0x3e8
	Toshi::TGenericEmitter m_oUnitsRemovedEmitter; // 0x3f4

	static TFLOAT ms_fGlobalUpdateInterval;
	static TFLOAT ms_fThinkIntervalNonPlayer;
	static TFLOAT ms_fThinkIntervalPlayer;
	static TFLOAT ms_fAIIntervalNonPlayer;
	static TFLOAT ms_fAIIntervalPlayer;

public:
	static AFunctionTable ms_oFunctionTable;
};
