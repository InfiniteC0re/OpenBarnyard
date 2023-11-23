#pragma once
#include "ATerrainVISGroup.h"
#include "ATerrainLocator.h"
#include "ATerrainLODBlock.h"
#include "Assets/AMaterialLibrary.h"

class ATerrainVIS
{
public:
	ATerrainLocatorManager* GetLocatorManager() { return m_pLocatorManager; }

private:
	const char* m_szKeyLibrary;
	const char* m_szMatLibrary;
	const char* m_szSkelFilePath;
	TUINT32 m_uiUnknown;
	AMaterialLibrary* m_pMaterialLibrary;
	Toshi::TTRB* m_pMaterialLibraryTRB;
	ATerrainLocatorManager* m_pLocatorManager;
	TINT32 m_iNumGroups;
	ATerrainVISGroup* m_pGroups;
	TUINT32 m_uiHighLODSize;
	TINT32 m_iHighLODNum;
	ATerrainLODBlock** m_ppHighLODs;
	TUINT32 m_uiLowLODSize;
	TINT32 m_iLowLODNum;
	ATerrainLODBlock** m_ppLowLODs;
	TUINT32 m_uiPersistantTerrainBlockSize;
	ATerrainLODBlock* m_pPersistantTerrainBlock;
};
