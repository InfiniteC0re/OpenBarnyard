#pragma once
#include "FSM/AFunctionTable.h"
#include "GameInterface/AHackedQuickSave.h"
#include "Physics/ACollisionObjectModel.h"
#include "Physics/ACollisionModelInstance.h"

#include <Toshi/TObject.h>
#include <Toshi/TSingleton.h>
#include <Toshi/T2SList.h>
#include <Toshi/TPString8.h>
#include <Toshi/TSceneObject.h>
#include <Math/TVector2.h>
#include <Math/TVector4.h>
#include <Math/TQuaternion.h>
#include <Math/TMatrix44.h>
#include <File/TTRB.h>

class AInteriorArea;
class ACollisionModelSet;
class ATerrainLocatorList;

namespace Toshi
{
class TSkeleton;
class TManagedModel;
} // namespace Toshi

//-----------------------------------------------------------------------------
// Purpose: The objects of the world that regrow after the player takes them:
// the fruits of the trees, the vegetables, the crates, the milk bottles...
// (Data/treeregrowth/treeregrowth.trb). The objects are placed by the terrain
// locators (CreateFromLocators) and the trees (CreateFromSkeleton), each one
// belongs to a group (the locator list / tree it was created by) and has a type
// (the "EnumNames" of the TRB, GrowthMapping describes how it grows and which
// models it uses). The manager renders the objects itself (Render), creates
// their collision and keeps track of the time each one started to grow.
// TClass 0x007922b0 (version 1.0, size 0x1342c), singleton at 0x007922e0,
// vtable 0x00734220.
//-----------------------------------------------------------------------------
class ARegrowthManager
    : public Toshi::TObject
    , public Toshi::TSingleton<ARegrowthManager>
{
public:
	TDECLARE_CLASS( ARegrowthManager, Toshi::TObject );

	static constexpr TINT MAX_NUM_TYPES             = 61;
	static constexpr TINT MAX_NUM_OBJECTS           = 1727;
	static constexpr TINT MAX_NUM_GROUPS            = 1;
	static constexpr TINT MAX_NUM_COLLISION_SETS    = 300;
	static constexpr TINT MAX_NUM_COLLISION_NODES   = 1000;
	static constexpr TINT MAX_NUM_MODEL_INSTANCES   = 28;
	static constexpr TINT MAX_NUM_RENDERED_OBJECTS  = 240;
	static constexpr TINT MAX_NUM_COLLISION_ENTRIES = 1000;
	static constexpr TINT NUM_LIGHT_SLOTS           = 22;

	// Time of the objects that were taken (or never grew)
	static constexpr TFLOAT TIME_EMPTY = -100000000.0f;

	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE          = 0,
		FLAGS_ACTIVE        = BITFLAG( 0 ), // Activate was called (the objects are rendered and updated)
		FLAGS_COLLISION     = BITFLAG( 1 ), // the collision model sets are created
		FLAGS_ALLOW_COLL    = BITFLAG( 2 ),
		FLAGS_ALLOW_COLL2   = BITFLAG( 3 ),
		FLAGS_MODELS_LOADED = BITFLAG( 4 ),
		FLAGS_IN_USE        = BITFLAG( 5 ), // the regrowth objects replace the instances (see AInstanceManager)
		FLAGS_INTERIOR      = BITFLAG( 7 ), // render the objects of the current floor only (RenderInterior)
	};

	// Bits of GrowthMapping::uiFlags (and the lower bits of TypeMask)
	using TYPEFLAG = TINT;
	enum TYPEFLAG_ : TYPEFLAG
	{
		TYPEFLAG_FRUIT      = 0, // grows on a tree (GrowFruits)
		TYPEFLAG_CRATE      = 1, // "crate"
		TYPEFLAG_MILK       = 2, // "milk"
		TYPEFLAG_HEAVY      = 3, // "heavy"
		TYPEFLAG_LIGHT      = 4, // "light"
		TYPEFLAG_DAYRESPAWN = 5, // "dayrespawn", respawns every day (and has the contents of a crate schema)
		TYPEFLAG_COLLISION  = 6, // always gets a collision
		TYPEFLAG_INSTANT    = 7, // respawns grown
	};

	// How the objects of a type look while they grow
	using GROWTHSTYLE = TUINT8;
	enum GROWTHSTYLE_ : GROWTHSTYLE
	{
		GROWTHSTYLE_MODELS = 0, // one model per stage
		GROWTHSTYLE_SCALE  = 1, // the models are scaled up
	};

	// Filter of the objects some functions look for: TYPEFLAG bits + a bit per type (8 + type).
	struct TypeMask
	{
		TypeMask()
		{
			auiBits[ 0 ] = 0;
			auiBits[ 1 ] = 0;
		}
		TypeMask( const TypeMask& a_rcOther );

		TypeMask& operator|=( const TypeMask& a_rcOther );
		TypeMask  operator&( const TypeMask& a_rcOther ) const;

		TBOOL IsEmpty() const { return auiBits[ 0 ] == 0 && auiBits[ 1 ] == 0; }

		// The bit of a_iType
		static TypeMask FromType( TUINT a_iType );

		// The flags and the bit of a_iType
		static TypeMask FromTypeWithFlags( TUINT a_iType );

		// The types that don't have the flag a_uiFlag... in theory: the loop over the types has no effect in retail,
		// the result is the inverted a_uiFlag in the lower bits (NonTrees: every flag but TYPEFLAG_FRUIT)
		static TypeMask WithoutFlag( TUINT a_uiFlag );

		// The types that aren't trees
		static TypeMask NonTrees();

		// The crates
		static TypeMask Crates();

		// The filter of the objects called a_rcName ("fruit", "crate", "light", "heavy", "milk", "dayrespawn", "non_trees" or the name of a type)
		static TypeMask FromName( const Toshi::TPString8& a_rcName );

		TUINT32 auiBits[ 2 ];
	};

	// The names of the object types and the bones of the tree models ("EnumNames" and "BoneNames" of the TRB).
	class Types
	{
	public:
		Types();
		~Types();

		void Load( Toshi::TTRB* a_pTRB );

		// Index of the type whose name starts with a_szName, -1 if there's none
		TINT FindEnum( const TCHAR* a_szName ) const;

		TINT          GetNumEnums() const { return m_iNumEnums; }
		const TCHAR** GetBoneNames() const { return m_ppBoneNames; }

	private:
		const TCHAR** m_ppEnumNames; // 0x0
		const TCHAR** m_ppBoneNames; // 0x4
		TINT          m_iNumEnums;   // 0x8
	};

	// How the objects of a type grow ("GrowthMapping" of the TRB, indexed by the type)
	struct GrowthMapping
	{
		TFLOAT       fStagesPerSecond; // 0x00
		TINT         iNumStages;       // 0x04
		TINT         iScreenNameId;    // 0x08, locale string
		const TCHAR* szUnitType;       // 0x0c, unit spawned when the object is taken
		const TCHAR* szInteraction;    // 0x10, interaction that takes the object
		const TCHAR* szEffect1;        // 0x14, played where the object is knocked over
		const TCHAR* szEffect2;        // 0x18, played where the object is knocked over
		TINT         iSound;           // 0x1c, played where the object is knocked over
		TUINT8       uiFirstModel;     // 0x20, index of the model of the first stage (0xff: not rendered)
		GROWTHSTYLE  eGrowthStyle;     // 0x21
		TUINT8       uiFlags;          // 0x22, TYPEFLAG bits
		TUINT8       uiNumFruits;      // 0x23, objects grown by GrowGroup
		TUINT8       uiNumRegrow;      // 0x24
	};

	// Probabilities of the contents of a crate ("CrateSchema" of the TRB)
	struct CrateSchema
	{
		struct Content
		{
			const TCHAR* szName;  // 0x0
			TINT         iUnk;    // 0x4
			TFLOAT       fChance; // 0x8
		};

		// Picks one of the contents
		Content* PickRandom();

		const TCHAR* szName;    // 0x0, crates whose name contain it use the schema
		Content*     pContents; // 0x4
	};

	struct Model
	{
		Model();
		~Model();

		Toshi::TManagedModel* pModel;       // 0x0
		Toshi::TSceneObject*  pSceneObject; // 0x4
	};

	// A regrowable object
	struct GrowthObject
	    : public Toshi::T2SList<GrowthObject>::Node
	{
		// Scale of the fully grown object (0.8 - 1.2)
		TFLOAT GetScale() const;
		void   SetScale( TFLOAT a_fScale );

		void GetMatrix( Toshi::TMatrix44& a_rOutMatrix ) const;

		// The contents of the crate (see CrateSchema)
		CrateSchema::Content* PickCrateContent();

		// Key of the object in the save files
		TFLOAT GetSaveKey() const { return vecPosition.z * 1000.0f + vecPosition.x; }

		Toshi::TQuaternion quatRotation; // 0x04
		Toshi::TVector4    vecPosition;  // 0x14
		TFLOAT             fGrowTime;    // 0x24, game time the object started to grow (TIME_EMPTY if it was taken)
		TUINT8             uiGroup;      // 0x28
		TUINT8             uiCrate;      // 0x29, the crate schema (<< 3) and the respawn flags (& 7)
		TUINT8             uiScale;      // 0x2a
	};

	// The taken objects (llEmpty) and the growing ones (llGrowing) of a type
	struct TypeLists
	{
		void SetGrowing( GrowthObject* a_pObject, TFLOAT a_fGrowTime );
		void SetGrowing( GrowthObject* a_pObject, TFLOAT a_fGrowTime, GrowthObject*& a_rpNext );
		void SetEmpty( GrowthObject* a_pObject );
		void SetEmpty( GrowthObject* a_pObject, GrowthObject*& a_rpNext );

		Toshi::T2SList<GrowthObject> llEmpty;   // 0x0
		Toshi::T2SList<GrowthObject> llGrowing; // 0x4
	};

	// Collision of an object
	class ACollisionObjectModelNode
	    : public ACollisionObjectModel
	    , public Toshi::T2SList<ACollisionObjectModelNode>::Node
	{
	public:
		ACollisionObjectModelNode();
		~ACollisionObjectModelNode();

		GrowthObject* m_pObject; // 0xa8
	};

	// Collision model instance of a group
	struct ModelInstanceNode
	    : public Toshi::T2SList<ModelInstanceNode>::Node
	{
		ModelInstanceNode();
		~ModelInstanceNode();

		ACollisionModelInstance oInstance; // 0x4
	};

	// Collision of the objects of a group
	struct Group
	{
		Toshi::T2SList<ACollisionObjectModelNode> llCollisionNodes; // 0x0
		Toshi::T2SList<ModelInstanceNode>         llModelInstances; // 0x4
		TUINT8                                    uiGroupId;        // 0x8

		// $Barnyard: FUNCTION 005e2840
		~Group() {}
	};

	// An object that will get a collision (see CreateCollisions)
	struct CollisionEntry
	{
		using KIND = TUINT8;
		enum KIND_ : KIND
		{
			KIND_NONE   = 0, // skipped
			KIND_SIMPLE = 1, // a capsule
			KIND_MESH   = 2, // uses the collision mesh of the model
		};

		GrowthObject* pObject; // 0x0
		KIND          eKind;   // 0x4
		TUINT8        uiType;  // 0x5
	};

	struct RenderData
	{
		Toshi::TMatrix44 oMatrix; // 0x00
		RenderData*      pNext;   // 0x40
	};

