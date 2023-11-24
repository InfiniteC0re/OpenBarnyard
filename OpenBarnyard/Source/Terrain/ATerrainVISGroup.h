#pragma once
#include "ATerrainLODBlock.h"
#include "Assets/AMaterialLibrary.h"

#include <Toshi/Render/TModelRegistry.h>
#include <Toshi2/T2SList.h>

class ATerrainVISGroup
{
public:
	struct ModelData : 
		Toshi::T2SList<ModelData>::Node
	{
		Toshi::TModelRef m_ModelRef;
	};

	enum FLAGS : TUINT32
	{
		FLAGS_HIGH_LOD_LOADED = BITFIELD(0),
		FLAGS_LOW_LOD_LOADED = BITFIELD(1),
		FLAGS_HIGH_LOD_LOADING = BITFIELD(2),
		FLAGS_LOW_LOD_LOADING = BITFIELD(3),
		FLAGS_HIGH_LOD_PROCESSED = BITFIELD(4),
		FLAGS_LOW_LOD_PROCESSED = BITFIELD(5),
	};

	friend class ATerrain;

public:
	void LoadCollision();
	void LoadModels(ATerrainLODType a_eLODType);
	void LoadMatlib(ATerrainLODType a_eLODType);

	void UnloadMatlib(ATerrainLODType a_eLODType);
	void DestroyLOD(ATerrainLODType a_eLODType);

	TBOOL IsMatLibLoaded(ATerrainLODType a_eLODType) const;

	TBOOL IsLODProcessed(ATerrainLODType a_eLODType) const { return (m_eFlags & (16 << a_eLODType)) == 0; }
	void SetLODProcessed(ATerrainLODType a_eLODType, TBOOL a_bProcessed);

	TBOOL IsLODLoaded(ATerrainLODType a_eLODType) const { return m_eFlags & (1 << (a_eLODType)); }
	void SetLODLoaded(ATerrainLODType a_eLODType, TBOOL a_bLoaded);

	TBOOL IsLODLoading(ATerrainLODType a_eLODType) const { return m_eFlags & (1 << (a_eLODType + ATerrainLODType_NUMOF)); }
	void SetLODLoading(ATerrainLODType a_eLODType, TBOOL a_bLoading);

	TBOOL IsLODEmpty(ATerrainLODType a_eLODType) const { return (m_eFlags & (64 << (a_eLODType & 0x1f))); }
	void SetLODEmpty(ATerrainLODType a_eLODType, TBOOL a_bEmpty);

private:
	const char* m_szName;
	const char** m_ppHighLODNames;
	const char** m_ppLowLODNames;
	TINT32 m_iNumHighLODs;
	TINT32 m_iNumLowLODs;
	TUINT32 m_Unk1;
	TUINT32 m_Unk2;
	const char* m_szCollisionFile;
	ModelData* m_pCollisionModelData;
	ModelData** m_ppLODModelsData[ATerrainLODType_NUMOF];
	const char* m_szHighLODMatLibName;
	Toshi::TTRB* m_pMatLibHighTRB;
	AMaterialLibrary* m_pMatLibHigh;
	const char* m_szLowLODMatLibName;
	Toshi::TTRB* m_pMatLibLowTRB;
	AMaterialLibrary* m_pMatLibLow;
	ATerrainLODType* m_pLODTypes;
	TUINT32 m_eFlags;
	TUINT16 m_ui16NumHighLODBlocks;
	TUINT16 m_ui16NumLowLODBlocks;
	ATerrainLODBlock** m_ppHighLODBlocks;
	ATerrainLODBlock** m_ppLowLODBlocks;
	TUINT8* m_pHighLODToBlock;
	TUINT8* m_pLowLODToBlock;
	TUINT8 m_ui8AllocCollisionBlock;
	TUINT8 m_ui8AllocMatlibsHighBlock;
	TUINT8 m_ui8AllocMatlibsLowBlock;
};
