#include "Toshi/File/TTRB.h"
#include "Toshi/Core/THashTable.h"

class ALevelInformation : Toshi::TTRB
{
public:
	struct LevelProperties
	{
		char* levelName;
		char* levelNamePath;
		char* levelFileName;
		char smth[0x58];
	};

	struct LevelInformations
	{
		int Count;
		LevelProperties* Strings;
	};

	ALevelInformation() : TTRB()
	{
		m_pData = TNULL;
		m_pHashTable = new Toshi::THashTable();
	}

	void Create(const char* a_levelInfo);
	void MakeStringToIDHashTable();


	static Toshi::THashTable::t_ItemCompareFunc LevelNameHashCompareFunc;


	uint8_t* m_pData; // 0x14
	Toshi::THashTable* m_pHashTable; // 0x18
};