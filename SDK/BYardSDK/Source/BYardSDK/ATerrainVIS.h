#pragma once
#include "ATerrainSection.h"
#include "ATerrainLocator.h"
#include "ATerrainLODBlock.h"

class AMaterialLibrary;

struct ATerrainOrderDVIS
{
	struct Info
	{
		TUINT16         uiGroupIndex;
		ATerrainLODType eLODType;
	};

	TINT  iCount;
	Info* pInfo;
};

class ATerrainVIS
{
public:
	friend class ATerrainInterface;
	friend class ATerrainSection;

public:
	void LoadSkeleton();

	ATerrainLocatorList* GetLocatorList() { return m_pLocatorList; }

public:
	const char*          m_szKeyLibrary;
	const char*          m_szMatLibrary;
	const char*          m_szSkelFilePath;
	TUINT32              m_uiUnknown;
	AMaterialLibrary*    m_pMaterialLibrary;
	Toshi::TTRB*         m_pMaterialLibraryTRB;
	ATerrainLocatorList* m_pLocatorList;
	TINT32               m_iNumSections;
	ATerrainSection*     m_pSections;
	TUINT32              m_uiHighBlockSize;
	TINT32               m_iNumHighBlocks;
	ATerrainLODBlock**   m_ppHighBlocks;
	TUINT32              m_uiLowBlockSize;
	TINT32               m_iNumLowBlocks;
	ATerrainLODBlock**   m_ppLowBlocks;
	TUINT32              m_uiPersistantTerrainBlockSize;
	ATerrainLODBlock*    m_pPersistantTerrainBlock;
};
