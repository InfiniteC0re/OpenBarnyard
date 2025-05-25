#pragma once
#include "Render/ALockedMesh.h"
#include "Collision/ACollisionObjectSimple.h"
#include "Terrain/ATerrainLocator.h"

#include <Toshi/T2SList.h>

class ATreeManager
    : public Toshi::TSingleton<ATreeManager>
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
			TUINT  bFlag1 : 1;
			TUINT  bFlag2 : 1;
		};

		TUINT uiUnk3;

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
		friend ATreeManager;

	public:
		Model();
		~Model();

		void Load( const TCHAR* a_szModelName );

	private:
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
	ATreeManager();
	~ATreeManager();

	// Initialises models and returns TTRUE if succeded
	TBOOL Initialise();

	void CreateInstances( ATerrainLocatorList* a_pLocatorList );

	void Render();

private:
	// ...
	TINT   m_iNumTrunks;
	Model* m_pTrunks;
	TINT   m_iNumFOBs;
	Model* m_pFOBs;

	TreeInstanceRenderData** m_pRenderData;

	TreeInstance                 m_aTreeInstances[ MAX_INSTANCES ];
	Toshi::T2SList<TreeInstance> m_llUsedTreeInstances;
	Toshi::T2SList<TreeInstance> m_llFreeTreeInstances;
	ALockedMesh                  m_aLockedMeshes[ 2 ];
	// ...
	TFLOAT m_fLODSwitchDistance;
	TFLOAT m_fRenderDistance;
	TFLOAT m_fBlendingDistance;
	TBOOL  m_bCreateCollision;
	TBOOL  m_bFlag;
};