public:
	ARegrowthManager();
	~ARegrowthManager();

	virtual AFunctionTable* GetFunctionTable(); // +0x0c

	// Fills the free list of the objects, a_bAllowCollision allows the collision of the objects
	TBOOL ResetGrowthObject( TBOOL a_bAllowCollision );

	// Loads the models and creates the render lists, a_bCollision creates the collision model sets
	void Activate( TBOOL a_bCollision );
	void Deactivate();
	void LoadModels();

	void  Update( TFLOAT a_fDeltaTime );
	TBOOL Render();

	TBOOL Save( AHackedQuickSave::Buffer* a_pBuffer );
	TBOOL Load( AHackedQuickSave::Buffer* a_pBuffer );

	// Creates the objects of the locators of a_pLocatorList, returns the id of their group
	TUINT8 CreateFromLocators( ATerrainLocatorList* a_pLocatorList );

	// Creates the objects of the bones of a tree (called by ATreeManager)
	void CreateFromSkeleton( Toshi::TSkeleton* a_pSkeleton, const Toshi::TMatrix44& a_rcMatrix, TUINT8 a_uiGroup, TUINT a_uiType, TBOOL a_bGrown );

	// Destroys the objects of a group
	void DestroyGroup( TUINT a_uiGroup );

	// A new group id, 0xff if there's none left
	TUINT8 AllocateGroupId();

	// Grows the fruits of every tree, then regrows the day respawn objects
	void NewDay( TBOOL a_bSkipSimPersist, TBOOL a_bAllDayRespawn );
	void NewDayAll();
	// NOTE: the original takes an argument it doesn't use (ABarnExploreMinigameState passes if it's night)
	void NewDayFruits( TBOOL a_bUnused );
	void NewDayFruits2();

	// Regrows every object with the TYPEFLAG_INSTANT flag (or the respawn flags) at once
	void RespawnInstant();
	void RespawnDayRespawn();

	// The grown object of a_rcMask in front of a_rcPosition (a_rcDirection is the XZ direction)
	GrowthObject* FindObjectInFront( const Toshi::TVector4& a_rcPosition, const Toshi::TVector2& a_rcDirection, const TypeMask& a_rcMask, TINT& a_rOutType, TFLOAT a_fMaxDistance, TFLOAT a_fMinDot );

	// The closest visible grown object of a_rcMask
	GrowthObject* FindClosestVisible( const Toshi::TVector4& a_rcPosition, const TypeMask& a_rcMask, TFLOAT a_fMaxDistance );

	// Takes the object (it starts to regrow)
	TBOOL TakeObject( TINT a_iType, GrowthObject* a_pObject );

	// Takes a random grown object of a_uiGroup, its position goes to a_rOutPosition
	TBOOL TakeRandomObject( TUINT a_uiGroup, Toshi::TVector4& a_rOutPosition );

	// Makes the objects of a_uiGroup empty, returns their number
	TINT EmptyGroup( TUINT a_uiGroup );

	// Starts growing a_pObject (of a_iType) a_fAge seconds ago
	TBOOL StartGrowing( TINT a_iType, GrowthObject* a_pObject, TFLOAT a_fAge );

	// Grows a_iCount empty objects of a_uiGroup (FSM native "GrowFruits")
	TINT GrowFruits( TINT a_uiGroup, TINT a_iCount, TFLOAT a_fAge );

	// Number of the empty objects of a_uiGroup (FSM native "FreeFruitSlotCount")
	TINT CountEmpty( TINT a_uiGroup );

	TINT CountInArea( AInteriorArea* a_pArea, TINT a_iType );

	// Number of the objects of a_uiGroup at a_iStage (> 100: grown)
	TINT CountObjectsAtStage( TUINT a_uiGroup, TINT a_iStage );
	TINT CountTypeObjectsAtStage( TINT a_iStage, TINT a_iType );

	// A random object of a_uiGroup at a_iStage (> 100: grown), its type goes to a_rOutType
	GrowthObject* FindRandomObjectAtStage( TUINT a_uiGroup, TINT a_iStage, TINT& a_rOutType );

	// The type the objects of a_uiGroup have, -1 if the group has none
	TINT FindTypeOfGroup( TUINT a_uiGroup );
	TINT FindTypeOfObject( GrowthObject* a_pObject );

	TINT GetTypeIndex( const Toshi::TPString8& a_rcName );

	// The name of the unit given by the objects of a_iType (TNULL if none)
	Toshi::TPString8 GetUnitType( TINT a_iType );
	Toshi::TPString8 GetUnitTypeIfInteraction( TINT a_iType );

	// The interaction that takes the objects of a_iType ("defaulttree" by default)
	Toshi::TPString8 GetInteraction( TINT a_iType );

	TINT GetScreenNameId( TINT a_iType ) const;
	TINT GetNumFruits( TINT a_iType ) const;
	TINT GetNumRegrow( TINT a_iType ) const;

	FLAGS GetFlags() const { return m_eFlags; }

	// Set once the day respawn / instant objects have been regrown (the "egg" of ATreasureRequirement)
	TBOOL HasRespawned() const { return m_iNumRespawned != 0; }

	// Called when the egg of the henhouse is taken (AShakeTreeMicroGame)
	void  DecrementRespawned() { m_iNumRespawned--; }
	TBOOL IsActive() const { return m_eFlags & FLAGS_ACTIVE; }

	void SetInUse( TBOOL a_bInUse )
	{
		if ( a_bInUse ) m_eFlags |= FLAGS_IN_USE;
		else m_eFlags &= ~FLAGS_IN_USE;
	}

	void SetInteriorMode( TBOOL a_bInterior )
	{
		if ( a_bInterior ) m_eFlags |= FLAGS_INTERIOR;
		else m_eFlags &= ~FLAGS_INTERIOR;
	}

	// TYPEFLAG a_eFlag of a_uiType
	static TBOOL HasTypeFlag( TUINT a_uiType, TYPEFLAG a_eFlag );

	// a_uiType matches a_rcMask
	static TBOOL TypeMatches( TUINT a_uiType, const TypeMask& a_rcMask );

	static Types* GetTypes() { return ms_pTypes; }

	const GrowthMapping& GetGrowthMapping( TINT a_iType ) const { return m_pGrowthMapping[ a_iType ]; }
	TypeLists&           GetTypeLists( TINT a_iType ) { return m_aTypeLists[ a_iType ]; }

