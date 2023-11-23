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

	using LODType = TUINT8;
	enum LODType_ : LODType
	{
		LODType_High,
		LODType_Low,
		LODType_NUMOF,
	};

public:
	

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
	ModelData** m_ppLODModelsData[LODType_NUMOF];
	const char* m_szHighLODMatLibName;
	Toshi::TTRB* m_pMatLibHighTRB;
	AMaterialLibrary* m_pMatLibHigh;
	const char* m_szLowLODMatLibName;
	Toshi::TTRB* m_pMatLibLowTRB;
	AMaterialLibrary* m_pMatLibLow;
	TUINT8* m_pLODTypes;
	TUINT32 m_eFlags;
	TUINT16 m_ui16NumHighLODBlocks;
	TUINT16 m_ui16NumLowLODBlocks;
	ATerrainLODBlock** m_ppHighLODBlocks;
	ATerrainLODBlock** m_ppLowLODBlocks;
	TUINT32 m_Unk3;
	TUINT32 m_Unk4;
	TUINT8 m_ui8LODBlock;
};
