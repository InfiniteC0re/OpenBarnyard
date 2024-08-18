#pragma once
#include "ATerrainSection.h"
#include "ATerrainLocator.h"
#include "ATerrainLODBlock.h"
#include "Assets/AMaterialLibrary.h"

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

	ATerrainLocatorManager* GetLocatorManager() { return m_pLocatorManager; }

private:
	const TCHAR*            m_szKeyLibrary;
	const TCHAR*            m_szMatLibrary;
	const TCHAR*            m_szSkelFilePath;
	TUINT32                 m_uiUnknown;
	AMaterialLibrary*       m_pMaterialLibrary;
	Toshi::TTRB*            m_pMaterialLibraryTRB;
	ATerrainLocatorManager* m_pLocatorManager;
	TINT32                  m_iNumSections;
	ATerrainSection*        m_pSections;
	TUINT32                 m_uiHighBlockSize;
	TINT32                  m_iNumHighBlocks;
	ATerrainLODBlock**      m_ppHighBlocks;
	TUINT32                 m_uiLowBlockSize;
	TINT32                  m_iNumLowBlocks;
	ATerrainLODBlock**      m_ppLowBlocks;
	TUINT32                 m_uiPersistantTerrainBlockSize;
	ATerrainLODBlock*       m_pPersistantTerrainBlock;
};