private:
	TBOOL LoadTRB();
	void  UnloadModels();
	void  SetCollisionEnabled( TBOOL a_bEnabled );
	void  DestroyCollisionModelSets();

	Group* FindGroup( TUINT a_uiGroup );
	Group* FindOrCreateGroup( TUINT a_uiGroup );
	void   ClearGroup( Group* a_pGroup );
	void   ReleaseGroupCollisionNodes( TUINT a_uiGroup );
	void   ClearAllObjects();

	TINT  FindCrateSchema( const TCHAR* a_szName );
	TINT  CountEmptyInType( TUINT a_uiGroup, TINT a_iType );
	TINT  CountInType( TUINT a_uiGroup, TINT a_iType );
	TBOOL HasGroupInType( TUINT a_uiGroup, TINT a_iType );
	TBOOL IsGrowingInType( GrowthObject* a_pObject, TINT a_iType );
	TBOOL IsInType( GrowthObject* a_pObject, TINT a_iType );

	// Grows a_iCount random empty objects of a_uiGroup and a_iType, returns the number of grown objects
	TINT GrowObjects( TUINT a_uiGroup, TINT a_iCount, TFLOAT a_fAge, TINT a_iType );
	TINT GrowGroup( TUINT a_uiGroup );
	void GrowAllInType( TINT a_iType, TINT a_iCount );

	// Moves the collision of a_pObject to the pending list (it's activated once the object is grown)
	void DeactivateObjectCollision( GrowthObject* a_pObject );

	// Adds an object, returns its model index (-1 if it can't be added)
	TINT AddObject( const Toshi::TMatrix44& a_rcMatrix, const Toshi::TMatrix44& a_rcObjectMatrix, const TCHAR* a_szName, TUINT8 a_uiGroup, TINT a_iType, TBOOL a_bGrown, TINT& a_rOutType, GrowthObject*& a_rpOutObject, TINT a_iScale );

	// Collects the objects that need a collision
	void AddCollisionEntry( GrowthObject* a_pObject, TINT* a_piNumEntries, TINT* a_piNumMeshes, TINT* a_piNumCollisions, const TCHAR* a_szName, TINT a_iType, TINT a_iModel, ACollisionModelSet** a_ppModelSets, CollisionEntry* a_pEntries, TBOOL* a_pbUsedModels );
	void GetModelCollisionSets( TBOOL* a_pbUsedModels, ACollisionModelSet** a_ppOutModelSets );
	void CreateCollisions( TINT a_iNumCollisions, TINT a_iNumMeshes, TUINT8 a_uiGroup, TBOOL* a_pbUsedModels, ACollisionModelSet** a_ppModelSets, CollisionEntry* a_pEntries, const Toshi::TMatrix44& a_rcMatrix );

	void CreateFromLocatorList( ATerrainLocatorList* a_pLocatorList, const Toshi::TMatrix44& a_rcMatrix, TUINT8 a_uiGroup, TUINT a_uiType, TBOOL a_bGrown );

	void BuildRenderList();
	void BuildRenderListInterior();

	static TFLOAT DecodeGrowTime( TUINT8 a_uiValue );
	static TUINT8 EncodeGrowTime( TFLOAT a_fGrowTime );

	static ACollisionObjectModelNode* FindCollisionNode( GrowthObject* a_pObject, Toshi::T2SList<ACollisionObjectModelNode>* a_pList );
	static GrowthObject*              FindObjectBySaveKey( TFLOAT a_fKey, Toshi::T2SList<GrowthObject>* a_pList );
	static TINT                       FindTypeByPrefix( const TCHAR* a_szName, const TCHAR** a_ppNames, TINT a_iNumNames );

