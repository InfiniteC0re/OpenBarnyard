#pragma once
#include "ATerrainVISGroup.h"
#include "ATerrainLocator.h"
#include "ATerrainLODBlock.h"
#include "Assets/AMaterialLibrary.h"

struct ATerrainOrderDVIS
{
	struct Info
	{
		TUINT16 uiGroupIndex;
		ATerrainLODType eLODType;
	};

	TINT iCount;
	Info* pInfo;
};

class ATerrainVIS
{
public:
	friend class ATerrain;
	friend class ATerrainVISGroup;

public:
	void LoadSkeleton();

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
	TUINT32 m_uiHighBlockSize;
	TINT32 m_iNumHighBlocks;
	ATerrainLODBlock** m_ppHighBlocks;
	TUINT32 m_uiLowBlockSize;
	TINT32 m_iNumLowBlocks;
	ATerrainLODBlock** m_ppLowBlocks;
	TUINT32 m_uiPersistantTerrainBlockSize;
	ATerrainLODBlock* m_pPersistantTerrainBlock;
};
