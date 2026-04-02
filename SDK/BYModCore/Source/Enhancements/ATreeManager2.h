#pragma once
#include <Toshi/T2SList.h>
#include <Render/TModel.h>
#include <Render/TModelManager.h>

#include <BYardSDK/ATerrainLocator.h>

class ACollisionObjectSimple;

struct ALockedMesh
{
	TCHAR PADDING[ 0x48 ];
};

class ATreeManager2
{
public:
	struct TreeInstance
	    : Toshi::T2SList<TreeInstance>::Node
	{
		ATerrainLocatorList* pLocatorList;
		TUINT16              uiLocatorId;

		union
		{
			TUINT8 iTrunkIndex : 6;
			TUINT8 iColor : 2;
		};

		TUINT8 uiUnk3;

		ACollisionObjectSimple* pSimpleCollisionObjects = TNULL;
		void*                   unk                     = TNULL;
	};

	struct TreeInstanceRenderData
	{
		Toshi::TMatrix44        oMatrix;
		TreeInstanceRenderData* pPrev;
	};

	class Model
	{
	public:
		Toshi::TManagedModel* m_pManagedModel;
		Toshi::TSceneObject*  m_pSceneObject;
		TUINT                 m_uiID;
		TINT                  m_iNumFOBs;
		Toshi::TMatrix44*     m_pFOBMatrices;
		TFLOAT                m_fGroundOffset;
		TFLOAT                m_fRadius;
	};

	inline static constexpr TINT MAX_INSTANCES = 1302;

public:
	void Render();

private:
	TCHAR  PADDING1[ 8 ];
	TINT   m_iNumTrunks;
	Model* m_pTrunks;
	TINT   m_iNumFOBs;
	Model* m_pFOBs;

	TreeInstanceRenderData** m_pRenderData;

	ACollisionObjectSimple* m_pCollisionObjects;

	TreeInstance                 m_aTreeInstances[ MAX_INSTANCES ];
	Toshi::T2SList<TreeInstance> m_llUsedTreeInstances;
	Toshi::T2SList<TreeInstance> m_llFreeTreeInstances;
	ALockedMesh                  m_aLockedMeshes[ 2 ];

	TCHAR PADDING2[ 40 ];

	TFLOAT m_fLODSwitchDistance;
	TFLOAT m_fRenderDistance;
	TFLOAT m_fBlendingDistance;
	TBOOL  m_bCreateCollision;
	TBOOL  m_bFlag;

	TCHAR PADDING3[ 26 ];
};
