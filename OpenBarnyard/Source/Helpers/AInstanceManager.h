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

	struct CollObjectModel
	{
	};

	using ACollisionObjectModelNode   = Toshi::T2SListNodeWrapper<ACollisionObjectModel>;
	using ACollisionModelInstanceNode = Toshi::T2SListNodeWrapper<ACollisionModelInstance>;

	struct S1 : public Toshi::T2SList<S1>::Node
	{
	};

	struct List
	{
		Toshi::T2SList<ACollisionObjectModelNode>   llCollModelNodes;
		Toshi::T2SList<ACollisionModelInstanceNode> llCollModelInstances;
		ATerrainLocatorList*                        pLocators = TNULL;
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

	struct RenderListEntry
	{
		Toshi::TMatrix44 matTransform;
		RenderListEntry* pNext;
	};

public:
	AInstanceManager();
	~AInstanceManager();

	TBOOL LoadModels( TINT a_iInstanceLibIndex, TINT a_iNumModels, TUINT* a_pModelIndices, TBOOL a_bCreateCollisionModelNodes, TBOOL a_bCreateCollisionModelInstances, TINT a_iNumCollisionModelNodes );
	void  Reset();

	TBOOL Render();

	void CreateInstances( ATerrainLocatorList* a_pLocatorList );

private:
	void FillRenderList_All( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic );
	void FillRenderList_Visible( const Toshi::T2SList<StaticInstanceEntry>& a_rcInstanceList, const Toshi::TMatrix44& a_rcWorldTransform, RenderListEntry* a_pOutRenderData, TINT& a_rNumInstances, TBOOL a_bDynamic );

private:
	void DestroyCollisionSets();
	void ClearList( List& a_rList );
	void SetupCollisions( TBOOL a_bCreateCollisionSets );

private:
	TINT                      m_iNumKeyLibraries;
	Toshi::TKeyframeLibrary** m_ppKeyLibs;

	TINT   m_iNumModels;
	Model* m_pModels;

	RenderListEntry** m_pModelsRenderLists;

	StaticInstanceEntry                 m_aInstances[ MAX_INSTANCES ];
	Toshi::T2SList<StaticInstanceEntry> m_llUsedInstances;
	Toshi::T2SList<StaticInstanceEntry> m_llFreeInstances;
	Toshi::T2SList<StaticInstanceEntry> m_llOtherInstances_Unused;

	// TODO: replace StaticInstanceEntry with other type
	Toshi::T2SList<StaticInstanceEntry> m_llUsedDynamicInstances;
	Toshi::T2SList<StaticInstanceEntry> m_llFreeDynamicInstances;
	TINT                                m_iNumDynamicInstances;
	void*                               m_pDynamicInstances;

	TFLOAT m_flLOD0Distance;
	TFLOAT m_flVisibilityDistanceBlendEnd;
	TFLOAT m_flVisibilityDistanceBlendStart;

	TBOOL  m_aInstanceVisMasks[ MAX_RENDERED_INSTANCES ];
	TUINT8 m_aInstanceFlags[ MAX_RENDERED_INSTANCES ];
	TUINT8 m_aInstanceLightIDs[ MAX_RENDERED_INSTANCES ];

	TINT                                             m_iInstanceLibIndex;
	Toshi::T2Vector<ACollisionModelSet*, NUM_MODELS> m_vecCollisionSets;
	ACollisionObjectModelNode*                       m_pCollObjectModelNodes;
	ACollisionModelInstanceNode*                     m_pCollisionModelInstances;
	Toshi::T2SList<ACollisionObjectModelNode>        m_llFreeCollisionModelNodes;
	Toshi::T2SList<ACollisionModelInstanceNode>      m_llFreeCollisionModelInstances;

	List   m_aLists[ 3 ];
	TUINT8 m_uiFlags;
};
