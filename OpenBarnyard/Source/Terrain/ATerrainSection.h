#pragma once
#include "ATerrainLODBlock.h"
#include "Assets/AMaterialLibrary.h"
#include "Render/AModel.h"

#include <Toshi/Render/TModelRegistry.h>
#include <Toshi2/T2SList.h>

class ATerrainSection
{
public:
	struct ModelData : 
		Toshi::T2SList<ModelData>::Node
	{
		~ModelData();

		Toshi::TModelPtr m_ModelRef;
		Toshi::T2ModelInstance* m_pModelInstance;
	};

	enum FLAGS : TUINT32
	{
		FLAGS_HIGH_LOD_LOADED = BITFIELD(0),
		FLAGS_LOW_LOD_LOADED = BITFIELD(1),
		FLAGS_HIGH_LOD_LOADING = BITFIELD(2),
		FLAGS_LOW_LOD_LOADING = BITFIELD(3),
		FLAGS_HIGH_LOD_QUEUED = BITFIELD(4),
		FLAGS_LOW_LOD_QUEUED = BITFIELD(5),
		FLAGS_COLLISION_LOADING = BITFIELD(10)
	};

	friend class ATerrain;
	friend class ASectionDoneJob;
	friend class ACollisionDoneJob;

public:
	void LoadCollision();
	void LoadModels(ATerrainLODType a_eLODType);
	void LoadMatlib(ATerrainLODType a_eLODType);

	void UnloadMatlib(ATerrainLODType a_eLODType);
	void DestroyLOD(ATerrainLODType a_eLODType);

	void RemoveFromStreamingQueue();

	TBOOL IsMatLibLoaded(ATerrainLODType a_eLODType) const;

	TBOOL IsLODQueued(ATerrainLODType a_eLODType) const { return (m_eFlags & (16 << a_eLODType)); }
	void SetLODQueued(ATerrainLODType a_eLODType, TBOOL a_bQueued);

	TBOOL IsLODLoaded(ATerrainLODType a_eLODType) const { return m_eFlags & (1 << (a_eLODType)); }
	void SetLODLoaded(ATerrainLODType a_eLODType, TBOOL a_bLoaded);

	TBOOL IsLODLoading(ATerrainLODType a_eLODType) const { return m_eFlags & (1 << (a_eLODType + ATerrainLODType_NUMOF)); }
	void SetLODLoading(ATerrainLODType a_eLODType, TBOOL a_bLoading);

	// TODO: The flag's name is wrong
	TBOOL IsLODEmpty(ATerrainLODType a_eLODType) const { return (m_eFlags & (64 << (a_eLODType & 0x1f))); }
	void SetLODEmpty(ATerrainLODType a_eLODType, TBOOL a_bEmpty);

private:
	const char* m_szName;
	const char** m_pszHighModelFiles;
	const char** m_pszLowModelFiles;
	TINT32 m_iNumHighModelFiles;
	TINT32 m_iNumLowModelFiles;

	TUINT32 m_Unk1;
	TUINT32 m_Unk2;
	const char* m_szCollisionFilename;
	ModelData* m_pCollisionModelData;
	ModelData** m_ppLODModelsData[ATerrainLODType_NUMOF];
	
	const char* m_szHighMatLibFilename;
	Toshi::TTRB* m_pMatLibHighTRB;
	AMaterialLibrary* m_pMatLibHigh;
	
	const char* m_szLowMatLibFilename;
	Toshi::TTRB* m_pMatLibLowTRB;
	AMaterialLibrary* m_pMatLibLow;

	ATerrainLODType* m_pVisibility;
	TUINT32 m_eFlags;
	TUINT16 m_iNumHighMemBlocksUsed;
	TUINT16 m_iNumLowMemBlocksUsed;
	ATerrainLODBlock** m_ppHighLODBlocks;
	ATerrainLODBlock** m_ppLowLODBlocks;
	TUINT8* m_pHighLODToBlock;
	TUINT8* m_pLowLODToBlock;
	TUINT8 m_iCollisionMemBlockID;
	TUINT8 m_iHighMatLibMemBlockID;
	TUINT8 m_iLowMatLibMemBlockID;
};
