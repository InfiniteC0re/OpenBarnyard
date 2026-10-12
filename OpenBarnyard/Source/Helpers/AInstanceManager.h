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

class AInstanceManager
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