public:
	// DAT_00792280
	static AFunctionTable ms_oFunctionTable;

	// DAT_007962e8
	static Types* ms_pTypes;

private:
	TINT                                      m_iNumModels;                                     // 0x00004
	Model*                                    m_pModels;                                        // 0x00008
	GrowthMapping*                            m_pGrowthMapping;                                 // 0x0000c
	RenderData**                              m_ppRenderLists;                                  // 0x00010, two per model (near and far)
	ACollisionObjectModelNode*                m_pCollisionNodes;                                // 0x00014
	ModelInstanceNode*                        m_pModelInstanceNodes;                            // 0x00018
	TypeLists                                 m_aTypeLists[ MAX_NUM_TYPES ];                    // 0x0001c
	GrowthObject                              m_aObjects[ MAX_NUM_OBJECTS ];                    // 0x00204
	Toshi::T2SList<GrowthObject>              m_llFreeObjects;                                  // 0x12ad8
	TUINT32                                   m_auiUsedGroupIds[ 8 ];                           // 0x12adc
	Toshi::TTRB                               m_oTRB;                                           // 0x12afc
	TINT                                      m_iNumCollisionModelSets;                         // 0x12c3c
	ACollisionModelSet*                       m_apCollisionModelSets[ MAX_NUM_COLLISION_SETS ]; // 0x12c40
	Toshi::T2SList<ACollisionObjectModelNode> m_llFreeCollisionNodes;                           // 0x130f0
	Toshi::T2SList<ModelInstanceNode>         m_llFreeModelInstances;                           // 0x130f4
	Toshi::T2SList<ACollisionObjectModelNode> m_llPendingCollisionNodes;                        // 0x130f8, activated once the object is grown
	Group                                     m_aGroups[ MAX_NUM_GROUPS ];                      // 0x130fc
	TFLOAT                                    m_fCollisionUpdateTimer;                          // 0x13108
	TINT                                      m_aiLightIds[ NUM_LIGHT_SLOTS ];                  // 0x1310c
	TINT                                      m_aiLightUnk1[ NUM_LIGHT_SLOTS ];                 // 0x13164
	TINT                                      m_aiLightUnk2[ NUM_LIGHT_SLOTS ];                 // 0x131bc
	TINT                                      m_aiLightUnk3[ 5 ][ NUM_LIGHT_SLOTS ];            // 0x13214
	CrateSchema*                              m_pCrateSchemas;                                  // 0x133cc
	TFLOAT                                    m_fLODDistance;                                   // 0x133d0
	TFLOAT                                    m_fRenderDistance;                                // 0x133d4
	TFLOAT                                    m_fFadeDistance;                                  // 0x133d8
	RenderData*                               m_pRenderData;                                    // 0x133dc
	Toshi::TMatrix44                          m_oModelView;                                     // 0x133e0
	TFLOAT                                    m_fFadeScale;                                     // 0x13420
	FLAGS                                     m_eFlags;                                         // 0x13424
	TINT                                      m_iNumRespawned;                                  // 0x13428, set once the day respawn objects are regrown
};
