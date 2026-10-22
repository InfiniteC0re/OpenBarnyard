#pragma once
#include "Physics/ACollisionModelSet.h"
#include "Physics/ACollisionObjectModel.h"
#include "Physics/ACollisionModelInstance.h"
#include "Terrain/ATerrainLocator.h"

#include <Toshi/T2SList.h>
#include <Toshi/TSingleton.h>
#include <Toshi/TSceneObject.h>
#include <Toshi/T2Vector.h>
#include <Render/TModel.h>

class MODAPI AInstanceManager
    : public Toshi::TSingleton<AInstanceManager>
{
public:
	using FLAGS = TUINT8;
	enum FLAGS_ : FLAGS
	{
		FLAGS_NONE            = 0,
		FLAGS_UNK0            = BITFLAG( 0 ), // instances are only created for loaded models, cleared with the dynamic instances
		FLAGS_COLLISION_SETS  = BITFLAG( 1 ), // the collision model sets are created (SetupCollisions)
		FLAGS_COLLISION_NODES = BITFLAG( 2 ), // the collision model nodes are created
		FLAGS_UNK3            = BITFLAG( 3 ), // set and cleared with FLAGS_COLLISION_NODES
		FLAGS_INTERIOR        = BITFLAG( 4 ), // SetInteriorMode, the terrain section visibility isn't checked
	};

	// $Barnyard: FUNCTION 005dfbc0
	void GetVisibilityDistances( TFLOAT& a_rLOD0Distance, TFLOAT& a_rBlendEnd, TFLOAT& a_rBlendStart ) const
	{
		a_rLOD0Distance = m_flLOD0Distance;
		a_rBlendEnd     = m_flVisibilityDistanceBlendEnd;
		a_rBlendStart   = m_flVisibilityDistanceBlendStart;
	}

	// $Barnyard: FUNCTION 005dfbf0
	void SetVisibilityDistances( TFLOAT a_fLOD0Distance, TFLOAT a_fBlendEnd, TFLOAT a_fBlendStart )
	{
		m_flLOD0Distance                 = a_fLOD0Distance;
		m_flVisibilityDistanceBlendEnd   = a_fBlendEnd;
		m_flVisibilityDistanceBlendStart = a_fBlendStart;
	}

	struct Model
	{
		Model() = default;
		~Model();

		void Reset();

		Toshi::TManagedModel* pManagedModel = TNULL;
		Toshi::TSceneObject*  pSceneObject  = TNULL;
	};

	struct StaticInstanceEntry : Toshi::T2SList<StaticInstanceEntry>::Node
	{
		ATerrainLocatorList* pLocatorList;
		TUINT16              uiLocatorIndex;
		TUINT8               uiModelIndex;
		TUINT8               uiPackedScaleYaw;
	};

	// An instance that isn't placed by a locator (debris, see AInstanceDebrisManager)
	struct DynamicInstanceEntry : StaticInstanceEntry
	{
		Toshi::TMatrix44 matTransform; // 0x0c
	};

	struct CollObjectModel
	{
	};

	class ACollisionObjectModelNode
	    : public ACollisionObjectModel
	    , public Toshi::T2SList<ACollisionObjectModelNode>::Node
	{
	public:
		ACollisionObjectModelNode();
		~ACollisionObjectModelNode();
	};

	using ACollisionModelInstanceNode = Toshi::T2SListNodeWrapper<ACollisionModelInstance>;

	struct S1 : public Toshi::T2SList<S1>::Node
	{
	};

	struct List
	{
		Toshi::T2SList<ACollisionObjectModelNode>   llCollModelNodes;
		Toshi::T2SList<ACollisionModelInstanceNode> llCollModelInstances;
		ATerrainLocatorList*                        pLocators = TNULL;

		// $Barnyard: FUNCTION 005dfb10
		~List() {}
	};

	struct ModelDefinition
	{
		TINT         unk;
		const TCHAR* pchName;
		const TCHAR* pchPath;
	};

#include "AInstanceManagerModelList.inl"

	inline static constexpr TSIZE NUM_MODELS             = TARRAYSIZE( ms_aModelDefinitions );
	inline static constexpr TSIZE MAX_INSTANCES          = 5000;
	inline static constexpr TSIZE MAX_RENDERED_INSTANCES = 240;
	inline static constexpr TINT  MAX_NUM_LODS           = 2;

	struct RenderListEntry
	{
		Toshi::TMatrix44 matTransform;
		RenderListEntry* pNext;
	};

public:
	AInstanceManager();
	~AInstanceManager();

	// $Barnyard: FUNCTION 005dfc50
	Model* GetModel( TINT a_iIndex ) const { return &m_pModels[ a_iIndex ]; }

	// Renders the instances of the barn floor the player is on only (ABarnExploreMinigameState at night)
	void SetInteriorMode( TBOOL a_bInterior )
	{
		if ( a_bInterior ) m_uiFlags |= FLAGS_INTERIOR;
		else m_uiFlags &= ~FLAGS_INTERIOR;
	}

	// Returns the a_iIndex-th used instance of the model a_uiModelIndex, TNULL if there are fewer
	StaticInstanceEntry* FindInstance( TUINT8 a_uiModelIndex, TINT a_iIndex );

	// World transform of the bone a_iBone of the model a_iModelIndex placed at a_pInstance
	void GetInstanceBoneTransform( TINT a_iModelIndex, StaticInstanceEntry* a_pInstance, TINT a_iBone, Toshi::TMatrix44& a_rOutTransform );

	TBOOL LoadModels( TINT a_iInstanceLibIndex, TINT a_iNumModels, TUINT* a_pModelIndices, TBOOL a_bCreateCollisionModelNodes, TBOOL a_bCreateCollisionModelInstances, TINT a_iNumCollisionModelNodes );
	void  Reset();

	// Allocates a_iNumInstances dynamic instances (all free)
	void CreateDynamicInstances( TINT a_iNumInstances );

	// Frees the dynamic instances
	void DestroyDynamicInstances();

	TBOOL Render();

	void CreateInstances( ATerrainLocatorList* a_pLocatorList );

	// Removes the instances and the collisions created from a_pLocatorList
	void DestroyInstances( ATerrainLocatorList* a_pLocatorList );

#ifdef BARNYARD_COMMUNITY_PATCH
	// For a mod that culls and draws the static instances on the GPU
	struct CullInstance
	{
		Toshi::TMatrix44 matWorld; // with the scale of the locator
		Toshi::TSphere   oBounds;  // as FillRenderList_Visible tests it
		TINT             iModel;
		TUINT            uiLocator;
		TUINT8           uiExposure; // the baked sun exposure of the locator (0-15)
	};

	// The static instances, at most a_iMaxInstances
	TINT GetCullInstances( CullInstance* a_pOutInstances, TINT a_iMaxInstances ) const;

	// The [start, end) locator ranges of the terrain sections visible from the current one, as
	// FillRenderList_Visible tests them. -1 when every locator is
	TINT GetCullVISRanges( TUINT* a_pOutRanges, TINT a_iMaxRanges ) const;

	// The same without the interior mode, the trees use it too
	static TINT GetTerrainVISRanges( TUINT* a_pOutRanges, TINT a_iMaxRanges );

	TINT                 GetNumModels() const { return m_iNumModels; }
	Toshi::TSceneObject* GetModelSceneObject( TINT a_iModel ) const { return m_pModels[ a_iModel ].pSceneObject; }
	TFLOAT               GetLOD0Distance() const { return m_flLOD0Distance; }
	TBOOL                IsInteriorMode() const { return ( m_uiFlags & FLAGS_INTERIOR ) != 0; }

	// Changes whenever the instances or their models do
	static TUINT GetVersion() { return ms_uiVersion; }

	// Set by a mod that draws the static instances itself, Render then only draws the dynamic ones
	static void SetStaticInstancesHidden( TBOOL a_bHidden ) { ms_bStaticInstancesHidden = a_bHidden; }

private:
	inline static TUINT ms_uiVersion              = 0;
	inline static TBOOL ms_bStaticInstancesHidden = TFALSE;

public:
#endif

private:
	void FillRenderList_All( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic );
	void FillRenderList_Visible( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic );

private:
	void DestroyCollisionSets();
	void ClearList( List& a_rList );
	void SetupCollisions( TBOOL a_bCreateCollisionSets );

protected:
	TINT                      m_iNumKeyLibraries;
	Toshi::TKeyframeLibrary** m_ppKeyLibs;

	TINT   m_iNumModels;
	Model* m_pModels;

	RenderListEntry** m_pModelsRenderLists;

	StaticInstanceEntry                 m_aInstances[ MAX_INSTANCES ];
	Toshi::T2SList<StaticInstanceEntry> m_llUsedInstances;
	Toshi::T2SList<StaticInstanceEntry> m_llFreeInstances;
	Toshi::T2SList<StaticInstanceEntry> m_llOtherInstances_Unused;

	// The entries are DynamicInstanceEntry
	Toshi::T2SList<StaticInstanceEntry> m_llUsedDynamicInstances;
	Toshi::T2SList<StaticInstanceEntry> m_llFreeDynamicInstances;
	TINT                                m_iNumDynamicInstances;
	DynamicInstanceEntry*               m_pDynamicInstances;

	TFLOAT m_flLOD0Distance;
	TFLOAT m_flVisibilityDistanceBlendEnd;
	TFLOAT m_flVisibilityDistanceBlendStart;

	TUINT8 m_aInstanceVisMasks[ MAX_RENDERED_INSTANCES ]; // clip flags passed to TRenderContext::SetClipFlags
	TUINT8 m_aInstanceFlags[ MAX_RENDERED_INSTANCES ];
	TUINT8 m_aInstanceLightIDs[ MAX_RENDERED_INSTANCES ];

	TINT                                             m_iInstanceLibIndex;
	Toshi::T2Vector<ACollisionModelSet*, NUM_MODELS> m_vecCollisionSets;
	ACollisionObjectModelNode*                       m_pCollObjectModelNodes;
	ACollisionModelInstanceNode*                     m_pCollisionModelInstances;
	Toshi::T2SList<ACollisionObjectModelNode>        m_llFreeCollisionModelNodes;
	Toshi::T2SList<ACollisionModelInstanceNode>      m_llFreeCollisionModelInstances;

	List  m_aLists[ 3 ];
	FLAGS m_uiFlags;
};
