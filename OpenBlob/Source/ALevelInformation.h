#include "Toshi/File/TTRB.h"
#include "Toshi/Core/THashTable.h"

class ALevelInformation : public Toshi::TTRB, public Toshi::TSingleton<ALevelInformation>
{
public:
	struct LevelProperties
	{
		char* m_levelName;
		char* levelNamePath;
		char* levelFileName;
		char smth[0x58];
	};

	struct LevelHeader
	{
		int m_levelCount;
		LevelProperties* m_pLevelProps;
	};

	ALevelInformation()
	{
		m_pData = TNULL;
	}

	void Create(const char* a_levelInfo);
	void MakeStringToIDHashTable();
	LevelHeader* GetLevelHeader() { return reinterpret_cast<LevelHeader*>(m_pData); }
	const char* GetLevelName(int a_iLevelIndex);


	static Toshi::THashTable::t_ItemCompareFunc LevelNameHashCompareFunc;


	uint8_t* m_pData; // 0x14
	Toshi::THashTable m_pHashTable; // 0x18
};