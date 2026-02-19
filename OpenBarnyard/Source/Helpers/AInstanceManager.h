#pragma once
#include "Physics/ACollisionModelSet.h"
#include "Physics/ACollisionObjectModel.h"
#include "Physics/ACollisionModelInstance.h"

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
		TINT                                        iUnknown = 0;
	};

	struct ModelDefinition
	{
		TINT         unk;
		const TCHAR* pchName;
		const TCHAR* pchPath;
	};

	#include "AInstanceManagerModelList.inl"

	inline static constexpr TSIZE NUM_MODELS    = TARRAYSIZE( ms_aModelDefinitions );
	inline static constexpr TSIZE MAX_INSTANCES = 5000;

public:
	AInstanceManager();
	~AInstanceManager();

	TBOOL LoadModels( TINT a_iInstanceLibIndex, TINT a_iNumModels, TUINT* a_pModelIndices, TBOOL a_bCreateCollisionModelNodes, TBOOL a_bCreateCollisionModelInstances, TINT a_iNumCollisionModelNodes );
	void  Reset();

private:
	void DestroyCollisionSets();
	void ClearList( List& a_rList );
	void SetupCollisions( TBOOL a_bCreateCollisionSets );

private:
	TINT                      m_iNumKeyLibraries;
	Toshi::TKeyframeLibrary** m_ppKeyLibs;

	TINT   m_iNumModels;
	Model* m_pModels;

	void*                     m_pSomeBuffer;
	ACollisionObjectModelNode m_aS1[ MAX_INSTANCES ];

	Toshi::T2SList<S1> m_llUnk1;
	Toshi::T2SList<S1> m_llFreeS1;
	
	// ...

	TFLOAT m_flUnk1;
	TFLOAT m_flUnk2;
	TFLOAT m_flUnk3;

	// ...

	TINT                                             m_iInstanceLibIndex;
	Toshi::T2Vector<ACollisionModelSet*, NUM_MODELS> m_vecCollisionSets;
	ACollisionObjectModelNode*                       m_pCollObjectModelNodes;
	ACollisionModelInstanceNode*                     m_pCollisionModelInstances;
	Toshi::T2SList<ACollisionObjectModelNode>        m_llFreeCollisionModelNodes;
	Toshi::T2SList<ACollisionModelInstanceNode>      m_llFreeCollisionModelInstances;

	List   m_aLists[ 3 ];
	TUINT8 m_uiModelsFlags;
};
