#include "Toshi/File/TTRB.h"
#include "Toshi/Core/THashTable.h"

class ALevelInformation : Toshi::TTRB
{
public:

	ALevelInformation() : TTRB()
	{
		m_pData = TNULL;
		m_pHashTable = new Toshi::THashTable;
	}

	void Create(const char* a_levelInfo);
	void MakeStringToIDHashTable();
	bool LevelNameHashCompareFunc(void* unk, void* unk2, int unk3);

	uint8_t* m_pData; // 0x14
	Toshi::THashTable* m_pHashTable; // 0x18
};